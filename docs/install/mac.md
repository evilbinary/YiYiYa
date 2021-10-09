## Mac开发环境搭建

mac可以使用port或者brew。

### 安装scons

```bash
pip install scons
``` 

### 安装qemu

```bash
port install qemu
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
