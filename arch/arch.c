/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#include "arch.h"

boot_info_t* boot_info;

void arch_init(boot_info_t* boot){
    io_add_write_channel(putch);
    boot_info = boot;
    display_init();
    mm_init();
    cpu_init();
    interrupt_init();
}