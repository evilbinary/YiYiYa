#include "stdio.h"

#include "errno.h"
#include "fcntl.h"
#include "stdint.h"
#include "syscall.h"

FILE STDIN;
FILE STDOUT;
FILE STDERROR;

FILE* stdin = NULL;
FILE* stdout = NULL;
FILE* stderr = NULL;

int putchar(int ch) {
  if (stdout == NULL) {
    STDIN.fd = 0;
    STDOUT.fd = 1;
    STDERROR.fd=2;
    stdout = &STDOUT;
    stderr = &STDERROR;
    stdin = &STDIN;
  }
  int ret = ya_write((u32)stdout->fd, &ch, 1);
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
  int i = vsprintf(buf, format, arg);
  i = (fputs(buf, stream) < 0) ? 0 : i;
  return i;
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
  void* data = NULL;
  FILE* file = (FILE*)malloc(sizeof(FILE));
  file->data = data;
  file->fd = fd;
  file->eof = 0;
  file->error = 0;
  file->offset = 0;
  file->mode = flags;

  return file;
}

int fseek(FILE* stream, long int offset, int origin) {
  int rc;
  rc = ya_seek((u32)stream->fd, offset);
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
  for (size_t i = 0; i < size; ++i) {
    size_t r = ya_read(stream->fd, buffer, nmemb);
    stream->offset += nmemb;
    fseek(stream, stream->offset, SEEK_SET);
    if (r < 0) {
      return -1;
    }
    buffer += r;
    if (r < (int)size) {
      return i;
    }
  }
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

  rc = ya_read(stream->fd, &c, 1);
  stream->offset++;
  fseek(stream, stream->offset, SEEK_SET);

  if (rc == EOF || rc == 0) {
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
  rc = ya_write(stream->fd, &ch, 1);
  stream->offset++;
  fseek(stream, stream->offset, SEEK_SET);
  if (rc == EOF) {
    stream->eof = 1;
    return EOF;
  }
  if (rc != 1) {
    /// \todo confirm this action
    stream->error = 1;
    return EOF;
  }
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

int fflush(FILE* f) {
  if (!f) return EINVAL;
  // if (!f->write) return EINVAL;
  return (f);
}

int remove(const char* file) { return -1; }

int fileno(FILE* stream) { return stream->fd; }

int fputs(const char* str, FILE* stream) {
  int rc = fwrite(str, strlen(str), 1, stream);
  // int  rc = ya_write(stream->fd, str, strlen(str));
  return rc;
}

FILE* fdopen(int fd, const char* mode) {
  unsigned int flags = support_fmodes2flags(mode);
  if (fd < 0) return NULL;
  void* data = NULL;
  FILE* file = (FILE*)malloc(sizeof(FILE));
  file->data = data;
  file->fd = fd;
  file->eof = 0;
  file->error = 0;
  file->offset = 0;
  file->mode = flags;
  return file;
}

void rewind(FILE * f){
	fseek(f, 0, SEEK_SET);
}