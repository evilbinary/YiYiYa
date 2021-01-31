/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef CPU_H
#define CPU_H

#include "boot.h"
#include "libs/include/types.h"
#include "interrupt.h"

typedef uint32_t phys_address_t;
typedef uint32_t virtual_address_t;

#define debugger asm("xchg %bx,%bx\n")

typedef struct context_t {
  u32 esp0, ss0, ds0;
  u32 esp, ss, ds;
  u32 eip;
  tss_t* tss;
  u32* page_dir;
} context_t;

typedef struct stack_frame {
  struct stack_frame* prev;
  void* return_addr;
} __attribute__((packed)) stack_frame_t;

void cpu_init();
void cpu_halt();

#define KERNEL_MODE 0
#define USER_MODE 3

void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level);

void context_switch(interrupt_context_t* context,context_t** current, 
                    context_t* next_context);


#define context_restore(duck_context) interrupt_exit_context(duck_context->esp0)

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

#define FAA(ptr) __sync_fetch_and_add(ptr, 1)

int TAS(volatile int* addr, int newval);

#endif