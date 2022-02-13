#include "sys/stat.h"
#include "fcntl.h"

int mkdir(const char *path, mode_t m) {
  printf("unimpl mkdir\n");

  return 1;
}

int stat(const char *restrict path, struct stat *restrict buf) {
  return fstatat(AT_FDCWD, path, buf, 0);
}