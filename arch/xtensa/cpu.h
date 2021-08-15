/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef XTENSA_CPU_H
#define XTENSA_CPU_H

#include "arch/boot.h"
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

typedef u32 (*sys_call_fn)(u32 arg1, u32 arg2, u32 arg3, u32 arg4, u32 arg5,
                           u32 arg6);

#define sys_fn_call(duck_interrupt_context, fn)                               \
  duck_interrupt_context->r0 = ((                                             \
      sys_call_fn)fn)(duck_interrupt_context->r0, duck_interrupt_context->r1, \
                      duck_interrupt_context->r2, duck_interrupt_context->r3, \
                      duck_interrupt_context->r4, duck_interrupt_context->r5);

#define cpu_cli()  
#define cpu_sti()  
#define cpu_cpl() (cpu_get_cs() & 0x3)

#define context_switch_page(page_dir)  cpu_set_page(page_dir)// asm volatile("mov %0, %%cr3" : : "r" (page_dir))

#define context_fn(context) context->r7
#define context_ret(context) context->r0

#define context_restore(duck_context) \
  interrupt_exit_context(duck_context);

#define isb()  
#define dsb()  

#endif