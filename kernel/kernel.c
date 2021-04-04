/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "kernel.h"

extern void putch(char a);
extern context_t* current_context;

void kernel_init() {
  io_add_write_channel(putch);
  exception_init();
  syscall_init();
  cls();
  schedule_init();
  module_init();
  memory_init();
  vfs_init();
}

void kernel_run(){
  context_restore(current_context);
}
