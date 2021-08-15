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

#define IDT_NUMBER 256

typedef struct interrupt_context {
  // manual push
  u32 no;
  u32 code;

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
  u32 r11;  // fp
  u32 r12;  // ip
  u32 lr0;

  u32 sp;  // sp
  u32 lr;  // r14

} __attribute__((packed)) interrupt_context_t;

#define interrupt_process(X) 

#define interrupt_entering_code(VEC, CODE) 

#define interrupt_exit_context(duck_context) 

#define interrupt_entering(VEC) interrupt_entering_code(VEC, 0)

#define interrupt_exit()        

#define interrupt_exit2()         

#endif