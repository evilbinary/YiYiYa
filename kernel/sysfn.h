/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef SYSFN_H
#define SYSFN_H

#include "kernel.h"

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
    SYS_TEST,
    SYS_EXIT,
    SYS_MAP,
    SYS_UMAP,
    SYS_SEEK,
    SYS_VALLOC,
    SYS_VFREE,
    SYS_VHEAP
};

typedef struct exec{
    char filename[256];
    char** argv;
}exec_t;

typedef int (*entry_fn)(int, char**);

u32 sys_open(char* name, int attr);
size_t sys_ioctl(u32 fd, u32 cmd,...);
void sys_close(u32 fd);
size_t sys_write(u32 fd, void* buf, size_t nbytes);
size_t sys_read(u32 fd, void* buf, size_t nbytes);
size_t sys_yeild();

int sys_print(char* s);
int sys_print_at(char* s, u32 x, u32 y);
u32 sys_exec(char* filename,char *const argv[]);
void sys_test();
void sys_exit(int status);

void* sys_vmap(void* addr,size_t size);

void sys_vumap(void* ptr,size_t size);

void* sys_alloc_alignment(size_t size,u32 alignment);

void sys_free_alignment(void* ptr);
size_t sys_seek(u32 fd, ulong offset);

void* sys_valloc(void* addr,size_t size);
void sys_vfree(void* addr);

void* sys_vheap();

#endif