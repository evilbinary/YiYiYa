#include "libs/include/types.h"

void platform_init() {
  // io_add_write_channel(uart_send);
  //  gic_set_base();
  gic_init();
}

void platform_end() {}

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