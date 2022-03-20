#include "../ahci/ahci.h"

void test_ahci_port_read() {
  device_t* dev = device_find(DEVICE_SATA);
  if (dev == NULL) {
    kprintf("test ahci port failed\n");
  }
  ahci_device_t* ahci_dev = dev->data;
  sector_t sector;
  sector.startl = 0xFFFF000 / 512;
  sector.starth = 0;
  int count = 2;
  char buf[1024];
  char* p = buf;

  for (int i = 0; i < 1024; i++) {
    buf[i] = i % 128;
  }
  // start offset 0xFFFF000
  int ret = ahci_dev_port_write(ahci_dev, 0, sector, count, buf);
  if (ret < 0) {
    kprintf("test error at write %d\n", ret);
  }

  kmemset(buf, 0, 1024);
  ret = ahci_dev_port_read(ahci_dev, 0, sector, count, buf);
  if (ret < 0) {
    kprintf("test error at read %d\n", ret);
  }
  for (int i = 0; i < 1024; i++) {
    if (buf[i] != (i % 128)) {
      kprintf("test error ahci port at pos %d\n", i);
      break;
    }
  }
}

void test_ahci_read() {
  device_t* dev = device_find(DEVICE_SATA);
  if (dev == NULL) {
    kprintf("test ahci read failed\n");
  }
  ahci_device_t* ahci_dev = dev->data;
  int offset = 0xFFFF000;
  int count = 2 * 512;
  char buf[1024] = {0};
  dev->ioctl(dev, IOC_WRITE_OFFSET, offset);
  int ret = dev->read(dev, buf, count);
  if (ret < 0) {
    kprintf("test ahci read error at %d\n", ret);
  }
  for (int i = 0; i < 1024; i++) {
    if (buf[i] != (i % 128)) {
      kprintf("test ahci read error at pos %d\n", i);
      break;
    }
  }
}

void test_ahci() {
  test_ahci_port_read();
  test_ahci_read();
}