/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#ifndef LOCK_H
#define LOCK_H

#include "libs/include/types.h"

typedef struct {
  u32 ticket;
  u32 turn;
} lock_t;

void lock_init(lock_t* lock);

void lock_acquire(lock_t* lock);

void lock_release(lock_t* lock);

void acquire(u32* lock);
void release(u32* lock);


#endif