
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

void test_gui() {
#ifdef XTENSA
  char wheel[] = {'\\', '|', '/', '-'};
  screen_init();
  int i = 0, j = 0;
  for (;;) {
    screen_printf(0, 0, "Hello YiYiYa Os\n");
    screen_fill_rect(0, 40, 30, 30, 0x00ff00);
    screen_draw_line(0, 0, 140, 140, 0xff0000);
    screen_flush();
  }
#endif
}

void test_dev_read_write() {
  char* test = "hello,do serial thread\n";
  char wheel[] = {'\\', '|', '/', '-'};
  char buf[512];
  kmemset(buf, 1, 512);
  // syscall3(SYS_WRITE, DEVICE_SERIAL, test, kstrlen(test));
  syscall1(SYS_PRINT, "2");
  if (count % 100 == 0) {
    // syscall1(SYS_PRINT, "\n");
  }
  int fd = syscall2(SYS_OPEN, "/dev/sda/B.TXT", 0);
  test_gui();
  syscall1(SYS_PRINT, "\n");
  syscall3(SYS_READ, fd, buf, 512);
  syscall3(SYS_PRINT_AT, &wheel[i++], 100, 1);
  syscall3(DEV_READ, DEVICE_SATA, buf, 512);
  syscall3(DEV_IOCTL, DEVICE_SATA, IOC_WRITE_OFFSET, 0x400);
  memset(buf, 1, 512);
  syscall3(DEV_WRITE, DEVICE_SATA, buf, 512);
  memset(buf, 0, 512);
  syscall3(DEV_READ, DEVICE_SATA, buf, 512);
}

void test_syscall() {
  u8 scan_code;
  u8 shf_p = 0;
  u8 ctl_p = 0;
  u8 alt_p = 0;
  u32 col = 0;
  u32 row = 0;
   syscall1(SYS_PRINT, "1");
  if (count % 100 == 0) {
    syscall1(SYS_PRINT, "\n");
  }
  int ret = syscall3(SYS_READ, fd, &scan_code, 1);
  if (ret >= 1) {
    // kprintf("ret=%d %x", ret,scan_code);
    if (scan_code & 0x80) return;
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