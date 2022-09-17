#include "fcntl.h"
#include "stdio.h"
#include "sys/stat.h"

#define UNIMPL() printf("unimpl %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__)

int fstat(int fd, struct stat *st) { return ya_fstat(fd, stat); }

int fstatat(int fd, const char *restrict path, struct stat *restrict st,
            int flag) {
  int ret = 0;
  if (flag == AT_EMPTY_PATH && fd >= 0 && !*path) {
    ret = ya_fstat(fd, st);
  } else if ((fd == AT_FDCWD || *path == '/') && flag == AT_SYMLINK_NOFOLLOW) {
    ret = lstat(path, st);
  } else if ((fd == AT_FDCWD || *path == '/') && !flag) {
    ret = ya_stat(path, st);
  } else {
    //ret = ya_fstatat(fd, path, st, flag);
    UNIMPL();
  }

  return ret;
}
