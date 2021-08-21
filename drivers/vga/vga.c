/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "vga.h"

#include "dma/dma.h"

size_t vga_read(device_t* dev, void* buf, size_t len) {
  u32 ret = 0;
  return ret;
}

size_t vga_write(device_t* dev, const void* buf, size_t len) {
  u32 ret = 0;
  vga_device_t* vga = dev->data;
  if (vga == NULL) {
    kprintf("not found vga\n");
    return ret;
  }
  kstrncpy(vga->frambuffer, (const char*)buf, len);
  return ret;
}

size_t vga_ioctl(device_t* dev, u32 cmd, void* args) {
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
    if (vga->frambuffer != NULL&&vga->flip_buffer!=NULL) {
      u32 offset=(u32*)args;
      vga->flip_buffer(vga,offset%vga->framebuffer_count);
    }
  }
  else if (cmd == IOC_READ_FRAMBUFFER_INFO) {
   vga_device_t* buffer_info=(u32*)args;
   u32 size=(u32*)args;
   *buffer_info=*vga;
  }
  return ret;
}

void vga_init_device(device_t* dev) {
  pci_device_t* pdev = pci_find_class(0x300);
  if (pdev == NULL) {
    kprintf("can not find pci vga device\n");
    return;
  }
  u32 bar0 = pci_dev_read32(pdev, PCI_BASE_ADDR0) & 0xFFFFFFF0;
  // u32 bar1 =
  //     pci_read32(pdev->bus, pdev->slot, pdev->function, 0x14) & 0xFFFFFFF0;
  // u32 bar2 =
  //     pci_read32(pdev->bus, pdev->slot, pdev->function, 0x18) & 0xFFFFFFF0;

  // kprintf("bar0:%x ", bar0);
  vga_device_t* vga = kmalloc(sizeof(vga_device_t));
  vga->frambuffer = bar0;
  dev->data = vga;
  u32 addr = bar0;
  for (int i = 0; i < 128; i++) {
    map_page(addr, addr, PAGE_P | PAGE_USU | PAGE_RWW);
    addr += 0x1000;
  }

  // vga_init_mode(vga,VGA_MODE_80x25);
  vga_init_mode(vga, VGA_MODE_320x200x256);
}

int vga_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "vga";
  dev->read = vga_read;
  dev->write = vga_write;
  dev->ioctl = vga_ioctl;
  dev->id = DEVICE_VGA;
  dev->type = DEVICE_TYPE_VGA;
  device_add(dev);

  vga_init_device(dev);
  return 0;
}

void vga_exit(void) { kprintf("vga exit\n"); }

module_t vga_module = {.name = "vga", .init = vga_init, .exit = vga_exit};
