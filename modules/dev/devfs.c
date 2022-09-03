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

voperator_t no_rw_operator = {.close = device_close,
                              .read = device_read,
                              .write = device_write,
                              .open = vfs_open,
                              .find = vfs_find,
                              .mount = vfs_mount,
                              .readdir = vfs_readdir};

voperator_t device_operator = {.ioctl = device_ioctl,
                               .close = device_close,
                               .open = device_open,
                               .read = device_read,
                               .write = device_write,
                               .find = vfs_find,
                               .mount = vfs_mount,
                               .readdir = vfs_readdir};

vnode_t *devfs_create_device(device_t *dev) {
  vnode_t *t = vfs_create_node("dev", V_DIRECTORY);
  t->flags = V_BLOCKDEVICE | V_DIRECTORY;
  t->op = &no_rw_operator;
  t->device = dev;
  return t;
}

int devfs_init(void) {
  vnode_t *node_dev = vfs_create_node("dev", V_DIRECTORY);
  vfs_mount(NULL, "/", node_dev);
  char *name;
  vnode_t *root_super = NULL;
  for (int i = 0; i < 3; i++) {
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
    vfs_mount(NULL, "/dev", node_sda);
    if (root_super == NULL) {
      root_super = node_sda;
    }
  }

  // auto mount first dev as root
  vnode_t *root = vfs_find(NULL, "/");
  root->super = root_super;

  // SYS_READ,SYS_WRITE
  vnode_t *stdin = vfs_create_node("stdin", V_FILE);
  vnode_t *stdout = vfs_create_node("stdout", V_FILE);
  vnode_t *stderr = vfs_create_node("stderr", V_FILE);
  vfs_mount(NULL, "/dev", stdin);
  vfs_mount(NULL, "/dev", stdout);
  vfs_mount(NULL, "/dev", stderr);

  stdin->op = &device_operator;
  stdout->op = &device_operator;
  stderr->op = &device_operator;

  stdin->device = device_find(DEVICE_KEYBOARD);
  stdout->device = device_find(DEVICE_VGA);

  if (stdout->device == NULL) {
    stdout->device = device_find(DEVICE_VGA_QEMU);
  }
  if (stdout->device == NULL) {
    stdout->device = device_find(DEVICE_SERIAL);
  }
  stderr->device = stdout->device;

  // series
  vnode_t *series = vfs_create_node("series", V_FILE);
  vfs_mount(NULL, "/dev", series);
  series->device = device_find(DEVICE_SERIAL);

  series->op = &device_operator;

  // frambuffer
  device_t *fb_dev = device_find(DEVICE_VGA);
  if (fb_dev == NULL) {
    fb_dev = device_find(DEVICE_VGA_QEMU);
  }
  if (fb_dev != NULL) {
    vnode_t *frambuffer = vfs_create_node("fb", V_FILE);
    vfs_mount(NULL, "/dev", frambuffer);
    frambuffer->device = fb_dev;
    frambuffer->op = &device_operator;
  } else {
    kprintf("dev fb not found\n");
  }

  // mouse
  device_t *mouse_dev = device_find(DEVICE_MOUSE);
  if (mouse_dev != NULL) {
    vnode_t *mouse = vfs_create_node("mouse", V_FILE);
    vfs_mount(NULL, "/dev", mouse);
    mouse->device = mouse_dev;
    mouse->op = &device_operator;
  } else {
    kprintf("dev mouse not found\n");
  }

  // time
  device_t *rtc_dev = device_find(DEVICE_RTC);
  if (rtc_dev != NULL) {
    vnode_t *time = vfs_create_node("time", V_FILE);
    vfs_mount(NULL, "/dev", time);
    time->device = rtc_dev;
    time->op = &device_operator;
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

  // dsp
  vnode_t *dsp = vfs_create_node("dsp", V_FILE | V_BLOCKDEVICE);
  dsp->device = device_find(DEVICE_SB);
  dsp->op = &device_operator;

  vfs_mount(NULL, "/dev", dsp);

  // net
  vnode_t *net = vfs_create_node("net", V_FILE | V_BLOCKDEVICE);
  net->device = device_find(DEVICE_NET);
  net->op = &device_operator;
  vfs_mount(NULL, "/dev", net);

  return 0;
}

void devfs_exit(void) { kprintf("devfs exit\n"); }

module_t devfs_module = {
    .name = "devfs", .init = devfs_init, .exit = devfs_exit};