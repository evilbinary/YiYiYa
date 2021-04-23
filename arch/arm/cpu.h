/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef ARM_CPU_H
#define ARM_CPU_H

#include "libs/include/types.h"

#define debugger

typedef struct context_t {
  u32 esp0, ss0, ds0;
  u32 esp, ss, ds;
  u32 eip;
  tss_t* tss;
  u32* page_dir;
  u32* kernel_page_dir;
  u32 level;
} context_t;

typedef struct cpsr {
  union {
    struct {
      u32 M : 5;
      u32 T : 1;
      u32 F : 1;
      u32 I : 1;
      u32 A : 1;
      u32 E : 1;
      u32 RESERVED2 : 6;
      u32 GE : 4;
      u32 RESERVED1 : 4;
      u32 J : 1;
      u32 Res : 2;
      u32 Q : 1;
      u32 V : 1;
      u32 C : 1;
      u32 Z : 1;
      u32 N : 1;
    };
    u32 val;
  };
} __attribute__((packed)) cpsr_t;

#define sys_fn_call(duck_interrupt_context, fn)
// \
  // asm volatile(                                                             \
  //     " \
  //     push %1; \
  //     push %2; \
  //     push %3; \
  //     push %4; \
  //     push %5; \
  //     call *%6; \
  //     add $20,%%esp; \
  //   "                                                                   \
  //     : "=a"(duck_interrupt_context->eax)                                   \
  //     : "r"(duck_interrupt_context->edi), "r"(duck_interrupt_context->esi), \
  //       "r"(duck_interrupt_context->edx), "r"(duck_interrupt_context->ecx), \
  //       "r"(duck_interrupt_context->ebx), "r"(fn))

#define cpu_cli() asm("cpsid i" : : : "memory", "cc")
#define cpu_sti() asm("cpsie i" : : : "memory", "cc")
#define cpu_cpl() (cpu_get_cs() & 0x3)

#define context_switch_page( \
    page_dir)  // asm volatile("mov %0, %%cr3" : : "r" (page_dir))

#define context_r0(context) context->r0

#define context_restore(duck_context) cpu_sti();interrupt_exit_context(duck_context);

#endif