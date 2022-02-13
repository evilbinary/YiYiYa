#include "sys/select.h"

int select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout) {
  printf("unimpl select\n");
  return 1;
}