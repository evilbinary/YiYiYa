/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

void test_gui() {
#ifdef XTENSA
  char wheel[] = {'\\', '|', '/', '-'};
  // screen_init();
  int i = 0,j=0;
  for (;;) {
    // screen_printf(0, 0, "Hello YiYiYa Os\n");
    // screen_fill_rect(0, 40, 30, 30, 0x00ff00);
    // screen_draw_line(0, 0, 140, 140, 0xff0000);
    // screen_flush();
  }
#endif
}

void do_serial_thread(void) {
  u32 i = 0;
  u32 count = 0;
  char* test = "hello,do serial thread\n";
  char wheel[] = {'\\', '|', '/', '-'};
  char buf[512];
  kmemset(buf, 1, 512);
  // syscall1(SYS_PRINT, "2");
  // syscall3(SYS_WRITE, DEVICE_SERIAL, test, kstrlen(test));
  // int fd = syscall2(SYS_OPEN, "/dev/sda/B.TXT", 0);
  for (;;) {
    // kprintf("B%d ",count);
    // syscall1(SYS_PRINT, "2");
    // if(count%100==0){
    //   // syscall1(SYS_PRINT, "\n");
    // }
    // test_gui();
    // syscall1(SYS_PRINT, "\n");
    // syscall3(SYS_READ, fd, buf, 512);
    // syscall3(SYS_PRINT_AT, &wheel[i++], 100, 1);
    // syscall3(DEV_READ,DEVICE_SATA,buf,512);
    // syscall3(DEV_IOCTL, DEVICE_SATA, IOC_WRITE_OFFSET, 0x400);
    // memset(buf, 1, 512);
    // syscall3(DEV_WRITE, DEVICE_SATA, buf, 512);
    // memset(buf, 0, 512);
    // syscall3(DEV_READ, DEVICE_SATA, buf, 512);
    count++;
    if (i % 4 == 0) i = 0;
  }
}
