/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "serial.h"


int serial_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "serial";
//   dev->read = read;
//   dev->write = write;
  dev->id = DEVICE_SERIAL;
  dev->type = DEVICE_TYPE_CHAR;
  device_add(dev);

  // uart_init();
  return 0;
}

void serial_exit(void) { kprintf("serial exit\n"); }

module_t serial_module = {
    .name = "serial", .init = serial_init, .exit = serial_exit};
