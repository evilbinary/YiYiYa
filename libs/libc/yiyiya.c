#include <stdio.h>
#include <stdlib.h>
// #include <pthread.h>
// #include <sys/mman.h>
#include <unistd.h>

#include "syscall.h"
#include "time.h"
#include "yiyiya.h"

#define PAGE_SIZE 4096

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#if !defined(MAP_FAILED)
#define MAP_FAILED ((char*)-1)
#endif

#ifndef MAP_NORESERVE
#ifdef MAP_AUTORESRV
#define MAP_NORESERVE MAP_AUTORESRV
#else
#define MAP_NORESERVE 0
#endif
#endif

typedef struct free_block {
  size_t size;
  struct free_block* next;
} free_block_t;

// static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int page_size = -1;
void* last_heap_addr = NULL;

u32 time_fd=-1;

static free_block_t free_block_list_head = {0, 0};
static const size_t align_to = 16;

void* ya_sbrk(size_t size) {
  if (last_heap_addr == NULL) {
    last_heap_addr = syscall0(SYS_VHEAP);
  }
  void* addr = last_heap_addr;
  last_heap_addr += size;
  return addr;
}

void* ya_alloc(size_t size) {
  size_t s = size;
  size = (size + sizeof(free_block_t) + (align_to - 1)) & ~(align_to - 1);
  free_block_t* block = free_block_list_head.next;
  free_block_t** head = &(free_block_list_head.next);
  while (block != 0) {
    if (block->size >= size) {
      *head = block->next;
      void* addr=((char*)block) + sizeof(free_block_t);
      // memset(addr,0,s);
      return addr;
    }
    head = &(block->next);
    block = block->next;
  }

  block = (free_block_t*)ya_sbrk(size);
  block->size = size;

  void* addr = ((char*)block) + sizeof(free_block_t);
  // memset(addr,0,s);
  return addr;
}

void ya_free(void* ptr) {
  free_block_t* block = (free_block_t*)(((char*)ptr) - sizeof(free_block_t));
  block->next = free_block_list_head.next;
  free_block_list_head.next = block;  
}

void* ya_valloc(void* addr, size_t size) {
  return syscall2(SYS_VALLOC, addr, size);
}

void ya_vfree(void* addr) { syscall1(SYS_VFREE, addr); }

u32 ya_open(const char* filename, int flags) {
  return syscall2(SYS_OPEN, filename, flags);
}

u32 ya_close(int fd) { return syscall1(SYS_CLOSE, fd); }

size_t ya_write(u32 fd, void* buf, size_t nbytes) {
  return syscall3(SYS_WRITE, fd, buf, nbytes);
}
size_t ya_read(u32 fd, void* buf, size_t nbytes) {
  return syscall3(SYS_READ, fd, buf, nbytes);
}

size_t ya_seek(u32 fd, ulong offset) { return syscall2(SYS_SEEK, fd, offset); }

uint32_t secs_of_years(int years) {
  uint32_t days = 0;
  years += 2000;
  while (years > 1969) {
    days += 365;
    if (years % 4 == 0) {
      if (years % 100 == 0) {
        if (years % 400 == 0) {
          days++;
        }
      } else {
        days++;
      }
    }
    years--;
  }
  return days * 86400;
}

uint32_t secs_of_month(int months, int year) {
  year += 2000;

  uint32_t days = 0;
  switch (months) {
    case 11:
      days += 30;
    case 10:
      days += 31;
    case 9:
      days += 30;
    case 8:
      days += 31;
    case 7:
      days += 31;
    case 6:
      days += 30;
    case 5:
      days += 31;
    case 4:
      days += 30;
    case 3:
      days += 31;
    case 2:
      days += 28;
      if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))) {
        days++;
      }
    case 1:
      days += 31;
    default:
      break;
  }
  return days * 86400;
}

u32 ya_time(time_t* current) {
  if(time_fd==-1){
    time_fd= ya_open("/dev/time", 0);
  }
  if(time_fd<0) return 0;
  rtc_time_t time;
  time.day=1;
  time.hour=0;
  time.minute=0;
  time.month=1;
  time.second=0;
  time.year=1900;
  int ret = ya_read(time_fd, &time, sizeof(rtc_time_t));
  if(ret<0){
    return 0;
  }
  uint32_t seconds = secs_of_years(time.year - 1) +
                     secs_of_month(time.month - 1, time.year) +
                     (time.day - 1) * 86400 + time.hour * 3600 +
                     time.minute * 60 + time.second + 0;
  *current = seconds;
  return ret;
}

int ya_ioctl(int fd, int cmd, void* args) {
  return syscall3(SYS_IOCTL, fd, cmd, args);
}

int ya_fork() { return syscall0(SYS_FORK); }

int ya_getpid() { return syscall0(SYS_GETPID); }

int ya_getppid() { return syscall0(SYS_GETPPID); }

int ya_pipe(int fds[2]) { return syscall1(SYS_PIPE, fds); }

int ya_dup(int fd) { return syscall1(SYS_DUP, fd); }
int ya_dup2(int oldfd, int newfd) { return syscall2(SYS_DUP2, oldfd, newfd); }

int ya_exec(const char* pathname, char* const argv[], char* const envp[]) {
  return syscall3(SYS_EXEC, pathname, argv, envp);
}

void ya_exit(u32 ret){
  syscall1(SYS_EXIT, ret);
}

int ya_readdir(int fd,int index,struct dirent* dirent){
  return syscall3(SYS_READDIR, fd,index,dirent);
}

int ya_rename(const char *old, const char *new){
  return syscall2(SYS_RENAME,old,new);
}

void __main(){
  //printf("main call\n");
}