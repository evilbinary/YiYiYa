/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef AHCI_H
#define AHCI_H

#include "kernel/kernel.h"

#define IOC_SDHCI_MAGIC 's'
#define IOC_READ_OFFSET _IOW(IOC_SDHCI_MAGIC, 3, int)
#define IOC_WRITE_OFFSET _IOW(IOC_SDHCI_MAGIC, 4, int)


#define BYTE_PER_SECTOR 512


#define SDHCI_DIVIDER_SHIFT		8
#define SDHCI_DIVIDER_HI_SHIFT	6
#define SDHCI_DIV_MASK			0xFF
#define SDHCI_DIV_MASK_LEN		8
#define SDHCI_DIV_HI_MASK		0x300

#define SDHCI_MAX_DIV_SPEC_200	256
#define SDHCI_MAX_DIV_SPEC_300	2046

#define  SDHCI_INT_RESPONSE     0x00000001
#define  SDHCI_INT_DATA_END     0x00000002
#define  SDHCI_INT_BLK_GAP      0x00000004
#define  SDHCI_INT_DMA_END      0x00000008
#define  SDHCI_INT_SPACE_AVAIL  0x00000010
#define  SDHCI_INT_DATA_AVAIL   0x00000020
#define  SDHCI_INT_CARD_INSERT  0x00000040
#define  SDHCI_INT_CARD_REMOVE  0x00000080
#define  SDHCI_INT_CARD_INT     0x00000100
#define  SDHCI_INT_ERROR        0x00008000
#define  SDHCI_INT_TIMEOUT      0x00010000
#define  SDHCI_INT_CRC          0x00020000
#define  SDHCI_INT_END_BIT      0x00040000
#define  SDHCI_INT_INDEX        0x00080000
#define  SDHCI_INT_DATA_TIMEOUT 0x00100000
#define  SDHCI_INT_DATA_CRC     0x00200000
#define  SDHCI_INT_DATA_END_BIT 0x00400000
#define  SDHCI_INT_BUS_POWER    0x00800000
#define  SDHCI_INT_ACMD12ERR    0x01000000
#define  SDHCI_INT_ADMA_ERROR   0x02000000

typedef struct sdhci_device {
  u32 offsetl;
  u32 offseth;
  u32* cached_blocks;
  u8* cache_buffer;
  void* data;
} sdhci_device_t;

typedef struct sector{
  u32 startl;
  u32 starth;
} sector_t;


#endif