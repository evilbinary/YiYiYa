/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "spi.h"

size_t spi_read(device_t* dev, void* buf, size_t len) {
  u32 ret = 0;
  spi_t* spi = dev->data;
  if (spi == NULL) {
    kprintf("spi is null\n");
    return ret;
  }
  spi->read(spi, buf, len);
  return ret;
}

size_t spi_write(device_t* dev, const void* buf, size_t len) {
  u32 ret = 0;
  spi_t* spi = dev->data;
  if (spi == NULL) {
    kprintf("spi is null\n");
    return ret;
  }
  spi->write(spi, buf, len);
  return ret;
}

size_t spi_ioctl(device_t* dev, u32 cmd, void* args) {
  u32 ret = 0;
  spi_t* spi = dev->data;
  if (spi == NULL) {
    kprintf("spi is null\n");
    return ret;
  }
  return ret;
}

int spi_init(void) {
  kprintf("spi\n");

  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "spi";
  dev->read = spi_read;
  dev->write = spi_write;
  dev->ioctl = spi_ioctl;
  dev->id = DEVICE_SPI;
  dev->type = DEVICE_TYPE_CHAR;
  device_add(dev);

  spi_init_device(dev);

  return 0;
}

void spi_exit(void) { kprintf("spi exit\n"); }

module_t spi_module = {.name = "spi", .init = spi_init, .exit = spi_exit};
