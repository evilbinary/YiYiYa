/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "kernel/kernel.h"
#include "dma.h"

u8 dma_clear_reg[8] = {0x0C, 0x0C, 0x0C, 0x0C, 0xD8, 0xD8, 0xD8, 0xD8};
u8 dma_mode_reg[8] = {0x0B, 0x0B, 0x0B, 0x0B, 0xD6, 0xD6, 0xD6, 0xD6};
u8 dma_mask_reg[8] = {0x0A, 0x0A, 0x0A, 0x0A, 0xD4, 0xD4, 0xD4, 0xD4};

u8 dma_page_port[8] = {0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A};
u8 dma_addr_port[8] = {0x00, 0x02, 0x04, 0x06, 0xC0, 0xC4, 0xC8, 0xCC};
u8 dma_count_port[8] = {0x01, 0x03, 0x05, 0x07, 0xC2, 0xC6, 0xCA, 0xCE};

u32 dma_trans(u8 channel, u8 mode, void* addr, void* phyaddr, size_t size) {
  cpu_cli();
  mode+=channel;
  //init
  io_write8(dma_mask_reg[channel], 0x04 | channel);
  io_write8(dma_clear_reg[channel], 0x00);
  io_write8(dma_mode_reg[channel], mode);
  //src 
  io_write8(dma_addr_port[channel], (ulong)addr & 0x00FF);
  io_write8(dma_addr_port[channel], ((ulong)addr & 0xFF00) >> 8);

  //phyaddr
  io_write8(dma_page_port[channel], phyaddr);
  
  //size
  io_write8(dma_count_port[channel], size & 0x00FF);
  io_write8(dma_count_port[channel], (size & 0xFF00) >> 8);
  //clear mask
  io_write8(dma_mask_reg[channel], channel);
  cpu_sti();
  return 1;
}