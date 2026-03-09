# YiYiYa 操作系统文档

## 📚 文档导航

### 安装指南
- [Linux 安装](install/linux.md)
- [macOS 安装](install/mac.md)
- [Windows 安装](install/windows.md)
- [WSL 安装](install/wsl.md)

### 开发指南
- [简介](develop/简介.md)
- [开发环境](develop/开发环境.md)
- [源码目录](develop/源码目录.md)
- [内核开发](develop/内核开发.md)
- [驱动开发](develop/驱动开发.md)
- [应用开发](develop/应用开发.md)
- [移植适配](develop/移植适配.md)
- [微内核](develop/微内核.md)
- [标准库](develop/标准库.md)
- [性能分析](develop/性能分析.md)
- [开发板运行](develop/开发板运行.md)
- [硬件调试](develop/硬件调试.md)
- [实例 Demo](develop/实例Demo.md)
- [平台支持](develop/platform-support.md)

### 架构设计
- [系统架构](architecture/architecture.md)

### API 参考
- [API 文档](api/api-reference.md)

### 驱动文档
- [BCM2837 网络驱动](develop/driver/bcm2837-network.md)

### 参考资料
- [系统调用](material/syscall.md)
- [内存管理](material/memory.md)
- [ST7735 LCD](material/st7735.md)

### 效果展示
查看 `demo/` 目录下的截图和视频

## 📁 文档目录结构

```
docs/
├── README.md           # 文档导航
├── install/            # 安装指南
│   ├── linux.md
│   ├── mac.md
│   ├── windows.md
│   └── wsl.md
├── develop/            # 开发文档
│   ├── 简介.md
│   ├── 开发环境.md
│   ├── 内核开发.md
│   ├── 驱动开发.md
│   ├── 应用开发.md
│   ├── platform-support.md
│   ├── driver/         # 驱动详细文档
│   │   └── bcm2837-network.md
│   └── image/          # 文档图片
├── architecture/       # 架构设计
│   └── architecture.md
├── api/                # API 参考
│   └── api-reference.md
├── material/           # 参考资料
│   ├── syscall.md
│   ├── memory.md
│   └── st7735.md
├── demo/              # 演示截图/视频
└── image/             # 文档图片资源
```

## 🔗 快速链接

### 新手入门
1. [简介](develop/简介.md) - 了解 YiYiYa 操作系统
2. [开发环境](develop/开发环境.md) - 搭建开发环境
3. [内核开发](develop/内核开发.md) - 开始内核开发

### 驱动开发
1. [驱动开发指南](develop/驱动开发.md) - 驱动开发流程
2. [BCM2837 网络驱动](develop/driver/bcm2837-network.md) - Raspberry Pi 3 网络驱动实例

### 移植适配
1. [移植适配指南](develop/移植适配.md) - 如何移植到新平台
2. [平台支持](develop/platform-support.md) - 支持的硬件平台
3. [开发板运行](develop/开发板运行.md) - 在开发板上运行

### 架构和 API
1. [系统架构](architecture/architecture.md) - 微内核架构设计
2. [API 参考](api/api-reference.md) - 系统调用和内核 API

## 📝 文档贡献

欢迎贡献文档！请遵循以下规范：

1. 使用 Markdown 格式
2. 文档放在对应的目录下
3. 驱动详细文档放在 `develop/driver/` 目录
4. 参考资料（芯片手册、协议说明等）放在 `material/` 目录
5. 图片资源放在 `develop/image/` 或 `image/` 目录

## 🌟 特色

- 微内核架构
- 模块化设计
- 多平台支持
- 实时性
- 开源免费

## 📧 联系方式

- 作者: evilbinary
- 邮箱: rootdebug@163.com
