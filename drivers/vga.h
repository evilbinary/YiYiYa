/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef VGA_H
#define VGA_H

#include "kernel/kernel.h"
#include "pci.h"

enum VGA_MODE {
  VGA_MODE_80x25 = 1,
  VGA_MODE_320x200x256 ,
  VGA_MODE_1024x768x32
};

#define IOC_MAGIC 'v'
#define IOC_INIT _IO(IOC_MAGIC, 0)
#define IOC_READ_REG _IOW(IOC_MAGIC, 1, int)
#define IOC_WRITE_REG _IOW(IOC_MAGIC, 2, int)
#define IOC_READ_FRAMBUFFER _IOW(IOC_MAGIC, 3, int)
#define IOC_READ_FRAMBUFFER_WIDTH _IOW(IOC_MAGIC, 4, int)
#define IOC_READ_FRAMBUFFER_HEIGHT _IOW(IOC_MAGIC, 5, int)
#define IOC_READ_FRAMBUFFER_BPP _IOW(IOC_MAGIC, 6, int)


typedef struct vga_device vga_device_t;

typedef void (*write_pixel_fn)(vga_device_t* vga, u32 x, u32 y, u32 c);

typedef struct vga_device {
  u32* frambuffer;
  u32 inited;
  u32 width;
  u32 height;
  u32 bpp;
  u32 mode;
  write_pixel_fn write;
} vga_device_t;

size_t vga_read(device_t* dev, void* buf, size_t len);
size_t vga_write(device_t* dev, const void* buf, size_t len);
size_t vga_ioctl(device_t* dev, u32 cmd, ...);

#endif