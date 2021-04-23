/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../display.h"

#include "../boot.h"
#include "../io.h"
#include "../lock.h"
#include "arch/arm/gpio.h"

extern boot_info_t* boot_info;


void uart_send(unsigned int c) {
    do{}while(*UART0_FR&0x20);
    *UART0_DR=c;
}

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
  puts("hello,YiYiYa\n");
}

void display_init() {
  if (boot_info->disply.mode == 1) {
    
  }
  test_display();
}
