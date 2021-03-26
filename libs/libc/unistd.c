#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <support.h>
#include <unistd.h>

#include "limits.h"
#include "syscall.h"

char **environ = NULL;  ///< Global environment information

int access(const char *filename, int mode) { return 0; }

int chdir(const char *path) {
  int fd;
  int result = -1;

  fd = open(path, O_RDONLY);
  if (fd < 0) return -1;

  result = fchdir(fd);

  close(fd);
  return result;
}

int chown(const char *filename, uid_t uid, gid_t gid) {
  int fd;
  int result = -1;

  fd = open(filename, O_RDWR);
  if (fd < 0) return -1;

  result = fchown(fd, uid, gid);

  close(fd);
  return result;
}

int close(int fd) {
  // Exist?
  int rc;
  // Can close?
  rc = ya_close(fd);

  if (rc != 0) return -1;
  //   support_remove_file(fd);
  // free(file);
  return 0;
}

void _exit(int rc) { exit(rc); }

int fchown(int fd, uid_t uid, gid_t gid) {
  int rc;

  return rc;
}

int fchdir(int fd) {
  int rc;

  return rc;
}

int ftruncate(int fd, off_t offset) {
  int rc;

  return rc;
}

char *getcwd(char *buf, size_t size) {
  int rc;

  errno = rc;
  return NULL;
}

pid_t getpid(void) {
  int pid, rc;

  return (pid_t)pid;
}

gid_t getgid(void) {
  uid_t uid;
  gid_t gid;
  int rc;

  return gid;
}

uid_t getuid(void) {
  uid_t uid;
  gid_t gid;
  int rc;

  return uid;
}

char *getwd(char *buf) {
#warning This is probably not correct
  return getcwd(buf, PATH_MAX);
}

int isatty(int desc) { return 1; }

off_t lseek(int fd, off_t position, int whence) {
  FILE *file = fd;
  if (file == NULL) {
    return (position - 1);
  }

  if (fseek(file, position, whence) == 0) return file->offset;

  return (position - 1);
}

ssize_t read(int fd, void *buffer, size_t num) {
  return ya_read(fd, buffer, num);
}

int rmdir(const char *directory) {
  int rc;

  return rc;
}

unsigned int sleep(unsigned int seconds) {
  unsigned int rc;

  return rc;
}

int truncate(const char *filename, off_t offset) {
  int fd;
  int result = -1;

  fd = open(filename, O_RDWR);
  if (fd < 0) return -1;

  result = ftruncate(fd, offset);

  close(fd);
  return result;
}

ssize_t write(int fd, const void *buffer, size_t num) {
  return ya_write(fd, buffer, num);
}

int unlink(const char *pathname) {
  int rc;

  return rc;
}
