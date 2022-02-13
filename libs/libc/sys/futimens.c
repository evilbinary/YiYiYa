#include "sys/stat.h"
#include "time.h"

int futimens(int fd, const struct timespec times[2]) {
  return utimensat(fd, 0, times, 0);
}
