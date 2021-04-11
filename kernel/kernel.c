/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "kernel.h"

extern void putch(u8 a);
extern context_t* current_context;

void kernel_init() {
  io_add_write_channel(putch);
  kprintf("kernel init\n");
  kprintf("exception init\n");
  exception_init();
  kprintf("syscall init\n");
  syscall_init();
  cls();
  kprintf("schedule init\n");
  schedule_init();
  kprintf("module init\n");
  module_init();
  kprintf("memory init\n");
  memory_init();
  kprintf("vfs init\n");
  vfs_init();
  kprintf("kernel init end\n");
}

void kernel_run(){
  context_restore(current_context);
}
