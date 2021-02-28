#include "stdio.h"

#include "stdint.h"
#include "syscall.h"

int STDIN = NULL;
int STDOUT = NULL;

int putchar(int ch) {
  if (STDOUT == NULL) {
    STDOUT = syscall2(SYS_OPEN, "/dev/stdout", 0);
  }
  int ret = syscall3(SYS_WRITE, STDOUT, ch, 1);
  return ret;
}

int printf(const char* format, ...) {
  char** arg = (char**)&format;
  u8 c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0) {
    if (c != '%')
      putchar(c);
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
          while (*p) putchar(*p++);
          break;

        default:
          putchar(*((int*)arg++));
          break;
      }
    }
  }
}