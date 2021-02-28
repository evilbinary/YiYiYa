#include <stdarg.h>
#include <stddef.h>

#ifndef _STDIO_H
#define _STDIO_H	


#define		BUFSIZ 				8192
#define		FILENAME_MAX 		256
#define		FOPEN_MAX 			1024

#define _IOFBF	1
#define _IOLBF	2
#define _IONBF	4


#define		L_ctermid 			8
#define		L_cuserid 			8		// Legacy
#define		L_tmpnam 			8

#define		SEEK_SET 			0
#define		SEEK_CUR 			1
#define		SEEK_END 			2
#define		TMP_MAX 			238328


#define		EOF 				(-506)


#ifndef	NULL
#define NULL		0
#endif

#define P_tmpdir	"/temp/"

#ifndef _HAVE_OFF_T
#define _HAVE_OFF_T
typedef long int off_t;
#endif

/** This is the internal structure definition for a FILE*
 * stream.
 */
typedef struct
{
	int fd;				///< File descriptor. 
	void *data;			///< Data allocated by the OS.
	int eof;			///< EOF indicator for the stream.
	int error;			///< Error indicator for the stream.
	unsigned int mode;	///< The fcntl.h mode that the file was opened in.
} FILE;


typedef struct
{
  unsigned int __pos;
  unsigned int __state;
} fpos_t;




#ifndef _HAVE_SIZE_T
#define _HAVE_SIZE_T
typedef	long unsigned int	size_t;
#endif





#ifdef __cplusplus
extern "C" {
#endif

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;


void  clearerr(FILE *);
char *ctermid(char *);
char *cuserid(char *); // LEGACY
int fclose(FILE *);
FILE *fdopen(int, const char *);
int feof(FILE *);
int ferror(FILE *);
int fflush(FILE *);
int fgetc(FILE *);
int fgetpos(FILE *, fpos_t *);
char *fgets(char *, int, FILE *);
int fileno(FILE *);
void  flockfile(FILE *);
FILE *fopen(const char *, const char *);
int fprintf(FILE *, const char *, ...);
int fputc(int, FILE *);
int fputs(const char *, FILE *);
size_t   fread(void *, size_t, size_t, FILE *);
FILE *freopen(const char *, const char *, FILE *);
int fscanf(FILE *, const char *, ...);
int fseek(FILE *, long int, int);
int fseeko(FILE *, off_t, int);
int fsetpos(FILE *, const fpos_t *);
long int ftell(FILE *);
off_t ftello(FILE *);
int ftrylockfile(FILE *);
void  funlockfile(FILE *);
size_t   fwrite(const void *, size_t, size_t, FILE *);
int getc(FILE *);
int getchar(void);
int getc_unlocked(FILE *);
int getchar_unlocked(void);
int getopt(int, char * const[], const char*);  // LEGACY
char *gets(char *);
int getw(FILE *);
int pclose(FILE *);
void  perror(const char *);
FILE *popen(const char *, const char *);
int printf(const char *, ...);
int putc(int, FILE *);
int putchar(int);
int putc_unlocked(int, FILE *);
int putchar_unlocked(int);
int puts(const char *);
int putw(int, FILE *);
int remove(const char *);
int rename(const char *, const char *);
void  rewind(FILE *);
int scanf(const char *, ...);
void  setbuf(FILE *, char *);
int setvbuf(FILE *, char *, int, size_t);
int snprintf(char *, size_t, const char *, ...);
int sprintf(char *, const char *, ...);
int sscanf(const char *, const char *, ...);
char *tempnam(const char *, const char *);
FILE *tmpfile(void);
char *tmpnam(char *);
int ungetc(int, FILE *);
int vfprintf(FILE *, const char *, va_list);
int vprintf(const char *, va_list);
int vsnprintf(char *, size_t, const char *, va_list);
int vsprintf(char *, const char *, va_list);


#ifdef __cplusplus
}
#endif



#endif

