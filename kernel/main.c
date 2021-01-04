/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#include "main.h"

void start(int argc, char* argv[], char** envp) {
  boot_info_t* boot_info = envp[0];
  // arch_init(boot_info);
  kmain(argc, argv);
  for(;;){
    // cpu_halt();
  }
}

 

int kmain(int argc, char* argv[]) { 
  // kernel start here  

  return 0;
}


