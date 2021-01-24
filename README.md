# YiYiYa
YiYiYa is an os

## 使用

### 编译
提前安装 bochs
```
./configure --with-sdl --enable-disasm --enable-all-optimizations --enable-readline  --disable-debugger-gui --enable-x86-debugger --enable-a20-pin --enable-fast-function-calls --enable-iodebug --enable-gdb-stub 
```

编译

```
scons
```

### 运行镜像

```
scons --run=bochs
```
或者
```
scons --run=qemu
```
