/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "boot.h"
#include "libs/include/types.h"

#define ISR_TIMER 0x20
#define ISR_KEYBOARD 0x21
#define ISR_SYSCALL 0x30

#define IDT_NUMBER 512

typedef struct interrupt_context {
  u32 ds;
  u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;  // pushal
  u32 no, code;
  u32 eip, cs, eflags, useresp, ss;  // auto push
} __attribute__((packed)) interrupt_context_t;

typedef void (*interrupt_handler_t)(interrupt_context_t* context);

void timer_init(int hz);

#define INTERRUPT_SERVICE __attribute__((naked))

void interrutp_regist(u32 vec, interrupt_handler_t handler);

#define interrupt_process(X) asm("\t call  " #X "\n")

#define interrupt_entering_code(VEC, CODE) \
  asm("cli\n"                              \
      "pushl %0 \n"                        \
      "pushl %1 \n"                        \
      "pushal\n"                           \
      "mov %%ds,%%ax\n"                    \
      "push %%eax\n"                       \
      "mov %2,%%ax\n"                      \
      "mov %%ax,%%ds\n"                    \
      "mov %%ax,%%es\n"                    \
      "mov %%ax,%%fs\n"                    \
      :                                    \
      : "i"(CODE), "i"(VEC), "i"(GDT_ENTRY_32BIT_DS * GDT_SIZE));

#define interrupt_entering(VEC) \
  asm("cli\n"                   \
      "pushl %0 \n"             \
      "pushal\n"                \
      "mov %%ds,%%ax\n"         \
      "push %%eax\n"            \
      "mov %1,%%ax\n"           \
      "mov %%ax,%%ds\n"         \
      "mov %%ax,%%es\n"         \
      "mov %%ax,%%fs\n"         \
      :                         \
      : "i"(VEC), "i"(GDT_ENTRY_32BIT_DS * GDT_SIZE));

#define interrupt_exit() \
  asm("pop %%ebx\n"      \
      "mov %%bx,%%ds\n"  \
      "mov %%bx,%%es\n"  \
      "mov %%bx,%%fs\n"  \
      "popal\n"          \
      "add $8,%%esp \n"  \
      "sti\n"            \
      "iret\n"           \
      :                  \
      :);

void interrupt_init();

#endif