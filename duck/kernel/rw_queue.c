
/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "rw_queue.h"

#include "thread.h"

rw_queue_t* rw_queue_create(u32 size) {
  rw_queue_t* rwq = kmalloc(sizeof(rw_queue_t));
  rwq->read_queue = cqueue_create(size, CQUEUE_DROP);
  rwq->write_queue = cqueue_create(size, CQUEUE_DROP);
  rwq->lock = 0;
  return rwq;
}

void rw_queue_read_wait(rw_queue_t* rwq) {
  acquire(&rwq->lock);
  thread_t* current = thread_current();
#ifdef DEBUG_RW_QUEUE
  kprintf("tid:%d read wait\n", current->id);
#endif
  thread_t* t = cqueue_peek(rwq->read_queue);
  if (t == NULL) {
    cqueue_put(rwq->read_queue, current);
  } else if (t != current) {
    cqueue_put(rwq->read_queue, current);
  }
  release(&rwq->lock);
  thread_wait(current);
}

void rw_queue_read_notify(rw_queue_t* rwq) {
  acquire(&rwq->lock);
  thread_t* current = thread_current();
  thread_t* t = cqueue_poll(rwq->read_queue);
  release(&rwq->lock);
  if (t != NULL) {
#ifdef DEBUG_RW_QUEUE
    kprintf("tid:%d read notify %d\n", current->id, t->id);
#endif
    thread_wake(t);
  } else {
#ifdef DEBUG_RW_QUEUE
    kprintf("tid:%d read notify null\n", current->id);
#endif
  }
}

void rw_queue_write_wait(rw_queue_t* rwq) {
  acquire(&rwq->lock);
  thread_t* current = thread_current();
#ifdef DEBUG_RW_QUEUE
  kprintf("tid:%d write wait\n", current->id);
#endif
  thread_t* t = cqueue_peek(rwq->read_queue);
  if (t == NULL) {
    cqueue_put(rwq->write_queue, current);
  } else if (t != current) {
    cqueue_put(rwq->write_queue, current);
  }
  release(&rwq->lock);
  thread_wait(current);
}

void rw_queue_write_notify(rw_queue_t* rwq) {
  acquire(&rwq->lock);
  thread_t* current = thread_current();
  thread_t* t = cqueue_poll(rwq->write_queue);
  release(&rwq->lock);
  if (t != NULL) {
#ifdef DEBUG_RW_QUEUE
    kprintf("tid:%d write notify %d\n", current->id, t->id);
#endif
    thread_wake(t);
  }
}

void rw_queue_destroy(rw_queue_t* rwq) {
  if (rwq == NULL) return;
  cqueue_destroy(rwq->read_queue);
  cqueue_destroy(rwq->write_queue);
  kfree(rwq);
}