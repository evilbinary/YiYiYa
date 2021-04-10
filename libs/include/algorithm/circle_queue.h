/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef CIRCLE_QUEUE_H
#define CIRCLE_QUEUE_H

#include "types.h"
#include "fun_define.h"


#define CQUEUE_DROP 1
#define CQUEUE_RESIZE 2

typedef struct cqueue {
  u32 head;
  u32 tail;
  u32 is_full;
  void** elements;
  u32 size;
  u32 type;
} cqueue_t;

typedef void (*cqueue_loop_fn)(void*);

extern cqueue_t* cqueue_create(u32 size,u32 type);
void cqueue_destroy(cqueue_t* queue);
int cqueue_put(cqueue_t* queue, void* element);
void* cqueue_poll(cqueue_t* queue);

void* cqueue_peek(cqueue_t* queue);
u32 cqueue_is_empty(cqueue_t* queue);
void cqueue_resize(cqueue_t* queue);
u32 cqueue_count(cqueue_t* queue);
void cqueue_for_each(cqueue_t* queue, cqueue_loop_fn fun);
u32 cqueue_is_full(cqueue_t* queue);

u8* cqueue_poll_byte(cqueue_t* queue);
u8* cqueue_peek_byte(cqueue_t* queue);
int cqueue_put_byte(cqueue_t* queue, u8* element);
void cqueue_for_each_byte(cqueue_t* queue, cqueue_loop_fn fun);

#endif
