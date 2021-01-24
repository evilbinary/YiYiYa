#ifndef _STRING_H
#define _STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../types.h"

void    *kmemccpy(void *, const void *, int, size_t);
void    *kmemchr(const void *, int, size_t);
int      kmemcmp(const void *, const void *, size_t);
void    *kmemcpy(void *, const void *, size_t);
void    *kmemmove(void *, const void *, size_t);
void    *kmemset(void *, int, size_t);
char    *kstrcat(char *, const char *);
char    *kstrchr(const char *, int);
int      kstrcmp(const char *, const char *);
int      kstrcoll(const char *, const char *);
char    *kstrcpy(char *, const char *);
size_t   kstrcspn(const char *, const char *);
char    *kstrdup(const char *);
char    *kstrerror(int);
size_t   kstrlen(const char *);
char    *kstrncat(char *, const char *, size_t);
int      kstrncmp(const char *, const char *, size_t);
char    *kstrncpy(char *, const char *, size_t);
char    *kstrpbrk(const char *, const char *);
char    *kstrrchr(const char *, int);
size_t   kstrspn(const char *, const char *);
char    *kstrstr(const char *, const char *);
char    *kstrtok(char *, const char *);
char    *kstrtok_r(char *, const char *, char **);
size_t   kstrxfrm(char *, const char *, size_t);

char*	kstrndup(const char *s, size_t n);	// This is not in the standard.
char*	kstrdup(const char *s);				// This is not in the standard.

#ifdef __cplusplus
}
#endif

#endif

