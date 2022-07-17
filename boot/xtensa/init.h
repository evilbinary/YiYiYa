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
#define KERNEL_BASE 0x40082000
#define KERNEL_ORIGIN_BASE KERNEL_BASE

// #define KERNEL_BIN 1

#define PDT_BASE 0x9000
#define BOOT_VERSION 0x01

#define KERNEL_FLASH_ADDR 0x10000

/* Cache MMU block size */
#define MMU_BLOCK_SIZE    0x00010000

/* Cache MMU address mask (MMU tables ignore bits which are zero) */
#define MMU_FLASH_MASK    (~(MMU_BLOCK_SIZE - 1))

#define PHDR_NUM 8
#define SHDR_NUM 10

void init_boot();
void init_display();

void* load_kernel();
void init_memory();
void init_kernel();
void start_kernel();




#endif