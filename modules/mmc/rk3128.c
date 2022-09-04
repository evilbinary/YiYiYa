/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "sdhci.h"

void sdhci_dev_init(sdhci_device_t *sdhci_dev) {
 
}

int sdhci_dev_port_read(sdhci_device_t *sdhci_dev, int no, sector_t sector,
                        u32 count, u32 buf) {
//   size_t buf_size = count * BYTE_PER_SECTOR;
//   u32 ret = 0;

// #ifdef CACHE_ENABLED
//   if (count == 1) {
//     int index = sector.startl & CACHE_MASK;
//     void *cache_p = (void *)(sdhci_dev->cache_buffer + SECTOR_SIZE * index);
//     if (sdhci_dev->cached_blocks[index] != sector.startl) {
//       ret = sd_read(cache_p, buf_size, sector.startl);
//       sdhci_dev->cached_blocks[index] = sector.startl;
//     }
//     kmemmove(buf, cache_p, SECTOR_SIZE);
//   } else {
// #endif
//     ret = sd_read(buf, buf_size, sector.startl);
//     if (ret < buf_size) {
//       return SD_ERROR;
//     }
// #ifdef CACHE_ENABLED
//   }
// #endif
  // kprintf("sd read offset:%x %x count:%d\n",sector.startl*BYTE_PER_SECTOR,sector.starth*BYTE_PER_SECTOR,count);
  // print_hex(buf,buf_size);
  // kprintf("ret %d\n",ret);
//   return ret;
}

int sdhci_dev_port_write(sdhci_device_t *sdhci_dev, int no, sector_t sector,
                         u32 count, u32 buf) {
//   u32 ret = 0;
//   size_t buf_size = count * BYTE_PER_SECTOR;
//   ret = sd_write((uint8_t *)buf, buf_size, sector.startl);
//   if (ret < buf_size) {
//     return SD_ERROR;
//   }
// #ifdef CACHE_ENABLED
//   int i;
//   u8* p=buf;
//   for (i = 0; i < count; i++) {
//     int index = (sector.startl + i) & CACHE_MASK;
//     void *cache_p = (void *)(sdhci_dev->cache_buffer + SECTOR_SIZE * index);
   
//     kmemmove(cache_p,&p[SECTOR_SIZE * i], SECTOR_SIZE);
//   }
// #endif
//   return ret;
}