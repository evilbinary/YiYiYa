#include "buffer.h"

buffer_t* buffer_create(u32 size, buffer_write_wait_fn write,
                        buffer_read_wait_fn read) {
  buffer_t* buffer = fn_malloc(sizeof(buffer_t));
  buffer->buffer = cqueue_create(size, CQUEUE_DROP);
  buffer->write_wait = write;
  buffer->read_wait = read;
  return buffer;
}

size_t buffer_size(buffer_t* buffer) { return cqueue_count(buffer->buffer); }

size_t buffer_read(buffer_t* buffer, u8* buf, size_t len) {
  int i = 0;
  for (; i <len; i++) {
    if (!cqueue_is_empty(buffer->buffer)) {
      u8 byte = cqueue_poll_byte(buffer->buffer);
      buf[i] = byte;
    } else {
      if (buffer->read_wait != NULL) {
        buffer->read_wait(buffer);
      } else {
        break;
      }
    }
  }
  return i;
}

size_t buffer_write(buffer_t* buffer, u8* buf, size_t len) {
  int i = 0;
  for (; i < len; i++) {
    if (!cqueue_is_full(buffer->buffer)) {
      u8 byte = buf[i];
      cqueue_put_byte(buffer->buffer, byte);
    } else {
      if (buffer->write_wait != NULL) {
        buffer->write_wait(buffer);
      } else {
        break;
      }
    }
  }
  return i;
}

void buffer_destroy(buffer_t* buffer) {
  cqueue_destroy(buffer->buffer);
  fn_free(buffer);
}