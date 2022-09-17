/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../display.h"

#include "../boot.h"
#include "../io.h"
#include "../lock.h"

extern boot_info_t* boot_info;

void cls() {}
void putch(u8 c) {}

void puts(char* text) {
  for (; *text != 0; text++) {
    putch(*text);
  }
}

void set_cursor(int x, int y) {}

void display_init() {
  puts("Hello,YiYiYa OS\n\r");
  puts("display init\n\r");
  if (boot_info->disply.mode == 1) {
  }
}
