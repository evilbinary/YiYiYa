#include "sys/stat.h"

int fstat(int fd, struct stat *st) {
  printf("unimpl fstat\n");
  return 1;
}

int fstatat(int fd, const char *restrict path, struct stat *restrict st,
            int flag) {
  printf("unimpl fstatat\n");
  return 1;
}
