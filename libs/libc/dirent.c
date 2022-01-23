#include "dirent.h"

#include <errno.h>
#include <unistd.h>

DIR *opendir(const char *dirname) {
  int fd = open(dirname, O_RDONLY);
  if (fd < 0) {
    return NULL;
  }
  DIR *dir = (DIR *)malloc(sizeof(DIR));
  dir->fd = fd;
  dir->buffer_pos = 0;
  return dir;
}

int closedir(DIR *dir) {
  if (dir && (dir->fd != -1)) {
    int ret = close(dir->fd);
    free(dir);
    return ret;
  } else {
    return -EBADF;
  }
}

struct dirent *readdir(DIR *dir) {
  struct dirent *de;
  if (dir->buffer_pos >= dir->buffer_end) {
    int len = ya_readdir(dir->fd, dir->buffer, sizeof dir->buffer);
    if (len <= 0) {
      if (len < 0 && len != -ENOENT) errno = -len;
      return 0;
    }
    dir->buffer_end = len;
    dir->buffer_pos = 0;
  }
  de = (void *)(dir->buffer + dir->buffer_pos);
  dir->buffer_pos += de->d_reclen;
  return de;
}