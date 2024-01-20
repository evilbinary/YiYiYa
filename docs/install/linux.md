## Linux开发环境搭建

linux下开发环境比较简单，以ubuntu举例。

### 安装gcc

```bash
#arm 版本的gcc
sudo apt-get install gcc-arm-none-eabi
# i386版本的gcc
apt-get install libc6-dev:i386 gcc:i386

#  安装工具
sudo apt install mtools u-boot-tools dosfstools

```


### 安装yymake

```bash

pip install yymake

```

### 安装qemu

```bash

apt install qemu

```

### 编译与运行

```bash
#清理
rm -rf build
#编译
ya -m debug -j8
#运行
ya -m debug -r qemu
```
