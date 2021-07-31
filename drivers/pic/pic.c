/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "pic.h"

#include "kernel/kernel.h"

void pic_eof(u8 irq) {
  if (irq >= 0x28) {
    io_write8(PIC2_COMMAND, PIC_EOI);
  }
  io_write8(PIC1_COMMAND, PIC_EOI);
}


void pic_enable(u8 irq) {
  u16 port;
  u8 value;
  irq -= 0x20;
  if (irq < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    irq -= 8;
  }
  value = io_read8(port) & ~(1 << irq);
  io_write8(port, value);
}

void pic_disable(u8 irq) {
  u16 port;
  u8 value;
  irq -= 0x20;
  if (irq < 8) {
    port = PIC1_DATA;
  } else {
    port = PIC2_DATA;
    irq -= 8;
  }
  value = io_read8(port) | (1 << irq);
  io_write8(port, value);
}

