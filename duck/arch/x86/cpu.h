/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef X86_CPU_H
#define X86_CPU_H

#include "libs/include/types.h"
#include "arch/boot.h"

#define debugger asm("xchg %bx,%bx\n")

#define sys_fn_call(duck_interrupt_context, fn)                             \
  asm volatile(                                                             \
      " \
      push %1; \
      push %2; \
      push %3; \
      push %4; \
      push %5; \
      call *%6; \
      add $20,%%esp; \
    "                                                                   \
      : "=a"(duck_interrupt_context->eax)                                   \
      : "r"(duck_interrupt_context->edi), "r"(duck_interrupt_context->esi), \
        "r"(duck_interrupt_context->edx), "r"(duck_interrupt_context->ecx), \
        "r"(duck_interrupt_context->ebx), "r"(fn))

#define cpu_cli() asm("cli")
#define cpu_sti() asm("sti")
#define cpu_cpl() (cpu_get_cs()&0x3)

int cpu_get_number();
int cpu_start_id(u32 id,u32 entry);
int cpu_init_id(u32 id);
u32 cpu_get_id();

#endif