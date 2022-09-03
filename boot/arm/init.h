/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef INIT_H
#define INIT_H

#include "arch/arch.h"
#include "config.h"

#define KERNEL_MOVE 
#define READ_BLOCK_SIZE 512
#define KERNEL_BASE 0x100000
#ifdef V3S
#define KERNEL_ORIGIN_BASE 0x40000000
#else
#define KERNEL_ORIGIN_BASE 0x10000+512*12
#endif
#define PDT_BASE 0x9000
#define BOOT_VERSION 0x01

#define SECOND_BOOT_ENTRY 0x8000 //qemu 0x1000 real board 0x8000
 
void init_boot();
void init_display();

void* load_kernel();
void init_memory();
void init_kernel();

typedef int (*entry)(int, char**, char**);
#define getch() uart_getc()
#define putc(s)  uart_send(s)

#endif