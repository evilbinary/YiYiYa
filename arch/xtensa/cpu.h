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
      u32 LINTLEVEL : 4;
      u32 EXCM : 1;  // 0 normal operation 1 exception mode
      u32 UM : 1;    // 0 → kernel vector mode  1 → user vector mode
      u32 RING : 2;  // Privilege level [MMU Option]
      u32 OWB : 4;   // Old window base [Windowed Register Option]
      u32 CALLINC : 2;
      u32 WOE : 1;
      u32 RESERVED1 : 13;
    };
    u32 val;
  };
} __attribute__((packed)) cpsr_t;

typedef u32 (*sys_call_fn)(u32 arg1, u32 arg2, u32 arg3, u32 arg4, u32 arg5,
                           u32 arg6);

#define sys_fn_call(duck_interrupt_context, fn)               \
  duck_interrupt_context->a0 = ((sys_call_fn)fn)(             \
      duck_interrupt_context->a0, duck_interrupt_context->a2, \
      duck_interrupt_context->a3, duck_interrupt_context->a4, \
      duck_interrupt_context->a5, duck_interrupt_context->a6);

#define cpu_cli()
#define cpu_sti()
#define cpu_cpl() (cpu_get_cs() & 0x3)

#define context_switch_page(page_dir) \
  cpu_set_page(page_dir)  // asm volatile("mov %0, %%cr3" : : "r" (page_dir))

#define context_fn(context) context->a7
#define context_ret(context) context->a0

#define context_restore(duck_context) interrupt_exit_context(duck_context);

#define isb()
#define dsb()

#endif