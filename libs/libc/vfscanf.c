#include <ctype.h>
#include <limits.h>
#include "stdarg.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#include "stdio.h"

int vfscanf(FILE *restrict f, const char *restrict fmt, va_list ap) {
  UNIMPL();
  
  return 1;
}

weak_alias(vfscanf, __isoc99_vfscanf);
