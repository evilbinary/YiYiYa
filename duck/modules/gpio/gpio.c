/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "gpio.h"

#include "kernel/kernel.h"


size_t gpio_read(device_t* dev, void* buf, size_t len) {
  u32 ret = 0;
  
  return ret;
}

size_t gpio_write(device_t* dev, const void* buf, size_t len) {
  u32 ret = 0;
  

  return ret;
}

size_t gpio_ioctl(device_t* dev, u32 cmd, void* args) {
  u32 ret = 0;

  return ret;
}

int gpio_init(void) {
  kprintf("gpio init\n");
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "gpio";
  dev->read = gpio_read;
  dev->write = gpio_write;
  dev->ioctl = gpio_ioctl;
  dev->id = DEVICE_I2C;
  dev->type = DEVICE_TYPE_CHAR;
  device_add(dev);
  
  gpio_init_device(dev);
  return 0;
}

void gpio_exit(void) { kprintf("gpio exit\n"); }

module_t gpio_module = {.name = "gpio", .init = gpio_init, .exit = gpio_exit};