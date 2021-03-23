#include <ctype.h>
#include <sys/types.h>

int strcasecmp(const char *s1, const char *s2) {
  while (1) {
    int a = tolower(*s1++);
    int b = tolower(*s2++);

    if ((a == b)) {
      if (a == 0) break;
      continue;
    }

    if (a < b) return -1;
    return 1;
  }

  return 0;
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
  while (n-- > 0) {
    int a = tolower(*s1++);
    int b = tolower(*s2++);

    if ((a == b)) {
      if (a == 0) break;
      continue;
    }

    if (a < b) return -1;
    return 1;
  }

  return 0;
}
