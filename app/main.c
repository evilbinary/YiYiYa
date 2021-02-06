/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

extern context_t* current_context;
extern module_t keyboard_module;
extern module_t hello_module;
extern module_t pci_module;
extern module_t vga_module;
extern module_t qemu_module;
extern module_t mouse_module;
extern module_t serial_module;
extern void serial_write(char a);
extern void putch(char a);

extern void do_screen_thread(void);
extern void do_shell_thread(void);
extern void do_serial_thread();

void start(int argc, char* argv[], char** envp) {
  boot_info_t* boot_info = envp[0];
  arch_init(boot_info);
  kmain(argc, argv);
  for (;;) {
    cpu_halt();
  }
}

int kmain(int argc, char* argv[]) {
  io_add_write_channel(putch);
  exception_init();
  syscall_init();
  cls();
  schedule_init();
  module_init();
  module_regit(&pci_module);
  module_regit(&serial_module);
  module_regit(&keyboard_module);
  io_add_write_channel(serial_write);

  // module_regit(&vga_module);
  module_regit(&qemu_module);
  // module_regit(&mouse_module);

  thread_t* t0 = thread_create((u32*)&do_shell_thread);
  thread_t* t1 = thread_create((u32*)&do_serial_thread);
  thread_t* t2 = thread_create((u32*)&do_screen_thread);

  thread_run(t0);
  thread_run(t1);
  thread_run(t2);

  context_restore(current_context);

  return 0;
}
