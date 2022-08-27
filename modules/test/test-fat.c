#include "../ahci/ahci.h"
#include "../fat/fat.h"
#include "kernel/kernel.h"

#define PRINT_WIDTH 24
#define READ_BUFFER 24 * 20

void test_fat_read() {
  device_t* dev = device_find(DEVICE_SATA);
  if (dev == NULL) {
    kprintf("test ahci port failed\n");
  }
  ahci_device_t* ahci_dev = dev->data;

  vnode_t* node = kmalloc(sizeof(vnode_t));
  node->device = dev;

  int offset = 0xFFFF000;
  int count = 2 * 512;
  char buf[1024] = {0};

  int ret = fat_read_bytes(node, offset, count, buf);
  if (ret < 0) {
    kprintf("test fat error %d\n", ret);
  }
  for (int i = 0; i < 1024; i++) {
    if (buf[i] != (i % 128)) {
      kprintf("test error fat at pos %d\n", i);
      break;
    }
  }
}

void test_fat_read_small() {
  device_t* dev = device_find(DEVICE_SATA);
  if (dev == NULL) {
    kprintf("test ahci port failed\n");
  }
  ahci_device_t* ahci_dev = dev->data;

  vnode_t* node = kmalloc(sizeof(vnode_t));
  node->device = dev;

  int offset = 0xFFFF000;
  char buf[READ_BUFFER] = {0};

  int ret = fat_read_bytes(node, offset, READ_BUFFER, buf);
  if (ret < 0) {
    kprintf("test fat small error %d\n", ret);
  }
  for (int i = 0; i < READ_BUFFER; i++) {
    if (buf[i] != (i % 128)) {
      kprintf("test error fat small at pos %d\n", i);
      break;
    }
  }
}

void test_fat_read_file() {
  vnode_t* node = vfs_find(NULL, "/dev/sda");
  if (node == NULL) {
    kprintf("test read file failed\n");
  }
  vnode_t* duck = node->op->find(node, "duck.png");
  if (node == NULL) {
    kprintf("test read file failed duck not found\n");
  }
  int offset = 0x2a0;
  char* buffer = kmalloc(READ_BUFFER);
  int ret = duck->op->read(duck, offset, READ_BUFFER, buffer);
  if (ret <= 0) {
    kprintf("read <=0\n");
  }
  if (buffer[0] != 0x19) {
    kprintf("test read file error 1\n");
  }
  if (buffer[1] != 0x29) {
    kprintf("test read file error 2\n");
  }
  if (buffer[2] != 0x4c) {
    kprintf("test read file error 3\n");
  }
  if (buffer[384] != 0xff) {
    kprintf("test read file error 4\n");
  }
  if (buffer[385] != 0x5f) {
    kprintf("test read file error 5\n");
  }
}

void test_fat() {
  test_fat_read();
  test_fat_read_small();
  test_fat_read_file();
}
