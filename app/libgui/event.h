/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef EVENT_H
#define EVENT_H

#include "types.h"

enum {
  KEY_DOWN = 1,
  KEY_UP,
  MOUSE_MOTION,
  MOUSE_BUTTON_DOWN,
  MOUSE_BUTTON_UP,
};

#define BUTTON_MASK 0x7

#define BUTTON_LEFT 1
#define BUTTON_RIGHT 2
#define BUTTON_MIDDLE 4

typedef struct mouse_data {
  u32 sate;
  i32 x;
  i32 y;
} mouse_data_t;

typedef struct event {
  u32 type;
  u32 key;
  mouse_data_t mouse;
} event_t;

typedef struct event_info {
  mouse_data_t mouse;
  int mouse_fd;
  int is_init;
  int input_fd;
  u32 scan_code;
}event_info_t;

int event_init();
int event_poll(event_t* event);

#endif