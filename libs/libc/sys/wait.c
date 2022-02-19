#include "sys/wait.h"

pid_t wait(int *status) { return waitpid((pid_t)-1, status, 0); }

pid_t waitpid(pid_t pid, int *status, int options) {
  printf("unimpl waitpid\n");
  return 1;
}