#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#define ASCBUFSIZE (26 + 2)

static struct tm __UNIX_tmbuf;
static char __UNIX_ascbuf[ASCBUFSIZE];

clock_t clock(void) {
  clock_t ans;

  /*if ( os_clock( &ans ) == NOTIMPLEMENTED )
                  os_freakout();
  */
  return ans;
}

double difftime(time_t time2, time_t time1) { return (time1 - time2); }

time_t time(time_t *tp) {
  time_t seconds;
  int rc;
  rc = ya_time(&seconds);
  if (rc != 0) return 0;
  if (tp != NULL) *tp = seconds;
  return seconds;
}

// -------------------------------------------------------------------
// HERE FORTH BEGINS CODE FROM Sanos. See ADDITIONAL_LICENSES

#define YEAR0 1900
#define EPOCH_YR 1970
#define SECS_DAY (24L * 60L * 60L)
#define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year) (LEAPYEAR(year) ? 366 : 365)
#define FIRSTSUNDAY(timp) (((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
#define FIRSTDAYOF(timp) (((timp)->tm_wday - (timp)->tm_yday + 420) % 7)

#define TIME_MAX 2147483647L

int _daylight = 0;   // Non-zero if daylight savings time is used
long _dstbias = 0;   // Offset for Daylight Saving Time
long _timezone = 0;  // Difference in seconds between GMT and local time
char *_tzname[2] = {"GMT", "GMT"};  // Standard/daylight savings time zone names

const char *_days[] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                       "Thursday", "Friday", "Saturday"};

const char *_days_abbrev[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

const char *_months[] = {"January",   "February", "March",    "April",
                         "May",       "June",     "July",     "August",
                         "September", "October",  "November", "December"};

const char *_months_abbrev[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

const int _ytab[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                          {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

struct tm *gmtime_r(const time_t *timer, struct tm *tmbuf) {
  time_t time = *timer;
  unsigned long dayclock, dayno;
  int year = EPOCH_YR;

  dayclock = (unsigned long)time % SECS_DAY;
  dayno = (unsigned long)time / SECS_DAY;

  tmbuf->tm_sec = dayclock % 60;
  tmbuf->tm_min = (dayclock % 3600) / 60;
  tmbuf->tm_hour = dayclock / 3600;
  tmbuf->tm_wday = (dayno + 4) % 7;  // Day 0 was a thursday
  while (dayno >= (unsigned long)YEARSIZE(year)) {
    dayno -= YEARSIZE(year);
    year++;
  }
  tmbuf->tm_year = year - YEAR0;
  tmbuf->tm_yday = dayno;
  tmbuf->tm_mon = 0;
  while (dayno >= (unsigned long)_ytab[LEAPYEAR(year)][tmbuf->tm_mon]) {
    dayno -= _ytab[LEAPYEAR(year)][tmbuf->tm_mon];
    tmbuf->tm_mon++;
  }
  tmbuf->tm_mday = dayno + 1;
  tmbuf->tm_isdst = 0;

  return tmbuf;
}

struct tm *localtime_r(const time_t *timer, struct tm *tmbuf) {
  time_t t;

  t = *timer - _timezone;
  return gmtime_r(&t, tmbuf);
}

struct tm *gmtime(const time_t *timer) {
  return gmtime_r(timer, &__UNIX_tmbuf);
}

struct tm *localtime(const time_t *timer) {
  return localtime_r(timer, &__UNIX_tmbuf);
}

time_t mktime(struct tm *tmbuf) {
  long day, year;
  int tm_year;
  int yday, month;
  /*unsigned*/ long seconds;
  int overflow;
  long dst;

  tmbuf->tm_min += tmbuf->tm_sec / 60;
  tmbuf->tm_sec %= 60;
  if (tmbuf->tm_sec < 0) {
    tmbuf->tm_sec += 60;
    tmbuf->tm_min--;
  }
  tmbuf->tm_hour += tmbuf->tm_min / 60;
  tmbuf->tm_min = tmbuf->tm_min % 60;
  if (tmbuf->tm_min < 0) {
    tmbuf->tm_min += 60;
    tmbuf->tm_hour--;
  }
  day = tmbuf->tm_hour / 24;
  tmbuf->tm_hour = tmbuf->tm_hour % 24;
  if (tmbuf->tm_hour < 0) {
    tmbuf->tm_hour += 24;
    day--;
  }
  tmbuf->tm_year += tmbuf->tm_mon / 12;
  tmbuf->tm_mon %= 12;
  if (tmbuf->tm_mon < 0) {
    tmbuf->tm_mon += 12;
    tmbuf->tm_year--;
  }
  day += (tmbuf->tm_mday - 1);
  while (day < 0) {
    if (--tmbuf->tm_mon < 0) {
      tmbuf->tm_year--;
      tmbuf->tm_mon = 11;
    }
    day += _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
  }
  while (day >= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon]) {
    day -= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
    if (++(tmbuf->tm_mon) == 12) {
      tmbuf->tm_mon = 0;
      tmbuf->tm_year++;
    }
  }
  tmbuf->tm_mday = day + 1;
  year = EPOCH_YR;
  if (tmbuf->tm_year < year - YEAR0) return (time_t)-1;
  seconds = 0;
  day = 0;  // Means days since day 0 now
  overflow = 0;

  // Assume that when day becomes negative, there will certainly
  // be overflow on seconds.
  // The check for overflow needs not to be done for leapyears
  // divisible by 400.
  // The code only works when year (1970) is not a leapyear.
  tm_year = tmbuf->tm_year + YEAR0;

  if (TIME_MAX / 365 < tm_year - year) overflow++;
  day = (tm_year - year) * 365;
  if (TIME_MAX - day < (tm_year - year) / 4 + 1) overflow++;
  day += (tm_year - year) / 4 + ((tm_year % 4) && tm_year % 4 < year % 4);
  day -=
      (tm_year - year) / 100 + ((tm_year % 100) && tm_year % 100 < year % 100);
  day +=
      (tm_year - year) / 400 + ((tm_year % 400) && tm_year % 400 < year % 400);

  yday = month = 0;
  while (month < tmbuf->tm_mon) {
    yday += _ytab[LEAPYEAR(tm_year)][month];
    month++;
  }
  yday += (tmbuf->tm_mday - 1);
  if (day + yday < 0) overflow++;
  day += yday;

  tmbuf->tm_yday = yday;
  tmbuf->tm_wday = (day + 4) % 7;  // Day 0 was thursday (4)

  seconds = ((tmbuf->tm_hour * 60L) + tmbuf->tm_min) * 60L + tmbuf->tm_sec;

  if ((TIME_MAX - seconds) / SECS_DAY < day) overflow++;
  seconds += day * SECS_DAY;

  // Now adjust according to timezone and daylight saving time
  if (((_timezone > 0) && (TIME_MAX - _timezone < seconds)) ||
      ((_timezone < 0) && (seconds < -_timezone)))
    overflow++;
  seconds += _timezone;

  if (tmbuf->tm_isdst)
    dst = _dstbias;
  else
    dst = 0;

  if (dst > seconds) overflow++;  // dst is always non-negative
  seconds -= dst;

  if (overflow) return (time_t)-1;

  if ((time_t)seconds != seconds) return (time_t)-1;
  return (time_t)seconds;
}

char *asctime(const struct tm *tp) {
  char *ascbuf = __UNIX_ascbuf;
  strftime(ascbuf, ASCBUFSIZE, "%c\n", tp);
  return ascbuf;
}

char *ctime(const time_t *tp) { return asctime(localtime(tp)); }

char *_strdate(char *s) {
  time_t now;

  time(&now);
  strftime(s, 9, "%D", localtime(&now));
  return s;
}

char *_strtime(char *s) {
  time_t now;

  time(&now);
  strftime(s, 9, "%T", localtime(&now));
  return s;
}

void _tzset() {}

#define TM_YEAR_BASE 1900

#define DAYSPERLYEAR 366
#define DAYSPERNYEAR 365
#define DAYSPERWEEK 7

static char *_fmt(const char *format, const struct tm *t, char *pt,
                  const char *ptlim);
static char *_conv(const int n, const char *format, char *pt,
                   const char *ptlim);
static char *_add(const char *str, char *pt, const char *ptlim);

size_t strftime(char *s, size_t maxsize, const char *format,
                const struct tm *t) {
  char *p;
  p = _fmt(((format == NULL) ? "%c" : format), t, s, s + maxsize);
  if (p == s + maxsize) return 0;
  *p = '\0';
  return p - s;
}

static char *_fmt(const char *format, const struct tm *t, char *pt,
                  const char *ptlim) {
  for (; *format; ++format) {
    if (*format == '%') {
      if (*format == 'E')
        format++;  // Alternate Era
      else if (*format == 'O')
        format++;  // Alternate numeric symbols

      switch (*++format) {
        case '\0':
          --format;
          break;

        case 'A':
          pt =
              _add((t->tm_wday < 0 || t->tm_wday > 6) ? "?" : _days[t->tm_wday],
                   pt, ptlim);
          continue;

        case 'a':
          pt = _add((t->tm_wday < 0 || t->tm_wday > 6)
                        ? "?"
                        : _days_abbrev[t->tm_wday],
                    pt, ptlim);
          continue;

        case 'B':
          pt =
              _add((t->tm_mon < 0 || t->tm_mon > 11) ? "?" : _months[t->tm_mon],
                   pt, ptlim);
          continue;

        case 'b':
        case 'h':
          pt = _add((t->tm_mon < 0 || t->tm_mon > 11)
                        ? "?"
                        : _months_abbrev[t->tm_mon],
                    pt, ptlim);
          continue;

        case 'C':
          pt = _conv((t->tm_year + TM_YEAR_BASE) / 100, "%02d", pt, ptlim);
          continue;

        case 'c':
          pt = _fmt("%a %b %e %H:%M:%S %Y", t, pt, ptlim);
          continue;

        case 'D':
          pt = _fmt("%m/%d/%y", t, pt, ptlim);
          continue;

        case 'd':
          pt = _conv(t->tm_mday, "%02d", pt, ptlim);
          continue;

        case 'e':
          pt = _conv(t->tm_mday, "%2d", pt, ptlim);
          continue;

        case 'F':
          pt = _fmt("%Y-%m-%d", t, pt, ptlim);
          continue;

        case 'H':
          pt = _conv(t->tm_hour, "%02d", pt, ptlim);
          continue;

        case 'I':
          pt = _conv((t->tm_hour % 12) ? (t->tm_hour % 12) : 12, "%02d", pt,
                     ptlim);
          continue;

        case 'j':
          pt = _conv(t->tm_yday + 1, "%03d", pt, ptlim);
          continue;

        case 'k':
          pt = _conv(t->tm_hour, "%2d", pt, ptlim);
          continue;

        case 'l':
          pt = _conv((t->tm_hour % 12) ? (t->tm_hour % 12) : 12, "%2d", pt,
                     ptlim);
          continue;

        case 'M':
          pt = _conv(t->tm_min, "%02d", pt, ptlim);
          continue;

        case 'm':
          pt = _conv(t->tm_mon + 1, "%02d", pt, ptlim);
          continue;

        case 'n':
          pt = _add("\n", pt, ptlim);
          continue;

        case 'p':
          pt = _add((t->tm_hour >= 12) ? "pm" : "am", pt, ptlim);
          continue;

        case 'R':
          pt = _fmt("%H:%M", t, pt, ptlim);
          continue;

        case 'r':
          pt = _fmt("%I:%M:%S %p", t, pt, ptlim);
          continue;

        case 'S':
          pt = _conv(t->tm_sec, "%02d", pt, ptlim);
          continue;

        case 's': {
          struct tm tm;
          char buf[32];
          time_t mkt;

          tm = *t;
          mkt = mktime(&tm);
          sprintf(buf, "%lu", mkt);
          pt = _add(buf, pt, ptlim);
          continue;
        }

        case 'T':
          pt = _fmt("%H:%M:%S", t, pt, ptlim);
          continue;

        case 't':
          pt = _add("\t", pt, ptlim);
          continue;

        case 'U':
          pt = _conv((t->tm_yday + 7 - t->tm_wday) / 7, "%02d", pt, ptlim);
          continue;

        case 'u':
          pt = _conv((t->tm_wday == 0) ? 7 : t->tm_wday, "%d", pt, ptlim);
          continue;

        case 'V':  // ISO 8601 week number
        case 'G':  // ISO 8601 year (four digits)
        case 'g':  // ISO 8601 year (two digits)
        {
          int year;
          int yday;
          int wday;
          int w;

          year = t->tm_year + TM_YEAR_BASE;
          yday = t->tm_yday;
          wday = t->tm_wday;
          while (1) {
            int len;
            int bot;
            int top;

            len = LEAPYEAR(year) ? DAYSPERLYEAR : DAYSPERNYEAR;
            bot = ((yday + 11 - wday) % DAYSPERWEEK) - 3;
            top = bot - (len % DAYSPERWEEK);
            if (top < -3) top += DAYSPERWEEK;
            top += len;
            if (yday >= top) {
              ++year;
              w = 1;
              break;
            }
            if (yday >= bot) {
              w = 1 + ((yday - bot) / DAYSPERWEEK);
              break;
            }
            --year;
            yday += LEAPYEAR(year) ? DAYSPERLYEAR : DAYSPERNYEAR;
          }
          if (*format == 'V')
            pt = _conv(w, "%02d", pt, ptlim);
          else if (*format == 'g')
            pt = _conv(year % 100, "%02d", pt, ptlim);
          else
            pt = _conv(year, "%04d", pt, ptlim);

          continue;
        }

        case 'v':
          pt = _fmt("%e-%b-%Y", t, pt, ptlim);
          continue;

        case 'W':
          pt = _conv((t->tm_yday + 7 - (t->tm_wday ? (t->tm_wday - 1) : 6)) / 7,
                     "%02d", pt, ptlim);
          continue;

        case 'w':
          pt = _conv(t->tm_wday, "%d", pt, ptlim);
          continue;

        case 'X':
          pt = _fmt("%H:%M:%S", t, pt, ptlim);
          continue;

        case 'x':
          pt = _fmt("%m/%d/%y", t, pt, ptlim);
          continue;

        case 'y':
          pt = _conv((t->tm_year + TM_YEAR_BASE) % 100, "%02d", pt, ptlim);
          continue;

        case 'Y':
          pt = _conv(t->tm_year + TM_YEAR_BASE, "%04d", pt, ptlim);
          continue;

        case 'Z':
          pt = _add("?", pt, ptlim);
          continue;

        case 'z': {
          long absoff;
          if (_timezone >= 0) {
            absoff = _timezone;
            pt = _add("+", pt, ptlim);
          } else {
            absoff = _timezone;
            pt = _add("-", pt, ptlim);
          }
          pt = _conv(absoff / 3600, "%02d", pt, ptlim);
          pt = _conv((absoff % 3600) / 60, "%02d", pt, ptlim);

          continue;
        }

        case '+':
          pt = _fmt("%a, %d %b %Y %H:%M:%S %z", t, pt, ptlim);
          continue;

        case '%':
        default:
          break;
      }
    }

    if (pt == ptlim) break;
    *pt++ = *format;
  }

  return pt;
}

static char *_conv(const int n, const char *format, char *pt,
                   const char *ptlim) {
  char buf[32];

  sprintf(buf, format, n);
  return _add(buf, pt, ptlim);
}

static char *_add(const char *str, char *pt, const char *ptlim) {
  while (pt < ptlim && (*pt = *str++) != '\0') ++pt;
  return pt;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
  printf("not imple nanosleep\n");
  return 0;
}

int clock_gettime(clockid_t clockid, struct timespec *ts) {
  printf("not imple clock_gettime %d\n",clockid);
  if(clockid==1){
    time_t seconds;
    int rc = ya_time(&seconds);
    ts->tv_sec=seconds;
    ts->tv_nsec=0;
    return 0;
  }
  return 0;
}

char *ctime_r(const time_t *t, char *buf) {
  printf("not imple ctime_r\n");
  return 1;
}

char *asctime_r(const struct tm *restrict tm, char *restrict buf) {
  printf("not imple asctime_r\n");
  return 1;
}

int utimes(const char *path, const struct timeval* t)
{
  printf("not imple utimes\n");
  return 0;
}
