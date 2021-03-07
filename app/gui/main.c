#include "screen.h"
#include "stdio.h"

char* buf = "hello,gui gaga\n";
int main(int argc, char* argv[]) {
  printf(buf);
  screen_init();
  screen_printf(200, 10, "hello,YiYiYa OS");
  screen_info_t* screen = screen_info();
  for (;;) {
    for (u32 y = 0; y < screen->height; y++) {
      screen_put_pixel((screen->width - screen->height) / 2 + y, y, 0x0000ff);
      screen_put_pixel((screen->height + screen->width) / 2 - y, y, 0xff0000);
    }
    screen_draw_line(0, 0, 140, 140, 0xff0000);
    screen_fill_rect(10, 20, 30, 30, 0xff0000);
    // // syscall3(DEV_READ, DEVICE_MOUSE,&mouse_data,sizeof(mouse_data_t));
    screen_printf(10, 100, "%d %d", screen->mouse.x, screen->mouse.y);
    screen_fill_rect(screen->mouse.x, screen->height - screen->mouse.y, 4, 4, 0x00ff00);
  }
  return 0;
}