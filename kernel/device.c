/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "device.h"

device_t* devices[MAX_DEVICE];
u32 device_number = 0;

device_t* device_create(u8* name, u32 id, u32 type) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = name;
  dev->read = NULL;
  dev->write = NULL;
  dev->ioctl = NULL;
  dev->id = id;
  dev->type = type;
  return dev;
}

void device_add(device_t* device) {
  if (device_number > MAX_DEVICE) {
    kprintf("add device full %d\n", device_number);
    return;
  }
  devices[device_number++] = device;
}

void device_remove() {}

device_t* device_find(u32 id) {
  for (int i = 0; i < device_number; i++) {
    if (devices[i]->id == id) {
      return devices[i];
    }
  }
  kprintf("not fond device %d\n", id);
  return NULL;
}

device_t* device_find_name(char* name) {
  for (int i = 0; i < device_number; i++) {
    if (kstrcmp(devices[i]->name, name) == 0) {
      return devices[i];
    }
  }
  kprintf("not fond device name %s\n", name);
  return NULL;
}