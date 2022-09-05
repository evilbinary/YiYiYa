#include "libs/include/types.h"
#include "libs/include/archcommon/gic2.h"
#include "gpio.h"

static void io_write32(uint port, u32 data) { *(u32 *)port = data; }

static u32 io_read32(uint port) {
  u32 data;
  data = *(u32 *)port;
  return data;
}



void uart_send_ch(unsigned int c) {
  while (((io_read32(MMIO_BASE + 0x60014)) & 0x20) == 0);	
			io_write32(MMIO_BASE + 0x60000,c);
}

void uart_send(unsigned int c) {
  if (c == '\n') {
    uart_send_ch(c);
    c = '\r';
  }
  uart_send_ch(c);
}

void platform_init() {
  io_add_write_channel(uart_send);
  //  gic_set_base();
  gic_init();
}

void platform_end() {}

void timer_init(int hz) {
  kprintf("timer init %d\n", hz);

}

void timer_end(){

}

void lcpu_send_start(u32 cpu, u32 entry) {

}

void ipi_enable(int cpu) {
  if (cpu < 0 || cpu > 4) return;
  //默认0 irq
  gic_enable(cpu, 0);
}

void ipi_send(int cpu, int vec) {
  if (cpu < 0 || cpu > 4) return;
  gic_send_sgi(cpu,vec);
}

void ipi_clear(int cpu) {}


