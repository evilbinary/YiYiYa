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

#define BLANK_CHAR (' ')
#define BLANK_ATTR (0x07)

typedef enum COLOUR_TAG {
  BLACK,
  BLUE,
  GREEN,
  CYAN,
  RED,
  MAGENTA,
  BROWN,
  WHITE,
  GRAY,
  LIGHT_BLUE,
  LIGHT_GREEN,
  LIGHT_CYAN,
  LIGHT_RED,
  LIGHT_MAGENTA,
  YELLOW,
  BRIGHT_WHITE
} COLOUR;

static i32 screen_x = 0;
static i32 screen_y = 0;
static i32 attrib = 0x0F;
static i32 screen_width = 80;
static i32 screen_height = 25;
static u16* video_addr = 0;

void move_cursor(void) {
  unsigned temp;
  temp = screen_y * screen_width + screen_x;
  io_write8(0x3D4, 0x0E);
  io_write8(0x3D5, temp >> 8);
  io_write8(0x3D4, 0x0F);
  io_write8(0x3D5, temp);
}

void set_cursor(int x, int y) {
  screen_x = x;
  screen_y = y;
}

void get_cursor(int* x, int* y) {
  *x = screen_x;
  *y = screen_y;
}

void set_color(u8 forecolor, u8 backcolor) {
  attrib = (backcolor << 4) | (forecolor & 0x0F);
}

void cls() {
  unsigned blank;
  int i;
  blank = 0x20 | (attrib << 8);
  for (i = 0; i < screen_width * screen_height; i++) {
    *(video_addr + i) = blank;
  }
  screen_x = 0;
  screen_y = 0;
}

void scroll(void) {
  unsigned blank, temp;
  blank = 0x20 | (attrib << 8);
  if (screen_y >= screen_height) {
    temp = screen_y - screen_height + 1;
    kmemcpy(video_addr, video_addr + temp * screen_width,
           (screen_height - temp) * screen_width * 2);
    kmemset(video_addr + (screen_height - temp) * screen_width, blank,
           screen_width);
    screen_y = screen_height - 1;
  }
}

void putch(u8 c) {
  unsigned short* where;
  unsigned att = attrib << 8;
  if (c == 0x08) {
    if (screen_x != 0) screen_x--;
  } else if (c == 0x09) {
    screen_x = (screen_x + 8) & ~(8 - 1);
  } else if (c == '\r') {
    screen_x = 0;
  } else if (c == '\n') {
    screen_x = 0;
    screen_y++;
  } else if (c >= ' '&&c<=0xff) {
    where = video_addr + (screen_y * screen_width + screen_x);
    *where = c | att;
    screen_x++;
  }
  if (screen_x >= screen_width) {
    screen_x = 0;
    screen_y++;
  }
  scroll();
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
    video_addr = (unsigned short*)boot_info->disply.video;
    screen_width = boot_info->disply.width;
    screen_height = boot_info->disply.height;
  }
  io_add_write_channel(putch);
  test_display();
}
