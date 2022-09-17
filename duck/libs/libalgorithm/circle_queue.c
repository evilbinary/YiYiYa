#include "circle_queue.h"

cqueue_t* cqueue_create(u32 size, u32 type) {
  cqueue_t* queue = fn_malloc(sizeof(cqueue_t));
  if (queue) {
    queue->type = type;
    queue->elements = fn_malloc(size * sizeof(void*)+1);
    if (queue->elements) {
      queue->size = size;
      queue->head = 0;
      queue->tail = 0;
      queue->is_full = 0;
    } else {
      fn_free(queue);
      queue = NULL;
    }
  }
  return queue;
}

void cqueue_destroy(cqueue_t* queue) {
  if (queue) {
    fn_free(queue->elements);
    fn_free(queue);
  }
}

u32 cqueue_is_full(cqueue_t* queue) { return queue->is_full == 1; }

u32 cqueue_is_empty(cqueue_t* queue) {
  return (queue->head == queue->tail) && !queue->is_full;
}

void* cqueue_poll(cqueue_t* queue) {
  void* data = NULL;
  if (!cqueue_is_empty(queue)) {
    if (queue->is_full) {
      queue->is_full = 0;
    }
    data = queue->elements[queue->head++];
    if (queue->head == queue->size) {
      queue->head = 0;
    }
  }
  return data;
}

int cqueue_put(cqueue_t* queue, void* element) {
  unsigned int result;
  if (queue->is_full) {
    if (queue->type == CQUEUE_DROP) {
      cqueue_poll(queue);
    } else if (queue->type == CQUEUE_RESIZE) {
      cqueue_resize(queue);
    }
    if (queue->is_full) {
      result = 0;
    }
  }
  if (!queue->is_full) {
    queue->elements[queue->tail++] = element;
    if (queue->tail == queue->size) {
      queue->tail = 0;
    }
    if (queue->tail == queue->head) {
      queue->is_full = 1;
    }
    result = 1;
  }
  return result;
}

void* cqueue_peek(cqueue_t* queue) {
  void* data = NULL;
  if (!cqueue_is_empty(queue)) {
    data = queue->elements[queue->head];
  }
  return data;
}

void cqueue_resize(cqueue_t* queue) {
  void** temp = fn_malloc(queue->size * 2 * sizeof(void*));
  if (temp) {
    unsigned int i = 0;
    unsigned int h = queue->head;
    do {
      temp[i] = queue->elements[h++];
      if (h == queue->size) {
        h = 0;
      }
      i++;
    } while (h != queue->tail);
    fn_free(queue->elements);
    queue->elements = temp;
    queue->head = 0;
    queue->tail = queue->size;
    queue->size *= 2;
    queue->is_full = 0;
  }
}


int cqueue_put_byte(cqueue_t* queue, u8* element) {
  unsigned int result;
  if (queue->is_full) {
    if (queue->type == CQUEUE_DROP) {
      cqueue_poll_byte(queue);
    } else if (queue->type == CQUEUE_RESIZE) {
      cqueue_resize(queue);
    }
    if (queue->is_full) {
      result = 0;
    }
  }
  if (!queue->is_full) {
    ((u8*)queue->elements)[queue->tail++] = element;
    if (queue->tail == queue->size) {
      queue->tail = 0;
    }
    if (queue->tail == queue->head) {
      queue->is_full = 1;
    }
    result = 1;
  }
  return result;
}

u8* cqueue_poll_byte(cqueue_t* queue) {
  void* data = NULL;
  if (!cqueue_is_empty(queue)) {
    if (queue->is_full) {
      queue->is_full = 0;
    }
    data = ((u8*)queue->elements)[queue->head++];
    if (queue->head == queue->size) {
      queue->head = 0;
    }
  }
  return data;
}

u8* cqueue_peek_byte(cqueue_t* queue) {
  void* data = NULL;
  if (!cqueue_is_empty(queue)) {
    data = ((u8*)queue->elements)[queue->head];
  }
  return data;
}

u32 cqueue_count(cqueue_t* queue) {
  u32 count;
  if (cqueue_is_empty(queue)) {
    count = 0;
  } else if (queue->is_full) {
    count = queue->size;
  } else if (queue->tail > queue->head) {
    count = queue->tail - queue->head;
  } else {
    count = queue->size - queue->head;
    if (queue->tail > 0) {
      count += queue->tail - 1;
    }
  }
  return count;
}

void cqueue_for_each(cqueue_t* queue, cqueue_loop_fn fun) {
  if (!cqueue_is_empty(queue)) {
    u32 h = queue->head;
    do {
      fun(queue->elements[h++]);
      if (h == queue->size) {
        h = 0;
      }
    } while (h != queue->tail);
  }
}

void cqueue_for_each_byte(cqueue_t* queue, cqueue_loop_fn fun) {
  if (!cqueue_is_empty(queue)) {
    u32 h = queue->head;
    do {
      fun(((u8*)queue->elements)[h++]);
      if (h == queue->size) {
        h = 0;
      }
    } while (h != queue->tail);
  }
}
