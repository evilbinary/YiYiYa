/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

void do_serial_thread(void) {
  u32 i = 0;
  u32 count = 0;
  char* test = "hello,do_thread1\n";
  char wheel[] = {'\\', '|', '/', '-'};
  syscall3(SYS_WRITE, DEVICE_SERIAL, test, kstrlen(test));
  for (;;) {
    syscall3(SYS_PRINT_AT, &wheel[i++], 100, 1);
    count++;
    if (i % 4 == 0) i = 0;
  }
}
