/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "string.h"

void* kmemcpy(void* /* restrict */ s1, const void* /* restrict */ s2,
              size_t n) {
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

char* kstrcpy(char* /* restrict */ s1, const char* /* restrict */ s2) {
  int i = 0;
  do {
    s1[i] = s2[i];
  } while (s2[i++] != '\0');
  return s1;
}

char* kstrncpy(char* /* restrict */ s1, const char* /* restrict */ s2,
               size_t n) {
  int i;

  for (i = 0; i < n; i++) {
    s1[i] = s2[i];
    if (s2[i] == '\0') break;
  }

  while (i < n) s1[i++] = '\0';

  return s1;
}

char* kstrcat(char* /* restrict */ s1, const char* /* restrict */ s2) {
  char* tmp = s1;
  while (*s1 != '\0') s1++;
  while ((*s1++ = *s2++) != '\0')
    ;
  return tmp;
}

char* kstrncat(char* /* restrict */ s1, const char* /* restrict */ s2,
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

int kmemcmp(const void* s1, const void* s2, size_t n) {
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

int kstrcmp(const char* s1, const char* s2) {
  char c;

  while (1) {
    if ((c = *s1 - *s2++) != 0 || !*s1++) break;
  }

  return c;
}

int kstrncmp(const char* s1, const char* s2, size_t n) {
  char c = 0;

  while (n > 0) {
    if ((c = *s1 - *s2++) != 0 || !*s1++) break;
    n--;
  }

  return c;
}

void* kmemchr(const void* s, int c, size_t n) {
  int i;
  for (i = 0; i < n; i++)
    if (((char*)s)[i] == c) return (void*)(s + i);

  return NULL;
}

char* kstrchr(const char* s, int c) {
  int i;
  for (i = 0; i < kstrlen(s); i++)
    if (s[i] == c) return (char*)(s + i);
  return NULL;
}

size_t kstrcspn(const char* s1, const char* s2) {
  int len1;
  int len2;
  int i;
  int j;
  char c;

  len1 = kstrlen(s1);
  len2 = kstrlen(s2);

  for (i = 0; i < len1; i++) {
    c = s1[i];

    for (j = 0; j < len2; j++)
      if (s2[j] == c) return i;
  }

  return len1;
}

char* kstrpbrk(const char* s1, const char* s2) {
  int len1;
  int len2;
  int i;
  int j;
  char c;

  len1 = kstrlen(s1);
  len2 = kstrlen(s2);

  for (i = 0; i < len1; i++) {
    c = s1[i];

    for (j = 0; j < len2; j++)
      if (s2[j] == c) return (char*)(s1 + i);
  }

  return NULL;
}

char* kstrrchr(const char* s, int c) {
  int i;
  for (i = kstrlen(s) - 1; i >= 0; i--)
    if (s[i] == c) return (char*)(s + i);
  return NULL;
}

size_t kstrspn(const char* s1, const char* s2) {
  int len1;
  int len2;
  int i;
  int j;
  char c;

  len1 = kstrlen(s1);
  len2 = kstrlen(s2);

  for (i = 0; i < len1; i++) {
    c = s1[i];

    for (j = 0; j < len2; j++)
      if (s2[j] != c) return i;
  }

  return len1;
}

char* kstrstr(const char* s1, const char* s2) {
  int len1;
  int len2;
  int i;

  len2 = kstrlen(s2);
  if (len2 == 0) return (char*)s1;
  len1 = kstrlen(s1);

  for (i = 0; i < len1; i++) {
    if (kstrcmp(s1 + i, s2) == 0) return (char*)(s1 + i);
  }

  return NULL;
}

void* kmemset(void* s, int c, size_t n) {
  int i;
  for (i = 0; i < n; i++) ((char*)s)[i] = c;

  return s;
}

char* kstrerror(int errnum) {
  // switch (errnum) {
  //   case EDOM:
  //     return "domain error";
  //   case ERANGE:
  //     return "range error";
  //   default:
  //     return "unknown error number.";
  // }
}

size_t kstrlen(const char* s) {
  size_t ans = 0;
  while (s[ans++] != '\0') {
  };
  return (ans - 1);
}