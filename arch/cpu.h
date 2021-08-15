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
  #include "arm/cpu.h"
#elif defined(X86)
  #include "x86/cpu.h"
#elif defined(XTENSA)
  #include "xtensa/cpu.h"
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


void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level);

void context_switch(interrupt_context_t* context,context_t** current, 
                    context_t* next_context);



#define FAA(ptr) __sync_fetch_and_add(ptr, 1)

int TAS(volatile int* addr, int newval);

#endif