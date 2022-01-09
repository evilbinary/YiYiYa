/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"


void do_monitor_thread(void) {
  u32 i = 0;
  u32 count = 0;
  st7735_test();

  for (;;) {
    count++;
    if (i % 4 == 0) i = 0;
  }
}
