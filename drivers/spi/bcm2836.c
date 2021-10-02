/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "spi.h"
#include "kernel/kernel.h"

int spi_init_device(device_t* dev) {
   spi_t* spi = kmalloc(sizeof(spi_t));
  dev->data = spi;

  spi->inited = 0;

  return 0;
}
