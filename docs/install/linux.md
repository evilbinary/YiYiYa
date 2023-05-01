## Linux开发环境搭建

linux下开发环境比较简单，以ubuntu举例。

### 安装gcc

```bash
#arm 版本的gcc
apt install arm-none-eabi-gcc
# i386版本的gcc
apt-get install libc6-dev:i386 gcc:i386
```


### 安装scons

```bash

apt install scons

```

### 安装qemu

```bash

apt install qemu

```

### 编译与运行

```bash
#清理
scons -c
#编译
scons -j8
#运行
scons --run=q
```
