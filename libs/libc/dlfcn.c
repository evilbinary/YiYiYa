#include "dlfcn.h"
#include "stdio.h"

int dlclose(void *p) {
  printf("unimpl dlclose\n");
  return 1;
}
char * dlerror(void) {
  printf("unimpl dlerror\n");
  return NULL;
}
void *dlopen(const char *p, int m) { printf("unimpl dlopen\n"); }
void *dlsym(void *__restrict p, const char *__restrict s) {
  printf("unimpl dlsym\n");
}
