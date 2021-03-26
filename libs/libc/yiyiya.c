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

// static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int page_size = -1;
void* last_heap_addr = NULL;
u32 time_fd;

int liballoc_lock() {
  // pthread_mutex_lock( &mutex );
  return 0;
}

int liballoc_unlock() {
  // pthread_mutex_unlock( &mutex );
  return 0;
}

void* liballoc_alloc(int pages) {
  // if ( page_size < 0 ) page_size = getpagesize();
  // unsigned int size = pages * page_size;

  // char *p2 = (char*)mmap(0, size, PROT_NONE,
  // MAP_PRIVATE|MAP_NORESERVE|MAP_ANONYMOUS, -1, 0); if ( p2 == MAP_FAILED)
  // return NULL;

  // if(mprotect(p2, size, PROT_READ|PROT_WRITE) != 0)
  // {
  // 	munmap(p2, size);
  // 	return NULL;
  // }
  if (last_heap_addr == NULL) {
    last_heap_addr = syscall0(SYS_VHEAP);
  }
  last_heap_addr += PAGE_SIZE * pages;
  char* addr = last_heap_addr;
  memset(addr,0,PAGE_SIZE);
  return addr;
}

int liballoc_free(void* ptr, int pages) {
  // return munmap( ptr, pages * page_size );
  for (int i = 0; i < pages; i++) {
    syscall1(SYS_VFREE, ptr);
    ptr += PAGE_SIZE;
  }

  return 1;
}

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
  if (time_fd == NULL) {
    time_fd = ya_open("/dev/time", 0);
  }
  rtc_time_t time;
  int ret = ya_read(time_fd, &time, sizeof(rtc_time_t));
  uint32_t seconds = secs_of_years(time.year - 1) +
                  secs_of_month(time.month - 1, time.year) +
                  (time.day - 1) * 86400 +
                  time.hour * 3600 + time.minute * 60 +
                  time.second + 0;
  *current = seconds;
  return ret;
}