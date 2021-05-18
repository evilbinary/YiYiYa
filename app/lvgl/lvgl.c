#include "lvgl.h"

#include <stdio.h>
#include <stdlib.h>

#include "sys/time.h"
#include "lv_demo_widgets.h"

#define LVGL_TICK 5

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

void lv_example_chart_1(void) {
  /*Create a chart*/
  lv_obj_t *chart;
  chart = lv_chart_create(lv_scr_act());
  lv_obj_set_size(chart, 200, 150);
  // lv_obj_center(chart);
  lv_obj_align(chart, LV_ALIGN_TOP_MID, 0, 0);

  lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/

  /*Add two data series*/
  lv_chart_series_t *ser1 = lv_chart_add_series(
      chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_series_t *ser2 = lv_chart_add_series(
      chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_SECONDARY_Y);

  /*Set the next points on 'ser1'*/
  lv_chart_set_next_value(chart, ser1, 10);
  lv_chart_set_next_value(chart, ser1, 10);
  lv_chart_set_next_value(chart, ser1, 10);
  lv_chart_set_next_value(chart, ser1, 10);
  lv_chart_set_next_value(chart, ser1, 10);
  lv_chart_set_next_value(chart, ser1, 10);
  lv_chart_set_next_value(chart, ser1, 10);
  lv_chart_set_next_value(chart, ser1, 30);
  lv_chart_set_next_value(chart, ser1, 70);
  lv_chart_set_next_value(chart, ser1, 90);

  /*Directly set points on 'ser2'*/
  ser2->points[0] = 90;
  ser2->points[1] = 70;
  ser2->points[2] = 65;
  ser2->points[3] = 65;
  ser2->points[4] = 65;
  ser2->points[5] = 65;
  ser2->points[6] = 65;
  ser2->points[7] = 65;
  ser2->points[8] = 65;
  ser2->points[9] = 65;

  lv_chart_refresh(chart); /*Required after direct set*/
}

static void draw_part_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_draw_part_dsc_t *dsc = lv_event_get_param(e);
  /*If the cells are drawn...*/
  if (dsc->part == LV_PART_ITEMS) {
    uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
    uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

    /*Make the texts in the first cell center aligned*/
    if (row == 0) {
      dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
      dsc->rect_dsc->bg_color = lv_color_mix(
          lv_palette_main(LV_PALETTE_BLUE), dsc->rect_dsc->bg_color, LV_OPA_20);
      dsc->rect_dsc->bg_opa = LV_OPA_COVER;
    }
    /*In the first column align the texts to the right*/
    else if (col == 0) {
      dsc->label_dsc->flag = LV_TEXT_ALIGN_RIGHT;
    }

    /*MAke every 2nd row grayish*/
    if ((row != 0 && row % 2) == 0) {
      dsc->rect_dsc->bg_color = lv_color_mix(
          lv_palette_main(LV_PALETTE_GREY), dsc->rect_dsc->bg_color, LV_OPA_10);
      dsc->rect_dsc->bg_opa = LV_OPA_COVER;
    }
  }
}

void lv_example_table_1(void) {
  lv_obj_t *table = lv_table_create(lv_scr_act());
  lv_obj_align(table, LV_ALIGN_TOP_LEFT, 0, 150);

  /*Fill the first column*/
  lv_table_set_cell_value(table, 0, 0, "Name");
  lv_table_set_cell_value(table, 1, 0, "Apple");
  lv_table_set_cell_value(table, 2, 0, "Banana");
  lv_table_set_cell_value(table, 3, 0, "Lemon");
  lv_table_set_cell_value(table, 4, 0, "Grape");
  lv_table_set_cell_value(table, 5, 0, "Melon");
  lv_table_set_cell_value(table, 6, 0, "Peach");
  lv_table_set_cell_value(table, 7, 0, "Nuts");

  /*Fill the second column*/
  lv_table_set_cell_value(table, 0, 1, "Price");
  lv_table_set_cell_value(table, 1, 1, "$7");
  lv_table_set_cell_value(table, 2, 1, "$4");
  lv_table_set_cell_value(table, 3, 1, "$6");
  lv_table_set_cell_value(table, 4, 1, "$2");
  lv_table_set_cell_value(table, 5, 1, "$5");
  lv_table_set_cell_value(table, 6, 1, "$1");
  lv_table_set_cell_value(table, 7, 1, "$9");

  /*Set a smaller height to the table. It'll make it scrollable*/
  lv_obj_set_height(table, 200);
  // lv_obj_center(table);

  /*Add an event callback to to apply some custom drawing*/
  lv_obj_add_event_cb(table, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN,
                      NULL);
}

static void ta_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }

  if (code == LV_EVENT_DEFOCUSED) {
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }
}

void lv_example_keyboard_1(void) {
  /*Create a keyboard to use it with an of the text areas*/
  lv_obj_t *kb = lv_keyboard_create(lv_scr_act());

  /*Create a text area. The keyboard will write here*/
  lv_obj_t *ta;
  ta = lv_textarea_create(lv_scr_act());
  lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
  lv_textarea_set_placeholder_text(ta, "Hello");

  ta = lv_textarea_create(lv_scr_act());
  lv_obj_align(ta, LV_ALIGN_TOP_RIGHT, -10, 10);
  lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);

  lv_keyboard_set_textarea(kb, ta);
}

uint32_t custom_tick_get(void) {
  static uint64_t start_ms = 0;
  if (start_ms == 0) {
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
    start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
  }

  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  uint64_t now_ms;
  now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

  uint32_t time_ms = now_ms - start_ms;
  return time_ms;
}

int main(int argc, char **argv) {
  lv_init();

  lv_port_disp_init();
  lv_port_indev_init();
  // lv_port_fs_init();

  // lv_example_get_started_1();
  // lv_example_chart_1();
  // lv_example_table_1();
  // lv_example_keyboard_1();

  lv_demo_widgets();

  int tickets = 0;
  while (1) {
    // 先调用 lv_tick_inc 再调用 lv_task_handler
    if (tickets % (100 * LVGL_TICK)==0) {
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