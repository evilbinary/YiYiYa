/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

void do_shell_thread(void) {
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
  move_cursor();
  char buf[2] = {0};

  int fd = syscall2(SYS_OPEN, "/dev/stdin", 0);
  // syscall2(SYS_EXEC,"/dev/sda/hello.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/gui.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/file.elf",NULL);
  syscall2(SYS_EXEC,"/dev/sda/etk.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/test.elf",NULL);
  // syscall2(SYS_EXEC,"/dev/sda/microui.elf",NULL);
  // kprintf("fd=>%d\n",fd);
  for (;;) {

    int ret = 0;
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
  }
}