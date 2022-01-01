#include "init.h"

extern int _sdata;
extern int _edata;
extern int _sbss;
extern int _ebss;
extern int _estack;
extern int _sidata;
extern int __bss_start__,__bss_end__;

boot_info_t *_boot_info = NULL;
boot_info_t _boot_data;

void init_memory() {
  // read memory info
  int count = 0;
  memory_info_t* ptr = _boot_info->memory;
  _boot_info->total_memory = 0;
 
  ptr->length=64*1024-(&_ebss - &_sdata);//64k
  ptr->base  =&_ebss; 
  ptr->type=1;
  _boot_info->total_memory += ptr->length;
  ptr++;
  count++;

  _boot_info->memory_number = count;
  // page setup
}

void init_boot() {
  unsigned *dst = NULL;
  unsigned *src = NULL;

  // copy _sidata from flash to sram
  for(src=&_sidata,dst=&_sdata; dst< &_edata;src++,dst++){
    *dst=*src;
  }

  //init bss
  for (dst = &__bss_start__; dst < &__bss_end__; dst++) {
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