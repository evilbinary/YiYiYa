#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <support.h>
#include <unistd.h>
#include "syscall.h"

int open(const char *filename, int flags, ...) {
  void *data;
  FILE *file;
  int fd;
  va_list vargs;
  mode_t mode;

  va_start(vargs, flags);
  mode = va_arg(vargs, mode_t);
  va_end(vargs);

  fd = syscall2(SYS_OPEN, filename, flags);

  if (fd < 0) return -1;

  file = (FILE *)malloc(sizeof(FILE));
  file->data = data;
  file->fd = fd;
  file->eof = 0;
  file->error = 0;
  file->offset = 0;
  file->mode = flags;
 

  return file;
}

int creat(const char *filename, u32 mode) {
  return open(filename, O_CREAT | O_WRONLY | O_TRUNC);
}