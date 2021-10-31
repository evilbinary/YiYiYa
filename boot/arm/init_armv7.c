#include "init.h"

extern char _sdata;
extern char _edata;
extern char _sbss;
extern char _ebss;

boot_info_t *_boot_info = NULL;
boot_info_t _boot_data;

void init_memory() {
  // read memory info
  // enable_a20();
  
  int count = 0;
  memory_info_t* ptr = _boot_info->memory;
  _boot_info->total_memory = 0;
 
  
  ptr->base  =_edata; 
  ptr->length=0x1000*64;//64k
  ptr->type=1;
  _boot_info->total_memory += ptr->length;
  ptr++;
  count++;

  _boot_info->memory_number = count;
  // page setup
}

void init_boot() {
  char *dst = NULL;
  char *src = NULL;

  for (dst = &_sbss; dst < &_ebss; dst++) {
    *dst = 0;
  }


  _boot_info=&_boot_data;

  init_memory();


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