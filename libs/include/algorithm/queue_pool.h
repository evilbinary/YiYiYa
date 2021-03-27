/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef QUEUE_POOL_H
#define QUEUE_POOL_H

#include "circle_queue.h"
#include "fun_define.h"
#include "pool.h"
#include "types.h"

typedef struct queue_pool {
  cqueue_t* queue;
  pool_t* pool;
  u32 capacity;
  u32 bytes;
} queue_pool_t;

queue_pool_t* queue_pool_create(u32 size, u32 bytes);

queue_pool_t* queue_pool_create_align(u32 size, u32 bytes,u32 align);

int queue_pool_put(queue_pool_t* q, void* e);

void* queue_pool_poll(queue_pool_t* q);

void queue_pool_destroy(queue_pool_t* q);

#endif
