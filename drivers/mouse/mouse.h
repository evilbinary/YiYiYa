/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef MOUSE_H
#define MOUSE_H

#include "kernel/kernel.h"

#define MOUSE_DATA 0x60
#define MOUSE_STATUS 0x64
#define MOUSE_COMMAND 0x64

#define EVENT_NUMBER 64

typedef struct mouse_event{
  u32 sate;
  i32 x;
  i32 y;
}mouse_event_t;

typedef struct mouse_device {
  queue_pool_t* events;
  void* data;
} mouse_device_t;

#endif