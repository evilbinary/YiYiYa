/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef SYSCALL_H
#define SYSCALL_H
#include "stdint.h"

#define ISR_SYSCALL 0x30

#if defined(ARM) || defined(X86)
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
  SYS_UMASK=60,
  SYS_DUP2 = 63,
  SYS_GETPPID = 64,
  SYS_READDIR=89,
  SYS_STAT=106,
  SYS_FSTAT=108,
  SYS_CLONE= 120,
  SYS_FCHDIR=133,
  SYS_LLSEEK=140,
  SYS_WRITEV=146,
  SYS_READV=145,
  SYS_YIELD = 158,
  SYS_GETCWD=183,
  SYS_MMAP2=192,
  SYS_MPROTECT=125,
  SYS_RT_SIGPROCMASK=175,
  SYS_GETDENTS64=217,
  SYS_MUNMAP=91,
  SYS_FCNT64=221,
  SYS_CLOCK_NANOSLEEP=230,
  SYS_SET_THREAD_AREA=243,
  SYS_PRINT = 400,
  SYS_PRINT_AT = 401,
  SYS_DEV_READ = 402,
  SYS_DEV_WRITE = 403,
  SYS_DEV_IOCTL = 404,
  SYS_TEST = 405,
  SYS_MAP = 406,
  SYS_UMAP = 407,
  SYS_VALLOC = 408,
  SYS_VFREE = 409,
  SYS_VHEAP = 410,
  SYS_DUMPS=411,
  SYS_SELF= 412,
};

#else
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
  SYS_UMASK=60,
  SYS_DUP2 = 63,
  SYS_GETPPID = 64,
  SYS_READDIR=89,
  SYS_STAT=106,
  SYS_FSTAT=108,
  SYS_CLONE= 120,
  SYS_FCHDIR=133,
  SYS_LLSEEK=140,
  SYS_WRITEV=146,
  SYS_READV=145,
  SYS_YIELD = 158,
  SYS_GETCWD=183,
  SYS_MMAP2=192,
  SYS_MPROTECT=125,
  SYS_RT_SIGPROCMASK=175,
  SYS_GETDENTS64=217,
  SYS_MUNMAP=91,
  SYS_FCNT64=221,
  SYS_CLOCK_NANOSLEEP=230,
  SYS_SET_THREAD_AREA=243,
  SYS_PRINT = 400,
  SYS_PRINT_AT = 401,
  SYS_DEV_READ = 402,
  SYS_DEV_WRITE = 403,
  SYS_DEV_IOCTL = 404,
  SYS_TEST = 405,
  SYS_MAP = 406,
  SYS_UMAP = 407,
  SYS_VALLOC = 408,
  SYS_VFREE = 409,
  SYS_VHEAP = 410,
  SYS_DUMPS=411,
  SYS_SELF= 412,
};
#endif

void* syscall0(u32 num);
void* syscall1(u32 num, void* arg0);
void* syscall2(u32 num, void* arg0, void* arg1);
void* syscall3(u32 num, void* arg0, void* arg1, void* arg2);
void* syscall4(u32 num, void* arg0, void* arg1, void* arg2, void* arg3);
void* syscall5(u32 num, void* arg0, void* arg1, void* arg2, void* arg3,
               void* arg4);

#endif