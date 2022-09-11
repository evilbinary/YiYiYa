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

#ifdef ARM
  #ifdef ARMV7_A
    #include "armv7-a/cpu.h"
  #elif defined(ARMV7)
    #include "armv7/cpu.h"
  #elif defined(ARMV5)
    #include "armv5/cpu.h"
  #elif defined(ARMV8_A)
    #include "armv8-a/cpu.h"
  #else
    #error "no support arm"
  #endif
#elif defined(X86)
  #ifdef X86_DUCK
    #include "x86-duck/cpu.h"
  #else
    #include "x86/cpu.h"
  #endif
#elif defined(LX6)
  #include "lx6/cpu.h"
#else
    #error "no support"
#endif


typedef uint32_t phys_address_t;
typedef uint32_t virtual_address_t;

typedef struct stack_frame {
  struct stack_frame* prev;
  void* return_addr;
} __attribute__((packed)) stack_frame_t;

void cpu_init();
void cpu_halt();

#define KERNEL_MODE 0
#define USER_MODE 3
#define GET_CPL(x) (((x)&0x03))  //0-3


#define cpu_faa(ptr) __sync_fetch_and_add(ptr, 1)

int cpu_tas(volatile int* addr, int newval);

#endif