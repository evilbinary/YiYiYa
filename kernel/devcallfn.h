/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef DEVCALLFN_H
#define DEVCALLFN_H

#include "arch/arch.h"


size_t dev_ioctl(int fd, u32 cmd, ...);

size_t dev_read(int fd, void* buf, size_t nbytes);

size_t dev_write(int fd, void* buf, size_t nbytes);

#endif