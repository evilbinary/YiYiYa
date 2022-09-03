/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "arch.h"

boot_info_t* boot_info = NULL;
boot_info_t boot_data = {0};
extern u32 write_channel_number;

void arch_init(boot_info_t* boot, int cpu) {
  if (cpu == 0) {
    kmemmove(&boot_data, boot, sizeof(boot_info_t));
    boot_info = &boot_data;
    write_channel_number = 0;
    platform_init();
    cpu_init(cpu);
    display_init();
    mm_init();
    interrupt_init(cpu);
    platform_end();
  } else {
#ifdef MP_ENABLE
    ap_init(cpu);
#endif
  }
}