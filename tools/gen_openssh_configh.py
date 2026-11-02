#!/usr/bin/env python3
# coding:utf-8
"""从 OpenSSH 的 configure.ac 提取所有 AC_CHECK_FUNCS/HEADERS/DECLS 检测项，
用 musl 的 libc.a 符号表 + 头文件目录核对，自动生成 config.h 的补丁片段。

为什么需要：
    OpenSSH 的 configure 在交叉编译 + 裸机环境下整批检测失败，导致 config.h 里
    所有 HAVE_* 都是 #undef。于是 OpenSSH 一边编自己的垫片、一边重造系统结构体
    （struct sockaddr_un / struct pollfd / timespec ……），和 musl 撞车。
    手工一条条试是无穷无尽的（实测踩了 10 多轮）。

    正确做法是「全部声明有，只对 musl 真没有的留垫片」——但不能靠猜：补了不存在
    的宏会在链接期 undefined。所以这里用符号表自动核对。

用法：
    python3 tools/gen_openssh_configh.py [-o app/openssh/foot-musl-defs.h]
"""
import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
AC = os.path.join(ROOT, 'app', 'openssh', 'configure.ac')
PLAT = os.environ.get('YA_PLAT', 'raspi5')
ARCH = os.environ.get('YA_ARCH', 'armv8-a')
MODE = os.environ.get('YA_MODE', 'debug')
BUILD = os.path.join(ROOT, 'build', PLAT, ARCH, MODE)
LIBC_A = os.path.join(BUILD, 'musl', 'lib', 'libc.a')
MUSL_INC = os.path.join(BUILD, 'musl', 'include')


def _collect(text, macro):
    """抓 AC_CHECK_XXX([a b c]) 里的条目。

    configure.ac 里这些列表内部还可能嵌 m4 片段（AC_DEFINE、AC_MSG_*、
    `[1]` 之类），非贪婪正则会一并抓进来，所以这里按条目形状过滤：
    - 函数/声明：合法 C 标识符，且不是 AC_ 开头的 m4 宏
    - 头文件：形如 path/to/x.h
    """
    out = set()
    for m in re.finditer(r'%s\(\[(.*?)\]\)' % macro, text, re.S):
        # 成员检测是 `struct X.field` 这种带空格的项，不能按空白拆
        if macro == 'AC_CHECK_TYPES':
            # `AC_CHECK_TYPES([long long, unsigned long long])` 里的类型名
            # 含空格，必须按逗号切，不能按空白
            for item in m.group(1).split(','):
                item = ' '.join(item.split()).strip('[]').strip()
                if item and not item.startswith('#'):
                    out.add(item)
            continue
        if macro == 'AC_CHECK_MEMBERS':
            for item in m.group(1).split(','):
                # 非贪婪匹配会把 `struct pollfd.fd` 后面的 `]` 一起抓进来
                item = ' '.join(item.split()).strip('[]').strip()
                if re.match(r'^struct [A-Za-z0-9_]+\.[A-Za-z0-9_]+$', item):
                    out.add(item)
            continue
        for tok in re.split(r'[\s,]+', m.group(1)):
            tok = tok.strip()
            if not tok or tok.startswith('#'):
                continue
            if macro == 'AC_CHECK_HEADERS':
                if re.match(r'^[A-Za-z0-9_./+-]+\.h$', tok):
                    out.add(tok)
            else:
                if (re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', tok)
                        and not tok.startswith('AC_')):
                    out.add(tok)
    return out


def _member_ok(spec, cc='aarch64-none-elf-gcc'):
    """AC_CHECK_MEMBERS 的项用真编译测试，例如 struct pollfd.fd。

    这类检测在交叉编译下同样整批失败，缺了会导致 OpenSSH 在自己头里
    重造结构体（struct pollfd / statfs / passwd 的字段），和 musl 撞车。
    """
    struct, field = spec.rsplit('.', 1)
    heads = ('sys/types.h sys/stat.h sys/socket.h netinet/in.h poll.h '
             'pwd.h sys/statfs.h sys/vfs.h sys/mount.h sys/time.h '
             'sys/statvfs.h').split()
    # 用 __has_include 保护：musl 没有 sys/statfs.h 之类，硬 include 会让
    # 整个测试失败，从而误判「成员不存在」。
    body = '\n'.join('#if __has_include(<%s>)\n#include <%s>\n#endif' % (h, h)
                     for h in heads)
    body += '\nint main(void){ return (int)sizeof(((%s*)0)->%s); }\n' % (
        struct, field)
    import tempfile
    cc = shutil.which(cc) or cc
    try:
        # 不能用 -o /dev/null：GCC 在 macOS 上会因此直接失败，
        # 表现为「所有成员都检测不到」（踩过，pollfd.fd 就是这么丢的）。
        with tempfile.NamedTemporaryFile(suffix='.o', delete=True) as tf:
            r = subprocess.run(
                [cc, '-I', MUSL_INC, '-x', 'c', '-c', '-o', tf.name, '-'],
                input=body.encode(), stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL)
            return r.returncode == 0
    except Exception:
        return False


def _type_ok(name, cc='aarch64-none-elf-gcc'):
    """AC_CHECK_TYPES 的项用真编译测试（例如 nfds_t / fd_mask / long long）。"""
    import tempfile
    cc = shutil.which(cc) or cc
    heads = ('sys/types.h sys/select.h poll.h stdint.h signal.h '
             'sys/statvfs.h sys/param.h netinet/in.h arpa/inet.h '
             'sys/socket.h').split()
    body = '\n'.join('#if __has_include(<%s>)\n#include <%s>\n#endif' % (h, h)
                     for h in heads)
    body += '\nint main(void){ %s v; return (int)sizeof(v); }\n' % name
    try:
        with tempfile.NamedTemporaryFile(suffix='.o', delete=True) as tf:
            r = subprocess.run(
                [cc, '-I', MUSL_INC, '-x', 'c', '-c', '-o', tf.name, '-'],
                input=body.encode(), stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL)
            return r.returncode == 0
    except Exception:
        return False


def main():
    if not os.path.exists(AC):
        sys.exit('找不到 %s' % AC)
    if not os.path.exists(LIBC_A):
        sys.exit('找不到 %s（先构建 musl）' % LIBC_A)

    ac = open(AC, 'r', errors='replace').read()
    funcs = _collect(ac, 'AC_CHECK_FUNCS')
    headers = _collect(ac, 'AC_CHECK_HEADERS')
    decls = _collect(ac, 'AC_CHECK_DECLS')

    nm = subprocess.run(['nm', '-g', LIBC_A], stdout=subprocess.PIPE,
                        stderr=subprocess.DEVNULL).stdout.decode('utf-8', 'replace')
    exported = set()
    for line in nm.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[-2] in ('T', 'W', 'D', 'B'):
            exported.add(parts[-1])

    def macro_of(name):
        return 'HAVE_' + re.sub(r'[^A-Za-z0-9]', '_', name).upper()

    has, miss = [], []
    for f in sorted(funcs):
        (has if f in exported else miss).append(f)
    head_has, head_miss = [], []
    for h in sorted(headers):
        (head_has if os.path.exists(os.path.join(MUSL_INC, h)) else head_miss).append(h)
    decl_has, decl_miss = [], []
    for d in sorted(decls):
        (decl_has if d in exported else decl_miss).append(d)

    # AC_CHECK_TYPES：nfds_t → HAVE_NFDS_T，long long → HAVE_LONG_LONG
    types = _collect(ac, 'AC_CHECK_TYPES')
    type_has, type_miss = [], []
    for t in sorted(types):
        (type_has if _type_ok(t) else type_miss).append(t)

    # AC_CHECK_MEMBERS：struct X.field → HAVE_STRUCT_X_FIELD
    members = _collect(ac, 'AC_CHECK_MEMBERS')
    mem_has, mem_miss = [], []
    for m in sorted(members):
        if '.' not in m or not m.startswith('struct '):
            continue
        (mem_has if _member_ok(m) else mem_miss).append(m)

    lines = [
        '/* 自动生成，勿手改 —— tools/gen_openssh_configh.py */',
        '/* 依据：%s 的导出符号 + %s 的头文件 */' % (
            os.path.relpath(LIBC_A, ROOT), os.path.relpath(MUSL_INC, ROOT)),
        '',
        '/* ---- 函数：libc.a 里有 ---- */',
    ]
    for f in has:
        lines.append('#define %s 1' % macro_of(f))
    lines.append('')
    lines.append('/* ---- 头文件：musl 里有 ---- */')
    for h in head_has:
        lines.append('#define %s 1' % macro_of(h))
    lines.append('')
    lines.append('/* ---- AC_CHECK_DECLS 有 ---- */')
    for d in decl_has:
        lines.append('#define %s 1' % macro_of(d))
    lines.append('')
    lines.append('/* ---- 类型（编译测试验证过）---- */')
    for t in type_has:
        lines.append('#define %s 1' % macro_of(t))
    lines.append('')
    lines.append('/* ---- 结构体成员（编译测试验证过）---- */')
    for m in mem_has:
        lines.append('#define HAVE_STRUCT_%s 1' %
                     m.replace('struct ', '').replace('.', '_').upper())
    lines.append('')
    lines.append('/* ---- 以下 musl 没有，保留 OpenSSH 垫片 ---- */')
    for m in mem_miss:
        lines.append('/* no member %s */' % m)
    for f in miss:
        lines.append('/* no %s */' % f)
    for h in head_miss:
        lines.append('/* no %s */' % h)

    out = sys.argv[2] if len(sys.argv) > 2 and sys.argv[1] == '-o' else \
        os.path.join(ROOT, 'app', 'openssh', 'foot-musl-defs.h')
    open(out, 'w').write('\n'.join(lines) + '\n')

    print('函数: 有 %d / 无 %d' % (len(has), len(miss)))
    print('头文件: 有 %d / 无 %d' % (len(head_has), len(head_miss)))
    print('声明: 有 %d / 无 %d' % (len(decl_has), len(decl_miss)))
    print('类型: 有 %d / 无 %d  %s' % (len(type_has), len(type_miss),
                                      ' '.join(type_miss)))
    print('结构体成员: 有 %d / 无 %d' % (len(mem_has), len(mem_miss)))
    if miss:
        print('保留垫片的函数: %s' % ' '.join(miss))
    if head_miss:
        print('保留垫片的头: %s' % ' '.join(head_miss))
    print('已写入 %s' % out)


if __name__ == '__main__':
    main()
