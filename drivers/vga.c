/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "vga.h"

static size_t read(device_t* dev, void* buf, size_t len) {
  u32 ret = 0;
  return ret;
}

static size_t write(device_t* dev, const void* buf, size_t len) {
  u32 ret = 0;
  vga_device_t* vga = dev->data;
  //   *(vga->frambuffer) = 0xff55ff44ff22;
  //   *(vga->frambuffer+1) = 0xff55ff44ff22;
  kstrncpy(vga->frambuffer,(const char*)buf, len);
  return ret;
}

void vga_init_device(device_t* dev) {
  pci_device_t* pdev = pci_find_class(0x300);
  if (pdev == NULL) {
    kprintf("can not find pci device\n");
    return;
  }
  u32 bar0 =
      pci_read32(pdev->bus, pdev->slot, pdev->function, 0x10) & 0xFFFFFFF0;
  u32 bar1 =
      pci_read32(pdev->bus, pdev->slot, pdev->function, 0x14) & 0xFFFFFFF0;
  u32 bar2 =
      pci_read32(pdev->bus, pdev->slot, pdev->function, 0x18) & 0xFFFFFFF0;

  // kprintf("bar0:%x ", bar0);
  vga_device_t* vga = kmalloc(sizeof(vga_device_t));
  vga->frambuffer = bar0;
  dev->data = vga;
  u32 addr = bar0;
  for (int i = 0; i < 128; i++) {
    map_page(addr, addr, PAGE_P | PAGE_USU | PAGE_RWW);
    addr += 0x1000;
  }
}

int vga_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "vga";
  dev->read = read;
  dev->write = write;
  dev->id = DEVICE_VGA;
  dev->type = DEVICE_TYPE_VGA;
  device_add(dev);

  vga_init_device(dev);
  return 0;
}

void vga_exit(void) { kprintf("Goodbye World\n"); }

module_t vga_module = {.name = "vga", .init = vga_init, .exit = vga_exit};

module_regit(vga_module);
