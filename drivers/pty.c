#include "pty.h"

size_t pty_ioctl(vnode_t *node, u32 cmd, va_list args) {
  if (cmd == IOC_SLAVE) {
    pty_t *pty = node->device;
    return pty->slave->child_size;
  }
  return -1;
}

u32 pty_open(vnode_t *node) {
  pty_t *pty = node->device;
  return pyt_slave_add(pty->slave);
}

u32 pty_master_read(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  pty_t *pty = node->device;
  if(pty==NULL){
      kprintf("pyt read null\n");
      return -1;
  }
  return buffer_read(pty->out, buffer, size);
}

u32 pty_master_write(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  pty_t *pty = node->device;
  if(pty==NULL){
      kprintf("pyt write null\n");
      return -1;
  }
  return buffer_write(pty->in, buffer, size);
}

u32 pty_slave_read(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  pty_t *pty = node->device;
  if(pty==NULL){
      kprintf("pyt slave read null\n");
      return -1;
  }
  return buffer_read(pty->in, buffer, size);
}

u32 pty_slave_write(vnode_t *node, u32 offset, u32 size, u8 *buffer) {
  pty_t *pty = node->device;
  if(pty==NULL){
      kprintf("pyt slave write null\n");
      return -1;
  }
  return buffer_write(pty->out, buffer, size);
}


size_t pty_buffer_read_wait(buffer_t *buffer) {
  thread_sleep(1);
  return 1;
}

size_t pty_buffer_write_wait(buffer_t *buffer) {
  thread_sleep(1);
  return 1;
}

int pyt_slave_add(vnode_t *pts) {
  char buf[128];
  int i = pts->child_size;
  sprintf(buf, "%d", i);
  vnode_t *child = vfs_create(buf, V_FILE);
  child->device = pts;
  vfs_add_child(pts, child);
  return i;
}

int pty_create(vnode_t **ptm, vnode_t **pts) {
  pty_t *pty = kmalloc(sizeof(pty_t));
  pty->in =
      buffer_create(PTY_BUFFER, pty_buffer_write_wait, pty_buffer_read_wait);
  pty->out =
      buffer_create(PTY_BUFFER, pty_buffer_write_wait, pty_buffer_read_wait);

  pty->master = vfs_create("ptm", V_DIRECTORY);
  pty->slave = vfs_create("pts", V_DIRECTORY);

  *ptm = pty->master;
  *pts = pty->slave;

  pty->master->open = pty_open;
  pty->master->read = pty_master_read;
  pty->master->write = pty_master_write;
  pty->master->ioctl = pty_ioctl;

  pty->slave->read = pty_slave_read;
  pty->slave->write = pty_slave_write;
  pty->slave->device=pty;

  pty->master->device = pty;

  return 1;
}
