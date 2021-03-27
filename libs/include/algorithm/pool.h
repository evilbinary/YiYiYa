/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef POOL_H
#define POOL_H

#include "fun_define.h"
#include "types.h"

typedef struct pool {
  u32 *next;
  u32 *end;
} pool_t;

pool_t *pool_create(u32 size);

void *pool_alloc(pool_t *p, size_t size, u32 align);

void pool_destroy(pool_t *p);

size_t pool_available(pool_t *p);

#endif
