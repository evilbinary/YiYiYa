/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef SERIAL_H
#define SERIAL_H

#include "kernel/kernel.h"
#include "pci.h"


#define PORT_COM1 0x3f8
#define PORT_COM2	0x2F8
#define PORT_COM3	0x3E8
#define PORT_COM4	0x2E8


void serial_printf(char *fmt, ...);
void serial_write(char a);

#endif
