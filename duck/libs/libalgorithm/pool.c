#include "pool.h"

pool_t *pool_create(u32 size) {
  pool_t *p = (pool_t *)fn_malloc(size + sizeof(pool_t));
  p->next = (u32 *)&p[1];
  p->end = p->next + size;
  return p;
}

void *pool_alloc(pool_t *p, size_t size, u32 align) {
  size_t real_size = size + align - 1;

  if (pool_available(p) < real_size) return NULL;
  void *mem = (void *)p->next;
  void *ptr = (void *)(((u32)mem + align - 1) & ~(u32)(align - 1));
  p->next += real_size;
  return ptr;
}

void pool_destroy(pool_t *p) { fn_free(p); }

size_t pool_available(pool_t *p) { return p->end - p->next; }
