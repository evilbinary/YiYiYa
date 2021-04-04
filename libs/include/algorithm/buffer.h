/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef BUFFER_H
#define BUFFER_H

#include "circle_queue.h"
#include "fun_define.h"
#include "types.h"

typedef struct buffer buffer_t;

typedef size_t (*buffer_read_wait_fn)(buffer_t* buffer);
typedef size_t (*buffer_write_wait_fn)(buffer_t* buffer);

typedef struct buffer {
  cqueue_t* buffer;
  buffer_read_wait_fn read_wait;
  buffer_write_wait_fn write_wait;
  void* data;
} buffer_t;

buffer_t* buffer_create(u32 size, buffer_write_wait_fn write,
                        buffer_read_wait_fn read);
size_t buffer_size(buffer_t* buffer);
size_t buffer_read(buffer_t* buffer, u8* buf, size_t len);
size_t buffer_write(buffer_t* buffer, u8* buf, size_t len);
void buffer_destroy(buffer_t* buffer);

#endif
