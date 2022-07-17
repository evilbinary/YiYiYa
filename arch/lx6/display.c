/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../display.h"

#include "../boot.h"
#include "../io.h"
#include "../lock.h"
#include "gpio.h"

extern boot_info_t* boot_info;


void set_cursor(int x, int y) {
  
}
void cls() {

}
void putch(u8 c) {
  uart_send(c);
}

void puts(char* text) {
  for (; *text != 0; text++) {
    putch(*text);
  }
}

void test_display() {
  cls();
  puts("test display hello,YiYiYa\n\r");
}

void display_init() {
  puts("Hello,YiYiYa OS\n\r");
  puts("display init\n\r");
  if (boot_info->disply.mode == 1) {
    
  }
  test_display();
}
