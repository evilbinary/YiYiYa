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
      "stmfd r0!, {r4-r11}\n"              \ 
      "mov r1,%0\n"                        \
      "mov r2,%1\n"                        \
      "stmfd r0!, {r1,r2} \n"              \
      :                                    \
      : "i"(VEC), "i"(CODE))

#define interrupt_exit_ret()   \
  asm volatile(                              \ 
      "mov r0,r0 \n"                           \
      "ldmfd r0!,{r1,r2}\n"                    \
      "ldmfd r0!,{r4-r11}\n"                   \
      "msr psp, r0\n"                          \
      "mov lr,#0xFFFFFFFD\n"                   \
      "isb\n"                                  \
      "bx lr\n"                                \
                                             : \
                                             : )

#define interrupt_exit_context(duck_context)   \
  asm volatile(                              \ 
      "ldr r0,%0 \n"                           \
      "ldmfd r0!,{r1,r2}\n"                    \
      "ldmfd r0!,{r4-r11}\n"                   \
      "msr psp, r0\n"                          \
      "mov lr,#0xFFFFFFFD\n"                   \
      "isb\n"                                  \
      "bx lr\n"                                \
                                             : \
                                             : "m"(duck_context->esp))

#define interrupt_entering(VEC) interrupt_entering_code(VEC, 0)

#define interrupt_exit()     \
  asm volatile(              \
      "ldmfd r0!,{r1,r2}\n"  \
      "ldmfd r0!,{r4-r11}\n" \
      "msr psp, r0\n"        \
      "ldr lr,[r0,#20 ]\n"   \
      "mov lr,#0xFFFFFFFD\n" \
      "isb\n"                \
      "bx lr\n"              \
      :                      \
      :)

#define interrupt_exit2() interrupt_exit()


#define context_switch_page(page_dir) \
  cpu_set_page(page_dir)  // asm volatile("mov %0, %%cr3" : : "r" (page_dir))

#define context_fn(context) context->r6
#define context_ret(context) context->r0
#define context_set_entry(context, entry) \
  ((interrupt_context_t*)((context)->esp0))->lr = entry + 4;
  
#define context_restore(duck_context)          \
  asm volatile(                              \ 
      "ldr r0,%0 \n"                           \
      "ldmfd r0!,{r1,r2}\n"                    \
      "ldmfd r0!,{r4-r11}\n"                   \
      "ldr lr,[r0,#20 ]\n"                     \
      "ldr r3,[r0,#24 ]\n"                     \
      "msr psp, r0\n"                          \
      "isb\n"                                  \
      "mov r0, #2\n"                           \
      "msr control, r0\n"                      \
      "cpsie i\n"                              \
      "bx r3\n"                                \
                                             : \
                                             : "m"(duck_context->esp))
// interrupt_exit_context(duck_context)

void context_clone(context_t* context, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3);
void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level, int cpu);
void context_dump(context_t* c);



#endif