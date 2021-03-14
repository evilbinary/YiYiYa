#include <stdio.h>
#include <stdlib.h>
// #include <pthread.h>
// #include <sys/mman.h>
#include <unistd.h>

#include "syscall.h"

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
void* last_heap_addr=NULL;

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
  if(last_heap_addr==NULL){
    last_heap_addr = syscall0(SYS_HEAP);
  }
  last_heap_addr+=PAGE_SIZE*pages;
  char* addr=last_heap_addr;
  memset(addr,0,PAGE_SIZE);
  return addr;
}

int liballoc_free(void* ptr, int pages) {
  // return munmap( ptr, pages * page_size );
  syscall1(SYS_FREE, ptr);
  return 1;
}
