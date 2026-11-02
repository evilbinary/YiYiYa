# 移植第三方库 / 程序到 YiYiYa

> 本文是实操手册，来自 OpenSSL(libcrypto)、OpenSSH(sshd)、libxcrypt 三个真实移植的全部踩坑记录。
> 目标平台：`aarch64-none-elf` + musl（`eggs/libmusl`）+ ymake。

---

## 1. 先决定放哪

| 产物 | 位置 | 注册处 |
|---|---|---|
| `.a` 静态库 | `eggs/libxxx/` | `eggs/ya.py` 的 `c_libs` 列表 |
| 可执行程序 | `app/xxx/` | 根 `ya.py` 的 `apps` 列表 |

`eggs/ya.py` 和根 `ya.py` 都是**显式列表**（不是 glob），漏了就会报 `not found target xxx`。

程序类 target 用 `set_type("cli")`；库用 `set_kind("static")`。

---

## 2. 接入模板（automake 模式）

第三方库几乎都是 autotools，**不要**手工 `add_files` 列源文件——OpenSSL 那种源文件清单是按 configure 结果筛的，手抄必错。照 `eggs/libmusl/ya.py` 用 out-of-tree 构建：

```python
import os as _pyos

target("crypt")            # target 名（不是产物名）
set_kind("static")         # 程序用 set_type("cli")
add_packages(get_config('default_libc'))

_xc_src = _pyos.path.abspath(os.scriptdir())     # os.scriptdir() 是 ymake 注入的
_xc_oot = _pyos.path.abspath(_pyos.path.join(
    'build', str(get_plat()), str(get_arch()), str(get_config('mode')), 'libxcrypt-oot'))
_pyos.makedirs(_xc_oot, exist_ok=True)

# wrapper：ymake 固定调用 sourcedir/configure，这里补 target 和裁剪项
_configure_wrapper = (
    '#!/bin/sh\n'
    'if [ -f config.status ] && [ -f Makefile ]; then\n'
    '    echo "already configured, skip"; exit 0\n'
    'fi\n'
    'exec "%s/configure" --host=%s --disable-shared --enable-static \\\n'
    '     --disable-werror "$@"\n' % (_xc_src, get_toolchain()))
# 写 wrapper（内容不变就不重写）
# ...

set_sourcedir(_xc_oot)
add_includedirs(_pyos.path.join(_xc_src, 'include'), public=True)

add_deps("gcc")
_tool = get_toolchain()      # 【必须在 automake() 之前】

automake("crypt")            # 【参数就是产物名】libcrypt.a

configure(
    'CC=' + _tool + '-gcc',
    'AR=' + _tool + '-ar',
    'RANLIB=' + _tool + '-ranlib',
    'CFLAGS=' + _cflags,
    'CPPFLAGS=-I' + _musl_inc,
    'LDFLAGS=' + _ldflags,
    'LIBS=' + _libs,
)
```

### 五条硬规则（都踩过）

1. **`automake(name)` 的 name 是产物名**，ymake 用它拼 `lib<name>.a` 判断「是否已构建」。写错（比如 `automake("c")` 想表达"用 C 编译器"）会导致产物永远找不到 → 每次 `ya` 都重跑 configure。
2. **`get_toolchain()` 必须在 `automake()` 之前**。`automake()` 会 `node_start` 一个新节点，之后取到的工具链是 `automake` 这个伪工具链，拼出 `automake-gcc: command not found`。
3. **`os.scriptdir()` 是 ymake 注入的**。`import os as _pyos` 拿到的是标准库，没有 `scriptdir`。
4. **`set_sourcedir()` 用绝对路径**。相对路径在 `cwd=` 解析时会出错（表现为 `make: No targets specified and no makefile found`）。
5. **`after_build(fn)` 必须在 `automake()` 之前注册**，否则钩子挂到 automake 节点上，ymake 在 target 节点找不到 → 函数永不执行。

---

## 3. 交叉编译三件套（configure 的链接测试）

configure 最早的 `checking whether the C compiler works` 会编译并**链接**一个空 main。裸机 gcc 没有 crt/libc，必然失败，报 `C compiler cannot create executables`。

关键是：**configure 的 test_ld 只用 `$LDFLAGS`（不带 `$CFLAGS`）**，所以 `-nostdlib` 和 crt 必须放在 LDFLAGS 里。参考 `eggs/libffmpeg/ya.py`：

```python
_musl_root = _pyos.path.abspath(_pyos.path.join(
    'build', str(get_plat()), str(get_arch()), str(get_config('mode')), 'musl'))
_musl_inc = _pyos.path.join(_musl_root, 'include')
_musl_lib = _pyos.path.join(_musl_root, 'lib')

_cflags  = '-I' + _musl_inc + ' -O2 -fno-strict-aliasing'
_ldflags = ' '.join(['-nostdlib', '-static', '-L' + _musl_lib,
                     _pyos.path.join(_musl_lib, 'crt1.o'),
                     _pyos.path.join(_musl_lib, 'crti.o')])
_libs    = ' '.join(['-L' + _musl_lib, '-lc', '-lgcc',
                     _pyos.path.join(_musl_lib, 'crtn.o')])
```

**坑**：musl 的头在**安装目录** `build/<plat>/<arch>/<mode>/musl/include`，不是 `musl-oot/include`（后者是 musl 自己的 out-of-tree 目录，里面是空的）。

---

## 4. 配置幂等（避免每次 ya 都重跑 configure）

ymake 的判定是 `os.path.exists(<build>/<target>/lib/<filename>)`。**只要产物没生成，它每次都调 wrapper**。所以 wrapper 里要挡住：

```sh
if [ -f config.status ] && [ -f Makefile ]; then
    echo "already configured, skip"; exit 0
fi
```

再加一个**配置指纹**（只绑参数，不绑注释/结构），参数真变了才删 `config.status`：

```python
_fp = '|'.join([_cfg_args, _cflags, _ldflags, _libs, 'v1'])
if _old_stamp != _fp:
    remove('config.status'); remove('Makefile')
    write(_stamp, _fp)
```

> 改注释就触发重配是很烦的，指纹一定要只包含**会影响配置的内容**。

**注意**：`after_build` 在「产物已存在」的分支里**也会被调用**（`automake.py:54-60`），所以钩子里要自己加幂等判断，否则每次 ya 都白干活（比如重拷 169 个头文件）。

---

## 5. 产物路径对齐（`filename`）

`get_build_target()` 拼的是 `<build>/<target>/lib/<prefix><name><ext>`，而第三方 `make install` 的实际落点常常不是这里：

```python
# OpenSSH 的 --prefix 实际是 <build>/openssh（比 build-dir 少一层 lib）
# install 落点：<build>/openssh/bin/sshd
# ymake 判定：  <build>/openssh/lib/... + filename
automake("sshd", filename="../bin/sshd")   # 用 ../ 回退一层直接指到真实位置
```

不要写「先 install 到 A 再钩子拷到 B」——多一次拷贝、多一个坑。能用 `filename=` 对齐就直接对齐。

---

## 6. 常见坑速查表

| 症状 | 原因 | 修法 |
|---|---|---|
| `unrecognized option '-pthread'` | 裸机 gcc 不认 glibc 的 driver 选项 | configure 加 `no-threads` |
| `linux/version.h: No such file`（OpenSSL AF_ALG） | 需要 Linux 内核头 | 加 `no-afalgeng` |
| `cannot find -lxxx` | `-L` 路径多/少拼了一层 | 打印链接命令核对 |
| `undefined reference to 'accept'/'gethostbyname'` | 静态链接单向解析，依赖库排在了 `-lc` 之后 | 把 `-lcrypto`/`-lssl` 放到 `-lc` **之前** |
| `#error "16 bit int type not found."` | 交叉编译 `AC_CHECK_SIZEOF` 测不出值 | configure 后往 `config.h` append `#define SIZEOF_SHORT_INT 2` 等 |
| `redefinition of 'struct timespec'` | `HAVE_STRUCT_TIMESPEC` 未定义 | 同上，append `#define HAVE_STRUCT_TIMESPEC 1` |
| `conflicting types for 'intmax_t'` | `HAVE_INTMAX_T` 未定义 | 同上 |
| `expected expression before 'void'` | musl 的 `TIMESPEC_TO_TIMEVAL` 是**表达式**（末尾 `(void)0`），第三方按 glibc 习惯当语句用、没写分号 | 源码补分号（做成幂等 patch） |
| `redefinition of 'struct stat' 里 __unused` | OpenSSH 把 `__unused` 定义成属性宏，musl 的 `bits/stat.h` 拿它当字段名 | 改 musl 字段名为 `__reserved`（字段名不进 ABI，安全） |
| `make install` 报 `Error 126` | install 依赖某个**要运行**的目标（如 `host-key` 跑 ssh-keygen） | 从 Makefile 的 install 依赖里摘掉：`sed -i "" "s/^install: \(.*\) host-key check-config$/install: \1/"` |
| `make install` 极慢 | 在装 man 手册（几千个文件） | configure 加 `no-docs no-demos` |
| `apps/openssl` 链接失败 | 命令行工具需要 crt，我们不需要它 | 加 `no-apps`；**注意** libcrypto.a 在这之前已经 install 成功 |
| `make: No targets specified and no makefile found` | `set_sourcedir` 用了相对路径 | 改成 `os.path.abspath(...)` |

---

## 7. 调试流程（从报错到定位）

1. **看 `config.log` 尾部**，不要只看屏幕输出：
   ```sh
   tail -30 build/<plat>/<arch>/<mode>/<xxx>-oot/config.log
   grep -nE "cannot find|error:|undefined" .../config.log | head
   ```
   > 注意：别在 configure 跑的时候 Ctrl-C，会写成 `configure: caught signal 2`，把真错误盖掉。
2. **单文件复现**，别每次都全量 `make`：
   ```sh
   cd build/.../<xxx>-oot && make 某个.o
   ```
3. **手工复现链接测试**（1 秒出结果）：
   ```sh
   echo 'int main(){return 0;}' > t.c
   aarch64-none-elf-gcc -o t t.c -nostdlib -static -L$M $M/crt1.o $M/crti.o -L$M -lc -lgcc $M/crtn.o
   ```
4. **看 install 到底装到哪**：
   ```sh
   grep -n "ginstall\|install.*->" /tmp/ya_xxx.log | head
   ```

---

## 8. 装进镜像（`app/resource`）

镜像里的路径约定：`app/resource/bin` ⇒ 运行时的 `/bin`，`app/resource/etc` ⇒ `/etc`。

用 `after_build` 钩子部署（**必须在 `automake()` 之前注册**）：

```python
def _deploy(t):
    import shutil
    res = os.path.join(os.path.abspath(os.projectdir()), 'app', 'resource')
    dst = os.path.join(res, 'bin', 'sshd')
    if not exists(dst) or getsize(dst) != getsize(src):
        shutil.copyfile(src, dst); os.chmod(dst, 0o755)

after_build(_deploy)
```

**不要在构建期生成需要"运行"才能产出的东西**（比如 host key）。交叉编译出来的 `ssh-keygen` 在 macOS 上跑不了，要用**宿主机的**同名工具预先生成。

---

## 9. 已移植案例（可直接抄参数）

### OpenSSL（`eggs/libopenssl` → `libcrypto.a`）

```sh
./Configure linux-aarch64 no-shared no-tests no-docs no-demos \
    no-dso no-async no-threads no-comp no-afalgeng no-apps
```

### OpenSSH（`app/openssh` → `sshd`）

```sh
./configure --host=aarch64-none-elf \
    --with-ssl-dir=<build>/crypto/lib \
    --sbindir='${prefix}/bin' --libexecdir='${prefix}/bin' \
    --without-pam --without-privsep --without-zlib --without-libedit \
    --without-selinux --without-audit --without-kerberos5 \
    --disable-strip --disable-etc-default-login \
    --disable-utmp --disable-wtmp --disable-btmp --disable-lastlog
```

> `--sbindir` 用**单引号**，让 configure 保存字面量 `${prefix}/bin`；用双引号会被 shell 当场展开成 `/bin`，装到宿主机上去。

### libxcrypt（`eggs/libxcrypt` → `libcrypt.a`）

```sh
./configure --host=aarch64-none-elf \
    --disable-shared --enable-static \
    --enable-hashes=strong --disable-werror
```

---

## 10. 一句话总结

> **产物名对齐 + 配置幂等 + 链接顺序**，这三件事做对，90% 的移植问题不会出现；
> 剩下 10% 全在 `config.log` 里，`tail -30` 就能看见。
