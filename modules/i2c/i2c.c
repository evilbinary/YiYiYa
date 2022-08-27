/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "i2c.h"

size_t i2c_read(device_t* dev, void* buf, size_t len) {
  u32 ret = 0;
  i2c_t* i2c = dev->data;
  if (i2c == NULL) {
    kprintf("i2c is null\n");
    return ret;
  }
  i2c->read(i2c, buf, len);
  return ret;
}

size_t i2c_write(device_t* dev, const void* buf, size_t len) {
  u32 ret = 0;
  i2c_t* i2c = dev->data;
  if (i2c == NULL) {
    kprintf("i2c is null\n");
    return ret;
  }
  i2c->write(i2c, buf, len);
  return ret;
}

size_t i2c_ioctl(device_t* dev, u32 cmd, void* args) {
  u32 ret = 0;
  i2c_t* i2c = dev->data;
  if (i2c == NULL) {
    kprintf("i2c is null\n");
    return ret;
  }
  return ret;
}

int i2c_init(void) {
  kprintf("i2c\n");

  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "i2c";
  dev->read = i2c_read;
  dev->write = i2c_write;
  dev->ioctl = i2c_ioctl;
  dev->id = DEVICE_I2C;
  dev->type = DEVICE_TYPE_CHAR;
  device_add(dev);

  i2c_init_device(dev);

  return 0;
}

void i2c_exit(void) { kprintf("i2c exit\n"); }

module_t i2c_module = {.name = "i2c", .init = i2c_init, .exit = i2c_exit};
