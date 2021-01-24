/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#include "kernel/module.h"
#include "kernel/device.h"


int devfs_init(void) {
  kprintf("devfs init\n");

  return 0;
}

void devfs_exit(void) { kprintf("devfs exit\n"); }


module_t devfs_module = {
    .name ="devfs",
    .init=devfs_init,
    .exit=devfs_exit
};

module_regit(devfs_module);