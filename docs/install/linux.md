## Linux开发环境搭建

linux下开发环境比较简单，以ubuntu举例。

### 安装scons

```bash

pip install scons

```

### 安装qemu

```bash

apt-get install qemu

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
