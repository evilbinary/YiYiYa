#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <support.h>
#include <unistd.h>
#include "syscall.h"

int open(const char *filename, int flags, ...) {
  FILE *file;
  int fd;
  va_list vargs;
  mode_t mode;

  va_start(vargs, flags);
  mode = va_arg(vargs, mode_t);
  va_end(vargs);

  fd = ya_open(filename, flags);

  if (fd < 0) return -1;
  return fd;
}

int creat(const char *filename, u32 mode) {
  return open(filename, O_CREAT | O_WRONLY | O_TRUNC);
}