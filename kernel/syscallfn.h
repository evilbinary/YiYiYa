/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef SYSCALLFN_H
#define SYSCALLFN_H

#include "kernel.h"

typedef struct exec{
    char* filename;
    char** argv;
}exec_t;

typedef int (*entry_fn)(int, char**);

u32 sys_open(char* name, int attr);
size_t sys_ioctl(u32 fd, u32 cmd,...);
void sys_close(u32 fd);
size_t sys_write(u32 fd, void* buf, size_t nbytes);
size_t sys_read(u32 fd, void* buf, size_t nbytes);
size_t sys_yeild(thread_t* thread);

int sys_print(char* s);
int sys_print_at(char* s, u32 x, u32 y);
u32 sys_exec(char* filename,char *const argv[]);
void sys_test();


#endif