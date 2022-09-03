#include "init.h"

#include "gpio.h"
#include "v3s-reg-ccu.h"

static void io_write32(uint port, u32 data) { *(u32 *)port = data; }

static u32 io_read32(uint port) {
  u32 data;
  data = *(u32 *)port;
  return data;
}

void uart_send_ch(unsigned int c) {
  unsigned int addr = 0x01c28000;  // UART0
  while ((io_read32(addr + 0x14) & (0x1 << 6)) == 0)
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

unsigned int uart_receive() {
  unsigned int c = 0;
  unsigned int addr = 0x01c28000;  // UART0
  while ((io_read32(addr + 0x14) & (0x1 << 0)) == 0) {
  }
  c = io_read32(addr + 0x00);
  return c;
}

extern int timer_count;

void timer_init(int hz) {
  kprintf("timer init %d\n", hz);
  timer_count = 0;
  ccnt_enable(0);
  ccnt_reset();
  timer_init2(hz);
  gic_init();

  // timer_watch();
  // gic_watch();
  // gic_poll();
}

void timer_end() {
  // kprintf("timer end %d\n",timer_count);
  gic_handler();
}

static inline void sdelay(int loops) {
  __asm__ __volatile__(
      "1:\n"
      "subs %0, %1, #1\n"
      "bne 1b"
      : "=r"(loops)
      : "0"(loops));
}

static void cpu_clock_set_pll_cpu(u32 clk) {
  int p = 0;
  int k = 1;
  int m = 1;
  int n = 32;
  u32 val;
  if (clk > 1152000000) {
    k = 2;
  } else if (clk > 768000000) {
    k = 3;
    m = 2;
  }
  k = 3;
  m = 2;
  n = 28;
  /* Switch to 24MHz clock while changing cpu pll */
  val = (2 << 0) | (1 << 8) | (1 << 16);
  io_write32(V3S_CCU_BASE + CCU_CPU_AXI_CFG, val);

  /* cpu pll rate = ((24000000 * n * k) >> p) / m */
  val = (0x1 << 31);
  val |= ((p & 0x3) << 16);
  // val |= ((((clk / (24000000 * k / m)) - 1) & 0x1f) << 8);
  val |= ((n - 1) & 0x1f) << 8;
  val |= (((k - 1) & 0x3) << 4);
  val |= (((m - 1) & 0x3) << 0);
  io_write32(V3S_CCU_BASE + CCU_PLL_CPU_CTRL, val);
  sdelay(200);

  /* Switch clock source */
  val = (2 << 0) | (1 << 8) | (2 << 16);
  io_write32(V3S_CCU_BASE + CCU_CPU_AXI_CFG, val);
}

void cpu_clock_init(void) {
  cpu_clock_set_pll_cpu(1008000000);

  /* pll video - 396MHZ */
  io_write32(V3S_CCU_BASE + CCU_PLL_VIDEO_CTRL, 0x91004107);

  /* pll periph0 - 600MHZ */
  io_write32(V3S_CCU_BASE + CCU_PLL_PERIPH0_CTRL, 0x90041811);
  while (!(io_read32(V3S_CCU_BASE + CCU_PLL_PERIPH0_CTRL) & (1 << 28)))
    ;

  /* ahb1 = pll periph0 / 3, apb1 = ahb1 / 2 */
  io_write32(V3S_CCU_BASE + CCU_AHB_APB0_CFG, 0x00003180);

  /* mbus  = pll periph0 / 4 */
  io_write32(V3S_CCU_BASE + CCU_MBUS_CLK, 0x81000003);

  /* Set APB2 to OSC24M/1 (24MHz). */
  io_write32(V3S_CCU_BASE + CCU_AHB2_CFG, 1 << 24 | 0 << 16 | 0);

  // Enable TWI0 clock gating
  u32 gate_reg = io_read32(V3S_CCU_BASE + CCU_BUS_CLK_GATE3);
  io_write32(V3S_CCU_BASE + CCU_BUS_CLK_GATE3, gate_reg | 1 << 0);
}

u32 cpu_get_rate(u32 prate) {
  u32 r, n, k, m, p;
  u32 rate = 0;
  r = io_read32(V3S_CCU_BASE + CCU_PLL_CPU_CTRL);
  n = ((r >> 8) & 0x1f) + 1;
  k = ((r >> 4) & 0x3) + 1;
  m = ((r >> 0) & 0x3) + 1;
  p = (r >> 16) & 0x3;

  // 90001b21
  // 90001521
  kprintf("V3S_CCU_BASE %x n:%d k:%d m:%d p:%d\n", r, n, k, m, p);
  rate = (((prate * n * k) >> p) / m);
  return rate;
}

void platform_init() {
  io_add_write_channel(uart_send);
  cpu_clock_init();
  // sys_dram_init();
}

void platform_end() {}


void ipi_enable(int cpu) {

}

void lcpu_send_start(u32 cpu, u32 entry) {

}

void ipi_send(int cpu, int vec) {

}

void ipi_clear(int cpu) {

}