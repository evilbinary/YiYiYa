#include <stdio.h>
#include <stdlib.h>

#include "lvgl.h"

#define LVGL_TICK 5

#ifdef ARM
#define TICK_RATE 10
#else
#define TICK_RATE 100
#endif


static void btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    static uint8_t cnt = 0;
    cnt++;

    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "Button: %d", cnt);
  }
}

void lv_example_get_started_1(void) {
  lv_obj_t *btn =
      lv_btn_create(lv_scr_act()); /*Add a button the current screen*/
  lv_obj_set_pos(btn, 10, 10);     /*Set its position*/
  lv_obj_set_size(btn, 120, 50);   /*Set its size*/
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL,
                      NULL); /*Assign a callback to the button*/

  lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
  lv_label_set_text(label, "Button");     /*Set the labels text*/
  lv_obj_center(label);
}


int main(int argc, char **argv) {
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
  // lv_port_fs_init();

  // lv_obj_remove_style_all(lv_scr_act());
  // lv_disp_set_bg_color(lv_disp_get_default(), lv_color_black());

  status_bar_create(lv_layer_top());
  // lv_example_get_started_1();


  int tickets = 0;
  while (1) {
    // 先调用 lv_tick_inc 再调用 lv_task_handler
    if (tickets % (TICK_RATE * LVGL_TICK) == 0) {
      lv_tick_inc(LVGL_TICK);
      tickets = 0;
    }
    lv_task_handler();
    usleep(5000);
    // delay_ms(LVGL_TICK);
    tickets++;
  }

  return 0;
}