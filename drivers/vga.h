/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef VGA_H
#define VGA_H

#include "kernel/kernel.h"
#include "pci.h"

typedef struct vga_device{
   u32* frambuffer;
}vga_device_t;

#endif