/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef SYSFN_H
#define SYSFN_H

#include "kernel.h"

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
  SYS_READDIR=89,
  SYS_WRITEV=146,
  SYS_READV=145,
  SYS_YIELD = 158,
  SYS_SET_THREAD_AREA=243,
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
  SYS_DUMPS=511,
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
  SYS_BRK,
  SYS_PRINT,
  SYS_PRINT_AT,
  SYS_WRITEV,
  SYS_READV,
  SYS_MMAP2,
  SYS_MPROTECT,
  SYS_DUMPS,
  SYS_RT_SIGPROCMASK
};
#endif

typedef struct iovec {
  void* iov_base;
  size_t iov_len;
}iovec_t;

u32 sys_open(char* name, int attr);
// size_t sys_ioctl(u32 fd, u32 cmd, ...);
size_t sys_ioctl(u32 fd, u32 cmd, void* args);
void sys_close(u32 fd);
size_t sys_write(u32 fd, void* buf, size_t nbytes);
size_t sys_read(u32 fd, void* buf, size_t nbytes);
size_t sys_yeild();

int sys_print(char* s);
int sys_print_at(char* s, u32 x, u32 y);
u32 sys_exec(char* filename, char* const argv[], char* const envp[]);
void sys_test();
void sys_exit(int status);

void* sys_vmap(void* addr, size_t size);

void sys_vumap(void* ptr, size_t size);

void* sys_alloc_alignment(size_t size, u32 alignment);

void sys_free_alignment(void* ptr);
size_t sys_seek(u32 fd, ulong offset);

void* sys_valloc(void* addr, size_t size);
void sys_vfree(void* addr);

void* sys_vheap();

int sys_fork();
int sys_pipe(int fd[2]);

int sys_getpid();

int sys_getppid();

int sys_dup(int oldfd);

int sys_dup2(int oldfd, int newfd);
int sys_readdir(int fd, int index, void* dirent);
int sys_brk(int addr);

int sys_readv(int fd, iovec_t* vector, int count);
int sys_writev(int fd, iovec_t* vector, int count);
int sys_chdir(const char *path);

void* sys_mmap2(void* addr, int length, int prot, int flags, int fd,
                int pgoffset);

int sys_mprotect(const void *start, size_t len, int prot);

int sys_rt_sigprocmask(int, void *set, void* old_set);


unsigned int sys_alarm(unsigned int seconds);

int sys_unlink(const char * pathname);
int sys_rename(const char* old,const char* new);

int sys_set_thread_area(void* set);

void sys_dumps();

#endif