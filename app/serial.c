/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

void do_serial_thread(void) {
  u32 i = 0;
  u32 count = 0;
  char* test = "hello,do serial thread\n";
  char wheel[] = {'\\', '|', '/', '-'};
  char buf[512];
  memset(buf, 1, 512);
  // syscall3(SYS_WRITE, DEVICE_SERIAL, test, kstrlen(test));
  //int fd = syscall2(SYS_OPEN, "/dev/sda/B.TXT", 0);
  for (;;) {
    //syscall3(SYS_READ, fd, buf, 512);
  
    // syscall3(SYS_PRINT_AT, &wheel[i++], 100, 1);
    // syscall3(DEV_READ,DEVICE_SATA,buf,512);
    // syscall3(DEV_IOCTL, DEVICE_SATA, IOC_WRITE_OFFSET, 0x400);
    // memset(buf, 1, 512);
    // syscall3(DEV_WRITE, DEVICE_SATA, buf, 512);
    // memset(buf, 0, 512);
    // syscall3(DEV_READ, DEVICE_SATA, buf, 512);

    // kprintf("A%d ",count);
    count++;
    if (i % 4 == 0) i = 0;
  }
}
