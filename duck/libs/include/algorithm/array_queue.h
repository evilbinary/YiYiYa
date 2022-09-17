/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef ARRAY_QUEUE_H
#define ARRAY_QUEUE_H

#include "types.h"

#ifndef ARRAY_QUEUE_TYPE
#define ARRAY_QUEUE_TYPE int
#endif

#ifndef ARRAY_QUEUE_TYPE_NULL
#define ARRAY_QUEUE_TYPE_NULL NULL
#endif

#ifndef ARRAY_QUEUE_SIZE
#define ARRAY_QUEUE_SIZE 64
#endif

typedef struct aqueue {
  ARRAY_QUEUE_TYPE elements[ARRAY_QUEUE_SIZE];
  u32 front;
  u32 rear;
} aqueue_t;

int aqueue_is_full(aqueue_t *q) {
  return (q->rear + 1) % ARRAY_QUEUE_SIZE == q->front;
}

int aqueue_is_empty(aqueue_t *q) { return q->front == q->rear; }

int aqueue_push(aqueue_t *q, ARRAY_QUEUE_TYPE n) {
  if (aqueue_is_full(q)) {
    return -1;
  }
  q->elements[q->rear] = n;
  q->rear = (q->rear + 1) % ARRAY_QUEUE_SIZE;
  return 1;
}

ARRAY_QUEUE_TYPE aqueue_pop(aqueue_t *q) {
  if (aqueue_is_empty(q)) {
    return ARRAY_QUEUE_TYPE_NULL;
  }
  ARRAY_QUEUE_TYPE temp = q->elements[q->front];
  q->front = (q->front + 1) % ARRAY_QUEUE_SIZE;
  return temp;
}

int aqueue_size(aqueue_t *q) {
  return (q->rear + ARRAY_QUEUE_SIZE - q->front) % ARRAY_QUEUE_SIZE;
}

ARRAY_QUEUE_TYPE aqueue_head(aqueue_t *q) {
  if (aqueue_is_empty(q)) {
    return ARRAY_QUEUE_TYPE_NULL;
  }
  return q->elements[q->front];
}

#endif
