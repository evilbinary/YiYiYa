#include "dlfcn.h"
#include "stdio.h"

int dlclose(void *p) {
  UNIMPL();
  return 1;
}
char *dlerror(void) {
  UNIMPL();
  return NULL;
}
void *dlopen(const char *p, int m) { UNIMPL(); }
void *dlsym(void *__restrict p, const char *__restrict s) { UNIMPL(); }
