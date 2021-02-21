/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "kernel/device.h"
#include "kernel/module.h"
#include "kernel/vfs.h"

vnode_t *devfs_create_device(device_t *dev) {
  vnode_t *t = kmalloc(sizeof(vnode_t));
  t->flags = V_BLOCKDEVICE;
  t->write = NULL;
  t->read = NULL;
  t->data=dev;
  return t;
}

u32 std_write(vnode_t *node, u32 offset, size_t nbytes, u8 *buf){
  u32 ret = 0;
  device_t* dev = node->device;
  if (dev == NULL) {
    return ret;
  }
  ret = dev->write(dev, buf, nbytes);
}

void std_read(vnode_t *node, u32 offset, size_t nbytes, u8 *buf){
  u32 ret = 0;
  device_t* dev = node->device;
  if (dev == NULL) {
    return ret;
  }
  ret = dev->read(dev, buf, nbytes);
}

int devfs_init(void) {
  vnode_t *node_dev = vfs_create("dev", V_DIRECTORY);
  vfs_mount(NULL, "/", node_dev);
  char *name;
  for (int i = 0; i < 6; i++) {
    device_t *dev = device_find(DEVICE_SATA + i);
    if (dev == NULL) {
      continue;
    }
    name = kmalloc(4);
    name[0] = 's';
    name[1] = 'd';
    name[2] = 0x61 + i;
    name[3] = 0;
    vnode_t *node_sda = devfs_create_device(dev);
    node_sda->name = name;
    node_sda->flags = V_BLOCKDEVICE;
    vfs_mount(NULL, "/dev", node_sda);
  }


  //SYS_READ,SYS_WRITE
  vnode_t *stdin_dev = vfs_create("stdin", V_FILE);
  vnode_t *stdout_dev = vfs_create("stdout", V_FILE);
  vfs_mount(NULL, "/dev", stdin_dev);
  vfs_mount(NULL, "/dev", stdout_dev);

  stdin_dev->read=std_read;
  stdout_dev->write=std_write;

  sys_open(stdin_dev,0);
  sys_open(stdout_dev,0);

  return 0;
}

void devfs_exit(void) { kprintf("devfs exit\n"); }

module_t devfs_module = {
    .name = "devfs", .init = devfs_init, .exit = devfs_exit};