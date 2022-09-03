/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "lcd.h"
#include "vga/vga.h"

size_t lcd_read(device_t* dev, void* buf, size_t len) {
  u32 ret = 0;
  return ret;
}

size_t lcd_write(device_t* dev, const void* buf, size_t len) {
  u32 ret = 0;
  vga_device_t* vga = dev->data;
  if (vga == NULL) {
    kprintf("not found lcd\n");
    return ret;
  }
  if(vga->frambuffer!=NULL){
    kstrncpy(vga->frambuffer, (const char*)buf, len);
  }else{
    vga->write(vga,buf,len);
  }
  return ret;
}

size_t lcd_ioctl(device_t* dev, u32 cmd, void* args) {
  u32 ret = 0;
  vga_device_t* vga = dev->data;
  if (vga == NULL) {
    kprintf("not found lcd\n");
    return ret;
  }
  if (cmd == IOC_READ_FRAMBUFFER) {
    ret = vga->frambuffer;
  } else if (cmd == IOC_READ_FRAMBUFFER_WIDTH) {
    ret = vga->width;
  } else if (cmd == IOC_READ_FRAMBUFFER_HEIGHT) {
    ret = vga->height;
  } else if (cmd == IOC_READ_FRAMBUFFER_BPP) {
    ret = vga->bpp;
  } else if (cmd == IOC_FLUSH_FRAMBUFFER) {
    if (vga->frambuffer != NULL && vga->flip_buffer != NULL) {
      u32 offset = (u32*)args;
      vga->flip_buffer(vga, offset % vga->framebuffer_count);
    }
  } else if (cmd == IOC_READ_FRAMBUFFER_INFO) {
    vga_device_t* buffer_info = (u32*)args;
    u32 size =(u32*)args;
    *buffer_info = *vga;
  }
  return ret;
}

void lcd_init_device(device_t* dev) {
  vga_device_t* vga = kmalloc(sizeof(vga_device_t));
  vga->frambuffer = 0;
  dev->data = vga;
  lcd_init_mode(vga, VGA_MODE_128x128x16);
  kprintf("lcd_init_device end\n");
}

int lcd_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "vga";
  dev->read = lcd_read;
  dev->write = lcd_write;
  dev->ioctl = lcd_ioctl;
  dev->id = DEVICE_VGA;
  dev->type = DEVICE_TYPE_VGA;
  device_add(dev);

  lcd_init_device(dev);
  return 0;
}

void lcd_exit(void) { kprintf("lcd exit\n"); }

module_t lcd_module = {.name = "vga", .init = lcd_init, .exit = lcd_exit};