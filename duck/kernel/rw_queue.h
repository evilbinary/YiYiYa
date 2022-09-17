
/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef RW_QUEUE_H
#define RW_QUEUE_H

#include "types.h"
#include "algorithm/circle_queue.h"

typedef struct rw_queue{
    cqueue_t* read_queue;
    cqueue_t* write_queue;
    u32 lock;
}rw_queue_t;

rw_queue_t* rw_queue_create(u32 size);
void rw_queue_read_wait(rw_queue_t* rwq);
void rw_queue_read_notify(rw_queue_t* rwq);
void rw_queue_write_wait(rw_queue_t* rwq);
void rw_queue_write_notify(rw_queue_t* rwq);
void rw_queue_destroy(rw_queue_t* rwq);

#endif