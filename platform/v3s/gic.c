/* Driver for the H3 GIC
 *
 * The GIC is the ARM "Generic Interrupt Controller"
 * It has two sections, "cpu" and "dist"
 *
 * Tom Trebisky  1-4-2017
 */

/* The H3 defines 157 interrupts (0-156)
 * We treat this as 160, more or less
 */
#define NUM_IRQ 157

#define IRQ_UART0 32
#define IRQ_TIMER0 50
#define IRQ_ETHER 114

#define NUM_CONFIG 64
#define NUM_TARGET 255 * sizeof(int)
#define NUM_PRIO 255 * sizeof(int)
#define NUM_MASK 32

struct v3s_gic_dist {
  volatile unsigned int ctl;         // 0x000
  volatile const unsigned int type;  // 0x004
  volatile const unsigned int iid;   // 0x008
  volatile unsigned int _res0[29];
  volatile unsigned int igroup[32];  // 0x080
  volatile unsigned int isenable[32];
  volatile unsigned int icenable[32];
  volatile unsigned int ispend[32];
  volatile unsigned int icpend[32];
  volatile unsigned int isactive[32];
  volatile unsigned int icactive[32];
  volatile unsigned char ipriority[256 * sizeof(int)];
  volatile unsigned char istargets[256 * sizeof(int)];
  volatile unsigned int icfg[64];
  volatile const unsigned int ppis;
  volatile unsigned int spis[127];
  volatile unsigned int sgi;
  volatile unsigned int cpendsgi[4];
  volatile unsigned int spendsgi[4];
  volatile const unsigned int pid[8];
  volatile const unsigned int cid[4];
};

struct v3s_gic_cpu {
  volatile unsigned int ctl;
  volatile unsigned int pm;
  volatile unsigned int bp;
  volatile const unsigned int ia;
  volatile unsigned int eoi;
  volatile const unsigned int rp;
  volatile const unsigned int hppi;
  volatile unsigned int abp;
  volatile const unsigned int aia;
  volatile unsigned int aeoi;
  volatile const unsigned int ahppi;
  volatile unsigned int _res0[(0xD0 - 0x2C) / sizeof(unsigned int)];
  volatile unsigned int ap;
  volatile unsigned int _res1[(0xE0 - 0xD4) / sizeof(unsigned int)];
  volatile unsigned int nasp;
  volatile unsigned int _res2[(0xFC - 0xE4) / sizeof(unsigned int)];
  volatile const unsigned int iid;
  volatile unsigned int _res3[(0x1000 - 0x100) / sizeof(unsigned int)];
  volatile unsigned int dir;
};

#define GIC_DIST_BASE ((struct v3s_gic_dist *)0x01c81000)
#define GIC_CPU_BASE ((struct v3s_gic_cpu *)0x01c82000)

#define G0_ENABLE 0x01
#define G1_ENABLE 0x02

extern volatile int timer_count;
/* This is TIMER 0 */
#define TIMER_MASK 0x40000

#ifdef notdef
#define GIC_CPU_CTRL 0x00
#define GIC_CPU_PRIMASK 0x04
#define GIC_CPU_BINPOINT 0x08
#define GIC_CPU_INTACK 0x0c
#define GIC_CPU_EOI 0x10
#define GIC_CPU_RUNNINGPRI 0x14
#define GIC_CPU_HIGHPRI 0x18

#define GIC_DIST_CTRL 0x000
#define GIC_DIST_CTR 0x004
#define GIC_DIST_ENABLE_SET 0x100
#define GIC_DIST_ENABLE_CLEAR 0x180
#define GIC_DIST_PENDING_SET 0x200
#define GIC_DIST_PENDING_CLEAR 0x280
#define GIC_DIST_ACTIVE_BIT 0x300
#define GIC_DIST_PRI 0x400
#define GIC_DIST_TARGET 0x800
#define GIC_DIST_CONFIG 0xc00
#define GIC_DIST_SOFTINT 0xf00
#endif

void gic_enable(int irq) {
  struct v3s_gic_dist *gp = GIC_DIST_BASE;
  int x = irq / 32;
  unsigned long mask = 1 << (irq % 32);
  gp->isenable[x] = mask;
}

void gic_unpend(int irq) {
  struct v3s_gic_dist *gp = GIC_DIST_BASE;
  int x = irq / 32;
  unsigned long mask = 1 << (irq % 32);

  gp->icpend[x] = mask;
}

void gic_handler(void) {
  struct v3s_gic_cpu *cp = GIC_CPU_BASE;
  int irq;

  irq = cp->ia;

  /* Do we need to EOI the spurious ? */
  if (irq == 1023) {
    uart_send('X');
    uart_send('\n');
    // return;
  }

  if (irq == IRQ_TIMER0) timer_handler(0);

  cp->eoi = irq;
  gic_unpend(IRQ_TIMER0);

  // uart_putc ( '.' );
  // uart_putc ( '\n' );

  kprintf("GIC iack = %08x\n", irq);
  // ms_delay ( 200 );
}

int gic_irqwho(void) {
  struct v3s_gic_cpu *cp = GIC_CPU_BASE;

  return cp->ia;
}

void gic_irqack(int irq) {
  struct v3s_gic_cpu *cp = GIC_CPU_BASE;

  cp->eoi = irq;
  gic_unpend(irq);
}

void gic_init(void) {
  struct v3s_gic_dist *gp = GIC_DIST_BASE;
  struct v3s_gic_cpu *cp = GIC_CPU_BASE;
  unsigned long *p;
  int i;

  p = (unsigned long *)&gp->istargets;
  // targets=0x1c81800
  kprintf("GIC target = %x\n", p);
  p = (unsigned long *)&gp->icfg;
  // confg=0x1c81c00
  kprintf("GIC config  = %x\n", p);
  p = (unsigned long *)&gp->sgi;
  // sgi=0x1c81f00
  kprintf("GIC soft = %x\n", p);

  // iid=0x1c81008
  kprintf("GIC iid = %x\n", &gp->iid);

  // int irq_id = IRQ_TIMER0;
  // int cpu_id = 0;

  // unsigned int reg = irq_id / 32;
  // unsigned int mask = 1 << (irq_id & 0x1f);

  // // init
  // gp->ctl = 0;

  // //clear
  // for (i = 0; i < 32; ++i) {
  //   gp->icpend[i] = 0xffffffff;
  // }

  // for (i = 0; i < 8; ++i) {
  //   gp->igroup[i] = 0;
  // }

  // // set priority
  // gp->ipriority[irq_id] = 0;

  // // set security
  // unsigned int value = gp->igroup[reg];

  // value &= ~mask;
  // gp->igroup[reg] = value;

  // // set cpu target
  // gp->istargets[irq_id] |= (1 << cpu_id) & 0xff;

  // // enable irq
  // gic_enable(IRQ_TIMER0);

  // gp->ctl |= G0_ENABLE|G1_ENABLE;

  // cp->pm=0xff;
  // cp->bp=7;
  // cp->ctl|= G0_ENABLE|G1_ENABLE;

  // /* Initialize the distributor */
  gp->ctl = 0;

  /* make all SPI level triggered */
  for (i = 2; i < NUM_CONFIG; i++) gp->icfg[i] = 0;

  for (i = 8; i < NUM_TARGET; i++) gp->istargets[i] = 0x01010101;

  for (i = 8; i < NUM_PRIO; i++) gp->ipriority[i] = 0xa0a0a0a0;

  for (i = 1; i < NUM_MASK; i++) gp->icenable[i] = 0xffffffff;

  for (i = 0; i < NUM_MASK; i++) gp->icpend[i] = 0xffffffff;

  gic_enable(IRQ_TIMER0);

  gp->ctl = G0_ENABLE | G1_ENABLE;

  /* ** now initialize the per CPU stuff.
   *  XXX - the following will need to be done by each CPU
   *  when we get multiple cores running.
   */

  /* enable all SGI, disable all PPI */
  gp->icenable[0] = 0xffff0000;
  gp->isenable[0] = 0x0000ffff;

  /* priority for PPI and SGI */
  for (i = 0; i < 8; i++) gp->ipriority[i] = 0xa0a0a0a0;

  cp->pm = 0xff;
  cp->ctl = G0_ENABLE | G1_ENABLE;
}

void gic_check(void) {
  struct v3s_gic_dist *gp = GIC_DIST_BASE;
  kprintf("GIC pending timer_count:%d ispend:\n", timer_count);
  for (int i = 0; i < 32; i++) {
    kprintf(" %d=%x", i, gp->ispend[i]);
  }
  kprintf("\n");
}

void gic_poll(void) {
  struct v3s_gic_dist *gp = GIC_DIST_BASE;

  for (;;) {
    // ms_delay ( 2000 );
    if (gp->ispend[1] & TIMER_MASK) {
      gic_check();
      gic_handler();
      kprintf("+GIC pending: %x %x %d\n", gp->ispend[0], gp->ispend[1],
              timer_count);
    }
  }
}

#define MS_300 5000000;

/* A reasonable delay for blinking an LED.
 * This began as a wild guess, but
 * in fact yields a 300ms delay
 * as calibrated below.
 */
void delay_x(void) {
  volatile int count = MS_300;

  while (count--)
    ;
}

#define MS_1 166667

void delay_ms(int ms) {
  volatile int count = ms * MS_1;

  while (count--)
    ;
}

void gic_watch(void) {
  for (;;) {
    delay_x();
    gic_check();
  }
}