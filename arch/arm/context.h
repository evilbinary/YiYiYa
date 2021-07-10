/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef CONTEXT_H
#define CONTEXT_H

#include "arch/boot.h"
#include "libs/include/types.h"

#define ISR_TIMER 0x0f
#define ISR_KEYBOARD 0x21
#define ISR_SYSCALL 2
#define ISR_MOUSE 0x2c

#define IDT_NUMBER 256

typedef struct interrupt_context {
  // manual push
  u32 no;
  u32 code;
  u32 sp;

  u32 psr;

  u32 r0;
  u32 r1;
  u32 r2;
  u32 r3;
  u32 r4;
  u32 r5;
  u32 r6;
  u32 r7;
  u32 r8;
  u32 r9;
  u32 r10;
  u32 r11;
  u32 r12;

  u32 lr;
} __attribute__((packed)) interrupt_context_t;

#define interrupt_process(X) \
  asm volatile(              \
      "push {r0-r7} \n"      \
      "blx " #X              \
      "\n"                   \
      "pop {r0-r7}\n"        \
      :                      \
      :)

#define interrupt_entering_code(VEC, CODE) \
  asm volatile(                            \
      "stmdb sp!, {r0-r12,lr} \n"          \
      "mrs r0,spsr\n"                      \
      "stmdb sp!, {r0} \n"                 \
      "mrs r0,cpsr \n"                     \ 
      "cps #0x1f \n  "                     \
      "mov r3,sp \n"                       \
      "msr cpsr,r0 \n"                     \ 
      "mov r1,%0\n"                        \
      "mov r2,%1\n"                        \
      "stmdb sp!, {r1,r2,r3} \n"           \
      "mov r0,sp\n"                        \
      :                                    \
      : "i"(VEC), "i"(CODE))

#define interrupt_exit_context(duck_context) \
  asm volatile(                              \
      "ldr sp,%0 \n"                         \
      "ldmia sp!,{r1,r2,r3}\n"               \
      "mrs r0,cpsr \n"                       \ 
      "cps #0x1f \n  "                       \
      "mov sp, r3 \n"                        \
      "msr cpsr,r0 \n"                       \ 
      "ldmia sp!,{r0}\n"                     \
      "msr spsr,r0\n"                        \
      "ldmia sp!,{r0-r12,lr}\n"              \
      "movs pc,lr\n"                         \
      :                                      \
      : "m"(duck_context->esp0))

#define interrupt_entering(VEC) interrupt_entering_code(VEC, 0)

#define interrupt_exit()         \
  asm volatile(                  \
      "ldmia sp!,{r1,r2,r3}\n"   \
      "mrs r0,cpsr \n"           \
      "cps #0x1f \n  "           \
      "mov sp, r3 \n"            \
      "msr cpsr,r0 \n"          \ 
      "ldmia sp!,{r0}\n"         \
      "msr spsr,r0\n"            \
      "ldmia sp!,{r0-r12,lr}\n" \
      "movs pc,lr\n"             \
      :                          \
      :)

#endif