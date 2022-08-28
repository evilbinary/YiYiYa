/* Driver for the H3 GIC
 *
 * The GIC is the ARM "Generic Interrupt Controller"
 * It has two sections, "cpu" and "dist"
 *
 * Tom Trebisky  1-4-2017
 */
#include "libs/include/types.h"

#define IRQ_UART0 32
#define IRQ_TIMER0 50
#define IRQ_ETHER 114

#define NUM_CONFIG 32
#define NUM_TARGET 512
#define NUM_PRIO 512
#define NUM_MASK 32

#ifdef __cplusplus
#define __I volatile  ///< defines 'read only' permissions
#else
#define __I volatile const  ///< defines 'read only' permissions
#endif
#define __O volatile   ///< defines 'write only' permissions
#define __IO volatile  ///< defines 'read / write' permissions

struct v3s_gic_dist {
  __IO uint32_t ctl;  ///< 0x000 Distributor Control Register
  __I uint32_t type;  ///< 0x004 Interrupt Controller Type Register
  __I uint32_t iid;   ///< 0x008 Distributor Implementer Identification Register
  __IO uint32_t RES1[5];       ///< 0x00C-0x01C Reserved
  __IO uint32_t IMP1[8];       ///< 0x020-0x03C IMPLEMENTATION DEFINED registers
  __IO uint32_t RES2[16];      ///< 0x040-0x07C Reserved
  __IO uint32_t igroup[16];    ///< 0x080 Interrupt Group Registers
  __IO uint32_t RES3[16];      ///< 0x080-0x0FC Reserved
  __IO uint32_t isenable[16];  ///< 0x100 Interrupt Set-Enable Registers
  __IO uint32_t RES4[16];      ///< Reserved
  __IO uint32_t icenable[16];  ///< 0x180 Interrupt Clear-Enable Registers
  __IO uint32_t RES5[16];      ///< Reserved
  __IO uint32_t ispend[16];    ///< 0x200 Interrupt Set-Pending Registers
  __IO uint32_t RES6[16];      ///< Reserved
  __IO uint32_t icpend[16];    ///< 0x280 Interrupt Clear-Pending Registers
  __IO uint32_t RES7[16];      ///< Reserved
  __IO uint32_t isactive[16];  ///< 0x300 GICv2 Interrupt Set-Active Registers
  __IO uint32_t RES8[16];      ///< Reserved
  __IO uint32_t icactive[16];  ///< 0x380 Interrupt Clear-Active Registers
  __IO uint32_t RES9[16];      ///< Reserved
  __IO uint32_t ipriority[128];  ///< 0x400 Interrupt Priority Registers
  __IO uint32_t RES10[128];      ///< Reserved
  __IO uint32_t itargets[128];  ///< 0x800 Interrupt Processor Targets Registers
  __IO uint32_t RES11[128];     ///< Reserved
  __IO uint32_t icfg[32];       ///< 0xC00 Interrupt Configuration Registers
  __IO uint32_t RES12[96];      ///< Reserved
  __IO uint32_t
      nsac[64];      ///< 0xE00 Non-secure Access Control Registers, optional
  __O uint32_t sgi;  ///< 0xF00 Software Generated Interrupt Register
  __IO uint32_t RES13[3];     ///< Reserved
  __IO uint32_t cpendsgi[4];  ///< 0xF10 SGI Clear-Pending Registers
  __IO uint32_t spendsgi[4];  ///< 0xF20 SGI Set-Pending Registers
  __IO uint32_t RES14[40];    ///< Reserved
  __I uint32_t PID4;          ///< Peripheral ID 4 Register
  __I uint32_t PID5;          ///< Peripheral ID 5 Register
  __I uint32_t PID6;          ///< Peripheral ID 6 Register
  __I uint32_t PID7;          ///< Peripheral ID 7 Register
  __I uint32_t PID0;          ///< Peripheral ID 0 Register
  __I uint32_t PID1;          ///< Peripheral ID 1 Register
  __I uint32_t PID2;          ///< Peripheral ID 2 Register [7:4] 0x2 for GICv2
  __I uint32_t PID3;          ///< Peripheral ID 3 Register
  __I uint32_t
      iccid[4];  ///< 0xFF0 Component ID Registers -> 0x00, 0xF0, 0x05, 0xB1
  __I uint32_t
      icpid[2];  ///< 0xFF0 Peripheral ID Registers -> 0x90, 0xB4 -> ARM GICv2

  // volatile unsigned int ctl;         // 0x000
  // volatile const unsigned int type;  // 0x004
  // volatile const unsigned int iid;   // 0x008
  // volatile unsigned int _res0[29];
  // volatile unsigned int igroup[32];  // 0x080
  // volatile unsigned int isenable[32];
  // volatile unsigned int icenable[32];
  // volatile unsigned int ispend[32];
  // volatile unsigned int icpend[32];
  // volatile unsigned int isactive[32];
  // volatile unsigned int icactive[32];
  // volatile unsigned char ipriority[512];
  // volatile unsigned int _res1[128];
  // volatile unsigned char itargets[512];
  // volatile unsigned int _res2[128];
  // volatile unsigned int icfg[32];
  // volatile unsigned int _res3[32];
  // volatile const unsigned int ppis;
  // volatile unsigned int spis[15];
  // volatile unsigned int _res4[112];
  // volatile unsigned int sgi;
  // volatile unsigned int _res5[3];
  // volatile unsigned char cpendsgi[16];
  // volatile unsigned char spendsgi[16];
  // volatile unsigned int _res6[40];
  // volatile const unsigned int pid[8];
  // volatile const unsigned int cid[4];
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
  volatile unsigned int _res1[41];
  volatile unsigned int ap;
  volatile unsigned int _res2[3];
  volatile unsigned int nasp;
  volatile unsigned int _res3[6];
  volatile const unsigned int iid;
  volatile unsigned int _res4[960];
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
  struct v3s_gic_dist *gp = GIC_DIST_BASE;
  struct v3s_gic_cpu *cp = GIC_CPU_BASE;
  int irq;
  irq = cp->ia;
  if (irq == 1023) {
    uart_send('X');
    return;
  }
  if (gp->ispend[1] & TIMER_MASK) {
    // kprintf("GIC iack = %x\n", irq);

    if (irq == IRQ_TIMER0) {
      timer_handler(0);
    }
    gic_irqack(irq);
    // cp->eoi = irq;
    // gic_unpend(IRQ_TIMER0);
  }

  // ms_delay ( 5 );
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

  p = (unsigned long *)&gp->itargets;
  // targets=0x1c81800
  kprintf("GIC target = %x\n", p);
  p = (unsigned long *)&gp->icfg;
  // confg=0x1c81c00
  kprintf("GIC config  = %x\n", p);
  p = (unsigned long *)&gp->sgi;
  // sgi=0x1c81f00
  kprintf("GIC soft = %x\n", p);

  // iid=0x1c81008
  kprintf("GIC iid = %x %x\n", &gp->iid, gp->iid);

  // kprintf("GIC ipriority = %x\n", &gp->ipriority);

  // kprintf("GIC itargets = %x\n", &gp->itargets);

  // kprintf("GIC icfg = %x\n", &gp->icfg);

  // kprintf("GIC ppis = %x\n", &gp->ppis);
  // kprintf("GIC spis = %x\n", &gp->spis);

  // kprintf("GIC cpendsgi = %x\n", &gp->cpendsgi);
  // kprintf("GIC spendsgi = %x\n", &gp->spendsgi);
  // kprintf("GIC pid = %x\n", &gp->pid);

  // kprintf("GIC cid = %x\n", &gp->cid);

  // kprintf("GIC ap = %x\n", &cp->ap);
  // kprintf("GIC nasp = %x\n", &cp->nasp);
  // kprintf("GIC iid = %x\n", &cp->iid);
  // kprintf("GIC dir = %x\n", &cp->dir);

  int irq_id = IRQ_TIMER0;
  int cpu_id = 0;

  unsigned int reg = irq_id / 32;
  unsigned int mask = 1 << (irq_id & 0x1f);

  // init
  gp->ctl = 0;

  // clear
  for (i = 0; i < 32; ++i) {
    gp->icpend[i] = 0xffffffff;
  }

  for (i = 0; i < 8; ++i) {
    gp->igroup[i] = 0;
  }

  // set priority
  gp->ipriority[irq_id] = 0;

  // set security
  unsigned int value = gp->igroup[reg];

  value &= ~mask;
  gp->igroup[reg] = value;

  // set cpu target
  gp->itargets[irq_id] |= (1 << cpu_id) & 0xff;

  // enable irq
  gic_enable(IRQ_TIMER0);
  gp->ctl = G0_ENABLE;

  cp->pm = 0xff;
  cp->bp = 0x7;
  cp->ctl = G0_ENABLE;
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