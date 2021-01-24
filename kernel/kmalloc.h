/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef KMALLOC_H
#define KMALLOC_H

#include "arch/arch.h"


void* kmalloc(size_t size);

void kfree(void* ptr);

#endif