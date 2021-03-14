#include "stdio.h"

#include "errno.h"
#include "fcntl.h"
#include "stdint.h"
#include "syscall.h"

FILE STDIN;
FILE STDOUT;

FILE* stdin = NULL;
FILE* stdout = NULL;
FILE* stderr = NULL;

int putchar(int ch) {
  if (stdout == NULL) {
    STDIN.fd = syscall2(SYS_OPEN, "/dev/stdout", 0);
    stdout = &STDIN;
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
  vsprintf(stream->fd, format, arg);
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

FILE* fopen(const char* filename, const char* mode) {
  int fd;
  unsigned int flags = support_fmodes2flags(mode);
  fd = open(filename, flags);
  if (fd < 0) return NULL;
  return fd;
}

int fseek(FILE* stream, long int offset, int origin) {
  int rc;
  rc=syscall2(SYS_SEEK, stream->fd, offset);
  return rc;
}

int fclose(FILE* stream) { return close(stream->fd); }

size_t fread(void* /* restrict */ ptr, size_t size, size_t nmemb,
             FILE* /* restrict */ stream) {
  unsigned char* buffer = (unsigned char*)ptr;
  int i, j, c;

  // C99 sanity check.
  if (size == 0) return 0;
  if (nmemb == 0) return 0;

  // For each member...
  for (i = 0; i < nmemb; i++) {
    for (j = 0; j < size; j++) {
      c = fgetc(stream);
      if ((feof(stream) != 0) || (ferror(stream) != 0)) return i;

      *buffer = (unsigned char)c;
      buffer++;
    }
  }

  // Apparently successful.
  return nmemb;
}

size_t fwrite(const void* /* restrict */ ptr, size_t size, size_t nmemb,
              FILE* /* restrict */ stream) {
  unsigned char* buffer = (unsigned char*)ptr;
  int i, j;

  // C99 sanity check.
  if (size == 0) return 0;
  if (nmemb == 0) return 0;

  // For each member...
  for (i = 0; i < nmemb; i++) {
    for (j = 0; j < size; j++) {
      if (fputc(*buffer, stream) != *buffer) return i;
      buffer++;
    }
  }

  // Apparently successful.
  return nmemb;
}

long int ftell(FILE* stream) {
  long int rc;

  return rc;
}

int fgetc(FILE* stream) {
  unsigned char c;
  int rc;
  if (feof(stream) != 0) return EOF;

  rc=syscall3(SYS_READ,stream->fd,&c, 1);
  stream->offset++;
  fseek(stream,stream->offset, SEEK_SET);
  
  if (rc == EOF||rc ==0) {
    stream->eof = 1;
    return EOF;
  }
  if (rc < 0) {
    stream->error = 1;
    return -1;
  }

  return (int)c;
}

int fputc(int c, FILE* stream) {
  int rc;
  unsigned char ch = (unsigned char)c;

  /*if ( os_write( stream->data, &ch, 1, &rc ) == NOTIMPLEMENTED )
                                  os_freakout();

  if ( rc == EOF )
  {
          stream->eof = 1;
          return EOF;
  }
  if ( rc != 1 )
  {
          /// \todo confirm this action
          stream->error = 1;
          return EOF;
  }
  */
  return c;
}

int feof(FILE* stream) {
  if (stream->eof != 0) return 1;
  return 0;
}

int ferror(FILE* stream) {
  if (stream->error != 0) return 1;
  return 0;
}

void perror(const char* s) {
  fprintf(stderr, "%s: %s\n", (s != NULL ? s : ""), strerror(errno));
}