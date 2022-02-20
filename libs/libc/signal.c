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

void (*signal(int sig, void (*func)(int)))(int) {
  printf("unimpl signal\n");
  return 0;
}

int sigemptyset(sigset_t *set) {
  printf("unimpl sigemptyset\n");

  return 1;
}

int sigaction(int sig, const struct sigaction *restrict sa,
              struct sigaction *restrict old) {
  printf("unimpl sigaction\n");

  return 0;
}

int sigfillset(sigset_t *set) {
  printf("unimpl sigfillset\n");

  return 0;
}

int sigaddset(sigset_t *set, int sig) {
  printf("unimpl sigaddset\n");

  return 0;
}

int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict old) {
  printf("unimpl sigprocmask\n");
  return 0;
}