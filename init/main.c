/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"


extern void driver_init();
extern void do_shell_thread(void);
extern void do_monitor_thread();

void kstart(int argc, char* argv[], char** envp) {
  boot_info_t* boot_info = envp[0];
  arch_init(boot_info);
  kmain(argc, argv);
  for (;;) {
    cpu_halt();
  }
}

int kmain(int argc, char* argv[]) {
  kernel_init();
  
  driver_init();

  thread_t* t1 = thread_create_name("monitor", (u32*)&do_monitor_thread, NULL);
  thread_t* t2 = thread_create_name("shell", (u32*)&do_shell_thread, NULL);
  thread_run(t1);
  thread_run(t2);

  kprintf("kernel run\n");
  kernel_run();

  return 0;
}
