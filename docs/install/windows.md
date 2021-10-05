## windows开发环境搭建

windows 安装比较麻烦，需要的步骤比较多，需要有点耐心。

### 安装python

1、到python官网下载windows版本 

在 [https://www.python.org/downloads/release/python-3100/](https://www.python.org/downloads/release/python-3100/) ，在Files下找到 `Windows installer (64-bit)`，点击它后就会自动下载了。

2、安装python记得勾选 `Add Python 3.10 to PATH` 。

如果搞不清楚可以参考这个 [win 10安装python（详细讲解）](https://zhuanlan.zhihu.com/p/122435116)

3、打开power shell 输入`pip install scons`


### 安装gcc

1、到 https://www.msys2.org/ 页面下载

早找 [msys2-x86_64-20210725.exe](https://github.com/msys2/msys2-installer/releases/download/2021-07-25/msys2-x86_64-20210725.exe) 点击下载，直接默认安装。

2、打开msys2.exe，运行命令

```bash
#查询并找到msys/gcc 可以看到软件大致分为了三大类:mingw32、mingw64、msys
pacman -Ss gcc
#安装，选择其中一种
pacman -S mingw32/mingw-w64-i686-gcc
pacman -S mingw32/mingw-w64-i686-nasm
```

3、将安装所在的目例如：`D:\Program Files (x86)\msys2\mingw32\bin`  加入到系统环境变量PATH，则在powershell下或者cmd下就可以用gcc了。

### 安装qemu

1、 进入[https://qemu.weilnetz.de/w32/](https://qemu.weilnetz.de/w32/) 去下载`qemu-w32-setup-20210825.exe` 

2、将  `D:\Program Files (x86)\msys2\usr\bin` 、`D:\Program Files (x86)\qemu` 加入到系统环境变量PATH


### 编译与运行

```bash
#清理
scons -c
#编译
scons -j8
#运行
scons --run=q
```
