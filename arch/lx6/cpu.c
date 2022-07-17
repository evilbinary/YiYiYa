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

void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level) {
  if (context == NULL) {
    return;
  }
  context->tss = NULL;
  context->eip = entry;
  context->level = level;
  context->esp0 = stack0;
  u32 cs, ds;
  cpsr_t cpsr;
  cpsr.val = 0;
  if (level == 0) {
    // kernel mode
    cpsr.UM = 0;
    cpsr.LINTLEVEL = 0;
    cpsr.EXCM = 0;
    interrupt_context_t* c = stack0;
  } else if (level == 3) {
    cpsr.UM = 1;
    cpsr.LINTLEVEL = 3;
    cpsr.EXCM = 0;

  } else {
    kprintf("not suppport level %d\n", level);
  }

  interrupt_context_t* user = stack0;
  kmemset(user, 0, sizeof(interrupt_context_t));
  user->pc = entry;
  user->ps = cpsr.val;

  user->a0 = 0x00000000;
  // user->a1 = 0x00010001;
  user->a2 = 0x00020002;
  user->a3 = 0x00030003;
  user->a4 = 0x00040004;
  user->a5 = 0x00050006;
  user->a6 = 0x00060006;
  user->a7 = 0x00070007;
  user->a8 = 0x00080008;
  user->a9 = 0x00090009;
  user->a10 = 0x00100010;
  user->a11 = 0x00110011;
  user->a12 = 0x00120012;
  user->a13 = 0x00130013;
  user->a14 = 0x00140014;
  user->a15 = 0x00160015;

  user->sp = stack3;
  context->esp = stack3;
  context->esp0 = stack0;

  ulong addr = (ulong)boot_info->pdt_base;
  context->kernel_page_dir = addr;
  context->page_dir = addr;
}

#define DEBUG 0
void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;
  interrupt_context_t* c = current_context->esp0;
#if DEBUG
  kprintf("\n=>lr:%x sp:%x lr:%x sp:%x fp:%x irq=> lr:%x sp:%x fp:%x\n",
          current_context->eip, current_context->esp, c->lr, c->sp, c->r11,
          context->lr, context->sp, context->r11);
#endif
  current_context->esp0 = context;
  current_context->esp = context->sp;
  current_context->eip = context->pc;
  *current = next_context;
  context_switch_page(next_context->page_dir);
#if DEBUG
  c = next_context->esp0;
  kprintf("  lr:%x sp:%x irq=> lr:%x sp:%x  fp:%x\n", next_context->eip,
          next_context->esp, c->lr, c->sp, c->r11);
#endif
}

void context_clone(context_t* des, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {
  kmemmove(des, src, sizeof(context_t));
  interrupt_context_t* d0 = stack0;
  interrupt_context_t* s0 = src->esp0;

  if (stack0 != NULL) {
    kmemmove(d0, s0, sizeof(interrupt_context_t));
    des->esp0 = (u32)d0;
  }
  if (stack3 != NULL) {
    cpsr_t cpsr;
    cpsr.val = 0;
    // cpsr.M = 0x10;
    d0->ps = cpsr.val;
    des->esp = d0->sp;
  }
}

ulong cpu_get_cs(void) {
  ulong result;

  return result;
}

int TAS(volatile int* addr, int newval) {
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
