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

#elif defined(X86)

#define ISR_TIMER 0x20
#define ISR_KEYBOARD 0x21
#define ISR_SYSCALL 0x30
#define ISR_MOUSE 0x2c


#else defined(V3S)
#define ISR_TIMER 50
#define ISR_KEYBOARD 0x21
#define ISR_SYSCALL 2
#define ISR_MOUSE 0x2c


#endif

#ifdef X86
#define IDT_NUMBER 512
#else

#ifdef ARMV7
#define IDT_NUMBER 64
#else
#define IDT_NUMBER 256
#endif
#endif

#endif