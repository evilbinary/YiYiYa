/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

#define VERSION "1.2"

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

void print_help() { print_string("supports help ps cd pwd command\n"); }

void ps_command() { syscall0(SYS_DUMPS); }

int do_exec(char* cmd, int count) {
  char buf[64];
  cmd[count] = 0;
  char* argv[10];
  int i = 0;
  const char* split = " ";
  char* ptr = kstrtok(cmd, split);
  argv[i++] = ptr;
  sprintf(buf, "/%s", argv[0]);
  while (ptr != NULL) {
    argv[i++] = ptr;
    ptr = kstrtok(NULL, split);
  }
  return syscall2(SYS_EXEC, buf, &argv[1]);
}

void cd_command(char* cmd, int count) {
  char buf[64];
  cmd[count] = 0;
  char* argv[64];
  int i = 0;
  const char* split = " ";
  char* ptr = kstrtok(cmd, split);
  ptr = kstrtok(NULL, split);
  print_string(ptr);
  syscall1(SYS_CHDIR, ptr);
}

void pwd_command() {
  char buf[128];
  kmemset(buf, 0, 128);
  syscall2(SYS_GETCWD, buf, 128);
  print_string(buf);
  print_string("\n");
}

void do_shell_cmd(char* cmd, int count) {
  if (count == 0) return;
  if (kstrncmp(cmd, "help", count) == 0) {
    print_help();
  } else if (kstrncmp(cmd, "cd", 2) == 0) {
    cd_command(cmd, count);
  } else if (kstrncmp(cmd, "pwd", 3) == 0) {
    pwd_command();
  } else if (kstrncmp(cmd, "ps", 2) == 0) {
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
  char buf[64];
  int count = 0;
  int ret = 0;
  print_logo();
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
  if (syscall2(SYS_DUP2, series, 2) < 0) {
    print_string("err in dup2\n");
  }

  syscall1(SYS_CHDIR, "/");
  pre_launch();

  for (;;) {
    int ch = 0;
    ret = syscall3(SYS_READ, 0, &ch, 1);
    if (ret > 0) {
      if (ch == '\r' || ch == '\n') {
        print_string("\n");
        do_shell_cmd(buf, count);
        count = 0;
        print_promot();
      } else {
        buf[count++] = ch;
        syscall3(SYS_WRITE, 1, &ch, 1);
      }
    }else{
      u32 tv[2] = { 1, 0 };
      syscall4(SYS_CLOCK_NANOSLEEP,0,0,&tv,&tv);
    }
  }
}

// must init global for armv7-a
char* scm_argv[] = {"/scheme", "-b", "/scheme.boot", "--verbose", NULL};
char* lua_argv[] = {"/lua", "/hello.lua", NULL};

char* nes_argv[] = {"infones", "/mario.nes", NULL};

char* mgba_argv[] = {"mgba", "/mario.gba", NULL};

char* cat_argv[] = {"/cat", "hello.lua", NULL};

char* showimg_argv[] = {"/showimage", "/pngtest.png", NULL};

char* gnuboy_argv[] = {"/gnuboy", "/pokemon.gbc", NULL};

void pre_launch() {
#ifdef X86
  // int fd = syscall2(SYS_OPEN, "/dev/stdin", 0);
  // syscall2(SYS_EXEC,"/hello",NULL);
  // syscall2(SYS_EXEC,"/gui",NULL);
  // syscall2(SYS_EXEC,"/test-file",NULL);
    // syscall2(SYS_EXEC,"/test-mem",NULL);
  // syscall2(SYS_EXEC,"/test-uncompress",NULL);
  // syscall2(SYS_EXEC,"/test-string",NULL);
  // syscall2(SYS_EXEC,"/test-stdlib",NULL);
  // syscall2(SYS_EXEC,"/test-stdio",NULL);

  // syscall2(SYS_EXEC, "/luat", NULL);

  // syscall2(SYS_EXEC, "/etk", NULL);
  // syscall2(SYS_EXEC,"/test-rs",NULL);
  // syscall2(SYS_EXEC, "/lua", lua_argv);
  // syscall2(SYS_EXEC,"/launcher",NULL);
  // syscall2(SYS_EXEC,"/track",NULL);
  // syscall2(SYS_EXEC,"/test",NULL);
  // syscall2(SYS_EXEC,"/microui",NULL);
  // syscall2(SYS_EXEC,"/lvgl",NULL);
  // kprintf("fd=>%d\n",fd);

  // syscall2(SYS_EXEC, "/infones", nes_argv);
  // syscall2(SYS_EXEC, "/mgba", mgba_argv);
  // syscall2(SYS_EXEC, "/scheme", scm_argv);
  // syscall2(SYS_EXEC, "/sdl2", NULL);
  // syscall2(SYS_EXEC, "/showimage", showimg_argv);
  // syscall2(SYS_EXEC, "/gnuboy", gnuboy_argv);

  

  // for (;;)
  //   ;
#elif defined(ARMV7)
  // test_lcd();
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

  // syscall2(SYS_EXEC,"/test-musl",NULL);
  // syscall2(SYS_EXEC, "/scheme", scm_argv);
  // syscall2(SYS_EXEC, "/sdl2", NULL);
  // syscall2(SYS_EXEC, "/mgba", mgba_argv);
  // syscall2(SYS_EXEC, "/player", mgba_argv);
  // syscall2(SYS_EXEC, "/cat", cat_argv);
  // syscall2(SYS_EXEC, "/infones", nes_argv);
  // syscall2(SYS_EXEC,"/test-file",NULL);

// test_cpu_speed();
//  for(;;);
#endif
}