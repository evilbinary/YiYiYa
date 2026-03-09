# 平台支持

## 支持的硬件平台

### ARM 平台

#### Raspberry Pi 系列
- **Raspberry Pi 3** (BCM2837, ARM Cortex-A53)
  - 架构: ARM64 (AArch64)
  - 主频: 1.2 GHz
  - 内存: 1GB LPDDR2
  - 网络: USB Ethernet (SMSC LAN9514)
  - 存储: MicroSD
  - 状态: ✅ 完全支持

- **Raspberry Pi 2** (BCM2836, ARM Cortex-A7)
  - 架构: ARMv7-A
  - 主频: 900 MHz
  - 内存: 1GB LPDDR2
  - 网络: USB Ethernet
  - 存储: MicroSD
  - 状态: ✅ 完全支持

#### Allwinner 系列
- **V3S** (ARM Cortex-A7)
  - 架构: ARMv7-A
  - 主频: 1.2 GHz
  - 内存: 64MB DDR2 (内置)
  - 显示: LCD/TV
  - 存储: SD卡/SPI Flash
  - 状态: ✅ 完全支持

- **F1C200S** (ARM926EJ-S)
  - 架构: ARMv5TE
  - 主频: 400 MHz
  - 内存: 32MB DDR2 (内置)
  - 显示: LCD/TV
  - 存储: SD卡/SPI Flash
  - 状态: ✅ 完全支持

- **T113-S3** (ARM Cortex-A7)
  - 架构: ARMv7-A
  - 双核
  - 内存: 64MB DDR2 (内置)
  - 状态: ✅ 完全支持

#### ST STM32 系列
- **STM32F4xx** (ARM Cortex-M4)
  - 架构: ARMv7E-M
  - 主频: 168 MHz
  - 内存: 192KB SRAM
  - 外设: FSMC, SDIO, USB
  - 状态: ✅ 完全支持

#### NXP i.MX 系列
- **i.MX6ULL** (ARM Cortex-A7)
  - 架构: ARMv7-A
  - 主频: 528 MHz
  - 内存: 512MB DDR3
  - 状态: 🚧 开发中

### RISC-V 平台

- **QEMU RISC-V Virt**
  - 架构: RV64GC
  - 状态: ✅ 支持

### x86 平台

- **QEMU i386/x86_64**
  - 架构: x86/x86_64
  - 状态: ✅ 支持

## 平台特性对比

| 平台 | 架构 | MMU | FPU | SMP | 实时性 |
|------|------|-----|-----|-----|--------|
| RasPi3 | ARM64 | ✅ | ✅ | ✅ | ✅ |
| RasPi2 | ARMv7 | ✅ | ✅ | ✅ | ✅ |
| V3S | ARMv7 | ✅ | ✅ | ❌ | ✅ |
| F1C200S | ARMv5 | ✅ | ❌ | ❌ | ✅ |
| T113-S3 | ARMv7 | ✅ | ✅ | ✅ | ✅ |
| STM32F4 | ARMv7-M | ❌ | ✅ | ❌ | ✅✅ |

## 平台选择指南

### 入门学习
推荐: **Raspberry Pi 3** 或 **QEMU**
- 文档完善
- 社区支持好
- 调试方便

### 嵌入式产品
推荐: **V3S** 或 **F1C200S**
- 成本低
- 功耗小
- 集成度高

### 实时应用
推荐: **STM32F4xx**
- 确定性响应
- 低延迟
- 丰富的外设

### 教学实验
推荐: **QEMU**
- 无需硬件
- 快速原型
- 易于调试

## 外设驱动支持

### 网络驱动
| 驱动 | 支持平台 | 状态 |
|------|---------|------|
| BCM2837 Net | RasPi2/3 | 🚧 基础框架 |
| E1000 | x86/QEMU | ✅ 完整支持 |

### 存储驱动
| 驱动 | 支持平台 | 状态 |
|------|---------|------|
| SD/MMC | 所有平台 | ✅ 支持 |
| SPI Flash | V3S/F1C200S | ✅ 支持 |

### 显示驱动
| 驱动 | 支持平台 | 状态 |
|------|---------|------|
| ST7735 | STM32 | ✅ 支持 |
| ILI9341 | STM32 | ✅ 支持 |
| Framebuffer | RasPi/V3S | ✅ 支持 |

### 输入设备
| 驱动 | 支持平台 | 状态 |
|------|---------|------|
| GPIO Keyboard | 所有平台 | ✅ 支持 |
| USB HID | RasPi | 🚧 开发中 |

## 编译和运行

### 编译命令

```bash
# Raspberry Pi 3
ya make -p raspi3

# Raspberry Pi 2
ya make -p raspi2

# V3S
ya make -p v3s

# STM32F4xx
ya make -p stm32f4xx

# QEMU (x86)
ya make -p qemu
```

### 运行命令

```bash
# QEMU x86
qemu-system-i386 -kernel build/kernel.elf

# QEMU ARM
qemu-system-arm -M raspi3 -kernel build/kernel.elf

# 开发板
# 使用 SD 卡启动或通过 JTAG 下载
```

## 移植新平台

要移植到新平台，需要实现：

1. **架构支持**
   - 启动代码
   - 中断处理
   - 上下文切换

2. **平台驱动**
   - UART (调试输出)
   - 定时器 (系统时钟)
   - GPIO (基本控制)

3. **可选驱动**
   - 存储设备
   - 网络设备
   - 显示设备

详见 [移植适配指南](移植适配.md)。

## 参考资料

- [开发板运行](开发板运行.md)
- [移植适配](移植适配.md)
- [驱动开发](驱动开发.md)
