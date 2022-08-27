/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "vga.h"

#define QEMU_VENDOR_ID 0x1234
#define QEMU_DEVICE_ID 0x1111

#define VBOX_VENDOR_ID 0x80EE
#define VBOX_DEVICE_ID 0xCAFE

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

#define VBE_DISPI_INDEX_ID 0x0
#define VBE_DISPI_INDEX_XRES 0x1
#define VBE_DISPI_INDEX_YRES 0x2
#define VBE_DISPI_INDEX_BPP 0x3
#define VBE_DISPI_INDEX_ENABLE 0x4
#define VBE_DISPI_INDEX_BANK 0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH 0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x7
#define VBE_DISPI_INDEX_X_OFFSET 0x8
#define VBE_DISPI_INDEX_Y_OFFSET 0x9
#define VBE_DISPI_INDEX_VIDEO_MEMORY_64K 0xa

#define VBE_DISPI_ID0 0xB0C0
#define VBE_DISPI_ID1 0xB0C1
#define VBE_DISPI_ID2 0xB0C2
#define VBE_DISPI_ID4 0xB0C4
#define VBE_DISPI_ID5 0xB0C5

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM 0x80

void qemu_write_reg(u16 index, u16 data) {
  io_write16(VBE_DISPI_IOPORT_INDEX, index);
  io_write16(VBE_DISPI_IOPORT_DATA, data);
}

u16 qemu_read_reg(u16 index) {
  io_write16(VBE_DISPI_IOPORT_INDEX, index);
  return io_read16(VBE_DISPI_IOPORT_DATA);
}

void qemu_clear_screen(vga_device_t* vga) {}

void qemu_flip_screen(vga_device_t* vga, u32 index) {
  vga->framebuffer_index=index;
  qemu_write_reg(VBE_DISPI_INDEX_Y_OFFSET, index*vga->height);
}

void qemu_init_mode(pci_device_t* pdev, vga_device_t* vga, int mode) {
  if (mode == VGA_MODE_80x25) {
    vga->width = 80;
    vga->height = 25;
  } else if (mode == VGA_MODE_320x200x256) {
    vga->width = 320;
    vga->height = 256;
  } else if (mode == VGA_MODE_1024x768x32) {
    vga->width = 1024;
    vga->height = 768;
    vga->bpp = 32;
  }
  vga->mode = mode;
  vga->write = NULL;
  u32 res_io = pci_dev_read32(pdev, PCI_BASE_ADDR2) & 0xFFFFFFF0;
  u16 id = qemu_read_reg(VBE_DISPI_INDEX_ID);
  if ((id & 0xfff0) != VBE_DISPI_ID0) {
    kprintf("qemu get error\n");
  }
  qemu_write_reg(VBE_DISPI_INDEX_ENABLE, 0);
  qemu_write_reg(VBE_DISPI_INDEX_BANK, 0);
  qemu_write_reg(VBE_DISPI_INDEX_BPP, vga->bpp);
  qemu_write_reg(VBE_DISPI_INDEX_XRES, vga->width);
  qemu_write_reg(VBE_DISPI_INDEX_YRES, vga->height);
  qemu_write_reg(VBE_DISPI_INDEX_VIRT_WIDTH, vga->width);
  qemu_write_reg(VBE_DISPI_INDEX_VIRT_HEIGHT, vga->height);
  qemu_write_reg(VBE_DISPI_INDEX_X_OFFSET, 0);
  qemu_write_reg(VBE_DISPI_INDEX_Y_OFFSET, 0);
  qemu_write_reg(VBE_DISPI_INDEX_ENABLE,
                 VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
  //qemu_write_reg(0, 0x20);
  //qemu_write_reg(VBE_DISPI_INDEX_ID,VBE_DISPI_ID5);

  qemu_clear_screen(vga);
}

int qemu_init_device(device_t* dev,u32 vendor_id,u32 device_id) {
  pci_device_t* pdev = pci_find_vendor_device(vendor_id, device_id);
  if (pdev == NULL) {
    kprintf("can not find pci qemu device\n");
    return -1;
  }
  u32 bar0 = pci_dev_read32(pdev, PCI_BASE_ADDR0) & 0xFFFFFFF0;

  // kprintf("bar0:%x ", bar0);
  vga_device_t* vga = kmalloc(sizeof(vga_device_t));
  dev->data = vga;
  u32 addr = bar0;

  qemu_init_mode(pdev, vga, VGA_MODE_1024x768x32);

  u32 size = 1024 * 768 * 32;
  vga->framebuffer_count = 2;
  vga->framebuffer_index = 0;
  vga->framebuffer_length = size;
  vga->frambuffer = bar0;
  vga->flip_buffer = qemu_flip_screen;
  // qemu_read_reg(VBE_DISPI_INDEX_VIDEO_MEMORY_64K)
  for (int i = 0; i < size* vga->framebuffer_count / PAGE_SIZE; i++) {
    map_page(addr, addr, PAGE_P | PAGE_USU | PAGE_RWW);
    addr += PAGE_SIZE;
  }
  

  // #ifdef DOUBLE_BUFFER
  //   vga->frambuffer = bar0 + size * 2;
  //   u32 phy = kmalloc_alignment(size, PAGE_SIZE);
  //   addr = vga->frambuffer;
  //   for (int i = 0; i < size / PAGE_SIZE; i++) {
  //     map_page(addr, phy, PAGE_P | PAGE_USU | PAGE_RWW);
  //     kprintf("qemu map %x %x\n", addr, phy);
  //     addr += PAGE_SIZE;
  //     phy += PAGE_SIZE;
  //   }
  // #else
  //   vga->frambuffer = bar0;
  // #endif
  return 1;
}

int qemu_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "qemu";
  dev->read = vga_read;
  dev->write = vga_write;
  dev->ioctl = vga_ioctl;
  dev->id = DEVICE_VGA_QEMU;
  dev->type = DEVICE_TYPE_VGA;
  device_add(dev);

  i32 ret=qemu_init_device(dev,QEMU_VENDOR_ID,QEMU_DEVICE_ID);
  if(ret<0){
    ret=qemu_init_device(dev,VBOX_VENDOR_ID,VBOX_DEVICE_ID);
  }

  return ret;
}

void qemu_exit(void) { kprintf("vga exit\n"); }

module_t qemu_module = {.name = "vga", .init = qemu_init, .exit = qemu_exit};
