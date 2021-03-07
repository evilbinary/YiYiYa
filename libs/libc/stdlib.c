#include "stdint.h"
#include "stdio.h"
#include "syscall.h"
#include "errno.h"
#include "float.h"
#include "math.h"

void itoa(char* buf, int base, int d) {
  char* p = buf;
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

void qsort(void* base, size_t n, size_t size,
           int (*cmp)(const void*, const void*)) {
  void *a, *b, *swap;
  int i, j;
  unsigned int pos = (unsigned int)base;
  static void* tmp=NULL;
  static int malloc_size=0;
  if(size>malloc_size){
	  if(tmp==NULL){
	  	tmp=malloc(size);
	  }else{
		  free(tmp);
		  tmp=malloc(size);
	  }
	  malloc_size=size;
  }

  // This is actually a bubble sort. It should be a quick sort.

  for (i = 0; i < (n - 1); i++) {
    a = (void*)(pos + i * size);
    swap = a;

    for (j = i + 1; j < n; j++) {
      b = (void*)(pos + j * size);
      if (cmp(b, swap) < 0) swap = b;
    }

    // Swap if we need to.
    if (swap != a) {
      memcpy(tmp, a, size);
      memcpy(a, swap, size);
      memcpy(swap, tmp, size);
    }
  }

  //free(tmp);
}

double strtod(const char *str, char **endptr)
{
	double number;
	int exponent;
	int negative;
	char *p = (char *) str;
	double p10;
	int n;
	int num_digits;
	int num_decimals;
	// Skip leading whitespace
	while (isspace(*p)) p++;
	// Handle optional sign
	negative = 0;
	switch (*p) 
	{             
		case '-': negative = 1; // Fall through to increment position
		case '+': p++;
	}
	number = 0.;
	exponent = 0;
	num_digits = 0;
	num_decimals = 0;
	// Process string of digits
	while (isdigit(*p))
	{
		number = number * 10. + (*p - '0');
		p++;
		num_digits++;
	}
	// Process decimal part
	if (*p == '.') 
	{
		p++;
		while (isdigit(*p))
		{
			number = number * 10. + (*p - '0');
			p++;
			num_digits++;
			num_decimals++;
		}
		exponent -= num_decimals;
	}
	if (num_digits == 0)
	{
		errno = ERANGE;
		return 0.0;
	}
	// Correct for sign
	if (negative) number = -number;
	// Process an exponent string
	if (*p == 'e' || *p == 'E') 
	{
		// Handle optional sign
		negative = 0;
		switch(*++p) 
		{   
			case '-': negative = 1;   // Fall through to increment pos
			case '+': p++;
		}
		// Process string of digits
		n = 0;
		while (isdigit(*p)) 
		{   
			n = n * 10 + (*p - '0');
			p++;
		}
		if (negative) exponent -= n;
				else exponent += n;
	}
	if (exponent < DBL_MIN_EXP  || exponent > DBL_MAX_EXP)
	{
		errno = ERANGE;
		return HUGE_VAL;
	}
	// Scale the result
	p10 = 10.;
	n = exponent;
	if (n < 0) n = -n;
	while (n) 
	{
		if (n & 1) 
		{
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