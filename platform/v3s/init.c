#include "gpio.h"
#include "init.h"

static u32 cntfrq = 0;

static void io_write32(uint port, u32 data) {
  *(u32 *) port = data;
}

static u32 io_read32(uint port) {
  u32 data;
  data=*(u32 *) port;
  return data;
}


void uart_send_ch(unsigned int c){
  unsigned int addr = 0x01c28000;
	while((io_read32(addr + 0x7c) & (0x1 << 1)) == 0);
	io_write32(addr + 0x00, c);
}

void uart_send(unsigned int c) {
  if(c=='\n'){
    uart_send_ch(c);
    c='\r';
  }
  uart_send_ch(c);
}

u32 read_core0timer_pending(void) {
  u32 tmp;
  tmp = io_read32(CORE0_IRQ_SOURCE);
  return tmp;
}

void timer_init(int hz) {
  kprintf("timer init\n");
  cntfrq = read_cntfrq();
  cntfrq=cntfrq/20;
  kprintf("cntfrq %d\n", cntfrq);
  write_cntv_tval(cntfrq);

  u32 val = read_cntv_tval();
  kprintf("val %d\n", val);
  io_write32(CORE0_TIMER_IRQCNTL, 0x08);
  enable_cntv(1);

  // v3s_timer_t *v3s=TIMER_BASE;
  // v3s->t0_ival = 0x80000000;
  // v3s->t0_ctrl = CTL_SRC_24M;
  // v3s->t0_ctrl |= CTL_RELOAD;
  // while ( v3s->t0_ctrl & CTL_RELOAD )
  //     ;
  //   v3s->t0_ctrl |= CTL_ENABLE;
	// kprintf ("timer i val: %x\n", v3s->t0_ival );
	// kprintf ("timer c val: %x\n", v3s->t0_cval );

}

void timer_end() {
  kprintf("timer end\n");
  if (read_core0timer_pending() & 0x08) {
    write_cntv_tval(cntfrq);
    kprintf("cntfrq:%x cnt val:%x\n", read_cntvct(),read_cntv_tval());
    // cpu_sti();
  }
}

void platform_init(){
  // sys_clock_init();
  // sys_dram_init();
}