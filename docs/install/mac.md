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
#或者
brew tap armmbed/homebrew-formulae
brew install armmbed/formulae/arm-none-eabi-gcc

#aarch64 安装
brew install --cask gcc-aarch64-embedded
#或者
brew install aarch64-elf-gcc


# i386版本的gcc
brew install i386-elf-binutils
#或者
brew tap nativeos/i386-elf-toolchain
brew install nativeos/i386-elf-toolchain/i386-elf-binutils
brew install nativeos/i386-elf-toolchain/i386-elf-gcc

# riscv版本
brew install riscv-gnu-toolchain
```

### 安装yymake

```bash
pip install yymake
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
rm -rf build
#编译
ya -m debug -j8
#运行
ya -m debug -r qemu
```
