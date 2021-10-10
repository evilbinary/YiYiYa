/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef SYSCALL_H
#define SYSCALL_H
#include "stdint.h"

#define ISR_SYSCALL 0x30

#if defined(ARM)|| defined(X86)
enum {
 SYS_EXIT = 1,
  SYS_FORK = 2,
  SYS_READ = 3,
  SYS_WRITE = 4,
  SYS_OPEN = 5,
  SYS_CLOSE = 6,
  SYS_UNLINK = 10,
  SYS_EXEC = 11,
  SYS_CHDIR= 12,
  SYS_SEEK = 19,
  SYS_GETPID = 20,
  SYS_ALARM = 27,
  SYS_RENAME=38,
  SYS_PIPE = 42,
  SYS_DUP = 41,
  SYS_BRK = 45,
  SYS_IOCTL = 54,
  SYS_DUP2 = 63,
  SYS_GETPPID = 64,
  SYS_WRITEV=146,
  SYS_READV=145,
  SYS_READDIR,
  SYS_YIELD = 158,
  SYS_PRINT = 500,
  SYS_PRINT_AT = 501,
  SYS_DEV_READ = 502,
  SYS_DEV_WRITE = 503,
  SYS_DEV_IOCTL = 504,
  SYS_TEST = 505,
  SYS_MAP = 506,
  SYS_UMAP = 507,
  SYS_VALLOC = 508,
  SYS_VFREE = 509,
  SYS_VHEAP = 510,
  SYS_MMAP2=192,
  SYS_MPROTECT=125,
  SYS_RT_SIGPROCMASK=175,
  

};

#else
enum {
  SYS_READ,
  SYS_WRITE,
  SYS_YIELD,
  SYS_IOCTL,
  SYS_OPEN,
  SYS_CLOSE,
  SYS_CHDIR,
  SYS_DEV_READ,
  SYS_DEV_WRITE,
  SYS_DEV_IOCTL,
  SYS_EXEC,
  SYS_TEST,
  SYS_EXIT,
  SYS_MAP,
  SYS_UMAP,
  SYS_SEEK,
  SYS_VALLOC,
  SYS_VFREE,
  SYS_VHEAP,
  SYS_FORK,
  SYS_PIPE,
  SYS_GETPID,
  SYS_GETPPID,
  SYS_DUP,
  SYS_DUP2,
  SYS_READDIR,
  SYS_RENAME,
  SYS_BRK,
  SYS_PRINT,
  SYS_PRINT_AT,
  SYS_WRITEV,
  SYS_READV
};
#endif

void* syscall0(u32 num);
void* syscall1(u32 num,void* arg0);
void* syscall2(u32 num,void* arg0,void* arg1);
void* syscall3(u32 num,void* arg0,void* arg1,void* arg2);
void* syscall4(u32 num, void* arg0, void* arg1, void* arg2,void* arg3);
void* syscall5(u32 num, void* arg0, void* arg1, void* arg2,void* arg3,void* arg4);

#endif