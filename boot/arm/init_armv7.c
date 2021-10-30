#include "init.h"


boot_info_t* _boot_info = NULL;
boot_info_t _boot_data;

void init_boot() {
  start_kernel();
  for (;;)
    ;
}


// start kernel
void start_kernel() {
  int argc = 0;
  char** argv = 0;
  char* envp[10];
  envp[0] = _boot_info;
  #ifdef KSTART
  kstart(argc, argv, envp);
  #endif
}