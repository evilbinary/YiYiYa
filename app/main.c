/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"
extern module_t keyboard_module;
extern module_t hello_module;
extern module_t pci_module;
extern module_t vga_module;
extern module_t qemu_module;
extern module_t mouse_module;
extern module_t serial_module;
extern module_t ahci_module;
extern module_t devfs_module;
extern module_t fat32_module;
extern module_t rtc_module;
extern module_t fat_module;

extern void serial_write(char a);
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
  kernel_init();

  kprintf("module regist\n");

#ifdef ARM 

#elif defined(X86)
  module_regist(&pci_module);
  module_regist(&serial_module);
  module_regist(&keyboard_module);
  io_add_write_channel(serial_write);
  module_regist(&rtc_module);

  // module_regist(&vga_module);
  module_regist(&qemu_module);
  module_regist(&mouse_module);
  module_regist(&ahci_module);
  module_regist(&devfs_module);
  // module_regist(&fat32_module);
    module_regist(&fat_module);


#endif

  thread_t* t0 = thread_create((u32*)&do_serial_thread,NULL);
  thread_t* t1 = thread_create((u32*)&do_shell_thread,NULL);

  thread_run(t0);
  thread_run(t1);
  kernel_run();

  return 0;
}
