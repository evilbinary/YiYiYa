#include "stdio.h"

#include "stdint.h"
#include "syscall.h"

FILE STDIN;
FILE STDOUT;

FILE* stdin = NULL;
FILE* stdout = NULL;
FILE* stderr = NULL;

int putchar(int ch) {
  if (stdout==NULL) {
    STDIN.fd=syscall2(SYS_OPEN, "/dev/stdout", 0);
    stdout =&STDIN; 
    stderr = stdout;
  }
  int ret = syscall3(SYS_WRITE, (u32)stdout->fd, &ch, 1);

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

int vfprintf(FILE* stream, const char* format, va_list arg) {
  char buf[1024];
  int i;
  vsprintf(stream->fd,format,arg);
  return 10;
}

int vprintf(const char* format, va_list arg) {
  return vfprintf(stdout, format, arg);
}

int fprintf(FILE* stream, const char* format, ...) {
  int rc;
  va_list ap;
  va_start(ap, format);
  rc = vfprintf(stream, format, ap);
  va_end(ap);
  return rc;
}