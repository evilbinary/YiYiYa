#include "buffer.h"

buffer_t* buffer_create(u32 size, buffer_write_wait_fn write,
                        buffer_read_wait_fn read, buffer_write_notify_fn wn,
                        buffer_read_notify_fn rn) {
  buffer_t* buffer = fn_malloc(sizeof(buffer_t));
  buffer->buffer = cqueue_create(size, CQUEUE_DROP);
  buffer->write_wait = write;
  buffer->read_wait = read;
  buffer->read_notify = rn;
  buffer->write_notify = wn;
  return buffer;
}

size_t buffer_size(buffer_t* buffer) { return cqueue_count(buffer->buffer); }

size_t buffer_read(buffer_t* buffer, u8* buf, size_t len) {
#ifdef DEBUG_BUFFER
  kprintf("buffer read size %d\n",cqueue_count(buffer->buffer));
#endif
  int i = 0;
  if (buffer->read_wait != NULL) {
    if (cqueue_is_empty(buffer->buffer)) {
      buffer->read_wait(buffer);
    }
  }
  // kprintf("buffer read dump\n");
  // buffer_dump(buffer);
  for (; i < len;) {
    if (!cqueue_is_empty(buffer->buffer)) {
      u8 byte = cqueue_poll_byte(buffer->buffer);
      buf[i] = byte;
      i++;
    }else{
      break;
    }
  }
  if (i > 0) {
    if (buffer->write_notify != NULL) {
      buffer->write_notify(buffer);
    }
  }
  return i;
}

size_t buffer_write(buffer_t* buffer, u8* buf, size_t len) {
#ifdef DEBUG_BUFFER
  kprintf("buffer write size %d\n",cqueue_count(buffer->buffer));
#endif
  int i = 0;
  if (buffer->write_wait != NULL) {
    if (cqueue_is_full(buffer->buffer)) {
      buffer->write_wait(buffer);
    }
  }
  for (; i < len;) {
    if (!cqueue_is_full(buffer->buffer)) {
      u8 byte = buf[i];
      if(cqueue_put_byte(buffer->buffer, byte)>0){
        i++;
      }
    } else {
      if (buffer->write_wait != NULL) {
        buffer->write_wait(buffer);
        break;
      }
    }
  }
  if (i > 0) {
    if (buffer->read_notify != NULL) {
      buffer->read_notify(buffer);
    }
  }
  return i;
}

void buffer_print(u8* ch){
  kprintf("%c",ch);
}

void buffer_dump(buffer_t* buffer){
  cqueue_for_each_byte(buffer->buffer,buffer_print);
  kprintf("\n");
}

void buffer_destroy(buffer_t* buffer) {
  cqueue_destroy(buffer->buffer);
  fn_free(buffer);
}