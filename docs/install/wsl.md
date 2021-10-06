## wsl开发环境搭建


### 安装wsl2

wsl需要支持loop设备，请参考[wsl2](https://docs.microsoft.com/zh-cn/windows/wsl/install-manual#step-4---download-the-linux-kernel-update-package),下载Linux 内核更新包后运行下面命令，然后其他过程参考linux。


```bash
wsl --set-default-version 2
wsl -l -v
wsl.exe --set-version Ubuntu-20.04 2
```


### 安装qemu

1、 进入[https://qemu.weilnetz.de/w32/](https://qemu.weilnetz.de/w32/) 去下载`qemu-w32-setup-20210825.exe` 

2、将  `D:\Program Files (x86)\msys2\usr\bin` 、`D:\Program Files (x86)\qemu` 加入到系统环境变量PATH

3、修改`D:\Program Files (x86)\qemu` 拷贝qemu目录下的qemu-system-i386.exe 为qemu-system-i386 方便wsl使用（貌似wsl没有qemu）。


### 安装scons

在打开的wsl的ubuntu20.01 tls版本下输入
```bash

apt install scons gcc

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

