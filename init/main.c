/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

#ifdef ARM 

#ifdef ARMV7
extern module_t hello_module;
#else

extern module_t gpu_module;
extern module_t devfs_module;
extern module_t sdhci_module;
extern module_t fat_module;
extern module_t serial_module;
extern module_t fat32_module;
extern module_t mouse_module;
extern module_t hello_module;
extern module_t i2c_module;
extern module_t gpio_module;
extern module_t spi_module;
#endif

#elif defined(X86)
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
extern module_t sb16_module;

#elif defined(XTENSA)
extern module_t hello_module;

#else 
extern module_t hello_module;

#endif

extern void serial_write(char a);
extern void do_shell_thread(void);
extern void do_serial_thread();

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

  kprintf("module regist\n");

#ifdef ARM 

#ifdef ARMV7

#else
  module_regist(&serial_module);
  module_regist(&i2c_module);
  module_regist(&gpio_module);
  module_regist(&spi_module);
  module_regist(&gpu_module);
  module_regist(&mouse_module);
  module_regist(&sdhci_module);
  module_regist(&devfs_module);
  module_regist(&fat_module);
  // module_regist(&fat32_module);
  // module_regist(&hello_module);

  #endif

#elif defined(X86)
  module_regist(&pci_module);
  module_regist(&serial_module);
  module_regist(&keyboard_module);
  module_regist(&rtc_module);
  // module_regist(&vga_module);
  module_regist(&qemu_module);
  module_regist(&mouse_module);
  module_regist(&ahci_module);
  module_regist(&devfs_module);
  // module_regist(&fat32_module);
  module_regist(&fat_module);
  module_regist(&sb16_module);

#elif defined(XTENSA)
  module_regist(&hello_module);

#else 
  module_regist(&hello_module);
#endif

  thread_t* t0 = thread_create((u32*)&do_serial_thread,NULL);
  thread_t* t1 = thread_create((u32*)&do_shell_thread,NULL);

  kprintf("thread run\n");
  thread_run(t0);
  thread_run(t1);
  kprintf("kernel run\n");
  kernel_run();

  return 0;
}
