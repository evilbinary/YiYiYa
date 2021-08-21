#include "event.h"
#include "jpeg_decoder.h"
#include "screen.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"

char* buf = "hello,gui\n";

void* load_bmp(char* name) {
  char* buffer = malloc(200 * 1024);
  memset(buffer, 0, 200 * 1024);
  FILE* fp;
  fp = fopen(name, "r+");
  // printf("fd=%d\n", fp->fd);
  u32 offset = 0;
  for (;;) {
    fseek(fp, offset, SEEK_SET);
    u32 ret = fread(buffer + offset, 1, 512, fp);
    if (ret <= 0) {
      break;
    }
    offset++;
  }
  // printf("read end=%d\n", fp->fd);
  fclose(fp);
  return buffer;
}

void display_time() {
  char buffer[80];
  struct tm* timeinfo;
  time_t lt;
  time(&lt);
  timeinfo = localtime(&lt);
  strftime(buffer, 80, "%Y-%m-%d %I:%M:%S", timeinfo);
  screen_printf(840, 10, "%s\n", buffer);
}

void yiyiya_gui() {
  int fd = open("/dev/mouse", 0);
  printf("mouse fd %d\n", fd);
  screen_init();
  screen_info_t* screen = screen_info();

  bitmap_t* bitmap = load_jpeg("/dev/sda/home.jpg");
  bitmap_t* bitmap2 = load_png("/dev/sda/normal.png");
  // void* bmp = load_bmp("/dev/sda/duck.bmp");

  mouse_data_t mouse;

  for (;;) {
    screen_show_bitmap(0, 0, 1024, 768, bitmap);

    screen_show_bitmap(mouse.x, screen->height - mouse.y, 32, 32, bitmap2);

    screen_printf(500, 10, "YiYiYa OS");
    event_read_mouse(&mouse, sizeof(mouse_data_t));
    // for (u32 y = 0; y < screen->height; y++) {
    //   screen_put_pixel((screen->width - screen->height) / 2 + y, y,
    //   0x0000ff); screen_put_pixel((screen->height + screen->width) / 2 - y,
    //   y, 0xff0000);
    // }
    // screen_draw_line(0, 0, 140, 140, 0xff0000);
    // screen_fill_rect(10, 20, 30, 30, 0xff0000);

    screen_printf(10, 100, "mouse=%d,%d", mouse.x, mouse.y);
    // screen_fill_rect(mouse.x, screen->height - mouse.y, 4, 4, 0x00ff00);

    display_time();

    // screen_show_bmp_picture(200, 200, bmp, 0, 0);
    screen_flush();
  }
}

void yiyiya_display() {
  printf("screen_init\n");
  screen_init();
  printf("screen_init end\n");
  for (;;) {
    // screen_fill_rect(0,0,640,490,0xff0000);
    screen_printf(0, 0, "hello display YiYiYa Os\n");
    screen_flush();
  }
}

void yiyiya_bitmap() {
  screen_init();
  // bitmap_t* bitmap = load_jpeg("/dev/sda/home.jpg");
  bitmap_t* bitmap = load_jpeg("/dev/sda/duck.jpg");
  //  bitmap_t* bitmap = load_png("/dev/sda/normal.png");
  // void* bmp = load_bmp("/dev/sda/bomb.bmp");
  for (;;) {
    screen_show_bitmap(0, 0, 227, 149, bitmap);
    // screen_show_bitmap(0, 0, 1024, 768, bitmap);
    // screen_show_bitmap(0, 0, 640, 480, bitmap);
    // screen_show_bmp_picture(200, 200, bmp, 0, 0);
    screen_flush();
  }
}

int main(int argc, char* argv[]) {
  printf(buf);
  // yiyiya_gui();
  yiyiya_display();
  // yiyiya_bitmap();

  return 0;
}