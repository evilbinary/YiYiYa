/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef INIT_H
#define INIT_H

#include "arch/arch.h"

/* KERNEL_SIZE 已改为运行时由链接符号 __start/__end 计算（见 init-armv8-a.c），
 * config.h 仅作兼容兜底（其构建期生成挂钩不可靠，值可能陈旧） */
#include "config.h"

#define READ_BLOCK_SIZE 512

#if defined(RASPI3) || defined(RASPI5) || defined(RK3568)
/* Kernel image is loaded directly into RAM by firmware/U-Boot */
#define KERNEL_ORIGIN_BASE 0
#else
#define KERNEL_ORIGIN_BASE 0x10000+512*12
#endif

#if defined(RASPI5)
/* Pi5 firmware loads the raw kernel image at 0x80000 */
#define KERNEL_BASE 0x80000
#elif defined(RK3568)
/* RK3568 U-Boot：kernel_addr_r = 0x02080000（Rockchip 标准布局）。
 * uImage.img 的 -a/-e 以及 app/xlinker/link-rk3568.ld 都用这个地址。 */
#define KERNEL_BASE 0x02080000
#else
#define KERNEL_BASE 0x100000
#endif

#define PDT_BASE 0x9000
#define BOOT_VERSION 0x01

#define SECOND_BOOT_ENTRY 0x8000
 
void init_boot();
void init_display();

void* load_kernel();
void init_memory();
void init_kernel();

typedef int (*entry)(int, char**, char**);


#endif
