/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "device.h"

device_t* devices[MAX_DEVICE];
u32 device_number = 0;

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