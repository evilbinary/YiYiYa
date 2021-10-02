/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "boot.h"
#include "libs/include/types.h"


#ifdef ARM
  #ifdef ARMV7_A
    #include "armv7-a/context.h"
  #elif defined(ARMV7)
    #include "armv7/context.h"
  #elif ARMV5
    #include "armv5/context.h"
  #else
    #error "no support arm"
  #endif
#elif defined(X86)
  #include "x86/context.h"
#elif defined(XTENSA)
  #include "xtensa/context.h"
#else
    #error "no support"
#endif

typedef void (*interrupt_handler_t)(interrupt_context_t* context);

void timer_init(int hz);

#define INTERRUPT_SERVICE __attribute__((naked))

void interrutp_regist(u32 vec, interrupt_handler_t handler);

void interrupt_init();

#endif