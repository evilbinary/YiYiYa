/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "kernel/device.h"
#include "kernel/fd.h"
#include "kernel/module.h"
#include "kernel/vfs.h"
#include "pty/pty.h"

vnode_t *devfs_create_device(device_t *dev) {
  vnode_t *t = kmalloc(sizeof(vnode_t));
  t->flags = V_BLOCKDEVICE;
  t->write = NULL;
  t->read = NULL;
  t->device = dev;
  return t;
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

  // SYS_READ,SYS_WRITE
  vnode_t *stdin = vfs_create("stdin", V_FILE);
  vnode_t *stdout = vfs_create("stdout", V_FILE);
  vfs_mount(NULL, "/dev", stdin);
  vfs_mount(NULL, "/dev", stdout);

  stdin->device = device_find(DEVICE_KEYBOARD);
  stdout->device = device_find(DEVICE_SERIAL);
  if (stdin->device == NULL) {
    stdin->device = device_find(DEVICE_SERIAL);
  }
  if (stdout->device == NULL) {
    stdout->device = device_find(DEVICE_VGA);
  }

  stdin->read = device_read;
  stdout->write = device_write;

  // frambuffer
  device_t *fb_dev = device_find(DEVICE_VGA);
  if (fb_dev == NULL) {
    fb_dev = device_find(DEVICE_VGA_QEMU);
  }
  if (fb_dev != NULL) {
    vnode_t *frambuffer = vfs_create("fb", V_FILE);
    vfs_mount(NULL, "/dev", frambuffer);
    frambuffer->device = fb_dev;
    frambuffer->write = device_write;
    frambuffer->ioctl = device_ioctl;
  } else {
    kprintf("dev fb not found\n");
  }

  // mouse
  device_t *mouse_dev = device_find(DEVICE_MOUSE);
  if (mouse_dev != NULL) {
    vnode_t *mouse = vfs_create("mouse", V_FILE);
    vfs_mount(NULL, "/dev", mouse);
    mouse->device = mouse_dev;
    mouse->read = device_read;
    mouse->ioctl = device_ioctl;
  } else {
    kprintf("dev mouse not found\n");
  }

  // time
  device_t *rtc_dev = device_find(DEVICE_RTC);
  if (rtc_dev != NULL) {
    vnode_t *time = vfs_create("time", V_FILE);
    vfs_mount(NULL, "/dev", time);
    time->device = rtc_dev;
    time->read = device_read;
    time->ioctl = device_ioctl;
  } else {
    kprintf("dev time not found\n");
  }

  // pty
  vnode_t *pts = NULL;
  vnode_t *ptm = NULL;
  pty_create(&ptm, &pts);
  vfs_mount(NULL, "/dev", ptm);
  vfs_mount(NULL, "/dev", pts);

  fd_std_init();

  return 0;
}

void devfs_exit(void) { kprintf("devfs exit\n"); }

module_t devfs_module = {
    .name = "devfs", .init = devfs_init, .exit = devfs_exit};