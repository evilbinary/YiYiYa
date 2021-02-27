/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef SYSCALL_H
#define SYSCALL_H

#include "arch/arch.h"

#define SYSCALL_NUMBER 20

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
    DEV_IOCTL
};

void* syscall0(u32 num);
void* syscall1(u32 num,void* arg0);
void* syscall2(u32 num,void* arg0,void* arg1);
void* syscall3(u32 num,void* arg0,void* arg1,void* arg2);


void syscall_init();


u32 sys_open(char* name, int attr);
size_t sys_ioctl(int fd, u32 cmd,...);
void sys_close(int fd);
size_t sys_write(int fd, void* buf, size_t nbytes);
size_t sys_read(int fd, void* buf, size_t nbytes);

#endif