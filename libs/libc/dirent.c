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
  dir->index = -1;
  return dir;
}

int closedir(DIR *dir) {
  if (dir && (dir->fd != -1)) {
    return close(dir->fd);
  } else {
    return -EBADF;
  }
}

struct dirent *readdir(DIR *dirp) {
  static struct dirent ent;
  int ret = ya_readdir(dirp->fd, ++dirp->index, &ent);
  if (ret < 0) {
    errno = -ret;
    memset(&ent, 0, sizeof(struct dirent));
    return NULL;
  }
  if (ret == 0) {
    memset(&ent, 0, sizeof(struct dirent));
    return NULL;
  }
  return &ent;
}