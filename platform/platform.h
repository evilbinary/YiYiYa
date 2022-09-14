/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef PLATFORM_H

#ifdef RASPI2

#define ISR_TIMER 0x0f
#define ISR_KEYBOARD 0x21
#define ISR_SYSCALL 2
#define ISR_MOUSE 0x2c

#define MP_ENABLE 1  //多cpu

#define IDT_NUMBER 512
#define EXCEPTION_NUMBER 512

#elif defined(X86)

#define ISR_TIMER 0x20
#define ISR_KEYBOARD 0x21
#define ISR_SYSCALL 0x30
#define ISR_MOUSE 0x2c

#define MP_ENABLE 1  //多cpu

#define IDT_NUMBER 512
#define EXCEPTION_NUMBER 512

#elif defined(STM32F4XX)
#define ISR_TIMER 50
#define ISR_KEYBOARD 0x21
#define ISR_SYSCALL 11
#define ISR_MOUSE 0x2c

#define IDT_NUMBER 256
#define EXCEPTION_NUMBER 32

#elif defined(V3S)
#define ISR_TIMER 50
#define ISR_KEYBOARD 0x21
#define ISR_SYSCALL 2
#define ISR_MOUSE 0x2c

#define IDT_NUMBER 256
#define EXCEPTION_NUMBER 32

#else

#define ISR_TIMER 50
#define ISR_KEYBOARD 0x21
#define ISR_SYSCALL 2
#define ISR_MOUSE 0x2c

#define IDT_NUMBER 256
#define EXCEPTION_NUMBER 32

#endif

#endif