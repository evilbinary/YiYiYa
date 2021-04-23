/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../cpu.h"

#include "context.h"
#include "cpu.h"

void cpu_init() {}

void cpu_halt() { asm("wfi"); }

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
    cpsr.T=0; //arm
    cpsr.M = 0x10;
  } else {
    kprintf("not suppport level %d\n", level);
  }

  interrupt_context_t* user = stack3;
  memset(user, 0, sizeof(interrupt_context_t));
  user->lr = entry;
  user->psr= cpsr.val;
  user->r0 = 0;
  user->r1 = 0x11111111;
  user->r2 = 0x22222222;
  user->r3 = 0x33333333;
  user->r4 = 0x44444444;
  user->r5 = 0x55555555;
  user->r6 = 0x66666666;
  user->r7 = 0x77777777;
  user->r8 = 0x88888888;
  user->r9 = 0x99999999;
  user->r10 = 0x101010;
  user->r11 = 0x111111;
  context->esp = stack3;
}

void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;
  if (context == NULL) {
    // context = current_context->esp0;
    context = current_context->esp;
  } else {
    // current_context->esp0 = context;
    current_context->esp = context;
  }
  *current = next_context;
}

void context_clone(context_t* context, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {
  *context = *src;
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