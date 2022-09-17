/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../cpu.h"

#include "context.h"
#include "cpu.h"

extern boot_info_t* boot_info;

u32 read_pc() {
  u32 val = 0;
  return val;
}

u32 read_fp() {
  u32 val = 0;
  return val;
}

u32 cpu_get_sp() {
  void* sp;
  asm volatile("mov %0, sp;" : "=r"(sp));
  return sp;
}

void cpu_set_vector(u32 addr) {
  kprintf("set vector a %x\n", addr);
  asm volatile("wsr.VECBASE %0" ::"r"(addr));
}

void cpu_set_page(u32 page_table) {}

void cpu_disable_page() {
  u32 reg;
  // read mmu
}

void cpu_enable_page() {
  u32 reg;

  dsb();
  isb();
}

inline void cpu_invalidate_tlbs(void) {}

void cpu_init() {}

void cpu_halt() {
  for (;;) {
  };
}

ulong cpu_get_cs(void) {
  ulong result;

  return result;
}

int cpu_tas(volatile int* addr, int newval) {
  int result = newval;
  // asm volatile("lock; xchg %0, %1"
  //              : "+m"(*addr), "=r"(result)
  //              : "1"(newval)
  //              : "cc");
  return result;
}

// void context_restore(context_t* context){
//   cpu_sti();
//   interrupt_exit_context(context);
// }

void cpu_backtrace(void) {
  int topfp = read_fp();
  for (int i = 0; i < 10; i++) {
    u32 fp = *(((u32*)topfp) - 3);
    u32 sp = *(((u32*)topfp) - 2);
    u32 lr = *(((u32*)topfp) - 1);
    u32 pc = *(((u32*)topfp) - 0);
    if (i == 0) {
      kprintf("top frame %x\n", pc);
    }  // top frame
    if (fp != 0) {
      kprintf(" %x\n", lr);
    }  // middle frame
    else {
      kprintf("bottom frame %x\n", pc);
    }  // bottom frame, lr invalid
    if (fp == 0) break;
    topfp = fp;
  }
}

void context_dump(context_t* c) {
  kprintf("ip:  %x\n", c->eip);
  kprintf("sp0: %x\n", c->esp0);
  kprintf("sp:  %x\n", c->esp);

  kprintf("page_dir: %x\n", c->page_dir);
  kprintf("kernel page_dir: %x\n", c->kernel_page_dir);
  kprintf("--interrupt context--\n");
  interrupt_context_t* context = c->esp0;
  // context_dump_interrupt(context);
}
