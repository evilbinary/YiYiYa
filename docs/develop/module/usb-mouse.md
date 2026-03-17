# USB 鼠标驱动开发文档

本文档描述了 YiYiYa 操作系统中的 USB 鼠标驱动实现，基于 USB HID 类协议。

## 概述

USB 鼠标驱动是 YiYiYa 操作系统的输入设备驱动之一，支持通过 USB 接口连接的标准鼠标设备。驱动实现了 USB HID (Human Interface Device) 类协议，可处理鼠标的移动、按键和滚轮事件。

## 硬件架构

```
+----------------+
|   USB 设备      |
|  (USB 鼠标)    |
+--------+-------+
         | USB Bus
         |
+--------v-------+
| DWC2 OTG      |
| USB Host      |
| Controller    |
| (BCM2837)    |
+--------+-------+
         |
+--------v-------+
|   USB Core    |
| (协议栈)      |
+--------+-------+
         |
+--------v-------+
|   HID Class   |
|   Driver      |
+--------+-------+
         |
+--------v-------+
| USB Mouse     |
| Driver        |
+--------+-------+
         |
+--------v-------+
|   Mouse API   |
|   (mouse.c)   |
+--------+-------+
```

## 文件结构

```
duck/modules/usb/
├── usb.h              # USB 核心头文件 (类型定义 + API)
├── usb.c              # USB 设备管理、枚举、控制传输
├── hcd.c              # 主机控制器驱动接口
├── dwc2.c             # DWC2 OTG 控制器驱动 (BCM2837/raspi3)
├── hub.c              # USB Hub 驱动
├── hid.c              # HID 类驱动
├── usb_mouse.c        # USB 鼠标驱动
└── ya.py              # 构建配置
```

## 核心数据结构

### USB 设备结构

```c
typedef struct usb_device {
    u8 address;           // USB 地址
    u8 speed;            // USB 速度 (LOW/FULL/HIGH)
    u8 port;             // 连接的端口
    u8 parent;           // 父设备 (Hub)
    u16 vendor;          // 供应商 ID
    u16 product;         // 产品 ID
    u8 class;            // 设备类
    u8 subclass;         // 子类
    u8 protocol;         // 协议
    u8 maxchild;         // 最大子设备数
    u8 config;           // 当前配置
    u8 num_endpoints;    // 端点数量
    usb_endpoint_descriptor_t ep[16];  // 端点描述符
    char name[32];       // 设备名称
    void* hc_private;     // 主机控制器私有数据
    struct usb_device* next;  // 下一个设备
} usb_device_t;
```

### USB 鼠标事件

```c
typedef struct usb_mouse_event {
    u8 buttons;    // 按钮状态 (bit0: 左键, bit1: 中键, bit2: 右键)
    s8 x;         // X 轴移动增量
    s8 y;         // Y 轴移动增量
    s8 wheel;     // 滚轮移动增量
} usb_mouse_event_t;
```

### HID 报告

```c
typedef struct hid_report {
    u8 report_id;   // 报告 ID
    u8* data;       // 报告数据
    u32 length;     // 数据长度
} hid_report_t;
```

## USB 速度等级

| 常量 | 值 | 说明 |
|------|-----|------|
| `USB_SPEED_LOW` | 0 | 低速 (1.5 Mbps) |
| `USB_SPEED_FULL` | 1 | 全速 (12 Mbps) |
| `USB_SPEED_HIGH` | 2 | 高速 (480 Mbps) |

## USB 类别

| 常量 | 值 | 说明 |
|------|-----|------|
| `USB_CLASS_HID` | 0x03 | 人机接口设备 |
| `USB_CLASS_HUB` | 0x09 | USB Hub |
| `USB_CLASS_MASS_STORAGE` | 0x08 | 大容量存储 |

## HID 类请求

| 请求 | 值 | 说明 |
|------|-----|------|
| `USB_HID_REQUEST_GET_REPORT` | 0x01 | 获取报告 |
| `USB_HID_REQUEST_GET_IDLE` | 0x02 | 获取空闲速率 |
| `USB_HID_REQUEST_GET_PROTOCOL` | 0x03 | 获取协议 |
| `USB_HID_REQUEST_SET_REPORT` | 0x09 | 设置报告 |
| `USB_HID_REQUEST_SET_IDLE` | 0x0A | 设置空闲速率 |
| `USB_HID_REQUEST_SET_PROTOCOL` | 0x0B | 设置协议 |

## 核心 API

### USB 核心

```c
// 初始化/退出
int usb_init(void);
void usb_exit(void);

// 设备管理
int usb_alloc_device(usb_device_t* dev);
int usb_free_device(usb_device_t* dev);
int usb_enumerate(void);
usb_device_t* usb_find_device(u8 address);
usb_device_t* usb_find_device_by_vid_pid(u16 vendor, u16 product);

// 传输
int usb_control_transfer(u8 device_address, u8 endpoint, 
                        u8 request_type, u8 request,
                        u16 value, u16 index, void* data, u16 len, u32 timeout);
int usb_interrupt_transfer(usb_device_t* dev, u8 endpoint, 
                         void* data, u16 len);
int usb_bulk_transfer(usb_device_t* dev, u8 endpoint,
                     void* data, u32 len);
```

### HID 驱动

```c
int hid_init(void);
int hid_parse_report(usb_device_t* dev, u8* report_desc, u32 length);
int hid_get_report(usb_device_t* dev, u8 report_id, void* buf, u32 len);
int hid_set_report(usb_device_t* dev, u8 report_id, void* buf, u32 len);
```

### USB 鼠标

```c
// 初始化/退出
int usb_mouse_init(void);
void usb_mouse_exit(void);

// 设备连接/断开
int usb_mouse_connect(usb_device_t* dev);
int usb_mouse_disconnect(usb_device_t* dev);

// 事件处理
void usb_mouse_handler(urb_t* urb);
int usb_mouse_parse_report(u8* data, u32 len, usb_mouse_event_t* event);
```

## 鼠标报告格式

标准 USB 鼠标使用 4 字节报告：

| 字节 | 位字段 | 说明 |
|------|--------|------|
| 0 | Bit0-2 | 按钮 (左/中/右键) |
| 1 | Bit0-7 | X 轴移动 (有符号) |
| 2 | Bit0-7 | Y 轴移动 (有符号) |
| 3 | Bit0-7 | 滚轮 (有符号) |

## 初始化流程

```
1. usb_init()
   ├─ hcd_init()                    // 初始化主机控制器
   │  ├─ dwc2_init()                // DWC2 控制器初始化
   │  ├─ Reset USB Core
   │  └─ Enable Host mode
   ├─ hid_init()                    // 初始化 HID 驱动
   └─ usb_enumerate()               // 枚举 USB 设备
      ├─ 获取设备描述符
      ├─ 设置地址
      ├─ 获取配置描述符
      ├─ 查找 HID 接口
      └─ 查找中断端点 (IN)

2. usb_mouse_init()
   └─ 注册鼠标事件回调

3. 设备连接时:
   usb_mouse_connect()
   ├─ 验证设备是 HID 鼠标
   ├─ 获取报告描述符
   ├─ 配置中断端点
   └─ 提交中断 URB
```

## 使用示例

### 在内核中使用 USB 鼠标

```c
#include "modules/usb/usb.h"
#include "modules/mouse/mouse.h"

void usb_mouse_demo(void) {
    // 初始化 USB 系统
    usb_init();
    
    // 等待设备连接
    // ...
    
    // 获取鼠标设备
    usb_device_t* mouse = usb_find_device_by_vid_pid(0x046D, 0xC52B);
    if (mouse) {
        log_info("USB Mouse found: %s\n", mouse->name);
        
        // 连接鼠标驱动
        usb_mouse_connect(mouse);
    }
}

// 鼠标事件处理回调
void mouse_event_handler(usb_mouse_event_t* event) {
    if (event->buttons & 0x01) {
        log_info("Left button pressed\n");
    }
    if (event->x != 0 || event->y != 0) {
        log_info("Mouse move: x=%d, y=%d\n", event->x, event->y);
    }
    if (event->wheel != 0) {
        log_info("Wheel: %d\n", event->wheel);
    }
}
```

### 解析 HID 报告

```c
int usb_mouse_parse_report(u8* data, u32 len, usb_mouse_event_t* event) {
    if (len < 4) {
        return -1;
    }
    
    // 解析鼠标报告
    event->buttons = data[0];
    event->x = (s8)data[1];
    event->y = (s8)data[2];
    event->wheel = (s8)data[3];
    
    return 0;
}
```

## 构建配置

在 `duck/modules/usb/ya.py` 中：

```python
target("mod-usb")
set_kind("static")

add_deps('kernel')

common_source = [
    'usb.c',
    'hcd.c',
    'hub.c',
    'hid.c',
    'usb_mouse.c',
]

# 根据平台添加控制器驱动
platform_source = {
    'raspi3': ['dwc2.c'],
}

add_files(common_source)
add_files(platform_source.get(platform(), []))
add_includedirs('../')
```

## 平台支持

| 平台 | 控制器驱动 | 基地址 |
|------|------------|--------|
| raspi3 | dwc2.c | 0x3F980000 |

## 当前实现状态

### ✅ 已完成
- [x] USB 核心框架
- [x] USB 设备枚举
- [x] 控制传输
- [x] 中断传输
- [x] HID 类驱动
- [x] USB 鼠标驱动
- [x] DWC2 控制器驱动 (raspi3)

### 🚧 进行中
- [ ] 批量传输实现
- [ ] 等时传输支持
- [ ] 电源管理
- [ ] 设备热插拔

### 📋 待实现
- [ ] USB 键盘支持
- [ ] USB 触摸屏支持
- [ ] 复合设备支持
- [ ] 驱动自动加载

## 调试方法

### 启用调试输出

在代码中添加：

```c
#define USB_DEBUG(fmt, ...) kprintf("[USB] " fmt, ##__VA_ARGS__)
#define USB_INFO(fmt, ...) kprintf("[USB] " fmt, ##__VA_ARGS__)
#define USB_ERROR(fmt, ...) kprintf("[USB ERROR] " fmt, ##__VA_ARGS__)
```

### 查看初始化日志

```bash
# 运行内核后查看日志
# 应该看到类似输出:
# [USB] USB core initialized
# [USB] DWC2 OTG controller found
# [USB] USB device connected: vid=0x046d pid=0xc52b
# [USB] USB mouse initialized
```

### 常见问题

1. **鼠标不工作**
   - 检查 USB 主机控制器是否初始化成功
   - 验证设备枚举是否完成
   - 确认中断端点配置正确

2. **移动不准确**
   - 检查报告解析是否正确
   - 验证 X/Y 坐标符号扩展

## 参考资源

1. **USB 2.0 Specification**
   - HID 类定义
   - 设备枚举流程

2. **HID Specification**
   - 报告描述符格式
   - 鼠标报告结构

3. **DesignWare DWC_otg Databook**
   - USB 控制器编程指南

4. **BCM2837 ARM Peripherals**
   - USB 控制器寄存器

## 许可证

Copyright 2021-present evilbinary
