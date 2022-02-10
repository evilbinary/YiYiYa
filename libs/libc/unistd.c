#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <support.h>
#include <unistd.h>

#include "limits.h"
#include "stdarg.h"
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
  rc=ya_fchdir(fd);

  return rc;
}

int ftruncate(int fd, off_t offset) {
  int rc;

  return rc;
}

char *getcwd(char *buf, size_t size) {
  int rc;
  rc=ya_getcwd(buf,size);
  errno = rc;

  if(rc<0){
    return NULL;
  }
  return buf;
}

pid_t getpid(void) { return ya_getpid(); }

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

pid_t fork() { return ya_fork(); }

pid_t getppid(void) { return ya_getppid(); }

int pipe(int fds[2]) { return ya_pipe(fds); }

int dup(int fd) { return ya_dup(fd); }
int dup2(int oldfd, int newfd) { return ya_dup2(oldfd, newfd); }

int execve(const char *pathname, char *const argv[], char *const envp[]) {
  return ya_exec(pathname, argv, envp);
}

int execle(const char *path, const char *argv0, ...) {
  int argc;
  va_list ap;
  va_start(ap, argv0);
  for (argc = 1; va_arg(ap, const char *); argc++)
    ;
  va_end(ap);
  {
    int i;
    char *argv[argc + 1];
    char **envp;
    va_start(ap, argv0);
    argv[0] = (char *)argv0;
    for (i = 1; i <= argc; i++) argv[i] = va_arg(ap, char *);
    envp = va_arg(ap, char **);
    va_end(ap);
    return execve(path, argv, envp);
  }
}

int execv(const char *pathname, char *const argv[]) {
  return execve(pathname, argv, environ);
}

int execl(const char *path, const char *argv0, ...) {
  int argc;
  va_list ap;
  va_start(ap, argv0);
  for (argc = 1; va_arg(ap, const char *); argc++)
    ;
  va_end(ap);
  {
    int i;
    char *argv[argc + 1];
    va_start(ap, argv0);
    argv[0] = (char *)argv0;
    for (i = 1; i < argc; i++) argv[i] = va_arg(ap, char *);
    argv[i] = NULL;
    va_end(ap);
    return execv(path, argv);
  }
}

int execvp(const char *file, char *const argv[]) {
  char *const envp=environ;
  const char *p, *z, *path = getenv("PATH");
  size_t l, k;
  int seen_eacces = 0;

  errno = ENOENT;
  if (!*file) return -1;

  if (strchr(file, '/')) return execve(file, argv, envp);

  if (!path) path = "/usr/local/bin:/bin:/usr/bin";
  k = strnlen(file, NAME_MAX + 1);
  if (k > NAME_MAX) {
    errno = ENAMETOOLONG;
    return -1;
  }
  l = strnlen(path, PATH_MAX - 1) + 1;

  for (p = path;; p = z) {
    char b[l + k + 1];
    z = strchrnul(p, ':');
    if (z - p >= l) {
      if (!*z++) break;
      continue;
    }
    memcpy(b, p, z - p);
    b[z - p] = '/';
    memcpy(b + (z - p) + (z > p), file, k + 1);
    execve(b, argv, envp);
    switch (errno) {
      case EACCES:
        seen_eacces = 1;
      case ENOENT:
      case ENOTDIR:
        break;
      default:
        return -1;
    }
    if (!*z++) break;
  }
  if (seen_eacces) errno = EACCES;
  return -1;
  // return execv(filename, argv);
}

int execlp(const char *file, const char *argv0, ...)
{
	int argc;
	va_list ap;
	va_start(ap, argv0);
	for (argc=1; va_arg(ap, const char *); argc++);
	va_end(ap);
	{
		int i;
		char *argv[argc+1];
		va_start(ap, argv0);
		argv[0] = (char *)argv0;
		for (i=1; i<argc; i++)
			argv[i] = va_arg(ap, char *);
		argv[i] = NULL;
		va_end(ap);
		return execvp(file, argv);
	}
}
