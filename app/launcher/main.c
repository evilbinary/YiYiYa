#include "lvgl.h"

#define LVGL_TICK 5

#ifdef ARM
#define TICK_RATE 1
#else
#define TICK_RATE 100
#endif


int main(int argc, char **argv) {
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
  // lv_port_fs_init();


   launcher_widgets();
  // lv_demo_widgets();
  printf("loop\n");

  int tickets = 0;
  while (1) {
    if (tickets % (TICK_RATE * LVGL_TICK)==0) {
      lv_tick_inc(LVGL_TICK);
      tickets = 0;
    }
    lv_task_handler();
    // usleep(5000);
    // delay_ms(LVGL_TICK);
    tickets++;
  }

  return 0;
}
