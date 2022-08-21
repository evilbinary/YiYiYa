/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef CONTEXT_H
#define CONTEXT_H

#include "arch/boot.h"
#include "libs/include/types.h"

typedef struct interrupt_context {
  // ds
  u32 gs, fs, es, ds;

  // all reg
  u32 edi, esi, ebp, esp_null, ebx, edx, ecx, eax;  // pushal

  u32 no;
  // interrup stack
  u32 code;
  u32 eip;
  u32 cs;
  u32 eflags;

  // priv change
  u32 esp;
  u32 ss;
} __attribute__((packed)) interrupt_context_t;

typedef void (*interrupt_handler_t)(interrupt_context_t* context);

void timer_init(int hz);

#define INTERRUPT_SERVICE __attribute__((naked))

void interrutp_regist(u32 vec, interrupt_handler_t handler);

#if defined(__WIN32__)

#define interrupt_process(X) \
  asm volatile(              \
      "push %esp\n"          \
      "call  _" #X            \
      " \n"                  \
      "add $4,%esp\n")
#else

#define interrupt_process(X) \
  asm volatile(              \
      "push %esp\n"          \
      "call  " #X            \
      " \n"                  \
      "add $4,%esp\n")

#endif





#define interrupt_entering_code(VEC, CODE) \
  asm volatile(                            \
      "cli\n"                              \
      "push %0 \n"                         \
      "push %1 \n"                         \
      "pushal\n"                           \
      "push %%ds\n"                        \
      "push %%es\n"                        \
      "push %%fs\n"                        \
      "push %%gs\n"                        \
      "mov %2,%%eax\n"                     \
      "mov %%ax,%%ds\n"                    \
      "mov %%ax,%%es\n"                    \
      "mov %%ax,%%gs\n"                    \
      "mov %%ax,%%fs\n"                    \
      :                                    \
      : "i"(CODE), "i"(VEC), "i"(GDT_ENTRY_32BIT_DS * GDT_SIZE))

#define interrupt_entering(VEC) \
  asm volatile(                 \
      "cli\n"                   \
      "push %0 \n"              \
      "pushal\n"                \
      "push %%ds\n"             \
      "push %%es\n"             \
      "push %%fs\n"             \
      "push %%gs\n"             \
      "mov %1,%%eax\n"          \
      "mov %%ax,%%ds\n"         \
      "mov %%ax,%%es\n"         \
      "mov %%ax,%%gs\n"         \
      "mov %%ax,%%fs\n"         \
      :                         \
      : "i"(VEC), "i"(GDT_ENTRY_32BIT_DS * GDT_SIZE))

#define interrupt_exit() \
  asm volatile(          \
      "pop %%gs\n"       \
      "pop %%fs\n"       \
      "pop %%es\n"       \
      "pop %%ds\n"       \
      "popal\n"          \
      "add $8,%%esp\n"   \
      "sti\n"            \
      "iret\n"           \
      :                  \
      :)

#define interrupt_exit_context(context) \
  asm volatile(                     \
      "mov %0,%%esp\n"              \
      "pop %%gs\n"                  \
      "pop %%fs\n"                  \
      "pop %%es\n"                  \
      "pop %%ds\n"                  \
      "popal\n"                     \
      "add $8,%%esp\n"              \
      "sti\n"                       \
      "iret\n"                      \
      :                             \
      : "m"(context->esp0))

#endif