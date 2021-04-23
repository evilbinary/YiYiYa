/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef IO_H
#define IO_H

#include "../io.h"

#include "libs/include/types.h"

void io_write8(uint port, u8 data) {
  *(u8 *) port = data;
}

u8 io_read8(uint port) {
  u8 data;
  data=*(u8 *) port;
  return data;
}

void io_write16(uint port, u16 data) {
  *(u16 *) port = data;
}

u16 io_read16(uint port) {
  u16 data;
  data=*(u16 *) port;
  return data;
}

void io_write32(uint port, u32 data) {
  *(u32 *) port = data;
}

u32 io_read32(uint port) {
  u32 data;
  data=*(u32 *) port;
  return data;
}

#endif