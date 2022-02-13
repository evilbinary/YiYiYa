#include <signal.h>

int raise(int sig) {
  int rc;
  printf("unimpl raise\n");
  return 1;
  // if ( os_signal( sig, &rc ) == NOTIMPLEMENTED )
  //		os_freakout();

  return rc;
}

int kill(pid_t pid, int attr) {
  printf("unimpl kill\n");
  return 1;
}
