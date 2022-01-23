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
  cmd[count] = 0;
  sprintf(buf, "/%s", cmd);
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
  if (series < 0) {
    kprintf("error open series\n");
  }

  if (syscall2(SYS_DUP2, series, 1) < 0) {
    print_string("err in dup2\n");
  }
  if (syscall2(SYS_DUP2, series, 0) < 0) {
    print_string("err in dup2\n");
  }
  pre_launch();

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

// must init global for armv7-a
char* scm_argv[] = {"/scheme", "-b", "/scheme.boot", NULL};
char* lua_argv[] = {
    "/lua",
    "hello.lua",
    NULL
};

void pre_launch() {
#ifdef X86
  // int fd = syscall2(SYS_OPEN, "/dev/stdin", 0);
  // syscall2(SYS_EXEC,"/hello",NULL);
  // syscall2(SYS_EXEC,"/gui",NULL);
  // syscall2(SYS_EXEC,"/file",NULL);
  // syscall2(SYS_EXEC, "/luat", NULL);

  syscall2(SYS_EXEC, "/etk", NULL);
  // syscall2(SYS_EXEC,"/test-rs",NULL);
  // char* argv[] = {
  //     "lua",
  //     "hello.lua",
  // };
  // syscall2(SYS_EXEC, "/lua", argv);
  // syscall2(SYS_EXEC,"/launcher",NULL);
  // syscall2(SYS_EXEC,"/track",NULL);
  // syscall2(SYS_EXEC,"/test",NULL);
  // syscall2(SYS_EXEC,"/mcroui",NULL);
  // syscall2(SYS_EXEC,"/lvgl",NULL);
  // kprintf("fd=>%d\n",fd);
  for (;;)
    ;
#elif defined(ARMV7)
  test_lcd();
#else defined(ARM)
  // syscall2(SYS_EXEC,"/hello-rs",NULL);
  // syscall2(SYS_EXEC,"/test-rs",NULL);
  // syscall2(SYS_EXEC,"/ls",NULL);
  // syscall2(SYS_EXEC, "/test", NULL);
  // syscall2(SYS_EXEC,"/hello",NULL);
  // syscall2(SYS_EXEC, "/lvgl", NULL);
  // syscall2(SYS_EXEC, "/launcher", NULL);

  // syscall2(SYS_EXEC,"/track",NULL);
  // syscall2(SYS_EXEC,"/gui",NULL);
  // syscall2(SYS_EXEC,"/etk",NULL);
  //  syscall2(SYS_EXEC,"/test",NULL);
  //  syscall2(SYS_EXEC,"/microui",NULL);

  // syscall2(SYS_EXEC, "/lua", lua_argv);

  syscall2(SYS_EXEC,"/test-musl",NULL);
  // syscall2(SYS_EXEC, "/scheme", scm_argv);
// test_cpu_speed();
#endif
}