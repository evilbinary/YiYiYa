#include "sys/stat.h"

#include "fcntl.h"

int mkdir(const char *path, mode_t m) {
  printf("unimpl mkdir\n");

  return 1;
}

int stat(const char *restrict path, struct stat *restrict buf) {
  return fstatat(AT_FDCWD, path, buf, 0);
}

mode_t umask(mode_t t) {
  printf("unimpl umask\n");
  return t;
}

int lstat(const char *restrict path, struct stat *restrict buf) {
  return fstatat(AT_FDCWD, path, buf, AT_SYMLINK_NOFOLLOW);
}

int chmod(const char *path, mode_t mode) {
  printf("unimpl chmod\n");
  return 1;
}