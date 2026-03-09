# BCM2837 网络驱动开发文档

## 概述

本文档描述了 YiYiYa 操作系统在 Raspberry Pi 3 (BCM2837) 上的网络驱动实现。

## 硬件架构

Raspberry Pi 3 使用 BCM2837 芯片，其网络接口通过以下方式实现：

```
+----------------+
| BCM2837 SoC    |
|                |
|  +-----------+ |
|  | USB Host  | |
|  |Controller | |
|  +-----+-----+ |
|        |       |
+--------|-------+
         | USB Bus
         |
+--------|-------+
| SMSC LAN9512/4 |
| USB Ethernet   |
| Controller     |
+--------+-------+
         |
      Ethernet
```

## 文件结构

```
duck/modules/net/
├── net.c              # 网络模块主文件（平台无关）
├── net.h              # 网络模块头文件
├── e1000.c            # Intel E1000 驱动（x86/PCI）
├── e1000.h            # E1000 头文件
├── bcm2837.c          # BCM2837 网络驱动
├── bcm2837.h          # BCM2837 网络驱动头文件
└── ya.py              # 构建配置（支持多平台）
```

## 关键组件

### 1. USB Host Controller (DWC OTG)

Raspberry Pi 3 使用 Synopsys DesignWare USB On-The-Go 控制器：

- **基地址**: `0x3F980000`
- **支持模式**: Host, Device, OTG
- **驱动状态**: 基础框架已实现

**寄存器映射**:
```c
#define USB_BASE            0x3F980000
#define USB_HOST_BASE       (USB_BASE + 0x0000)
#define USB_CORE_RESET      (USB_HOST_BASE + 0x010)
#define USB_CORE_INT_STAT   (USB_HOST_BASE + 0x014)
```

### 2. SMSC LAN9512/LAN9514

USB 以太网控制器特性：

- **Vendor ID**: 0x0424
- **Product ID**: 0xEC00
- **速度**: 10/100 Mbps
- **接口**: USB 2.0

**关键寄存器**:
```c
#define SMSC_TX_CTRL        0x0010
#define SMSC_RX_CTRL        0x0008
#define SMSC_MAC_CR         0x0100
#define SMSC_MAC_ADDR_HIGH  0x0118
#define SMSC_MAC_ADDR_LOW   0x0114
```

## 初始化流程

```
1. bcm2837_net_init()
   ├─ usb_host_init()          // 初始化 USB 控制器
   │  ├─ Reset USB controller
   │  ├─ Enable Host mode
   │  └─ Configure interrupts
   └─ smsc_lan95xx_init()      // 初始化网络芯片
      ├─ Reset SMSC chip
      ├─ Read MAC address
      └─ Configure MAC

2. 应用层通过 /dev/net 访问
```

## API 接口

### 读取数据包

```c
size_t bcm2837_net_read(device_t* dev, void* buf, size_t len);
```

**参数**:
- `dev`: 设备结构体
- `buf`: 接收缓冲区
- `len`: 缓冲区大小

**返回**: 实际接收的字节数

### 发送数据包

```c
size_t bcm2837_net_write(device_t* dev, const void* buf, size_t len);
```

**参数**:
- `dev`: 设备结构体
- `buf`: 发送缓冲区
- `len`: 数据长度

**返回**: 实际发送的字节数

### 设备控制

```c
int bcm2837_net_ioctl(device_t* dev, u32 cmd, void* args);
```

**支持的命令**:
- `0x01`: 获取 MAC 地址
- `0x02`: 获取链路状态
- `0x03`: 获取连接速度

## 使用示例

### 在内核中初始化网络模块

```c
#include "kernel/kernel.h"

int kmain(int argc, char* argv[]) {
    kernel_init();
    
    // 网络模块会自动初始化
    log_info("Network module loaded\n");
    
    // 获取网络设备
    device_t* net_dev = device_find(DEVICE_NET);
    if (net_dev) {
        // 读取 MAC 地址
        u8 mac[6];
        net_dev->ioctl(net_dev, 0x01, mac);
        log_info("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    
    kernel_run();
    return 0;
}
```

### 发送数据包

```c
device_t* net = device_find(DEVICE_NET);
if (net) {
    char packet[] = "Hello, Network!";
    size_t sent = net->write(net, packet, sizeof(packet));
    kprintf("Sent %d bytes\n", sent);
}
```

## 构建配置

在 `duck/modules/net/ya.py` 中：

```python
target("mod-net")
set_kind("static")

add_deps('kernel')

add_files('./*.c')

add_includedirs('..')
```

系统会根据目标平台自动选择正确的驱动：
- **RASPI2/RASPI3**: 使用 `bcm2837.c`
- **其他平台**: 使用 `e1000.c`

## 当前实现状态

### ✅ 已完成
- [x] 基础驱动框架
- [x] USB Host 控制器初始化
- [x] SMSC LAN95xx 基础配置
- [x] MAC 地址读取框架
- [x] 设备注册到 `/dev/net`
- [x] ioctl 接口

### 🚧 进行中
- [ ] 完整的 USB 协议栈
- [ ] USB 枚举过程
- [ ] USB 批量传输实现
- [ ] 数据包收发功能

### 📋 待实现
- [ ] TCP/IP 协议栈集成
- [ ] DHCP 客户端
- [ ] 网络调试工具
- [ ] 性能优化
- [ ] 中断处理优化

## 技术限制

当前实现有以下限制：

1. **USB 传输**: 仅实现了框架，实际数据传输需要完整 USB 协议栈
2. **性能**: 未使用 DMA 传输
3. **协议**: 未集成 TCP/IP 协议栈
4. **多线程**: 需要添加锁机制

## 调试方法

### 启用调试输出

在 `bcm2837.c` 中添加：

```c
#define DEBUG_NET 1

#if DEBUG_NET
#define NET_DEBUG(fmt, ...) kprintf("[BCM2837-Net] " fmt, ##__VA_ARGS__)
#else
#define NET_DEBUG(fmt, ...)
#endif
```

### 查看初始化日志

```bash
# 运行时查看内核日志
cat /proc/kmsg | grep Raspi3-Net
```

## 参考资源

1. **BCM2837 ARM Peripherals Manual**
   - USB Host Controller 章节
   - GPIO 配置章节

2. **SMSC LAN9512/LAN9514 Datasheet**
   - 寄存器映射
   - USB 命令接口

3. **USB 2.0 Specification**
   - 批量传输协议
   - 设备枚举流程

4. **DesignWare DWC_otg Databook**
   - USB Host Controller 编程指南

## 贡献指南

要改进网络驱动，请：

1. 实现 USB 协议栈的核心功能
2. 添加完整的 USB 枚举支持
3. 实现 USB 批量传输
4. 集成 lwIP 或其他 TCP/IP 协议栈
5. 添加网络工具（ping, ifconfig 等）

## 许可证

Copyright 2021-present evilbinary
