/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#include "common.h"

void* memcpy(void* /* restrict */ s1, const void* /* restrict */ s2, size_t n) {
  char* cdest;
  char* csrc;
  uint32_t* ldest = (uint32_t*)s1;
  uint32_t* lsrc = (uint32_t*)s2;

  while (n >= 4) {
    *ldest++ = *lsrc++;
    n -= 4;
  }

  cdest = (char*)ldest;
  csrc = (char*)lsrc;

  while (n > 0) {
    *cdest++ = *csrc++;
    n -= 1;
  }

  return s1;
}

void* memset(void* s, int c, size_t n) {
  int i;
  for (i = 0; i < n; i++) ((char*)s)[i] = c;
  return s;
}

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

void kprintf(const char* format, ...) {
  char** arg = (char**)&format;
  u8 c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0) {
    if (c != '%')
      print_char(c);
    else {
      char* p;

      c = *format++;
      switch (c) {
        case 'd':
        case 'u':
        case 'x':
          itoa(buf, c, *((int*)arg++));
          p = buf;
          goto string;
          break;

        case 's':
          p = *arg++;
          if (!p) p = "(null)";

        string:
          while (*p) print_char(*p++);
          break;

        default:
          print_char(*((int*)arg++));
          break;
      }
    }
  }
}