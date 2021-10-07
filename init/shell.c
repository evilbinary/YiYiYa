/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

void test_cpu_speed() {
  int* p = 0x70100000;
  for (;;) {
    for (int i = 0; i < 480; i++) {
      for (int j = 0; j < 272; j++) {
        // *p++=0xffffff;
      }
    }
    kprintf("flush=>\n");
  }
}

void do_shell_thread(void) {
  u8 scan_code;
  u8 shf_p = 0;
  u8 ctl_p = 0;
  u8 alt_p = 0;
  u32 col = 0;
  u32 row = 0;
  syscall1(SYS_PRINT, "#");
  char buf[2] = {0};
#ifdef X86
  // int fd = syscall2(SYS_OPEN, "/dev/stdin", 0);
  // syscall2(SYS_EXEC,"/dev/sda/hello",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/gui.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/file.elf",NULL);
  syscall2(SYS_EXEC, "/dev/sda/etk.elf", NULL);
  // syscall2(SYS_EXEC,"/dev/sda/launcher",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/track.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/microui.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/lvgl",NULL);
  // kprintf("fd=>%d\n",fd);
#else defined(ARM)
  // syscall2(SYS_EXEC,"/dev/sda/hello-rs",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test-rs",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test-musl.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test-musl",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/ls",NULL);
  //  syscall2(SYS_EXEC,"/dev/sda/test.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/hello",NULL);
  // syscall2(SYS_EXEC, "/dev/sda/lvgl", NULL);
  syscall2(SYS_EXEC,"/dev/sda/launcher",NULL);

  // syscall2(SYS_EXEC,"/dev/sda/track.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/gui.elf",NULL);
// syscall2(SYS_EXEC,"/dev/sda/etk.elf",NULL);
//  syscall2(SYS_EXEC,"/dev/sda/test.elf",NULL);
//  syscall2(SYS_EXEC,"/dev/sda/microui.elf",NULL);
// test_cpu_speed();
#endif
  int count = 0;
  for (;;) {
    int ret = 0;
    // kprintf("A%d ",count);
    // syscall1(SYS_PRINT, "1");
    // if(count%100==0){
    //   syscall1(SYS_PRINT, "\n");
    // }
    // read key
    // ret = syscall3(SYS_READ,fd, &scan_code, 1);
    // if (ret >= 1) {
    //   // kprintf("ret=%d %x", ret,scan_code);
    //   if (scan_code & 0x80) continue;
    //   buf[0] = key_map[scan_code & 0x7f][shf_p];
    //   // set_cursor(col, row);
    //   // kprintf("%x",key_map[scan_code & 0x7f][shf_p]);
    //   syscall3(SYS_PRINT_AT, buf, col, row);
    //   if (scan_code == 0x1c) {
    //     row++;
    //     col = 0;
    //     syscall1(SYS_PRINT, "#");
    //   }
    //   move_cursor();
    //   scan_code = 0;
    //   col++;
    // }
    count++;
  }
}