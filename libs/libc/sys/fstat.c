// #include "sys/stat.h"
#include "stdio.h"

#define UNIMPL() printf("unimpl %s:%d %s\n",__FILE__,__LINE__,__FUNCTION__)


int fstat(int fd, struct stat *st) {
  UNIMPL();
  return 0;
}

int fstatat(int fd, const char *restrict path, struct stat *restrict st,
            int flag) {
  UNIMPL();
  return 0;
}
