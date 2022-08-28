#include "arch/arch.h"
#include "gpio.h"
#include "libs/include/types.h"

static void io_write32(uint port, u32 data) { *(u32 *)port = data; }

static u32 io_read32(uint port) {
  u32 data;
  data = *(u32 *)port;
  return data;
}

static u32 cntfrq = 0;

void uart_send(unsigned int c) {
  while (io_read32(UART0_FR) & 0x20) {
  }
  io_write32(UART0_DR, c);
}

unsigned int uart_receive() {
  unsigned int c;
  while (io_read32(UART0_FR) & 0x10) {
  }
  c = io_read32(UART0_DR);
  return c;
}

u32 read_core0timer_pending(void) {
  u32 tmp;
  tmp = io_read32(CORE0_IRQ_SOURCE);
  return tmp;
}

void timer_init(int hz) {
  kprintf("timer init\n");
  cntfrq = read_cntfrq();
  cntfrq = cntfrq / hz;
  kprintf("cntfrq %d\n", cntfrq);
  write_cntv_tval(cntfrq);

  u32 val = read_cntv_tval();
  kprintf("val %d\n", val);
  io_write32(CORE0_TIMER_IRQCNTL, 0x08);
  enable_cntv(1);
}

void timer_end() {
  if (read_core0timer_pending() & 0x08) {
    write_cntv_tval(cntfrq);
    // kprintf("cntfrq:%x cnt val:%x\n", read_cntvct(),read_cntv_tval());
    // cpu_sti();
  }
}

void platform_init() {
  io_add_write_channel(uart_send);
}

void platform_end() {}

void ipi_enable(int cpu) {
  if (cpu < 0 || cpu > 4) return;
  u32 addr = CORE0_MBOX_IRQCNTL + cpu * 4;
  io_write32(addr, 1);
}

void test_smp_entry() {
  *((u32*) 0x8888 )=1+ *((u32*) 0x8888 );
  for (;;) cpu_halt();
}

void lcpu_send_start(u32 cpu, u32 entry){
  if (cpu < 0 || cpu > 4) return;

  u32 mailbox = 3;
  u32 addr = CORE0_MBOX0_SET + cpu * 0x10 + 4 * mailbox;
  kprintf("set addr %x\n", addr);
  io_write32(addr, entry);
  
}

void ipi_send(int cpu, int vec) {
  if (cpu < 0 || cpu > 4) return;
  //todo
}

void ipi_clear(int cpu) {
  if (cpu < 0 || cpu > 4) return;
  // u32 mailbox = 3;
  // int addr = CORE0_MBOX0_RDCLR + cpu * 0x10 + 4 * mailbox;
  // kprintf("clear addr %x\n", addr);
  // u32 val = io_read32(addr);
  // val = -1;
  // io_write32(addr, val);
}