/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#include "arch/boot.h"
#include "arch/arch.h"
#include "config.h"
#include "libs/include/types.h"

boot_info_t* boot_info = NULL;
boot_info_t boot_data;
typedef int (*entry)(int, char**, char**);

void init_boot() {
  // init_boot_info();
  // cls();
  // printf("boot info addr %x\n\r", boot_info);

  // init_gdt();

  // print_string("init display\n\r");
  // init_display();

  // print_string("init memory\n\r");
  // init_memory();
  
  // init_disk();

  // read_kernel();
  // init_cpu();


  //asm("jmpl %0, $start_kernel" ::"i"(GDT_ENTRY_32BIT_CS * GDT_SIZE));

  for (;;);
}
// start kernel
void start_kernel() {
 
  boot_info->kernel_entry = load_kernel(); 

  entry start = boot_info->kernel_entry;
  int argc = 0;
  char** argv = 0;
  char* envp[10];
  envp[0] = boot_info;
  start(argc, argv, envp);
}
