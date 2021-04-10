/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "arch/arch.h"


void schedule_init();
void schedule(interrupt_context_t* interrupt_context);
void schedule_next();

#endif