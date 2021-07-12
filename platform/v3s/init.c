#include "init.h"
#include "gpio.h"

static void io_write32(uint port, u32 data) { *(u32 *)port = data; }

static u32 io_read32(uint port) {
  u32 data;
  data = *(u32 *)port;
  return data;
}

void uart_send_ch(unsigned int c) {
  unsigned int addr = 0x01c28000;
  while ((io_read32(addr + 0x7c) & (0x1 << 1)) == 0)
    ;
  io_write32(addr + 0x00, c);
}

void uart_send(unsigned int c) {
  if (c == '\n') {
    uart_send_ch(c);
    c = '\r';
  }
  uart_send_ch(c);
}
extern int timer_count;

void timer_init(int hz) {
  kprintf("timer init\n");
  // ccnt_enable(0);
  // ccnt_reset();
  gic_init();
  timer_init2(hz);
  timer_count = 0;
  asm("cpsid i" : : : "memory", "cc");
  // asm("mrs	r0, cpsr\n"
  //     "bic	r0, r0, #0x80\n"
  //     "msr	cpsr, r0\n");
  // timer_watch();
  gic_watch();
}

void timer_end() {
  kprintf("timer end\n");
  timer_handler();
}

void platform_init() {
  // sys_clock_init();
  // sys_dram_init();
}