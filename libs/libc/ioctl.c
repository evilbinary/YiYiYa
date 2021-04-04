#include "ioctl.h"

#include "stdarg.h"

int ioctl(int fd, int cmd, ...) {
  va_list args;
  va_start(args, cmd);
  int ret=ya_ioctl(fd, cmd, args);
  va_end(args);
  return ret;
}