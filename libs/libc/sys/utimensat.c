#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "sys/stat.h"
#include "time.h"

int utimensat(int fd, const char *path, const struct timespec times[2],
              int flags) {
  printf("unimpl utimensat\n");
  return 1;
}
