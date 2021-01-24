/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#include "kernel/module.h"



int hello_init(void) {
  kprintf("Hello World\n");
  return 0;
}

void hello_exit(void) { kprintf("Goodbye World\n"); }


module_t hello_module = {
    .name ="hello",
    .init=hello_init,
    .exit=hello_exit
};

module_regit(hello_module);