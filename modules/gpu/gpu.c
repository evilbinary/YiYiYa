/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "gpu.h"
#include "vga/vga.h"

size_t gpu_read(device_t* dev, void* buf, size_t len) {
  u32 ret = 0;
  return ret;
}

size_t gpu_write(device_t* dev, const void* buf, size_t len) {
  u32 ret = 0;
  vga_device_t* vga = dev->data;
  if (vga == NULL) {
    kprintf("not found vga\n");
    return ret;
  }
  kstrncpy(vga->frambuffer, (const char*)buf, len);
  return ret;
}

size_t gpu_ioctl(device_t* dev, u32 cmd, void* args) {
  u32 ret = 0;
  vga_device_t* vga = dev->data;
  if (vga == NULL) {
    kprintf("not found vga\n");
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

void gpu_init_device(device_t* dev) {
  vga_device_t* vga = kmalloc(sizeof(vga_device_t));
  vga->frambuffer = 0;
  dev->data = vga;
  gpu_init_mode(vga, VGA_MODE_480x272x32);
  // gpu_init_mode(vga, VGA_MODE_1024x768x32);
  kprintf("gpu_init_device end\n");
}

int gpu_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "vga";
  dev->read = gpu_read;
  dev->write = gpu_write;
  dev->ioctl = gpu_ioctl;
  dev->id = DEVICE_VGA;
  dev->type = DEVICE_TYPE_VGA;
  device_add(dev);

  gpu_init_device(dev);
  return 0;
}

void gpu_exit(void) { kprintf("gpu exit\n"); }

module_t gpu_module = {.name = "vga", .init = gpu_init, .exit = gpu_exit};