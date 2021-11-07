/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef CONTEXT_H
#define CONTEXT_H

#include "arch/boot.h"
#include "libs/include/types.h"
#include "platform/platform.h"

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
  u32 r11;  // fp

  u32 sp;  // r13

  // auto push
  u32 r0;
  u32 r1;
  u32 r2;
  u32 r3;

  u32 r12;  // ip
  u32 lr;   // r14
  u32 pc;   // r15
  u32 psr;

} __attribute__((packed)) interrupt_context_t;

#define interrupt_process(X) \
  asm volatile(              \
      "push {r0-r12} \n"     \
      "blx " #X              \
      "\n"                   \
      "pop {r0-r12}\n"       \
      :                      \
      :)

#define interrupt_entering_code(VEC, CODE) \
  asm volatile(                            \
      "mrs r0, psp\n"                      \
      "mov r1, r0\n"                       \
      "stmfd r0!,{r1}\n"                   \
      "stmfd r0!, {r4-r11}\n"              \
      "mov r1,%0\n"                        \
      "mov r2,%1\n"                        \
      "stmfd r0!, {r1,r2} \n"              \
      "mov r0,sp\n"                        \
      :                                    \
      : "i"(VEC), "i"(CODE))

#define interrupt_exit_context(duck_context)   \
  asm volatile(                              \ 
      "ldr r0,%0 \n"                           \
      "ldmfd r0!,{r1,r2}\n"                    \
      "ldmfd r0!,{r4-r11}\n"                   \
      "ldmfd r0!,{r1}\n"                       \
      "ldr lr,[r0,#20 ] \n"                    \
      "msr psp, r1\n"                          \
      "bx lr\n"                                \
                                             : \
                                             : "m"(duck_context->esp0))

#define interrupt_entering(VEC) interrupt_entering_code(VEC, 0)

#define interrupt_exit()     \
  asm volatile(              \
      "ldmfd r0!,{r1,r2}\n"  \
      "ldmfd r0!,{r4-r11}\n" \
      "ldmfd r0!,{r1}\n"     \
      "msr psp, r1\n"        \
      "subs pc,lr,#4\n"      \
      "bx lr\n"              \
      :                      \
      :)

#define interrupt_exit2() interrupt_exit()

#endif