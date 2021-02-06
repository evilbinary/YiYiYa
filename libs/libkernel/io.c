/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "io.h"

write_channel_fn write_channels[10];
u32 write_channel_number = 0;

void io_add_write_channel(write_channel_fn fn) {
  write_channels[write_channel_number++] = fn;
}


void print_char(u8 ch) {
  for (int i = 0; i < write_channel_number; i++) {
    write_channel_fn fn = write_channels[i];
    if (fn != NULL) {
      fn(ch);
    }
  }
}

void kprintf(const char* format, ...) {
  char** arg = (char**)&format;
  u8 c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0) {
    if (c != '%')
      print_char(c);
    else {
      char* p;

      c = *format++;
      switch (c) {
        case 'd':
        case 'u':
        case 'x':
          itoa(buf, c, *((int*)arg++));
          p = buf;
          goto string;
          break;

        case 's':
          p = *arg++;
          if (!p) p = "(null)";

        string:
          while (*p) print_char(*p++);
          break;

        default:
          print_char(*((int*)arg++));
          break;
      }
    }
  }
}