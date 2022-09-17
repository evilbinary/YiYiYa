/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../cpu.h"

#include "context.h"
#include "cpu.h"

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