/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef STDLIB_H
#define STDLIB_H
#include "stdint.h"
#include "stddef.h"


#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0


typedef struct {
  int quot;
  int rem;
} div_t;

typedef struct {
  long quot;
  long rem;
} ldiv_t;


long a64l(const char *);
void abort(void);
int abs(int);
int atexit(void (*)(void));
double atof(const char *);
int atoi(const char *);
long int atol(const char *);
void *bsearch(const void *, const void *, size_t, size_t,
              int (*)(const void *, const void *));
void *calloc(size_t, size_t);
div_t div(int, int);
double drand48(void);
char *ecvt(double, int, int *, int *);
double erand48(unsigned short int[3]);
void exit(int);
char *fcvt(double, int, int *, int *);
void free(void *);

char *gcvt(double, int, char *);
char *getenv(const char *);
int getsubopt(char **, char *const *, char **);
int grantpt(int);
char *initstate(unsigned int, char *, size_t);
long int jrand48(unsigned short int[3]);
char *l64a(long);
long int labs(long int);
void lcong48(unsigned short int[7]);
ldiv_t ldiv(long int, long int);
long int lrand48(void);
void *malloc(size_t);
int mblen(const char *, size_t);
size_t mbstowcs(wchar_t *, const char *, size_t);
int mbtowc(wchar_t *, const char *, size_t);
char *mktemp(char *);
int mkstemp(char *);
long int mrand48(void);
long int nrand48(unsigned short int[3]);
char *ptsname(int);
int putenv(char *);
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
int rand(void);
int rand_r(unsigned int *);
long random(void);
void *realloc(void *, size_t);
char *realpath(const char *, char *);
unsigned short int seed48(unsigned short int[3]);
void setkey(const char *);
char *setstate(const char *);
void srand(unsigned int);
void srand48(long int);
void srandom(unsigned);
double strtod(const char *, char **);
long int strtol(const char *, char **, int);
unsigned long int strtoul(const char *, char **, int);
int system(const char *);
int unlockpt(int);
size_t wcstombs(char *, const wchar_t *, size_t);
int wctomb(char *, wchar_t);

char *getenv (const char *);

#endif