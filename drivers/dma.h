/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef DMA_H
#define DMA_H

#include "kernel/kernel.h"

#define DMA_MODE_READ_TRANSFER 4
#define DMA_MODE_WRITE_TRANSFER 8

#define DMA_MODE_TRANSFER_SINGLE  0x40
#define DMA_MODE_TRANSFER_BLOCK  0x80
#define DMA_MODE_TRANSFER_CASCADE  0xC0

u32 dma_trans(u8 channel, u8 mode, void* addr, void* phyaddr, size_t size);

#endif