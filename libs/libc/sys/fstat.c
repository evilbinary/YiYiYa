// #include "sys/stat.h"
#include "stdio.h"

int fstat(int fd, struct stat *st) {
  UNIMPL();
  return 0;
}

int fstatat(int fd, const char *restrict path, struct stat *restrict st,
            int flag) {
  UNIMPL();
  return 0;
}
