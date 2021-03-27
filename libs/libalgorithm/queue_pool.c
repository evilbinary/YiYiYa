#include "queue_pool.h"

queue_pool_t* queue_pool_create_align(u32 size, u32 bytes,u32 align){
  queue_pool_t* q = fn_malloc(sizeof(queue_pool_t));
  q->capacity = size;
  q->pool = pool_create(size * bytes);
  q->bytes = bytes;
  q->queue = cqueue_create(size, CQUEUE_RESIZE);
  for (int i = 0; i < size; i++) {
    void* e = pool_alloc(q->pool, bytes,align);
    cqueue_put(q->queue, e);
  }
  return q;
}

queue_pool_t* queue_pool_create(u32 size, u32 bytes) {
  return queue_pool_create_align(size,bytes,0);
}

int queue_pool_put(queue_pool_t* q, void* e) { return cqueue_put(q->queue, e); }

void* queue_pool_poll(queue_pool_t* q) { return cqueue_poll(q->queue); }

void queue_pool_destroy(queue_pool_t* q) {
  cqueue_destroy(q->queue);
  pool_destroy(q->pool);
  fn_free(q);
}
