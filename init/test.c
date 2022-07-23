#include "kernel/kernel.h"

#ifdef XTENSA

void test_gui() {


}
#else

void test_gui() {
  // char wheel[] = {'\\', '|', '/', '-'};
  // screen_init();
  // int i = 0, j = 0;
  // for (;;) {
  //   screen_printf(0, 0, "Hello YiYiYa Os\n");
  //   screen_fill_rect(0, 40, 30, 30, 0x00ff00);
  //   screen_draw_line(0, 0, 140, 140, 0xff0000);
  //   screen_flush();
  // }
}

int test_ioctl(int fd, int cmd, ...) {
  void* arg;
  va_list ap;
  va_start(ap, cmd);
  arg = va_arg(ap, void*);
  va_end(ap);
  int ret = syscall3(SYS_IOCTL, fd, cmd, arg);
  return ret;
}

#define RED 0xf800
#define BLUE 0x001f
#define GREEN 0x07e0
#define YELLOW 0xffe0
#define MAGENTA 0xF81F
#define CYAN 0xFFE0

#define IOC_READ_FRAMBUFFER_INFO _IOW('v', 8, int)
typedef struct framebuffer_info {
  u32 width;
  u32 height;
  u32 bpp;
  u32 mode;
  u32* frambuffer;
  u32 framebuffer_count;
  u32 framebuffer_index;
  u32 framebuffer_length;
  u32 inited;
  u32* write;
  u32* flip_buffer;
} my_framebuffer_info_t;

void test_lcd() {
  my_framebuffer_info_t fb;
  u16 buf[] = {12,12,BLUE};

  int fd = syscall2(SYS_OPEN, "/dev/fb", 0);
  kprintf("screen init fd:%d\n", fd);
  int myfd = fd;
  test_ioctl(fd, IOC_READ_FRAMBUFFER_INFO, &fb);
  kprintf("fd %d buf info %d %d\n", myfd, fb.width, fb.height);

  for (int i=0;i<10;i++) {
    int ret = syscall3(SYS_WRITE, 3, buf, sizeof(buf));
    kprintf("ret %d\n", ret);
  }
}

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



#define IOC_AHCI_MAGIC 'a'
#define IOC_READ_OFFSET _IOW(IOC_AHCI_MAGIC, 3, int)
#define IOC_WRITE_OFFSET _IOW(IOC_AHCI_MAGIC, 4, int)

#define memset kmemset
void test_SYS_DEV_READ_write() {
  char* test = "hello,do serial thread\n";
  char wheel[] = {'\\', '|', '/', '-'};
  char buf[512];
  int count = 0, i = 0;
  memset(buf, 1, 512);
  // syscall3(SYS_WRITE, DEVICE_SERIAL, test, kstrlen(test));
  syscall1(SYS_PRINT, "2");
  if (count % 100 == 0) {
    // syscall1(SYS_PRINT, "\n");
  }
  int fd = syscall2(SYS_OPEN, "/B.TXT", 0);
  test_gui();
  syscall1(SYS_PRINT, "\n");
  syscall3(SYS_READ, fd, buf, 512);
  syscall3(SYS_PRINT_AT, &wheel[i++], 100, 1);
  syscall3(SYS_DEV_READ, DEVICE_SATA, buf, 512);
  syscall3(SYS_DEV_IOCTL, DEVICE_SATA, IOC_WRITE_OFFSET, 0x400);
  memset(buf, 1, 512);
  syscall3(SYS_DEV_WRITE, DEVICE_SATA, buf, 512);
  memset(buf, 0, 512);
  syscall3(SYS_DEV_READ, DEVICE_SATA, buf, 512);
}

#endif

void test_syscall() {
  u8 scan_code;
  u8 shf_p = 0;
  u8 ctl_p = 0;
  u8 alt_p = 0;
  u32 col = 0;
  u32 row = 0;
  int count = 0, i = 0;
  int fd = 0;
  char buf[28];
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
    scan_code = 0;
    col++;
  }
}