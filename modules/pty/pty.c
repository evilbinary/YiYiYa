#include "pty.h"
#include "kernel/rw_queue.h"

voperator_t pty_master_operator = {.close = NULL,
                                   .read = pty_master_read,
                                   .write = pty_master_write,
                                   .open = pty_open,
                                   .ioctl = pty_ioctl};

voperator_t pty_slave_operator = {.close = NULL,
                                  .read = pty_slave_read,
                                  .write = pty_slave_write,
                                  .open = pty_slave_open,
                                  .ioctl = pty_ioctl};

size_t pty_ioctl(vnode_t *node, u32 cmd, va_list args) {
  if (cmd == IOC_SLAVE) {
    pty_t *pty = node->device;
    return pty->slave->child_number - 1;
  } else if (cmd == IOC_MASTER_READ_NOBLOCK) {
    pty_t *pty = node->device;
    pty->master_read_block = 0;
    return 1;
  }
  return -1;
}

u32 pty_open(vnode_t *node) {
  pty_t *pty = node->device;
#ifdef DEBUG_PTY
  kprintf("pty salve add:%s\n", node->name);
#endif
  return pty_slave_add(pty->slave);
}

u32 pty_master_read(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  pty_t *pty = node->device;
  if (pty == NULL) {
    kprintf("pyt read null\n");
    return -1;
  }
  buffer_read_wait_fn waitfn = pty->in->read_wait;
  if (pty->master_read_block == 0) {
    pty->in->read_wait = NULL;
  }
  u32 ret = buffer_read(pty->in, buffer, size);
  pty->in->read_wait = waitfn;
#ifdef DEBUG_PTY
  kprintf("pty master read ret:%d %s\n", ret, buffer);
#endif
  return ret;
}

u32 pty_master_write(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  pty_t *pty = node->device;
  if (pty == NULL) {
    kprintf("pyt write null\n");
    return -1;
  }
#ifdef DEBUG_PTY
  kprintf("pty master write %s\n", buffer);
#endif
  return buffer_write(pty->out, buffer, size);
}

u32 pty_slave_read(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  pty_t *pty = node->device;
  if (pty == NULL) {
    kprintf("pyt slave read null\n");
    return -1;
  }
#ifdef DEBUG_PTY
  kprintf("pty slave read\n");
#endif
  if (pty->out == NULL) {
    kprintf("pty slave read erro for out null\n");
    return -1;
  }
  u32 ret = buffer_read(pty->out, buffer, size);
#ifdef DEBUG_PTY
  kprintf("pty slave read ret:%d %s\n", ret, buffer);
#endif
  return ret;
}

u32 pty_slave_write(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  pty_t *pty = node->device;
  if (pty == NULL) {
    kprintf("pyt slave write null\n");
    return -1;
  }
#ifdef DEBUG_PTY
  kprintf("pty slave write %s\n", buffer);
#endif
  return buffer_write(pty->in, buffer, size);
}

u32 pty_slave_open(vnode_t *node) {
#ifdef DEBUG_PTY
  kprintf("pty slave open %s\n", node->name);
#endif
  // node->device = node->parent->device;
  // node->read = pty_slave_read;
  // node->write = pty_slave_write;
  return 1;
}

size_t pty_buffer_read_wait(buffer_t *buffer) {
  rw_queue_t *rwq = buffer->data;
  rw_queue_read_wait(rwq);
  return 1;
}

size_t pty_buffer_read_notify(buffer_t *buffer) {
  rw_queue_t *rwq = buffer->data;
  rw_queue_read_notify(rwq);
  return 1;
}

size_t pty_buffer_write_wait(buffer_t *buffer) {
  rw_queue_t *rwq = buffer->data;
  rw_queue_write_wait(rwq);
  return 1;
}

size_t pty_buffer_write_notify(buffer_t *buffer) {
  rw_queue_t *rwq = buffer->data;
  rw_queue_write_notify(rwq);
  return 1;
}

int pty_slave_add(vnode_t *pts) {
  char buf[128];
  int i = pts->child_size;
  sprintf(buf, "%d", i);
  vnode_t *child = vfs_create_node(buf, V_FILE);
  child->device = pts->device;
  child->op=&pty_slave_operator;
  vfs_add_child(pts, child);
  return i;
}

int pty_create(vnode_t **ptm, vnode_t **pts) {
  pty_t *pty = kmalloc(sizeof(pty_t));
  pty->in =
      buffer_create(PTY_BUFFER, pty_buffer_write_wait, pty_buffer_read_wait,
                    pty_buffer_write_notify, pty_buffer_read_notify);
  pty->out =
      buffer_create(PTY_BUFFER, pty_buffer_write_wait, pty_buffer_read_wait,
                    pty_buffer_write_notify, pty_buffer_read_notify);

  pty->master = vfs_create_node("ptm", V_DIRECTORY);
  pty->slave = vfs_create_node("pts", V_DIRECTORY);

  *ptm = pty->master;
  *pts = pty->slave;

  pty->master->op = &pty_master_operator;

  pty->master_read_block = 1;
  pty->slave->op = &pty_slave_operator;

  pty->slave->device = pty;

  pty->master->device = pty;

  rw_queue_t *inq = rw_queue_create(10);
  pty->in->data = inq;

  rw_queue_t *outq = rw_queue_create(10);
  pty->out->data = outq;

  return 1;
}
