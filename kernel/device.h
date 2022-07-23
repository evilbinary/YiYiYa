/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef DEVICE_H
#define DEVICE_H

#include "kernel/kernel.h"
#include "config.h"

enum {
  DEVICE_KEYBOARD = 0,
  DEVICE_PCI,
  DEVICE_VGA,
  DEVICE_VGA_QEMU,
  DEVICE_VGA_VBOX,
  DEVICE_MOUSE,
  DEVICE_SERIAL,
  DEVICE_RTC,
  DEVICE_SATA,
  DEVICE_SATA0,
  DEVICE_SATA1,
  DEVICE_SATA2,
  DEVICE_SATA3,
  DEVICE_SATA4,
  DEVICE_SATA5,
  DEVICE_SATA6,
  DEVICE_I2C,
  DEVICE_GPIO,
  DEVICE_SPI,
  DEVICE_SB,
  DEVICE_NET
};

enum {
  DEVICE_TYPE_CHAR = 1,
  DEVICE_TYPE_BLOCK,
  DEVICE_TYPE_VIRTUAL,
  DEVICE_TYPE_FILE,
  DEVICE_TYPE_VGA,
  DEVICE_TYPE_NET
};

#ifndef MAX_DEVICE
#define MAX_DEVICE 256
#endif

typedef struct dstat {
  size_t size;
  int is_dir;
} dstat_t;

typedef struct device device_t;

typedef size_t (*dev_read_fn)(device_t* dev, void* buf, size_t len);
typedef size_t (*dev_write_fn)(device_t* dev, const void* buf, size_t len);
typedef size_t (*dev_stat_fn)(device_t* dev, dstat_t* stat);
typedef size_t (*dev_ioctl_fn)(device_t* dev, u32 cmd, ...);

typedef struct device {
  char* name;
  u32 id;
  u32 type;  // char block vga
  dev_read_fn read;
  dev_write_fn write;
  dev_ioctl_fn ioctl;
  dev_stat_fn stat;
  void* data;  // data
} device_t;


void device_add(device_t* device);

void device_remove();

device_t* device_find(u32 id);

device_t* device_create(u8* name, u32 id, u32 type);

#endif