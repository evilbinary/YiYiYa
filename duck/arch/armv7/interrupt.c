/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "../interrupt.h"
#include "gpio.h"
#include "context.h"
#include "cpu.h"

extern boot_info_t* boot_info;

interrupt_handler_t* interrutp_handlers[IDT_NUMBER];
u32 idt[IDT_NUMBER] __attribute__((aligned(32)));

void interrupt_init() {
  kprintf("interrupt init\n");
  boot_info->idt_base = idt;
  boot_info->idt_number = IDT_NUMBER;
  for (int i = 0; i < boot_info->idt_number; i++) {
    interrutp_set(i);
  }
  u32 val = idt;

  SCB->VTOR=val;
  
}

void interrutp_regist(u32 vec, interrupt_handler_t handler) {
  interrutp_handlers[vec] = handler;
  interrutp_set(vec);
}

void interrutp_set(int i) {
  u32 base = (u32)interrutp_handlers[i];
    idt[i] = base;
}
