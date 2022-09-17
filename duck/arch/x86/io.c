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
  __asm__ volatile("outb %b0, %w1" : : "a"(data), "Nd"(port));
}

u8 io_read8(uint port) {
  u8 data;
  __asm__ volatile("inb %w1, %b0" : "=a"(data) : "Nd"(port));
  return data;
}

void io_write16(uint port, u16 data) {
  __asm__ volatile("outw %w0, %w1" : : "a"(data), "Nd"(port));
}

u16 io_read16(uint port) {
  u16 data;
  __asm__ volatile("inw %w1, %w0" : "=a"(data) : "Nd"(port));
  return data;
}

void io_write32(uint port, u32 data) {
  __asm__ volatile("outl %0, %w1" : : "a"(data), "Nd"(port));
}

u32 io_read32(uint port) {
  u32 data;
  __asm__ volatile("inl %w1, %0" : "=a"(data) : "Nd"(port));
  return data;
}


void mem_write8(uint port, u8 data) {
  *(u8 *) port = data;
}

u8 mem_read8(uint port) {
  u8 data;
  data=*(u8 *) port;
  return data;
}

void mem_write16(uint port, u16 data) {
  *(u16 *) port = data;
}

u16 mem_read16(uint port) {
  u16 data;
  data=*(u16 *) port;
  return data;
}

void mem_write32(uint port, u32 data) {
  *(u32 *) port = data;
}

u32 mem_read32(uint port) {
  u32 data;
  data=*(u32 *) port;
  return data;
}

#endif