#ifndef _TIME_H
#define _TIME_H

#include <inttypes.h>
#include <signal.h>
#include "sys/types.h"

struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
  long tm_gmtoff;
	const char *tm_zone;
};

#ifndef NULL
#define NULL 0
#endif

#define CLK_TCK (times())

#define CLOCKS_PER_SEC 1000000l

#ifndef _HAVE_CLOCK_T
#define _HAVE_CLOCK_T
typedef int32_t clock_t;
#endif

#ifndef _HAVE_CLOCKID_T
#define _HAVE_CLOCKID_T
typedef int32_t clockid_t;
#endif

#ifndef _HAVE_SIZE_T
#define _HAVE_SIZE_T
// typedef	uint32_t	size_t;
typedef long unsigned int size_t;
#endif

#ifndef _HAVE_TIME_T
#define _HAVE_TIME_T
typedef int32_t time_t;
#endif

#ifndef _HAVE_TIMER_T
#define _HAVE_TIMER_T
typedef int32_t timer_t;
#endif

struct timespec {
  time_t tv_sec;
  time_t tv_nsec;
};

struct itimerspec {
  struct timespec it_interval;
  struct timespec it_value;
};

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

#define CLOCK_REALTIME           0
#define CLOCK_MONOTONIC          1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID  3
#define CLOCK_MONOTONIC_RAW      4
#define CLOCK_REALTIME_COARSE    5
#define CLOCK_MONOTONIC_COARSE   6
#define CLOCK_BOOTTIME           7
#define CLOCK_REALTIME_ALARM     8
#define CLOCK_BOOTTIME_ALARM     9
#define CLOCK_SGI_CYCLE         10
#define CLOCK_TAI               11

#ifdef __cplusplus
extern "C" {
#endif

char *asctime(const struct tm *);
char *asctime_r(const struct tm *, char *);
clock_t clock(void);
int clock_getres(clockid_t, struct timespec *);
int clock_gettime(clockid_t, struct timespec *);
int clock_settime(clockid_t, const struct timespec *);
char *ctime(const time_t *);
char *ctime_r(const time_t *, char *);
double difftime(time_t, time_t);
struct tm *getdate(const char *);
struct tm *gmtime(const time_t *);
struct tm *gmtime_r(const time_t *, struct tm *);
struct tm *localtime(const time_t *);
struct tm *localtime_r(const time_t *, struct tm *);
time_t mktime(struct tm *);
int nanosleep(const struct timespec *, struct timespec *);
size_t strftime(char *, size_t, const char *, const struct tm *);
char *strptime(const char *, const char *, struct tm *);
time_t time(time_t *);
// int        timer_create(clockid_t, struct sigevent *, timer_t *);
int timer_delete(timer_t);
int timer_gettime(timer_t, struct itimerspec *);
int timer_getoverrun(timer_t);
int timer_settime(timer_t, int, const struct itimerspec *, struct itimerspec *);
void tzset(void);

extern int daylight;
extern long int timezone;
extern char *tzname[];

 

#ifdef __cplusplus
}
#endif

#endif
