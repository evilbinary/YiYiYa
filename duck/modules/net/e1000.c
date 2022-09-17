/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "kernel/kernel.h"
#include "pci.h"
#include "e1000.h"

size_t e1000_read(device_t* dev, void* buf, size_t len) {
  u32 ret = 0;
  e1000_t* e1000 = dev->data;
  if (e1000 == NULL) {
    kprintf("not found e1000\n");
    return ret;
  }
  kmemcpy(e1000->rx_buf, buf, len);
  // DATA(e1000->mmio,RDT) = 0;
  ret = len;

  return ret;
}

size_t e1000_write(device_t* dev, const void* buf, size_t len) {
  u32 ret = 0;
  e1000_t* e1000 = dev->data;
  if (e1000 == NULL) {
    kprintf("not found e1000\n");
    return ret;
  }
  if (len > TX_BUF_SIZE) {
    kprintf("len > e1000 buffer size\n");
    return -1;
  }
  kmemcpy(e1000->tx_buf, buf, len);

  // DATA(e1000->mmio + TDT) = DATA(e1000->mmio + TDT) + 1;
  ret = len;

  return ret;
}

int net_init_device(device_t* dev) {
  pci_device_t* pdev = pci_find_vendor_device(E1000_VENDOR_ID, E1000_DEVICE_ID);
  if (pdev == NULL) {
    kprintf("can not find pci e1000 device\n");
    return -1;
  }
  dev->write = e1000_write;
  dev->read = e1000_read;

  /*
      BAR0: 32 bit memory at 0xfebc0000 [0xfebdffff].
      BAR1: I/O at 0xc000 [0xc03f].
      BAR6: 32 bit memory at 0xffffffffffffffff [0x0003fffe].
  */
  u32 bar0 = pci_dev_read32(pdev, PCI_BASE_ADDR0) & 0xFFFFFFF0;
  u32 bar1 = pci_dev_read32(pdev, PCI_BASE_ADDR1) & 0xFFFFFFF0;

  u32 addr = bar0;
  for (int i = 0; i < 4; i++) {
    map_page(addr, addr, PAGE_P | PAGE_USU | PAGE_RWW);
    addr += 0x1000;
  }
  e1000_t* e1000 = kmalloc(sizeof(e1000_t));
  dev->data = e1000;
  e1000->mmio = bar0;

  // init send
  DATA(e1000->mmio + TDBAH) = 0;
  DATA(e1000->mmio + TDBAL) = e1000->tx_buf;
  DATA(e1000->mmio + TDLEN) = 1;
  DATA(e1000->mmio + TDH) = 0;
  DATA(e1000->mmio + TDT) = 0;

  DATA(e1000->mmio + TCTL) = 1 | 8;  // Transmit Enable  1b = Pad short packets.

  // init recv
  DATA(e1000->mmio + RDBAH) = 0;
  DATA(e1000->mmio + RDBAL) = e1000->tx_buf;
  DATA(e1000->mmio + RDLEN) = 1;
  DATA(e1000->mmio + RDH) = 0;
  DATA(e1000->mmio + RDT) = 0;

  DATA(e1000->mmio + TCTL) = 2;  // Enable
}