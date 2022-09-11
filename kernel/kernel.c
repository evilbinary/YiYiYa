/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "kernel.h"

void kernel_init() {
  int cpu=cpu_get_id();
  if(cpu==0){
    kprintf("kernel init\n");
    kprintf("exception init\n");
    exception_init();
    kprintf("syscall init\n");
    syscall_init();
    kprintf("schedule init\n");
    schedule_init();
    kprintf("module init\n");
    module_init();
    kprintf("memory init\n");
    memory_init();
    kprintf("vfs init\n");
    vfs_init();
    kprintf("kernel init end\n");
    thread_init();
  }else{
    kprintf("ap kernel init\n");
    schedule_init();
  }
}

void kernel_run(){
  context_t* context=thread_current_context();
  context_restore(context);
}
