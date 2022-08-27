/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef PIC_H
#define PIC_H

#include "kernel/kernel.h"

#define PIC1 0x20
#define PIC2 0xa0

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xa0
#define PIC2_DATA 0xa1

#define PIC_EOI 0x20

void pic_eof(u8 irq);
void pic_enable(u8 irq);
void pic_disable(u8 irq);

#endif