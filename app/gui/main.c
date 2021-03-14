#include "jpeg_decoder.h"
#include "screen.h"
#include "stdio.h"
#include "syscall.h"

char* buf = "hello,gui\n";

void* load_bmp(char* name) {
  char* buffer = malloc(200*1024);
  memset(buffer, 0, 200*1024);
  FILE* fp;
  fp = fopen("/dev/sda/DUCK.JPG", "r+");
  printf("fd=%d\n", fp->fd);
  u32 offset = 0;
  for (;;) {
    fseek(fp, offset, SEEK_SET);
    u32 ret = fread(buffer+offset, 1, 512, fp);
    if (ret <= 0) {
      break;
    }
    offset++;
  }
  printf("read end=%d\n", fp->fd);
  fclose(fp);
  return buffer;
}

int main(int argc, char* argv[]) {
  printf(buf);
  int fd = syscall2(SYS_OPEN, "/dev/mouse", 0);
  printf("mouse fd %d\n", fd);
  screen_init();
  screen_printf(200, 10, "hello,YiYiYa OS");
  screen_info_t* screen = screen_info();

  bitmap_t* jpeg = load_jpeg("/dev/sda/DUCK.JPG");
  // void* bmp = load_bmp("/dev/sda/DUCK.BMP");
  for (;;) {
    syscall3(SYS_READ, fd, &screen->mouse, sizeof(mouse_data_t));
    // for (u32 y = 0; y < screen->height; y++) {
    //   screen_put_pixel((screen->width - screen->height) / 2 + y, y,
    //   0x0000ff); screen_put_pixel((screen->height + screen->width) / 2 - y,
    //   y, 0xff0000);
    // }
    screen_draw_line(0, 0, 140, 140, 0xff0000);
    screen_fill_rect(10, 20, 30, 30, 0xff0000);

    screen_printf(10, 100, "mouse=%d,%d", screen->mouse.x, screen->mouse.y);
    screen_fill_rect(screen->mouse.x, screen->height - screen->mouse.y, 4, 4,
                     0x00ff00);

    screen_show_bitmap(400, 400, 227, 149, jpeg);
    // screen_show_bmp_picture(200, 200, bmp, 0, 0);
    screen_flush();
  }
  return 0;
}