/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

#include "module.h"
#include "schedule.h"
#include "syscall.h"
#include "thread.h"

void start(int argc, char* argv[], char** envp) {
  boot_info_t* boot_info = envp[0];
  arch_init(boot_info);
  kmain(argc, argv);
  for (;;) {
    cpu_halt();
  }
}

void do_shell_thread(void) {
  cls();
  static const char key_map[0x3a][2] = {
      /*00*/ {0x0, 0x0},   {0x0, 0x0}, {'1', '!'},   {'2', '@'},
      /*04*/ {'3', '#'},   {'4', '$'}, {'5', '%'},   {'6', '^'},
      /*08*/ {'7', '&'},   {'8', '*'}, {'9', '('},   {'0', ')'},
      /*0c*/ {'-', '_'},   {'=', '+'}, {'\b', '\b'}, {'\t', '\t'},
      /*10*/ {'q', 'Q'},   {'w', 'W'}, {'e', 'E'},   {'r', 'R'},
      /*14*/ {'t', 'T'},   {'y', 'Y'}, {'u', 'U'},   {'i', 'I'},
      /*18*/ {'o', 'O'},   {'p', 'P'}, {'[', '{'},   {']', '}'},
      /*1c*/ {'\n', '\n'}, {0x0, 0x0}, {'a', 'A'},   {'s', 'S'},
      /*20*/ {'d', 'D'},   {'f', 'F'}, {'g', 'G'},   {'h', 'H'},
      /*24*/ {'j', 'J'},   {'k', 'K'}, {'l', 'L'},   {';', ':'},
      /*28*/ {'\'', '\"'}, {'`', '~'}, {0x0, 0x0},   {'\\', '|'},
      /*2c*/ {'z', 'Z'},   {'x', 'X'}, {'c', 'C'},   {'v', 'V'},
      /*30*/ {'b', 'B'},   {'n', 'N'}, {'m', 'M'},   {',', '<'},
      /*34*/ {'.', '>'},   {'/', '?'}, {0x0, 0x0},   {'*', '*'},
      /*38*/ {0x0, 0x0},   {' ', ' '}};
  u8 scan_code;
  u8 shf_p = 0;
  u8 ctl_p = 0;
  u8 alt_p = 0;
  u32 col = 0;
  u32 row = 0;
  syscall1(SYS_PRINT, "#");
  char buf[2] = {0};
  for (;;) {
    int ret = 0;
    set_cursor(col, row);
    // read key
    ret = syscall3(SYS_READ, 0, &scan_code, 1);
    if (ret > 0) {
      if (scan_code & 0x80) continue;
      buf[0] = key_map[scan_code & 0x7f][shf_p];
      syscall1(SYS_PRINT, buf);
      if (scan_code == 0x1c) {
        row++;
        col = 0;
        syscall1(SYS_PRINT, "#");
      }
      scan_code = 0;
      col++;
    }
  }
}

void do_thread1(void) {
  u32 i = 0;
  u32 count = 0;
  char buf[2] = {0};
  char wheel[] = {'\\', '|', '/', '-'};
  for (;;) {
    //set_cursor(0, 0);
    buf[0] = wheel[i++];
    //syscall1(SYS_PRINT, buf);
    count++;
    if (i % 4 == 0) i = 0;
  }
}

void do_thread2(void) {
  u32 i = 0;
  u32 count = 0;
  char wheel[] = {'\\', '|', '/', '-'};
  for (;;) {
    set_cursor(100, 2);
    kprintf("task2:%d %c\n", count++, wheel[i++]);
    count++;
    if (i % 4 == 0) i = 0;
  }
}

extern context_t* current_context;
int kmain(int argc, char* argv[]) {
  exception_init();
  syscall_init();

  thread_t* t0 = thread_create((u32*)&do_shell_thread);
  thread_t* t1 = thread_create((u32*)&do_thread1);
  thread_t* t2 = thread_create((u32*)&do_thread2);

  thread_run(t0);
  thread_run(t1);
  // thread_run(t2);

  schedule_init();
  module_init();
  context_restore(current_context);

  return 0;
}
