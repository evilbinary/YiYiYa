#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <sys/types.h>

struct timeval {
  time_t tv_sec;
  suseconds_t tv_usec;
};

struct itimerval {
  struct timeval it_interval;
  struct timeval it_value;
};

typedef struct {
  int fds_bitlen;  //< Length of fds_bits
  long fds_bits[];
} fd_set;

#define ITIMER_REAL 1
#define ITIMER_VIRTUAL 2
#define ITIMER_PROF 3

#ifdef __cplusplus
extern "C" {
#endif

void FD_CLR(int fd, fd_set *fdset);
int FD_ISSET(int fd, fd_set *fdset);
void FD_SET(int fd, fd_set *fdset);
void FD_ZERO(fd_set *fdset);

// TODO: check this constant
#define FD_SETSIZE 1024

int getitimer(int, struct itimerval *);
int setitimer(int, const struct itimerval *, struct itimerval *);
int gettimeofday(struct timeval *, void *);
int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
int utimes(const char *, const struct timeval[2]);

#ifdef __cplusplus
}
#endif

#endif
