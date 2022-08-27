/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "vga.h"

#define VBOX_VENDOR_ID 0x80EE
#define VBOX_DEVICE_ID 0xCAFE


int vbox_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "vbox";
  dev->read = vga_read;
  dev->write = vga_write;
  dev->ioctl = vga_ioctl;
  dev->id = DEVICE_VGA_VBOX;
  dev->type = DEVICE_TYPE_VGA;
  device_add(dev);

  qemu_init_device(dev,VBOX_VENDOR_ID,VBOX_DEVICE_ID);
  return 0;
}

void vbox_exit(void) { kprintf("vga exit\n"); }

module_t vbox_module = {.name = "vga", .init = vbox_init, .exit = vbox_exit};
