#include "stdio.h"

#include "errno.h"
#include "fcntl.h"
#include "stdint.h"
#include "syscall.h"

FILE STDIN;
FILE STDOUT;
FILE STDERROR;

FILE *stdin = NULL;
FILE *stdout = NULL;
FILE *stderr = NULL;
char printf_buffer[512];

int putchar(int ch)
{
  if (stdout == NULL)
  {
    STDIN.fd = 0;
    STDOUT.fd = 1;
    STDERROR.fd = 2;
    stdout = &STDOUT;
    stderr = &STDERROR;
    stdin = &STDIN;
  }
  int ret = ya_write((u32)STDOUT_FILENO, &ch, 1);
  return ret;
}

int printf(const char *format, ...)
{
  memset(printf_buffer,0,512);
  int i;
	va_list args;
	va_start(args, format);
	i = vsprintf(printf_buffer, format, args);
	va_end(args);
  int len=strlen(printf_buffer);
  for(int i=0;i<len;i++){
    putchar(printf_buffer[i]);
  }
  return 0;
}

int vfprintf(FILE *stream, const char *format, va_list arg)
{
  if(format==NULL) return 0;
  char buf[1024];
  memset(buf,0,1024);
  int i = vsprintf(buf, format, arg);
  i = (fputs(buf, stream) < 0) ? 0 : i;
  return i;
}

int vprintf(const char *format, va_list arg)
{
  return vfprintf(stdout, format, arg);
}

int fprintf(FILE *stream, const char *format, ...)
{
  int rc;
  va_list ap;
  va_start(ap, format);
  rc = vfprintf(stream, format, ap);
  va_end(ap);
  return rc;
}

FILE *fopen(const char *filename, const char *mode)
{
  int fd;
  unsigned int flags = support_fmodes2flags(mode);
  fd = open(filename, flags);
  if (fd < 0)
    return NULL;
  void *data = NULL;
  FILE *file = (FILE *)malloc(sizeof(FILE));
  file->data = data;
  file->fd = fd;
  file->eof = 0;
  file->error = 0;
  file->offset = 0;
  file->mode = flags;

  return file;
}

int fseek(FILE *stream, long int offset, int origin)
{
  int rc;
  rc = ya_seek((u32)stream->fd, offset);
  return rc;
}

int fclose(FILE *stream) { return close(stream->fd); }

size_t fread(void * /* restrict */ ptr, size_t size, size_t nmemb,
             FILE * /* restrict */ stream)
{
  unsigned char *buffer = (unsigned char *)ptr;
  int i, j, c;

  // C99 sanity check.
  if (size == 0)
    return 0;
  if (nmemb == 0)
    return 0;
  for (size_t i = 0; i < size; ++i)
  {
    size_t r = ya_read(stream->fd, buffer, nmemb);
    stream->offset += nmemb;
    fseek(stream, stream->offset, SEEK_SET);
    if (r < 0)
    {
      return -1;
    }
    buffer += r;
    if (r < (int)size)
    {
      return i;
    }
  }
  return nmemb;
}

size_t fwrite(const void * /* restrict */ ptr, size_t size, size_t nmemb,
              FILE * /* restrict */ stream)
{
  unsigned char *buffer = (unsigned char *)ptr;
  int i, j;

  // C99 sanity check.
  if (size == 0)
    return 0;
  if (nmemb == 0)
    return 0;

  // For each member...
  for (i = 0; i < nmemb; i++)
  {
    for (j = 0; j < size; j++)
    {
      if (fputc(*buffer, stream) != *buffer)
        return i;
      buffer++;
    }
  }

  // Apparently successful.
  return nmemb;
}

long int ftell(FILE *stream)
{
  long int rc;

  return rc;
}

int fgetc(FILE *stream)
{
  unsigned char c;
  int rc;
  if (feof(stream) != 0)
    return EOF;

  rc = ya_read(stream->fd, &c, 1);
  stream->offset++;
  fseek(stream, stream->offset, SEEK_SET);

  if (rc == EOF || rc == 0)
  {
    stream->eof = 1;
    return EOF;
  }
  if (rc < 0)
  {
    stream->error = 1;
    return -1;
  }

  return (int)c;
}

int fputc(int c, FILE *stream)
{
  int rc;
  unsigned char ch = (unsigned char)c;
  rc = ya_write(stream->fd, &ch, 1);
  stream->offset++;
  fseek(stream, stream->offset, SEEK_SET);
  if (rc == EOF)
  {
    stream->eof = 1;
    return EOF;
  }
  if (rc != 1)
  {
    /// \todo confirm this action
    stream->error = 1;
    return EOF;
  }
  return c;
}

int feof(FILE *stream)
{
  if (stream->eof != 0)
    return 1;
  return 0;
}

int ferror(FILE *stream)
{
  if (stream->error != 0)
    return 1;
  return 0;
}

void perror(const char *s)
{
  fprintf(stderr, "%s: %s\n", (s != NULL ? s : ""), strerror(errno));
}

int fflush(FILE *f)
{
  if (!f)
    return EINVAL;
  // if (!f->write) return EINVAL;
  return (f);
}

int remove(const char *file) { return -1; }

int fileno(FILE *stream) { return stream->fd; }

int fputs(const char *str, FILE *stream)
{
  int rc = fwrite(str, strlen(str), 1, stream);
  // int  rc = ya_write(stream->fd, str, strlen(str));
  return rc;
}

FILE *fdopen(int fd, const char *mode)
{
  unsigned int flags = support_fmodes2flags(mode);
  if (fd < 0)
    return NULL;
  void *data = NULL;
  FILE *file = (FILE *)malloc(sizeof(FILE));
  file->data = data;
  file->fd = fd;
  file->eof = 0;
  file->error = 0;
  file->offset = 0;
  file->mode = flags;
  return file;
}

void rewind(FILE *f)
{
  fseek(f, 0, SEEK_SET);
}
char *fgets(char *s, int n, FILE *f)
{
  char *p = s;
  char *ret = NULL;
  size_t res = 0;

  while (n-- > 1)
  {
    res = fgetc(f);

    if (res == 0)
      break;

    else if (res < 0)
      return NULL;

    ret = s;
    if (*p++ == '\n')
      break;
  }

  *p = 0;
  return ret;
}

int getc(FILE *f)
{
  return fgetc(f);
}

FILE *freopen(const char *path, const char *mode, FILE *f)
{

  if (f)
    fclose(f);
  return fopen(path, mode);
}

FILE *tmpfile(void)
{
  char path[256];
  sprintf(path, "%s/tmpfile_%d", "/tmp", rand());
  return fopen(path, "wb+");
}

int ungetc(int c, FILE *f){
  if (c == EOF)
    return c;
  f->offset--;
  fputc(c,f);
  return (unsigned char)c;
}


void  clearerr(FILE *f){
  printf("not implement clearerr\n");
}


int setvbuf(FILE *restrict f, char *restrict buf, int type, size_t size){

  printf("not implement setvbuf\n");
  return 0;
}

int snprintf(char * buf, size_t n, const char * fmt, ...){
	va_list ap;
	int rv;
	va_start(ap, fmt);
	rv =vsprintf(buf, fmt, ap);
	va_end(ap);
	return rv;
}


char * tmpnam(char * buf){
	static char internal[L_tmpnam];
	char path[256];
	sprintf(path, "%s/tmpnam_%d", "/tmp", rand());
	return strcpy(buf ? buf : internal, path);
}