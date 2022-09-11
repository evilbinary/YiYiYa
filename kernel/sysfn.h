/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef SYSFN_H
#define SYSFN_H

#include "kernel.h"
#include "types.h"
#include "kernel/time.h"

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

typedef struct iovec {
  void* iov_base;
  size_t iov_len;
}iovec_t;

u32 sys_open(char* name, int attr);
// size_t sys_ioctl(u32 fd, u32 cmd, ...);
size_t sys_ioctl(u32 fd, u32 cmd, void* args);
int sys_close(u32 fd);
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
size_t sys_seek(u32 fd, ulong offset,int whence);

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

int sys_getdents64(unsigned int fd, vdirent_t* dir, unsigned int count);
int sys_munmap(void* addr, size_t size);
int sys_fcntl64(int fd, int cmd, void* arg);
int sys_getcwd(char *buf, size_t size);
int sys_fchdir(int fd);

int sys_clone(void* stack,void* fn,void* arg);
int sys_llseek(int fd, off_t offset_hi, off_t offset_lo,
                       off_t *result, int whence);

int sys_umask(int mask);

int sys_stat(const char *path, struct stat *buf);
int sys_fstat(int fd, struct stat *buf);
int sys_self(void* t);

#endif