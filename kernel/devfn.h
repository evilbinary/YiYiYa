/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef DEVCFN_H
#define DEVCFN_H

#include "vfs.h"


size_t dev_ioctl(int fd, u32 cmd, ...);

size_t dev_read(int fd, void* buf, size_t nbytes);

size_t dev_write(int fd, void* buf, size_t nbytes);


u32 device_write(vnode_t *node, u32 offset, size_t nbytes, u8 *buf);
u32 device_read(vnode_t *node, u32 offset, size_t nbytes, u8 *buf);
size_t device_ioctl(vnode_t *node, u32 cmd, va_list args);
u32 device_close(vnode_t *node) ;
u32 device_open(vnode_t *node);

#endif