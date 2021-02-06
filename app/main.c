/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "main.h"

extern context_t* current_context;
extern module_t keyboard_module;
extern module_t hello_module;
extern module_t pci_module;
extern module_t vga_module;
extern module_t qemu_module;
extern module_t mouse_module;
extern module_t serial_module;

extern void do_screen_thread(void);

static u32 main_lock = 0;

void start(int argc, char* argv[], char** envp) {
  boot_info_t* boot_info = envp[0];
  arch_init(boot_info);
  kmain(argc, argv);
  for (;;) {
    cpu_halt();
  }
}

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
  for (;;) {
    int ret = 0;
    // read key
    ret = syscall3(SYS_READ, 0, &scan_code, 1);
    if (ret >= 1) {
      // kprintf("ret=%d %x", ret,scan_code);
      if (scan_code & 0x80) continue;
      buf[0] = key_map[scan_code & 0x7f][shf_p];
      // set_cursor(col, row);
      // kprintf("%x",key_map[scan_code & 0x7f][shf_p]);
      syscall3(SYS_PRINT_AT, buf, col, row);
      if (scan_code == 0x1c) {
        row++;
        col = 0;
        syscall1(SYS_PRINT, "#");
      }
      move_cursor();
      scan_code = 0;
      col++;
    }
  }
}

void do_thread1(void) {
  u32 i = 0;
  u32 count = 0;
  char* test="hello,do_thread1\n";
  char wheel[] = {'\\', '|', '/', '-'};
  syscall3(SYS_WRITE, DEVICE_SERIAL, test,kstrlen(test));
  for (;;) {
    syscall3(SYS_PRINT_AT, &wheel[i++], 100, 1);
    count++;
    if (i % 4 == 0) i = 0;
  }
}

int kmain(int argc, char* argv[]) {
  exception_init();
  syscall_init();

  thread_t* t0 = thread_create((u32*)&do_shell_thread);
  thread_t* t1 = thread_create((u32*)&do_thread1);
  thread_t* t2 = thread_create((u32*)&do_screen_thread);

  thread_run(t0);
  thread_run(t1);
  thread_run(t2);

  schedule_init();
  module_init();
  cls();
  module_regit(&keyboard_module);
  module_regit(&pci_module);
  module_regit(&serial_module);

  // module_regit(&vga_module);
  module_regit(&qemu_module);
  module_regit(&mouse_module);

  context_restore(current_context);

  return 0;
}
