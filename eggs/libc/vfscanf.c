#include <ctype.h>
#include <limits.h>
#include "stdarg.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#include "stdio.h"

#define UNIMPL() printf("unimpl %s:%d %s\n",__FILE__,__LINE__,__FUNCTION__)


int vfscanf(FILE *restrict f, const char *restrict fmt, va_list ap) {
  UNIMPL();
  
  return 1;
}

weak_alias(vfscanf, __isoc99_vfscanf);
