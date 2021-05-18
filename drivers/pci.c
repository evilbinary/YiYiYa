/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "pci.h"

pci_device_t* pci_device[MAX_PCI_NUMBER];
u32 pci_number = 0;

static size_t read(fd_t* fd, void* buf, size_t len) {
  u32 ret = 0;
  return ret;
}

u32 pci_dev_read32(pci_device_t* pdev, u16 offset) {
  return pci_read32(pdev->bus, pdev->slot, pdev->function, offset);
}

u32 pci_read32(u16 bus, u16 slot, u16 func, u16 offset) {
  u32 address;
  u32 ret = 0;
  address = (u32)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) |
                  ((u32)PCI_BASE_ADDR));
  io_write32(PCI_CONFIG_ADDRESS, address);
  ret = io_read32(PCI_CONFIG_DATA);
  return ret;
}

u16 pci_read16(u16 bus, u16 slot, u16 func, u16 offset) {
  u32 address;
  u16 ret = 0;
  address = (u32)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) |
                  ((u32)PCI_BASE_ADDR));
  io_write32(PCI_CONFIG_ADDRESS, address);
  ret = (io_read32(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffffffff;
  return ret;
}

void pci_add_device(pci_device_t* pdev) {
  if (pci_number < MAX_PCI_NUMBER) {
    pci_device[pci_number++] = pdev;
  } else {
    kprintf("max pci limit %d\n",pci_number);
  }
}

pci_device_t* pci_get_device(u16 bus, u16 slot, u16 function) {
  u16 vendor = pci_read16(bus, slot, function, 0);
  u16 device = pci_read16(bus, slot, function, 2);
  u16 class = pci_read16(bus, slot, function, 0xa);
  u16 header_type = pci_read16(bus, slot, function, 0xd);
  pci_device_t* pdev = (pci_device_t*)kmalloc(sizeof(pci_device_t));
  pdev->vendor = vendor;
  pdev->device = device;
  pdev->function = function;
  pdev->class = class;
  pdev->bus = bus;
  pdev->slot = slot;
  pdev->function = function;
  // kprintf("bus:%d  slot:%d fun:%d ", bus, slot, function);
  // kprintf("vendorId:%x  deviceid:%x classId:%x", vendor, device, class);
  // kprintf("\n");
  return pdev;
}

void pci_probe() {
  for (u32 bus = 0; bus < 256; bus++) {
    for (u32 slot = 0; slot < 32; slot++) {
      for (u32 function = 0; function < 8; function++) {
        u16 vendor = pci_read16(bus, slot, function, 0);
        if (vendor == 0xffff) continue;
        pci_device_t* pdev = pci_get_device(bus, slot, function);
        pci_add_device(pdev);
      }
    }
  }
}

pci_device_t* pci_find_vendor_device(u32 vendor, u32 device) {
  for (int i = 0; i < pci_number; i++) {
    if (pci_device[i]->vendor == vendor && pci_device[i]->device == device) {
      return pci_device[i];
    }
  }
  kprintf("not fond pci device vendor id: 0x%x device id:0x%x\n", vendor,device);
  return NULL;
}

pci_device_t* pci_find_class(u32 class) {
  for (int i = 0; i < pci_number; i++) {
    if (pci_device[i]->class == class) {
      return pci_device[i];
    }
  }
  kprintf("not fond pci device class id:0x%x\n", class);
  return NULL;
}

int pci_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "pci";
  dev->read = read;
  dev->id = DEVICE_PCI;
  pci_number=0;
  device_add(dev);

  pci_probe();

  return 0;
}

void pci_exit(void) { kprintf("pci exit\n"); }

module_t pci_module = {.name = "pci", .init = pci_init, .exit = pci_exit};
