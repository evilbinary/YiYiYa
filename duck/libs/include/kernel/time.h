#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include "types.h"
#include "type.h"

struct timespec {
  time_t tv_sec;
  time_t tv_nsec;
};

struct timeval {
  time_t tv_sec;
  suseconds_t tv_usec;
};

struct itimerval {
  struct timeval it_interval;
  struct timeval it_value;
};


#define ITIMER_REAL 1
#define ITIMER_VIRTUAL 2
#define ITIMER_PROF 3

#ifdef __cplusplus
extern "C" {
#endif

int getitimer(int, struct itimerval *);
int setitimer(int, const struct itimerval *, struct itimerval *);
int gettimeofday(struct timeval *, void *);
int utimes(const char *, const struct timeval[2]);

#ifdef __cplusplus
}
#endif

#endif
