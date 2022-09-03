#include "arch/arch.h"
#include "gpio.h"
#include "libs/include/types.h"

static void io_write32(uint port, u32 data) { *(u32 *)port = data; }

static u32 io_read32(uint port) {
  u32 data;
  data = *(u32 *)port;
  return data;
}

static u32 cntfrq[MAX_CPU] = {
    0,
};

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

u32 read_core_timer_pending(int cpu) {
  u32 tmp;
  tmp = io_read32(CORE0_IRQ_SOURCE + 4 * cpu);
  return tmp;
}

void timer_init(int hz) {
  kprintf("timer init\n");
  int cpu = cpu_get_id();

  cntfrq[cpu] = read_cntfrq();
  cntfrq[cpu] = cntfrq[cpu] / hz;
  kprintf("cntfrq %d\n", cntfrq[cpu]);
  write_cntv_tval(cntfrq[cpu]);

  u32 val = read_cntv_tval();
  kprintf("val %d\n", val);
  io_write32(CORE0_TIMER_IRQCNTL + 0x4 * cpu, 0x08);
  enable_cntv(1);
}

void timer_end() {
  if (read_core_timer_pending(0) & 0x08) {
    write_cntv_tval(cntfrq[0]);
    // kprintf("cntfrq:%x cnt val:%x\n", read_cntvct(),read_cntv_tval());
    // cpu_sti();
  }
  if (read_core_timer_pending(1) & 0x08) {
    write_cntv_tval(cntfrq[1]);
    // kprintf("cpu1 cntfrq:%x cnt val:%x\n", read_cntvct(), read_cntv_tval());
  }
  if (read_core_timer_pending(2) & 0x08) {
    write_cntv_tval(cntfrq[2]);
    // kprintf("cpu2 cntfrq:%x cnt val:%x\n", read_cntvct(), read_cntv_tval());
  }
  if (read_core_timer_pending(3) & 0x08) {
    write_cntv_tval(cntfrq[3]);
    // kprintf("cpu3 cntfrq:%x cnt val:%x\n", read_cntvct(), read_cntv_tval());
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
  *((u32 *)0x8888) = 1 + *((u32 *)0x8888);
  for (;;) cpu_halt();
}

void lcpu_send_start(u32 cpu, u32 entry) {
  if (cpu < 0 || cpu > 4) return;
  u32 mailbox = 3;
  u32 addr = CORE0_MBOX0_SET + cpu * 0x10 + 4 * mailbox;
  io_write32(addr, entry);
}

void ipi_send(int cpu, int vec) {
  if (cpu < 0 || cpu > 4) return;
  u32 addr = CORE0_MBOX0_SET + cpu * 0x10 + 0x80;
  io_write32(addr, 1 << vec);
  dsb();
}

void ipi_clear(int cpu) {
  if (cpu < 0 || cpu > 4) return;
  int addr = CORE0_MBOX0_RDCLR + cpu * 0x10 + 0xC0;
  kprintf("clear addr %x\n", addr);
  u32 val = io_read32(addr);
  val = -1;
  io_write32(addr, val);
}