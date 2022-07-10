#include <signal.h>

#define UNIMPL() printf("unimpl %s:%d %s\n",__FILE__,__LINE__,__FUNCTION__)


int raise(int sig) {
  int rc;
  UNIMPL();
  return 1;
  // if ( os_signal( sig, &rc ) == NOTIMPLEMENTED )
  //		os_freakout();

  return rc;
}

int kill(pid_t pid, int attr) {
  UNIMPL();
  return 1;
}

void (*signal(int sig, void (*func)(int)))(int) {
  UNIMPL();
  return 0;
}

int sigemptyset(sigset_t *set) {
  UNIMPL();
  return 1;
}

int sigaction(int sig, const struct sigaction *restrict sa,
              struct sigaction *restrict old) {
  
  UNIMPL();
  return 0;
}

int sigfillset(sigset_t *set) {
  UNIMPL();

  return 0;
}

int sigaddset(sigset_t *set, int sig) {
  UNIMPL();

  return 0;
}

int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict old) {
  UNIMPL();
  return 0;
}