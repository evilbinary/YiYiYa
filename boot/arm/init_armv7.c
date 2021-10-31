#include "init.h"

extern char _sdata;
extern char _edata;
extern char _sbss;
extern char _ebss;

boot_info_t *_boot_info = NULL;
boot_info_t _boot_data;

void init_boot() {
  char *dst = NULL;
  char *src = NULL;

  for (dst = &_sbss; dst < &_ebss; dst++) {
    *dst = 0;
  }

  start_kernel();
  for (;;)
    ;
}

// start kernel
void start_kernel() {
  int argc = 0;
  char **argv = 0;
  char *envp[10];
  envp[0] = _boot_info;
#ifdef NO_KSTART

#else
  kstart(argc, argv, envp);
#endif
}