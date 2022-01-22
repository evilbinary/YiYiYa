/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

#define VERSION "1.1"

const char* logo =
    " _  _  _ _  _  _      \n"
    "| || || | || || |     \n"
    "| \\| |/ | \\| |/ __  __\n"
    " \\_   _/ \\_   _/  \\/ /\n"
    "   | |     | |( ()  < \n"
    "   |_|     |_| \\__/\\_\\\n\n"
    "2021 - 2080 Copyright by evilbinary \n";

const char* build_str = "version " VERSION " " __DATE__ " " __TIME__ "\n";

const char* welcome = "\nWelcome to YiYiYa Os ^_^! \n\n";

void print_string(char* str) { syscall1(SYS_PRINT, str); }

void print_logo() {
  print_string(logo);
  print_string(build_str);
  print_string(welcome);
}

void print_promot() { print_string("yiyiya$"); }

void print_help() { print_string("supports help ps command\n"); }

void ps_command() { syscall0(SYS_DUMPS); }

int do_exec(char* cmd, int count) {
  char buf[64];
  cmd[count]=0;
  sprintf(buf, "/dev/sda/%s", cmd);
  return syscall2(SYS_EXEC, buf, NULL);
}

void do_shell_cmd(char* cmd, int count) {
  print_string("\n");
  if (count == 0) return;
  if (kstrncmp(cmd, "help", count) == 0) {
    print_help();
  } else if (kstrncmp(cmd, "ps", count) == 0) {
    ps_command();
  } else {
    int ret = do_exec(cmd, count);
    if (ret < 0) {
      print_string(cmd);
      print_string(" command not support\n");
    }
  }
}

void pre_launch();


void do_shell_thread(void) {
  print_logo();
  int count = 0;
  char buf[64];
  int ret = 0;
  print_promot();
  int series = syscall2(SYS_OPEN, "/dev/series", 0);
  if(series<0){
    kprintf("error open series\n");
  }
  pre_launch();

  if (syscall2(SYS_DUP2, series, 1) < 0) {
    print_string("err in dup2\n");
  }
  if (syscall2(SYS_DUP2, series, 0) < 0) {
    print_string("err in dup2\n");
  }

  for (;;) {
    int ch = 0;
    ret = syscall3(SYS_READ, 0, &ch, 1);
    if (ret > 0) {
      if (ch == '\r' || ch == '\n') {
        do_shell_cmd(buf, count);
        count = 0;
        print_promot();
      } else {
        syscall3(SYS_WRITE, 1, &ch, 1);
        buf[count++] = ch;
      }
    }
  }
}

//must init global for armv7-a
char* argv[] = {
        "/dev/sda/scheme",
        NULL
};
void pre_launch() {
#ifdef X86
  // int fd = syscall2(SYS_OPEN, "/dev/stdin", 0);
  // syscall2(SYS_EXEC,"/dev/sda/hello",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/gui",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/file",NULL);
  // syscall2(SYS_EXEC, "/dev/sda/luat", NULL);

  syscall2(SYS_EXEC, "/dev/sda/etk", NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test-rs",NULL);
  // char* argv[] = {
  //     "lua",
  //     "hello.lua",
  // };
  // syscall2(SYS_EXEC, "/dev/sda/lua", argv);
  // syscall2(SYS_EXEC,"/dev/sda/launcher",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/track",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/mcroui",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/lvgl",NULL);
  // kprintf("fd=>%d\n",fd);
  for (;;)
    ;
#elif defined(ARMV7)
  test_lcd();
#else defined(ARM)
  // syscall2(SYS_EXEC,"/dev/sda/hello-rs",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test-rs",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/ls",NULL);
  // syscall2(SYS_EXEC, "/dev/sda/test", NULL);
  // syscall2(SYS_EXEC,"/dev/sda/hello",NULL);
  // syscall2(SYS_EXEC, "/dev/sda/lvgl", NULL);
  // syscall2(SYS_EXEC, "/dev/sda/launcher", NULL);

  // syscall2(SYS_EXEC,"/dev/sda/track",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/gui",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/etk",NULL);
  //  syscall2(SYS_EXEC,"/dev/sda/test",NULL);
  //  syscall2(SYS_EXEC,"/dev/sda/microui",NULL);

  // char* argv[] = {
  //     "lua",
  //     "hello.lua",
  // };
  // syscall2(SYS_EXEC, "/dev/sda/lua", argv);

  // syscall2(SYS_EXEC,"/test-musl",NULL);
  syscall2(SYS_EXEC, "/scheme", argv);


// test_cpu_speed();
#endif
}