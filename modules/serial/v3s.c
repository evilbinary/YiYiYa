/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "serial.h"


void serial_write(char a) {
  uart_send(a);
}

char serial_read() {
  return uart_receive();
}

void serial_printf(char* fmt, ...) {
  int i;
  char buf[1024];
  va_list args;
  va_start(args, fmt);
  i = vsprintf(buf, fmt, args);
  va_end(args);
  for (int j = 0; j < i; j++) {
    serial_write(buf[j]);
  }
}

static size_t read(device_t* dev, void* buf, size_t len) {
  u32 ret = len;
  for (int i = 0; i < len; i++) {
    int c = serial_read();
    if (c >= 0) {
      ret++;
      ((char*)buf)[i] = c;
    }
  }
  return ret;
}

static size_t write(device_t* dev, void* buf, size_t len) {
  u32 ret = len;
  for (int i = 0; i < len; i++) {
    serial_write(((char*)buf)[i]);
  }
  return ret;
}

int serial_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "serial";
  dev->read = read;
  dev->write = write;
  dev->id = DEVICE_SERIAL;
  dev->type = DEVICE_TYPE_CHAR;
  device_add(dev);

  // uart_init();
  return 0;
}

void serial_exit(void) { kprintf("serial exit\n"); }

module_t serial_module = {
    .name = "serial", .init = serial_init, .exit = serial_exit};
