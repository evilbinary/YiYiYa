#include "sys/ioctl.h"

#include "stdarg.h"

int ioctl(int fd, int cmd, ...) {
  void *arg;
  va_list ap;
  va_start(ap, cmd);
  arg = va_arg(ap, void *);
  va_end(ap);
  int ret=ya_ioctl(fd, cmd, arg);
  return ret;
}