/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>

// #include "limits.h"
// #include <stdlib.h>
#include <string.h>
#include "locale.h"

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

void* memmove(void* s1, const void* s2, size_t n) {
  char *dest, *src;
  int i;

  dest = (char*)s1;
  src = (char*)s2;
  for (i = 0; i < n; i++) dest[i] = src[i];

  return s1;
}

char* strcpy(char* /* restrict */ s1, const char* /* restrict */ s2) {
  int i = 0;
  do {
    s1[i] = s2[i];
  } while (s2[i++] != '\0');
  return s1;
}

char* strncpy(char* /* restrict */ s1, const char* /* restrict */ s2,
              size_t n) {
  int i;

  for (i = 0; i < n; i++) {
    s1[i] = s2[i];
    if (s2[i] == '\0') break;
  }

  while (i < n) s1[i++] = '\0';

  return s1;
}

char* strcat(char* /* restrict */ s1, const char* /* restrict */ s2) {
  char* tmp = s1;
  while (*s1 != '\0') s1++;
  while ((*s1++ = *s2++) != '\0')
    ;
  return tmp;
}

char* strncat(char* /* restrict */ s1, const char* /* restrict */ s2,
              size_t n) {
  char* tmp = s1;

  if (n > 0) {
    while (*s1) s1++;
    while ((*s1++ = *s2++)) {
      if (--n == 0) {
        *s1 = '\0';
        break;
      }
    }
  }

  return tmp;
}

int memcmp(const void* s1, const void* s2, size_t n) {
  char *s, *t;
  int i;

  s = (char*)s1;
  t = (char*)s2;
  for (i = 0; i < n; i++) {
    if (s[i] < t[i]) return -1;
    if (s[i] > t[i]) return 1;
  }
  return 0;
}

int strcmp(const char* s1, const char* s2) {
  char c;

  while (1) {
    if ((c = *s1 - *s2++) != 0 || !*s1++) break;
  }

  return c;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  char c = 0;

  while (n > 0) {
    if ((c = *s1 - *s2++) != 0 || !*s1++) break;
    n--;
  }

  return c;
}

void* memchr(const void* s, int c, size_t n) {
  int i;
  for (i = 0; i < n; i++)
    if (((char*)s)[i] == c) return (void*)(s + i);

  return NULL;
}

char* strchr(const char* s, int c) {
  int i;
  for (i = 0; i < strlen(s); i++)
    if (s[i] == c) return (char*)(s + i);
  return NULL;
}

size_t strcspn(const char* s1, const char* s2) {
  int len1;
  int len2;
  int i;
  int j;
  char c;

  len1 = strlen(s1);
  len2 = strlen(s2);

  for (i = 0; i < len1; i++) {
    c = s1[i];

    for (j = 0; j < len2; j++)
      if (s2[j] == c) return i;
  }

  return len1;
}

char* strpbrk(const char* s1, const char* s2) {
  int len1;
  int len2;
  int i;
  int j;
  char c;

  len1 = strlen(s1);
  len2 = strlen(s2);

  for (i = 0; i < len1; i++) {
    c = s1[i];

    for (j = 0; j < len2; j++)
      if (s2[j] == c) return (char*)(s1 + i);
  }

  return NULL;
}

char* strrchr(const char* s, int c) {
  int i;
  for (i = strlen(s) - 1; i >= 0; i--)
    if (s[i] == c) return (char*)(s + i);
  return NULL;
}

size_t strspn(const char* s1, const char* s2) {
  int len1;
  int len2;
  int i;
  int j;
  char c;

  len1 = strlen(s1);
  len2 = strlen(s2);

  for (i = 0; i < len1; i++) {
    c = s1[i];

    for (j = 0; j < len2; j++)
      if (s2[j] != c) return i;
  }

  return len1;
}

char* strstr(const char* haystack, const char* needle) {
  const char *a = haystack, *b = needle;
  for (;;) {
    if (!*b) {
      return (char*)haystack;
    } else if (!*a) {
      return NULL;
    } else if (*a++ != *b++) {
      a = ++haystack;
      b = needle;
    }
  }
}

void* memset(void* s, int c, size_t n) {
  int i;
  for (i = 0; i < n; i++) ((char*)s)[i] = c;

  return s;
}

char* strerror(int errnum) {
  switch (errnum) {
    case EDOM:
      return "domain error";
    case ERANGE:
      return "range error";
    default:
      return "unknown error number.";
  }
}

size_t strlen(const char* s) {
  size_t ans = 0;
  while (s[ans++] != '\0') {
  };
  return (ans - 1);
}

#warning NOT PART OF THE STANDARD
/// Not part of the standard.
char* strndup(const char* s, size_t n) {
  char* result;
  result = (char*)malloc(n + 1);
  memcpy(result, s, n + 1);
  result[n] = 0;
  return result;
}

#warning NOT PART OF THE STANDARD
/// Not part of the standard.
char* strdup(const char* s) {
  char* result;
  int len = strlen(s);
  result = (char*)malloc(len + 1);
  memcpy(result, s, len + 1);
  return result;
}

char* strchrnul(const char* s, int c) {
  c = (unsigned char)c;
  if (!c) return (char*)s + strlen(s);

// #ifdef __GNUC__
// #define ALIGN (sizeof(size_t))
// #define ONES ((size_t)-1 / UCHAR_MAX)
// #define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
// #define HASZERO(x) ((x)-ONES & ~(x)&HIGHS)

//   typedef size_t __attribute__((__may_alias__)) word;
//   const word* w;
//   for (; (uintptr_t)s % ALIGN; s++)
//     if (!*s || *(unsigned char*)s == c) return (char*)s;
//   size_t k = ONES * c;
//   for (w = (void*)s; !HASZERO(*w) && !HASZERO(*w ^ k); w++)
//     ;
//   s = (void*)w;
// #endif
  for (; *s && *(unsigned char*)s != c; s++)
    ;
  return (char*)s;
}


int strcoll(const char *l, const char *r)
{
	// return __strcoll_l(l, r, CURRENT_LOCALE);
  return strcmp(l, r);
}