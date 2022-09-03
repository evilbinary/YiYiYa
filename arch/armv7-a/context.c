/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "context.h"
#include "cpu.h"

extern boot_info_t* boot_info;

int context_get_mode(context_t* context) {
  int mode = 0;
  if (context != NULL) {
    interrupt_context_t* c = context->esp0;
    if ((c->psr & 0x1F) == 0x10) {
      return 3;  // user mode
    }
  }
  return mode;
}

void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level,int cpu) {
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
    cpsr.Z = 1;
    cpsr.C = 1;
    cpsr.A = 1;
    cpsr.I = 0;
    cpsr.F = 1;
    cpsr.M = 0x1f;
    interrupt_context_t* c = stack0;
  } else if (level == 3) {
    cpsr.I = 0;
    cpsr.F = 0;
    cpsr.T = 0;  // arm
    cpsr.M = 0x10;
  } else {
    kprintf("not suppport level %d\n", level);
  }

  interrupt_context_t* user = stack0;
  kmemset(user, 0, sizeof(interrupt_context_t));
  user->lr = entry;  // r14
  user->lr += 4;
  user->psr = cpsr.val;
  user->r0 = 0;
  user->r1 = 0x00010001;
  user->r2 = 0x00020002;
  user->r3 = 0x00030003;
  user->r4 = 0x00040004;
  user->r5 = 0x00050006;
  user->r6 = 0x00060006;
  user->r7 = 0x00070007;
  user->r8 = 0x00080008;
  user->r9 = 0x00090009;
  user->r10 = 0x00100010;
  user->r11 = 0x00110011;  // fp
  user->r12 = 0x00120012;  // ip
  user->sp = stack3;       // r13
  user->lr0 = user->lr;
  context->esp = stack3;
  context->esp0 = stack0;

  ulong addr = (ulong)boot_info->pdt_base;
  context->kernel_page_dir = addr;
  context->page_dir = addr;
#ifdef PAGE_CLONE
  context->page_dir = page_alloc_clone(addr);
#endif
}

// #define DEBUG 1
void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;
#if DEBUG
  kprintf("-----switch dump current------\n");
  context_dump(current_context);
#endif
  current_context->esp0 = context;
  current_context->esp = context->sp;
  current_context->eip = context->lr;
  *current = next_context;
  context_switch_page(next_context->page_dir);
#if DEBUG
  kprintf("-----switch dump next------\n");
  context_dump(next_context);
  kprintf("\n");
#endif
}

void context_dump(context_t* c) {
  kprintf("ip:  %x\n", c->eip);
  kprintf("sp0: %x\n", c->esp0);
  kprintf("sp:  %x\n", c->esp);

  kprintf("page_dir: %x\n", c->page_dir);
  kprintf("kernel page_dir: %x\n", c->kernel_page_dir);
  kprintf("--interrupt context--\n");
  interrupt_context_t* context = c->esp0;
  context_dump_interrupt(context);
}

void context_dump_interrupt(interrupt_context_t* context) {
  kprintf("lr:  %x cpsr:%x\n", context->lr, context->psr);
  kprintf("sp:  %x\n", context->sp);
  kprintf("r0:  %x\n", context->r0);
  kprintf("r1:  %x\n", context->r1);
  kprintf("r2:  %x\n", context->r2);
  kprintf("r3:  %x\n", context->r3);
  kprintf("r4:  %x\n", context->r4);
  kprintf("r5:  %x\n", context->r5);
  kprintf("r6:  %x\n", context->r6);
  kprintf("r7:  %x\n", context->r7);
  kprintf("r8:  %x\n", context->r8);
  kprintf("r9:  %x\n", context->r9);
  kprintf("r10: %x\n", context->r10);
  kprintf("r11(fp): %x\n", context->r11);
  kprintf("r12(ip): %x\n", context->r12);
}

void context_clone(context_t* des, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {
  interrupt_context_t* d0 = stack0;
  interrupt_context_t* s0 = src->esp0;
#if DEBUG
  kprintf("------context clone dump src--------------\n");
  context_dump(src);
#endif
  des->eip = src->eip;
  des->level= src->level;
  if (stack0 != NULL) {
    kmemmove(d0, s0, sizeof(interrupt_context_t));
    des->esp0 = (u32)d0;
  }
  if (stack3 != NULL) {
    cpsr_t cpsr;
    cpsr.val = 0;
    cpsr.Z = 1;
    cpsr.C = 1;
    cpsr.A = 1;
    cpsr.I = 1;
    cpsr.F = 1;
    cpsr.T =0;

    cpsr.M = 0x13;
    d0->psr = cpsr.val;
    d0->sp=stack3;
    des->esp=stack3;
  }
  des->page_dir =src->page_dir;
  des->page_dir = page_alloc_clone(src->page_dir);
  des->kernel_page_dir = src->kernel_page_dir;
#if DEBUG
  kprintf("------context clone dump des--------------\n");
#endif
  context_dump(des);
}