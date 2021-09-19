/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "../interrupt.h"

#include "context.h"
#include "cpu.h"
#define IRAM_ATTR _SECTION_ATTR_IMPL(".iram1", __COUNTER__)

extern boot_info_t* boot_info;

interrupt_handler_t* interrutp_handlers[IDT_NUMBER];
u64 idt[IDT_NUMBER]  __attribute__((aligned(1024)));

void interrupt_init() {
  kprintf("interrupt init %x\n",idt);
  u64* pidt=0x40000000;
  boot_info->idt_base = pidt;
  boot_info->idt_number = IDT_NUMBER;
  for (int i = 0; i < boot_info->idt_number; i++) {
    interrutp_set(i);
  }
  cpu_set_vector(pidt);
}

void interrutp_regist(u32 vec, interrupt_handler_t handler) {
  interrutp_handlers[vec] = handler;
  interrutp_set(vec);
}

void interrutp_set(int i) {
  u32 base = (u32)interrutp_handlers[i];
  u64* idt_base=boot_info->idt_base;
  idt_base[i] = base;
}
