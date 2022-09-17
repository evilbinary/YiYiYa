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

typedef struct context_t {
  u32 esp0, ss0, ds0;
  u32 esp, ss, ds;
  u32 eip;
  tss_t* tss;
  u32* page_dir;
  u32* kernel_page_dir;
  u32 level;
} context_t;


typedef struct interrupt_context {
  // manual push
  u32 no;    // 0
  u32 code;  // 4

  u32 pc;  // entry 8
  u32 ps;  // Processor state 12

  u32 a0;   // 16
  u32 sp;   // sp a1 20
  u32 a2;   // 24
  u32 a3;   // 28
  u32 a4;   // 32
  u32 a5;   // 36
  u32 a6;   // 40
  u32 a7;   // 44
  u32 a8;   // 48
  u32 a9;   // 52
  u32 a10;  // 56
  u32 a11;  // 60
  u32 a12;  // 64
  u32 a13;  // 68
  u32 a14;  // 72
  u32 a15;  // 76
  u32 sar;  // 80

  u32 exccause;  // Exception Causes 84
  u32 excvaddr;  // 88

} __attribute__((packed)) interrupt_context_t;

#define interrupt_process(X) kprintf("errro\n");

#define interrupt_entering_code(VEC, CODE) \
  asm volatile(                            \
      "s32i a2,sp,24 \n"                   \
      "s32i a3,sp,28 \n"                   \
      "s32i a4,sp,32 \n"                   \
      "s32i a5,sp,36 \n"                   \
      "s32i a6,sp,40 \n"                   \
      "s32i a7,sp,44 \n"                   \
      "s32i a8,sp,48 \n"                   \
      "s32i a9,sp,52 \n"                   \
      "s32i a10,sp,56 \n"                  \
      "s32i a11,sp,60 \n"                  \
      "s32i a12,sp,64 \n"                  \
      "s32i a13,sp,68 \n"                  \
      "s32i a14,sp,72 \n"                  \
      "s32i a15,sp,76 \n"                  \
      "movi a2,%0 \n"                      \
      "s32i a2,sp,0 \n"                    \
      "movi a2,%1 \n"                      \
      "s32i a2,sp,4 \n"                    \
      "addmi sp, sp, -88 \n"               \
      :                                    \
      : "i"(VEC), "i"(CODE))

#define interrupt_exit_context(duck_context) \
  asm volatile(                              \
      "l32i sp,%0\n"                         \
      "l32i a2,sp,+24 \n"                    \
      "l32i a3,sp,+28 \n"                    \
      "l32i a4,sp,32 \n"                     \
      "l32i a5,sp,36 \n"                     \
      "l32i a6,sp,40 \n"                     \
      "l32i a7,sp,44 \n"                     \
      "l32i a8,sp,48 \n"                     \
      "l32i a9,sp,52 \n"                     \
      "l32i a10,sp,56 \n"                    \
      "l32i a11,sp,60 \n"                    \
      "l32i a12,sp,64 \n"                    \
      "l32i a13,sp,68 \n"                    \
      "l32i a14,sp,72 \n"                    \
      "l32i a15,sp,76 \n"                    \
      "addmi sp, sp, -88 \n"                 \
      :                                      \
      : "m"(duck_context->esp0), "m"(duck_context->esp))

#define interrupt_entering(VEC) interrupt_entering_code(VEC, 0)

#define interrupt_exit()

#define interrupt_exit2()



#define context_switch_page(page_dir) \
  cpu_set_page(page_dir)  // asm volatile("mov %0, %%cr3" : : "r" (page_dir))

#define context_fn(context) context->a7
#define context_ret(context) context->a0
#define context_set_entry(context,entry) ((interrupt_context_t*)((context)));

#define context_restore(duck_context) interrupt_exit_context(duck_context);



#endif