
#include "kernel/kernel.h"

#ifdef ARM

#ifdef ARMV7
extern module_t hello_module;
extern module_t lcd_module;
extern module_t gpio_module;
extern module_t spi_module;
extern module_t devfs_module;
extern module_t serial_module;
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
extern module_t test_module;

#elif defined(XTENSA)
extern module_t hello_module;

#else
extern module_t hello_module;

#endif

void modules_init(void) {
  u32 i = 0;
  u32 count = 0;

  kprintf("module regist\n");

#ifdef ARM

#ifdef ARMV7
  module_regist(&gpio_module);
  module_regist(&serial_module);
  // module_regist(&hello_module);
  module_regist(&spi_module);
  module_regist(&lcd_module);
  module_regist(&devfs_module);

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

#elif defined(DUMMY)
  module_regist(&hello_module);

#elif defined(X86)
  module_regist(&pci_module);
  module_regist(&serial_module);
  module_regist(&keyboard_module);
  module_regist(&rtc_module);
  // module_regist(&vga_module);
  module_regist(&qemu_module);
  module_regist(&mouse_module);
  module_regist(&ahci_module);
  module_regist(&sb16_module);
  module_regist(&devfs_module);
  // module_regist(&fat32_module);
  module_regist(&fat_module);

  module_regist(&test_module);

#elif defined(XTENSA)
  module_regist(&hello_module);

#else
  module_regist(&hello_module);
#endif

}