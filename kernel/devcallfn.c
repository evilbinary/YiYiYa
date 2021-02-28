/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "kernel.h"


size_t dev_ioctl(int fd, u32 cmd, ...) {
  u32 ret = 0;
  device_t* dev = device_find(fd);
  if (dev == NULL) {
    return ret;
  }
  va_list args;
  va_start(args, cmd);
  ret = dev->ioctl(dev, cmd, args);
  va_end(args);
  return ret;
}

size_t dev_read(int fd, void* buf, size_t nbytes) {
  device_t* dev = device_find(fd);
  return dev->read(dev, buf, nbytes);
}

size_t dev_write(int fd, void* buf, size_t nbytes) {
  device_t* dev = device_find(fd);
  return dev->write(dev, buf, nbytes);
}