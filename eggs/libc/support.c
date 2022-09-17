#include "fcntl.h"
#include "support.h"

unsigned int support_fmodes2flags(const char* mode) {
  if ((strcmp(mode, "r") == 0) || (strcmp(mode, "rb") == 0)) return O_RDONLY;
  if ((strcmp(mode, "w") == 0) || (strcmp(mode, "wb") == 0))
    return O_WRONLY | O_CREAT | O_TRUNC;
  if ((strcmp(mode, "a") == 0) || (strcmp(mode, "ab") == 0))
    return O_WRONLY | O_CREAT | O_APPEND;

  if ((strcmp(mode, "r+") == 0) || (strcmp(mode, "rb+") == 0) ||
      (strcmp(mode, "r+b") == 0))
    return O_RDWR;

  if ((strcmp(mode, "w+") == 0) || (strcmp(mode, "wb+") == 0) ||
      (strcmp(mode, "w+b") == 0))
    return O_WRONLY | O_TRUNC | O_CREAT;

  if ((strcmp(mode, "a+") == 0) || (strcmp(mode, "ab+") == 0) ||
      (strcmp(mode, "a+b") == 0))
    return O_WRONLY | O_APPEND | O_CREAT;

  return 0;
}