/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "i2c.h"


u32 stm32_i2c_read_write(i2c_t* i2c, u32* data, u32 count) {
  if (data == NULL || count <= 0) {
    return 0;
  }
  i2c_msg_t* pmsg = data;
  return count;
}

int i2c_init_device(device_t* dev) {
  i2c_t* i2c = kmalloc(sizeof(i2c_t));
  dev->data = i2c;

  i2c->inited = 0;
  i2c->read = stm32_i2c_read_write;
  i2c->write = stm32_i2c_read_write;

  return 0;
}