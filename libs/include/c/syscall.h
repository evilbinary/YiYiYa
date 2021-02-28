/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef SYSCALL_H
#define SYSCALL_H
#include "stdint.h"

#define ISR_SYSCALL 0x30

enum {
    SYS_READ=0,
    SYS_WRITE,
    SYS_YIELD,
    SYS_PRINT,
    SYS_PRINT_AT,
    SYS_IOCTL,
    SYS_OPEN,
    SYS_CLOSE,
    DEV_READ,
    DEV_WRITE,
    DEV_IOCTL,
    SYS_EXEC,
    SYS_TEST
};

void* syscall0(u32 num);
void* syscall1(u32 num,void* arg0);
void* syscall2(u32 num,void* arg0,void* arg1);
void* syscall3(u32 num,void* arg0,void* arg1,void* arg2);


#endif