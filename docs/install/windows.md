## windows开发环境搭建

windows 安装比较麻烦，需要的步骤比较多，需要有点耐心。有两种选择，一种是使用msys2，一种是用wsl。

### 安装python

1、到python官网下载windows版本 

在 [https://www.python.org/downloads/release/python-3100/](https://www.python.org/downloads/release/python-3100/) ，在Files下找到 `Windows installer (64-bit)`，点击它后就会自动下载了。

2、安装python记得勾选 `Add Python 3.10 to PATH` 。

如果搞不清楚可以参考这个 [win 10安装python（详细讲解）](https://zhuanlan.zhihu.com/p/122435116)

3、打开power shell 输入`pip install scons`


### 安装gcc

1、下载gcc 到`D:\app\gcc` [https://github.com/lordmilko/i686-elf-tools/releases/download/7.1.0/i686-elf-tools-windows.zip](https://github.com/lordmilko/i686-elf-tools/releases/download/7.1.0/i686-elf-tools-windows.zip)

3、将安装所在的目例如：`D:\app\gcc`  加入到系统环境变量PATH，则在powershell下或者cmd下就可以用gcc了。

### 安装qemu

1、 进入[https://qemu.weilnetz.de/w32/](https://qemu.weilnetz.de/w32/) 去下载`qemu-w32-setup-20210825.exe` 。

2、将  `D:\Program Files (x86)\msys2\usr\bin` 、`D:\Program Files (x86)\qemu` 加入到系统环境变量PATH。

### 安装mtool


1、进入[https://www.gnu.org/software/mtools/](https://www.gnu.org/software/mtools/) 去下载`mtools-4.0.37.tar.gz` ，下载后记得解压。

2、打开powershell.exe或者terminal，运行命令

```bash
PS C:\Users\admin\Downloads\mtools-4.0.37> sh -c configure
configure: loading site script /usr/local/etc/config.site
checking for gcc... gcc
checking whether the C compiler works... yes
checking for C compiler default output file name... a.exe
checking for suffix of executables... .exe
checking whether we are cross compiling...
```

3、等待结束后，输入make -j8，等待编译后生成mtool.exe

4、将 `C:\Users\admin\Downloads\mtools-4.0.37` 加入到系统环境变量PATH。

### 编译与运行


打开windows terminal，进入YiYiYa目录，输入一下命令。

```bash
#清理
scons -c
#编译
scons -j8
#运行
scons --run=q
```
