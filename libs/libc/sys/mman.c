#include "sys/mman.h"

#include "sys/types.h"

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset) {
  printf("unimpl mmap\n");
  return NULL;
}
int munmap(void *addr, size_t length) {
  printf("unimpl munmap\n");
  return 1;
}

int mprotect(void *addr, size_t length, int prot) {
  printf("unimpl mprotect\n");
  return 1;
}
int msync(void *start, size_t len, int flags) {
  printf("unimpl msync\n");
  return 1;
}