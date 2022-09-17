/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "kernel/kernel.h"

int test_init(void) {
  kprintf("test hello\n");
  test_ahci();
  test_fat();

  return 0;
}

void test_exit(void) { kprintf("test exit\n"); }

module_t test_module = {.name = "test", .init = test_init, .exit = test_exit};
