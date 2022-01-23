/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "kernel.h"
#include "devfn.h"


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

u32 device_read(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  u32 ret = 0;
  device_t *dev = (device_t *)node->device;
  if (dev == NULL) {
    return ret;
  }
  ret = dev->read(dev, buffer, nbytes);
  return ret;
}

u32 device_write(vnode_t *node, u32 offset, size_t nbytes, u8 *buffer) {
  u32 ret = 0;
  device_t *dev = (device_t *)node->device;
  if (dev == NULL) {
    return ret;
  }
  ret = dev->write(dev, buffer, nbytes);
  return ret;
}

size_t device_ioctl(vnode_t *node, u32 cmd, va_list args) {
  u32 ret = 0;
  device_t *dev = node->device;
  if (dev == NULL) {
    return ret;
  }
  // va_list args;
  // va_start(args, cmd);
  ret = dev->ioctl(dev, cmd, args);
  // va_end(args);
  return ret;
}


u32 device_open(vnode_t *node) {
  u32 ret = 0;
  device_t *dev = (device_t *)node->device;
  if (dev == NULL) {
    return ret;
  }
  //todo 
  //ret = dev->close(dev, buffer, nbytes);
  return ret;
}

u32 device_close(vnode_t *node) {
  u32 ret = 0;
  device_t *dev = (device_t *)node->device;
  if (dev == NULL) {
    return ret;
  }
  //todo 
  //ret = dev->close(dev, buffer, nbytes);
  return ret;
}