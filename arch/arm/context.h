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
#define ISR_SYSCALL 0x30
#define ISR_MOUSE 0x2c

#define IDT_NUMBER 256

typedef struct interrupt_context {
  // manual push
  u32 no;
  u32 code;

  u32 r4;
  u32 r5;
  u32 r6;
  u32 r7;
  u32 r8;
  u32 r9;
  u32 r10;
  u32 r11;
  u32 r12;

  u32 psr;

  u32 r0;
  u32 r1;
  u32 r2;
  u32 r3;

  u32 lr;
  // u32 pc;
} __attribute__((packed)) interrupt_context_t;

#define interrupt_process(X) asm volatile("bl  " #X " \n")

#define interrupt_entering_code(VEC, CODE) \
  asm volatile(                            \
      "stmdb sp!, {lr} \n"                 \
      "stmdb sp!, {r0-r3} \n"              \
      "mrs r0,cpsr\n"                      \
      "stmdb sp!, {r0} \n"                 \
      "stmdb sp!, {r4-r12} \n"             \
      "mov r1,%0\n"                        \
      "mov r2,%1\n"                        \
      "stmdb sp!, {r1,r2} \n"              \
      "mov r0,sp\n"                        \
      :                                    \
      : "i"(VEC), "i"(CODE))

#define interrupt_exit_context(context) \
  asm volatile(                         \
      "ldr sp,%0 \n"                    \
      "ldmia sp!,{r0-r1}\n"             \
      "ldmia sp!,{r4-r12}\n"            \
      "ldmia sp!,{r0}\n"                \
      "msr spsr_cxsf,r0\n"                   \
      "ldmia sp!,{r0-r3}\n"             \
      "ldmia sp!,{lr}\n"                \
      "bx lr \n"                        \
      :                                 \
      : "m"(context->esp))

#define interrupt_entering(VEC)  \
  asm volatile(                  \
      "stmdb sp!, {r0-r3} \n"    \
      "mrs r0,cpsr\n"            \
      "stmdb sp!, {r0,pc,lr} \n" \
      "stmdb sp!, {r4-r11} \n"   \
      "mov r1,%0\n"              \
      "mov r2,#0 \n"             \
      "stmdb sp!, {r1,r2} \n"    \
      "mov r0,sp\n"              \
      :                          \
      : "i"(VEC))

#define interrupt_exit()     \
  asm volatile(              \
      "ldmia sp!,{r0-r1}\n"  \
      "ldmia sp!,{r4-r11}\n" \
      "ldmia sp!,{pc,lr}\n"  \
      "ldmia sp!,{r0}\n"     \
      "msr cpsr,r0\n"        \
      "ldmia sp!,{r0-r3}\n"  \
      "bx lr \n"             \
      :                      \
      :)

// #define interrupt_exit_context(context) \
//   asm volatile(                         \
//       "ldr r0,%0\n"                     \
//       "ldmia r0!,{r4-r11}\n"            \
//       "mrs r1,control\n"                \
//       "orr r1,r1,#2 \n"                 \
//       "msr control,r1 \n"               \
//       "msr psp,r0\n"                    \
//       "isb \n"                          \
//       "bx lr \n"                        \
//       :                                 \
//       : "m"(context->esp))

#endif