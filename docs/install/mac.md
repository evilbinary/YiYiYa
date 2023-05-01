## Mac开发环境搭建

mac可以使用port或者brew。

### 安装gcc

port 方式安装如下：

```bash
#arm 版本的gcc
sudo port install arm-none-eabi-gcc
# i386版本的gcc
sudo port install i386-elf-gcc
```

brew 方式安装如下：

```bash
#arm 版本的gcc
brew install --cask gcc-arm-embedded
# i386版本的gcc
brew install i386-elf-binutils
# riscv版本
brew install riscv-gnu-toolchain
```

### 安装scons

```bash
pip install scons
```

### 安装qemu

port 方式安装如下：

```bash
sudo port install qemu +target_arm 
```

brew 方式安装如下：

```bash
brew install qemu
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
