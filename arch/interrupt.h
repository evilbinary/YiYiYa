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
  #include "arm/context.h"
#elif defined(X86)
  #include "x86/context.h"
#else 
  
#endif

typedef void (*interrupt_handler_t)(interrupt_context_t* context);

void timer_init(int hz);

#define INTERRUPT_SERVICE __attribute__((naked))

void interrutp_regist(u32 vec, interrupt_handler_t handler);

void interrupt_init();

#endif