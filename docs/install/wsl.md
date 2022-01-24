## wsl开发环境搭建


### 安装wsl2

wsl需要支持loop设备，请参考[wsl2](https://docs.microsoft.com/zh-cn/windows/wsl/install-manual#step-4---download-the-linux-kernel-update-package),下载Linux 内核更新包后运行下面命令，然后其他过程参考linux。


```bash
wsl --set-default-version 2
wsl -l -v
wsl.exe --set-version Ubuntu-20.04 2
```


### 安装qemu

在打开的wsl的ubuntu20.01 tls版本下输入

```bash
apt install qemu
```


### 安装scons

在打开的wsl的ubuntu20.01 tls版本下输入
```bash

apt install scons gcc

```


### 安装x11

1、在windows上安装[VcXsrv](https://sourceforge.net/projects/vcxsrv/)

2、在wsl里面配置DISPLAY为VcXsrv得限制地址

```bash
export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
```

3、打开VcXsrv得XLaunch启动程序，配置如下，记得最后一步加-ac 否则会报错。

<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/image/vcxsrv-win?raw=true" width="400px" />

<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/image/vcxsrv-one?raw=true" width="400px" />

<img src="https://github.com/evilbinary/YiYiYa/blob/main/docs/image/vcxsrv-ac?raw=true" width="400px" />



### 编译与运行

```bash
#清理
scons -c
#编译
scons -j8
#运行
scons --run=q
```

