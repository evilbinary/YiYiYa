/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "fd.h"

#include "device.h"
#include "vfs.h"

fd_t fd_list[512];
int fd_number = 0;

fd_t* fd_find(u32 fd) {
  for (int i = 0; i < fd_number; i++) {
    if (fd_list[i].id == fd) {
      return &fd_list[i];
    }
  }
  return NULL;
}

fd_t* fd_new(u32* file, u32 type, char* name) {
  if (fd_number > 512) {
    kprintf("new fd limit \n");
    return NULL;
  }
  fd_list[fd_number].id = fd_number;
  fd_list[fd_number].type = type;
  fd_list[fd_number].data = file;
  fd_list[fd_number].offset = 0;
  fd_list[fd_number].name = name;
  return &fd_list[fd_number++];
}

int fd_std_init() {
  char* stdin = "/dev/stdin";
  vnode_t* file = vfs_open(NULL, stdin);
  fd_t* fdstdin = fd_new(file, DEVICE_TYPE_FILE, stdin);
  if (fdstdin == NULL) {
    kprintf(" new fdstdin error\n");
    return -1;
  }
  char* stdout = "/dev/stdout";
  file = vfs_open(NULL, stdout);
  fd_t* fdstdout = fd_new(file, DEVICE_TYPE_FILE, stdout);
  if (fdstdout == NULL) {
    kprintf(" new fdstdout error\n");
    return -1;
  }

  char* stderro = "/dev/stdout";
  fd_t* fdstderro = fd_new(file, DEVICE_TYPE_FILE, stderro);
  if (fdstderro == NULL) {
    kprintf(" new fdstderro error\n");
    return -1;
  }
  return 1;
}
int fd_init(){
  return 1;
}