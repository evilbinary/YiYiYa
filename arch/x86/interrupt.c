/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/

#include "../interrupt.h"
#include "platform/platform.h"

extern boot_info_t* boot_info;


idt_entry_t idt[IDT_NUMBER];
interrupt_handler_t* interrutp_handlers[IDT_NUMBER];

void interrupt_init(int cpu) {
  boot_info->idt_base = idt;
  boot_info->idt_number = IDT_NUMBER;
  for (int i = 0; i < boot_info->idt_number; i++) {
    u64* p = &idt[i];
    *p = 0;
  }
  for (int i = 0; i < boot_info->idt_number; i++) {
    interrutp_set(i);
  }

  idt_ptr_t idt_ptr;
  idt_ptr.limit = boot_info->idt_number;
  idt_ptr.base = (u32)boot_info->idt_base;
  __asm__("lidt	%0\n\t" ::"m"(idt_ptr));

  if(cpu==0){
    // pic 8259
    // icw1 init state
    io_write8(0x20, 0x11);
    io_write8(0xa0, 0x11);

    // icw2 map irqs  0-7 =>0x20-0x27 8-f => 0x28-0x2f
    io_write8(0x21, 0x20);
    io_write8(0xa1, 0x28);

    // icw3  IRQ2 is slave (0000 0100)
    io_write8(0x21, 0x04);
    io_write8(0xa1, 0x02);  //(0000 0010)

    // icw4 8086mode
    io_write8(0x21, 0x01);
    io_write8(0xa1, 0x01);

    // // disable pic
    // io_write8(0x21, 0xff);
    // io_write8(0xa1, 0xff);
  }
}

void timer_init(int hz) {
  unsigned int divisor = 1193180 / hz;
  io_write8(0x43,0x36);
  io_write8(0x40,divisor & 0xff);
  io_write8(0x40,divisor >> 8);
  io_write8(0x21,io_read8(0x21) & 0xfe);
}

void timer_end(){
      io_write8(0x20, 0x20);
}

static inline u16 pic_get_irr() {
  u16 val = 0;
  io_write8(0x20, 0x0e);
  val = (io_read8(0xa0) << 8);

  io_write8(0xa0, 0x0e);
  val = val | io_read8(0x20);
  return (io_read8(0xa0) << 8) | io_read8(0x20);
}

static inline u16 pic_get_isr() {
  io_write8(0x20, 0x0f);  // 0x0b isr data
  io_write8(0xa0, 0x0f);
  return (io_read8(0xa0) << 8) | io_read8(0x20);
}

void interrutp_regist(u32 vec, interrupt_handler_t handler) {
  interrutp_handlers[vec] = handler;
  interrutp_set(vec);
}

void interrutp_set(int i) {
  idt[i].selector = GDT_ENTRY_32BIT_CS * GDT_SIZE;
  u32 base = (u32)interrutp_handlers[i];
  idt[i].basel = base & 0xFFFF;
  idt[i].baseh = (base >> 16) & 0xFFFF;
  idt[i].attrs = 0x8e | 0x60;
  idt[i].zero = 0;
}

