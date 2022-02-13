#include "stdlib.h"

#include "errno.h"
#include "float.h"
#include "math.h"
#include "stdint.h"
#include "stdio.h"
#include "syscall.h"
#include "limits.h"

extern char **environ;

void exit(int status) { ya_exit(status); }

void itoa(char *buf, int base, int d) {
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  /* If %d is specified and D is minus, put `-' in the head.  */
  if (base == 'd' && d < 0) {
    *p++ = '-';
    buf++;
    ud = -d;
  } else if (base == 'x')
    divisor = 16;

  /* Divide UD by DIVISOR until UD == 0.  */
  do {
    int remainder = ud % divisor;

    *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
  } while (ud /= divisor);

  /* Terminate BUF.  */
  *p = 0;

  /* Reverse BUF.  */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2) {
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    p1++;
    p2--;
  }
}

void abort() {}

void qsort(void *base, size_t n, size_t size,
           int (*cmp)(const void *, const void *)) {
  void *a, *b, *swap;
  int i, j;
  unsigned int pos = (unsigned int)base;
  static void *tmp = NULL;
  static int malloc_size = 0;
  if (size > malloc_size) {
    if (tmp == NULL) {
      tmp = malloc(size);
    } else {
      free(tmp);
      tmp = malloc(size);
    }
    malloc_size = size;
  }

  // This is actually a bubble sort. It should be a quick sort.

  for (i = 0; i < (n - 1); i++) {
    a = (void *)(pos + i * size);
    swap = a;

    for (j = i + 1; j < n; j++) {
      b = (void *)(pos + j * size);
      if (cmp(b, swap) < 0) swap = b;
    }

    // Swap if we need to.
    if (swap != a) {
      memcpy(tmp, a, size);
      memcpy(a, swap, size);
      memcpy(swap, tmp, size);
    }
  }

  // free(tmp);
}

double strtod(const char *str, char **endptr) {
  double number;
  int exponent;
  int negative;
  char *p = (char *)str;
  double p10;
  int n;
  int num_digits;
  int num_decimals;
  // Skip leading whitespace
  while (isspace(*p)) p++;
  // Handle optional sign
  negative = 0;
  switch (*p) {
    case '-':
      negative = 1;  // Fall through to increment position
    case '+':
      p++;
  }
  number = 0.;
  exponent = 0;
  num_digits = 0;
  num_decimals = 0;
  // Process string of digits
  while (isdigit(*p)) {
    number = number * 10. + (*p - '0');
    p++;
    num_digits++;
  }
  // Process decimal part
  if (*p == '.') {
    p++;
    while (isdigit(*p)) {
      number = number * 10. + (*p - '0');
      p++;
      num_digits++;
      num_decimals++;
    }
    exponent -= num_decimals;
  }
  if (num_digits == 0) {
    errno = ERANGE;
    return 0.0;
  }
  // Correct for sign
  if (negative) number = -number;
  // Process an exponent string
  if (*p == 'e' || *p == 'E') {
    // Handle optional sign
    negative = 0;
    switch (*++p) {
      case '-':
        negative = 1;  // Fall through to increment pos
      case '+':
        p++;
    }
    // Process string of digits
    n = 0;
    while (isdigit(*p)) {
      n = n * 10 + (*p - '0');
      p++;
    }
    if (negative)
      exponent -= n;
    else
      exponent += n;
  }
  if (exponent < DBL_MIN_EXP || exponent > DBL_MAX_EXP) {
    errno = ERANGE;
    return HUGE_VAL;
  }
  // Scale the result
  p10 = 10.;
  n = exponent;
  if (n < 0) n = -n;
  while (n) {
    if (n & 1) {
      if (exponent < 0)
        number /= p10;
      else
        number *= p10;
    }
    n >>= 1;
    p10 *= p10;
  }

  if (number == HUGE_VAL) errno = ERANGE;
  if (endptr) *endptr = p;

  return number;
}

int abs(int n) {
  if (n < 0) return (-n);
  return n;
}

long labs(long n) {
  if (n < 0) return (-n);
  return n;
}

div_t div(int num, int denom) {
  div_t result;
  int tmp;
  int a_num;
  int a_denom;
  int sign;

  a_num = abs(num);
  a_denom = abs(denom);

  sign = -1;
  if ((num < 0) && (denom < 0)) sign = 1;
  if ((num > 0) && (denom > 0)) sign = 1;

  result.quot = 0;
  result.rem = 0;

  tmp = a_num;
  while (tmp >= a_denom) {
    result.quot++;
    tmp = tmp - a_denom;
  }

  result.rem = tmp;

  if (sign == -1) {
    result.quot = (-result.quot);
    result.rem = (-result.rem);
  }

  return result;
}

ldiv_t ldiv(long num, long denom) {
  ldiv_t result;
  long tmp;
  long a_num;
  long a_denom;
  long sign;

  a_num = labs(num);
  a_denom = labs(denom);

  sign = -1;
  if ((num < 0) && (denom < 0)) sign = 1;
  if ((num > 0) && (denom > 0)) sign = 1;

  result.quot = 0;
  result.rem = 0;

  tmp = a_num;
  while (tmp >= a_denom) {
    result.quot++;
    tmp = tmp - a_denom;
  }

  result.rem = tmp;

  if (sign == -1) {
    result.quot = (-result.quot);
    result.rem = (-result.rem);
  }

  return result;
}

double atof(const char *str) { return strtod(str, NULL); }

void *bsearch(const void *key, const void *base, size_t n, size_t size,
              int (*cmp)(const void *keyval, const void *datum)) {
  unsigned int pos = (unsigned int)base;
  void *a;
  int val;

  int middle;
  int top = 0;
  int bottom = (n - 1);

  while (top < bottom) {
    middle = (bottom + top) / 2;

    if ((middle == top) || (middle == bottom)) {
      a = (void *)(pos + top * size);
      if (cmp(key, a) == 0) return a;
      a = (void *)(pos + bottom * size);
      if (cmp(key, a) == 0) return a;

      return NULL;
    }

    a = (void *)(pos + middle * size);

    val = cmp(key, a);

    if (val == 0) return a;

    if (val < 0)
      bottom = middle;  // key is less than. move bottom up.
    else
      top = middle;  // key is more than. move top down.
  }

  return NULL;
}

static uint64_t seed;

void srand(unsigned s) { seed = s - 1; }

int rand(void) {
  seed = 6364136223846793005ULL * seed + 1;
  return seed >> 33;
}

char *getenv(const char *name) {
  char *p = strchrnul(name, '=');
  size_t l = p - name;
  if (l && !name[l] && environ)
    for (char **e = environ; *e; e++)
      if (!strncmp(name, *e, l) && l[*e] == '=') return *e + l + 1;
  return 0;
}

int system(const char *cmd) { return execv(cmd, NULL); }

int rename(const char *old, const char *new) { return ya_rename(old, new); }

long strtol(const char *nptr, char **endptr, int base) {
  const char *s;
  long acc, cutoff;
  int c;
  int neg, any, cutlim;

  /*
   * Skip white space and pick up leading +/- sign if any.
   * If base is 0, allow 0x for hex and 0 for octal, else
   * assume decimal; if base is already 16, allow 0x.
   */
  s = nptr;
  do {
    c = (unsigned char)*s++;
  } while (isspace(c));

  if (c == '-') {
    neg = 1;
    c = *s++;
  } else {
    neg = 0;
    if (c == '+') c = *s++;
  }

  if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
    c = s[1];
    s += 2;
    base = 16;
  }

  if (base == 0) base = c == '0' ? 8 : 10;

  /*
   * Compute the cutoff value between legal numbers and illegal
   * numbers.  That is the largest legal value, divided by the
   * base.  An input number that is greater than this value, if
   * followed by a legal input character, is too big.  One that
   * is equal to this value may be valid or not; the limit
   * between valid and invalid numbers is then based on the last
   * digit.  For instance, if the range for longs is
   * [-2147483648..2147483647] and the input base is 10,
   * cutoff will be set to 214748364 and cutlim to either
   * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
   * a value > 214748364, or equal but the next digit is > 7 (or 8),
   * the number is too big, and we will return a range error.
   *
   * Set any if any 'digits' consumed; make it negative to indicate
   * overflow.
   */
  cutoff = neg ? LONG_MIN : LONG_MAX;
  cutlim = cutoff % base;
  cutoff /= base;

  if (neg) {
    if (cutlim > 0) {
      cutlim -= base;
      cutoff += 1;
    }
    cutlim = -cutlim;
  }

  for (acc = 0, any = 0;; c = (unsigned char)*s++) {
    if (isdigit(c))
      c -= '0';
    else if (isalpha(c))
      c -= isupper(c) ? 'A' - 10 : 'a' - 10;
    else
      break;

    if (c >= base) break;

    if (any < 0) continue;

    if (neg) {
      if (acc < cutoff || (acc == cutoff && c > cutlim)) {
        any = -1;
        acc = LONG_MIN;
        errno = ERANGE;
      } else {
        any = 1;
        acc *= base;
        acc -= c;
      }
    } else {
      if (acc > cutoff || (acc == cutoff && c > cutlim)) {
        any = -1;
        acc = LONG_MAX;
        errno = ERANGE;
      } else {
        any = 1;
        acc *= base;
        acc += c;
      }
    }
  }

  if (endptr != 0) *endptr = (char *)(any ? s - 1 : nptr);

  return (acc);
}

int atoi(const char * nptr)
{
	return (int)strtol(nptr, NULL, 10);
}

unsigned long long strtoull(const char * nptr, char ** endptr, int base)
{
	const char * s;
	unsigned long long acc, cutoff;
	int c;
	int neg, any, cutlim;

	s = nptr;
	do {
		c = (unsigned char) *s++;
	} while (isspace(c));

	if (c == '-')
	{
		neg = 1;
		c = *s++;
	}
	else
	{
		neg = 0;
		if (c == '+')
			c = *s++;
	}

	if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
	{
		c = s[1];
		s += 2;
		base = 16;
	}

	if (base == 0)
		base = c == '0' ? 8 : 10;

	switch (base)
	{
    case 4:
    	cutoff = ULLONG_MAX / 4;
        cutlim = ULLONG_MAX % 4;
        break;

    case 8:
    	cutoff = ULLONG_MAX / 8;
        cutlim = ULLONG_MAX % 8;
        break;

    case 10:
    	cutoff = ULLONG_MAX / 10;
        cutlim = ULLONG_MAX % 10;
        break;

    case 16:
    	cutoff = ULLONG_MAX / 16;
        cutlim = ULLONG_MAX % 16;
        break;

	default:
		cutoff = ULLONG_MAX / base;
		cutlim = ULLONG_MAX % base;
		break;
	}

	for (acc = 0, any = 0;; c = (unsigned char) *s++)
	{
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;

		if (c >= base)
			break;

		if (any < 0)
			continue;

		if (acc > cutoff || (acc == cutoff && c > cutlim))
		{
			any = -1;
			acc = ULLONG_MAX;
			errno = ERANGE;
		}
		else
		{
			any = 1;
			acc *= (unsigned long long) base;
			acc += c;
		}
	}

	if (neg && any > 0)
		acc = -acc;

	if (endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);

	return (acc);
}

unsigned long strtoul(const char * nptr, char ** endptr, int base)
{
	const char * s;
	unsigned long acc, cutoff;
	int c;
	int neg, any, cutlim;

	/*
	 * See strtol for comments as to the logic used.
	 */
	s = nptr;
	do {
		c = (unsigned char) *s++;
	} while (isspace(c));

	if (c == '-')
	{
		neg = 1;
		c = *s++;
	}
	else
	{
		neg = 0;
		if (c == '+')
			c = *s++;
	}

	if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
	{
		c = s[1];
		s += 2;
		base = 16;
	}

	if (base == 0)
		base = c == '0' ? 8 : 10;

	cutoff = ULONG_MAX / (unsigned long) base;
	cutlim = ULONG_MAX % (unsigned long) base;

	for (acc = 0, any = 0;; c = (unsigned char) *s++)
	{
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;

		if (c >= base)
			break;

		if (any < 0)
			continue;

		if (acc > cutoff || (acc == cutoff && c > cutlim))
		{
			any = -1;
			acc = ULONG_MAX;
			errno = ERANGE;
		}
		else
		{
			any = 1;
			acc *= (unsigned long) base;
			acc += c;
		}
	}

	if (neg && any > 0)
		acc = -acc;

	if (endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);

	return (acc);
}

long long strtoll(const char * nptr, char ** endptr, int base)
{
	const char * s;
	long long acc, cutoff;
	int c;
	int neg, any, cutlim;

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	s = nptr;
	do {
		c = (unsigned char) *s++;
	} while (isspace(c));

	if (c == '-')
	{
		neg = 1;
		c = *s++;
	}
	else
	{
		neg = 0;
		if (c == '+')
			c = *s++;
	}

	if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
	{
		c = s[1];
		s += 2;
		base = 16;
	}

	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for long long is
	 * [-9223372036854775808..9223372036854775807] and the input base
	 * is 10, cutoff will be set to 922337203685477580 and cutlim to
	 * either 7 (neg==0) or 8 (neg==1), meaning that if we have
	 * accumulated a value > 922337203685477580, or equal but the
	 * next digit is > 7 (or 8), the number is too big, and we will
	 * return a range error.
	 *
	 * Set any if any 'digits' consumed; make it negative to indicate
	 * overflow.
	 */

	switch (base)
	{
	case 4:
		if (neg)
		{
			cutlim = LLONG_MIN % 4;
			cutoff = LLONG_MIN / 4;
		}
		else
		{
			cutlim = LLONG_MAX % 4;
			cutoff = LLONG_MAX / 4;
		}
		break;

	case 8:
		if (neg)
		{
			cutlim = LLONG_MIN % 8;
			cutoff = LLONG_MIN / 8;
		}
		else
		{
			cutlim = LLONG_MAX % 8;
			cutoff = LLONG_MAX / 8;
		}
		break;

	case 10:
		if (neg)
		{
			cutlim = LLONG_MIN % 10;
			cutoff = LLONG_MIN / 10;
		}
		else
		{
			cutlim = LLONG_MAX % 10;
			cutoff = LLONG_MAX / 10;
		}
		break;

	case 16:
		if (neg)
		{
			cutlim = LLONG_MIN % 16;
			cutoff = LLONG_MIN / 16;
		}
		else
		{
			cutlim = LLONG_MAX % 16;
			cutoff = LLONG_MAX / 16;
		}
		break;

	default:
		cutoff = neg ? LLONG_MIN : LLONG_MAX;
		cutlim = cutoff % base;
		cutoff /= base;
		break;
	}

	if (neg)
	{
		if (cutlim > 0)
		{
			cutlim -= base;
			cutoff += 1;
		}
		cutlim = -cutlim;
	}

	for (acc = 0, any = 0;; c = (unsigned char) *s++)
	{
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;

		if (c >= base)
			break;

		if (any < 0)
			continue;

		if (neg)
		{
			if (acc < cutoff || (acc == cutoff && c > cutlim))
			{
				any = -1;
				acc = LLONG_MIN;
				errno = ERANGE;
			}
			else
			{
				any = 1;
				acc *= base;
				acc -= c;
			}
		}
		else
		{
			if (acc > cutoff || (acc == cutoff && c > cutlim))
			{
				any = -1;
				acc = LLONG_MAX;
				errno = ERANGE;
			}
			else
			{
				any = 1;
				acc *= base;
				acc += c;
			}
		}
	}

	if (endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);

	return (acc);
}