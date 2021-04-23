/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "../interrupt.h"

#include "context.h"
#include "cpu.h"

#define CORE0_TIMER_IRQCNTL 0x40000040
#define CORE0_IRQ_SOURCE 0x40000060

extern boot_info_t* boot_info;

interrupt_handler_t* interrutp_handlers[IDT_NUMBER];
u32 idt[IDT_NUMBER * 2] __attribute__((aligned(32)));

void interrupt_init() {
  kprintf("interrupt init\n");
  boot_info->idt_base = idt;
  boot_info->idt_number = IDT_NUMBER;
  for (int i = 0; i < boot_info->idt_number; i++) {
    interrutp_set(i);
  }
  u32 val = idt;
  asm volatile("mcr p15, 0, %0, c12, c0, 0" ::"r"(val));
}

u32 read_cntv_tval(void) {
  u32 val;
  asm volatile("mrc p15, 0, %0, c14, c3, 0" : "=r"(val));
  return val;
}

void write_cntv_tval(u32 val) {
  asm volatile("mcr p15, 0, %0, c14, c3, 0" ::"r"(val));
  return;
}

u32 read_cntfrq(void) {
  u32 val;
  asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r"(val));
  return val;
}

void enable_cntv(void) {
  u32 cntv_ctl;
  cntv_ctl = 1;
  asm volatile("mcr p15, 0, %0, c14, c3, 1" ::"r"(cntv_ctl));  // write CNTV_CTL
}

void disable_cntv(void) {
  u32 cntv_ctl;
  cntv_ctl = 0;
  asm volatile("mcr p15, 0, %0, c14, c3, 1" ::"r"(cntv_ctl));  // write CNTV_CTL
}

u32 read_core0timer_pending(void) {
  u32 tmp;
  tmp = io_read32(CORE0_IRQ_SOURCE);
  return tmp;
}
static u32 cntfrq = 0;

void timer_init(int hz) {
  kprintf("timer init\n");
  cntfrq = read_cntfrq();
  kprintf("cntfrq %d\n", cntfrq);
  write_cntv_tval(cntfrq);

  u32 val = read_cntv_tval();
  kprintf("val %d\n", val);
  io_write32(CORE0_TIMER_IRQCNTL, 0x08);
  enable_cntv();
}

uint64_t read_cntvct(void) {
  uint64_t val;
  asm volatile("mrrc p15, 1, %Q0, %R0, c14" : "=r"(val));
  return (val);
}

void timer_end() {
  if (read_core0timer_pending() & 0x08) {
    write_cntv_tval(cntfrq);
    kprintf("cntfrq:%x cnt val:%x\n", read_cntvct(),read_cntv_tval());
    cpu_sti();
  }
}

void interrutp_regist(u32 vec, interrupt_handler_t handler) {
  interrutp_handlers[vec] = handler;
  interrutp_set(vec);
}

void interrutp_set(int i) {
  idt[i] = 0xe59ff000 +
           (IDT_NUMBER / 2 - 1) * 4;  // ldr	pc, [pc, #24] 0x24=36=4*8=32+4
  u32 base = (u32)interrutp_handlers[i];
  idt[i + IDT_NUMBER / 2 + 1] = base;
}
