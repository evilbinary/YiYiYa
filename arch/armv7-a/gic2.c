/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "gic2.h"

gic_t gic;

void gic_init_base(void *cpu_addr, void *dist_addr) {
  gic.cpu = cpu_addr;
  gic.dist = dist_addr;
}

void gic_init() {
  gic_dist_t *gp = gic.dist;
  gic_cpu_t *cp = gic.cpu;
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



}

void gic_enable(int irq) {
  int x = irq / 32;
  unsigned long mask = 1 << (irq % 32);
  //通过设置GICD_ICENABLERn寄存器，开启中断
  gic.dist->isenable[x] = mask;
}

void gic_unpend(int irq) {
  int x = irq / 32;
  unsigned long mask = 1 << (irq % 32);
  //GICD_ISPENDRn 寄存器，修改中断的pending状态
  gic.dist->icpend[x] = mask;
}

int gic_irqwho(void) { return gic.cpu->ia; }

void gic_irqack(int irq) {
  gic.cpu->eoi = irq;
  gic_unpend(irq);
}


void gic_send_sgi(int cpu,int intid){
  //通过 GICD_SGIR Software Generated Interrupt Register软中断
  
  intid&=0xf;//in the range 0-15
  gic.dist->sgi= cpu<<16;
}
