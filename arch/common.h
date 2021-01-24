/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef CPU_H
#define CPU_H

#include "libs/include/types.h"

void* memcpy(void* /* restrict */ s1, const void* /* restrict */ s2, size_t n);
void* memset(void* s, int c, size_t n);
void kprintf(const char* format, ...);

extern void putch(u8 ch);

#define print_char putch

#endif