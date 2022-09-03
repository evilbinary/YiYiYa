/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "io.h"
#include "stdarg.h"

write_channel_fn write_channels[10];
u32 write_channel_number = 0;

void io_add_write_channel(write_channel_fn fn) {
  for (int i = 0; i < write_channel_number; i++) {
    if (fn == write_channels[i]) {
      return;
    }
  }
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

void kprintf(const char* fmt, ...) {
  char printf_buffer[256];
  kmemset(printf_buffer,0,256);
  int i;
	va_list args;
	va_start(args, fmt);
	i = vsprintf(printf_buffer, fmt, args);
	va_end(args);
  int len=kstrlen(printf_buffer);
  for(int i=0;i<len;i++){
    print_char(printf_buffer[i]);
  }
}