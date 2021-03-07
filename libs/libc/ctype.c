#include <ctype.h>

int isalnum(int c) {
  if (isdigit(c) != 0) return 1;
  if (isalpha(c) != 0) return 1;
  return 0;
}

int isalpha(int c) {
  if (isupper(c) != 0) return 1;
  if (islower(c) != 0) return 1;
  return 0;
}

int isascii(int c) {
  if ((c - (c & 0x7F)) != 0) return 0;
  return 1;
}

int iscntrl(int c) {
  if ((c >= 0x00) && (c <= 0x1F)) return 1;
  if (c == 0x7F) return 1;
  return 0;
}

int isdigit(int c) {
  if ((c <= '9') && (c >= '0')) return 1;
  return 0;
}

int isgraph(int c) {
  if (isspace(c) == 0) return 1;
  return 0;
}

int islower(int c) {
  if ((c >= 'a') && (c <= 'z')) return 1;
  return 0;
}

int isprint(int c) {
  if ((c <= 0x7E) && (c >= 0x20)) return 1;
  return 0;
}

int ispunct(int c) {
  if (isalnum(c) != 0) return 0;
  if (isspace(c) != 0) return 0;
  return 1;
}

int isspace(int c) {
  switch (c) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      return 1;
  }
  return 0;
}

int isupper(int c) {
  if ((c >= 'A') && (c <= 'Z')) return 1;
  return 0;
}

int isxdigit(int c) {
  if ((toupper(c) <= 'F') && (toupper(c) >= 'A')) return 1;
  if ((toupper(c) <= '9') && (toupper(c) >= '0')) return 1;
  return 0;
}

int toascii(int c) { return (c & 0x7F); }

int tolower(int c) {
  if ((c >= 'A') && (c <= 'Z')) return (c - 'A' + 'a');
  return c;
}

int toupper(int c) {
  if ((c >= 'a') && (c <= 'z')) return (c - 'a' + 'A');
  return c;
}
