/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "gpio.h"
#include "main.h"

const char* logo =
    " _  _  _ _  _  _      \n"
    "| || || | || || |     \n"
    "| \\| |/ | \\| |/ __  __\n"
    " \\_   _/ \\_   _/  \\/ /\n"
    "   | |     | |( ()  < \n"
    "   |_|     |_| \\__/\\_\\\n"
    "\nWelcome to YiYiYa Os ^_^! \n\n";

void print_string(char* str) { syscall1(SYS_PRINT, str); }

void print_logo() { print_string(logo); }

void print_promot() { print_string("$"); }

void print_help() { print_string("hello,world help cmd\n"); }

void do_shell_cmd(char* cmd, int count) {
  print_string("\n");
  if (kstrncmp(cmd, "help", count) == 0) {
    print_help();
  } else {
    syscall3(SYS_DEV_WRITE, DEVICE_SERIAL, cmd, count);
    print_string(" command not support\n");
  }
}

void do_shell_thread(void) {
  print_logo();
#ifdef X86
  // int fd = syscall2(SYS_OPEN, "/dev/stdin", 0);
  // syscall2(SYS_EXEC,"/dev/sda/hello",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/gui.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/file.elf",NULL);
  // syscall2(SYS_EXEC, "/dev/sda/luat", NULL);
  syscall2(SYS_EXEC, "/dev/sda/etk.elf", NULL);

  // syscall2(SYS_EXEC,"/dev/sda/test-rs",NULL);
  // char* argv[] = {
  //     "lua",
  //     "hello.lua",
  // };
  // syscall2(SYS_EXEC, "/dev/sda/lua", argv);
  // syscall2(SYS_EXEC,"/dev/sda/launcher",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/track.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/mcroui.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/lvgl",NULL);
  // kprintf("fd=>%d\n",fd);
#elif defined(ARMV7)

#else defined(ARM)
  // syscall2(SYS_EXEC,"/dev/sda/hello-rs",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test-rs",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test-musl.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test-musl",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/ls",NULL);
  //  syscall2(SYS_EXEC,"/dev/sda/test.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/hello",NULL);
  // syscall2(SYS_EXEC, "/dev/sda/lvgl", NULL);
  syscall2(SYS_EXEC, "/dev/sda/launcher", NULL);

  // syscall2(SYS_EXEC,"/dev/sda/track.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/gui.elf",NULL);
// syscall2(SYS_EXEC,"/dev/sda/etk.elf",NULL);
//  syscall2(SYS_EXEC,"/dev/sda/test.elf",NULL);
//  syscall2(SYS_EXEC,"/dev/sda/microui.elf",NULL);
// test_cpu_speed();
#endif
  int count = 0;
  char buf[32];
  int ret = 0;
  print_promot();
  for (;;) {
    int ch = 0;
    ret = syscall3(SYS_DEV_READ, DEVICE_SERIAL, &ch, 1);
    if (ch == '\r' || ch == '\n') {
      do_shell_cmd(buf, count);
      count = 0;
      print_promot();
    } else {
      syscall3(SYS_DEV_WRITE, DEVICE_SERIAL, &ch, 1);
      buf[count++] = ch;
    }
  }
}