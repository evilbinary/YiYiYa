#include "lvgl.h"
#include "resouce.h"

#define BATT_USAGE_HEIGHT (lv_obj_get_style_height(ui.battery.img, 0) - 6)
#define BATT_USAGE_WIDTH (lv_obj_get_style_width(ui.battery.img, 0) - 4)

#define STATUS_BAR_HEIGHT 22

typedef enum { STYLE_TRANSP, STYLE_BLACK, STYLE_MAX } Style_t;

struct {
  lv_obj_t* cont;

  struct {
    lv_obj_t* img;
    lv_obj_t* label;
  } satellite;

  lv_obj_t* imgSD;

  lv_obj_t* labelClock;

  lv_obj_t* labelRec;

  struct {
    lv_obj_t* img;
    lv_obj_t* objUsage;
    lv_obj_t* label;
  } battery;
} ui;

void status_bar_set_style(Style_t style) {
  lv_obj_t* cont = ui.cont;
  if (style == STYLE_TRANSP) {
    lv_obj_add_state(cont, LV_STATE_DEFAULT);
    lv_obj_clear_state(cont, LV_STATE_USER_1);
  } else if (style == STYLE_BLACK) {
    lv_obj_add_state(cont, LV_STATE_USER_1);
  } else {
    return;
  }
}

static void status_bar_update(lv_timer_t* timer) {
  // /* satellite */
  // HAL::GPS_Info_t gps;
  // actStatusBar->Pull("GPS", &gps, sizeof(gps));
  // lv_label_set_text_fmt(ui.satellite.label, "%d", gps.satellites);

  // DataProc::Storage_Basic_Info_t sdInfo;
  // actStatusBar->Pull("Storage", &sdInfo, sizeof(sdInfo));
  // sdInfo.isDetect ? lv_obj_clear_flag(ui.imgSD, LV_OBJ_FLAG_HIDDEN) :
  // lv_obj_add_flag(ui.imgSD, LV_OBJ_FLAG_HIDDEN);

  // /* clock */
  // HAL::Clock_Info_t clock;
  // actStatusBar->Pull("Clock", &clock, sizeof(clock));
  // lv_label_set_text_fmt(ui.labelClock, "%02d:%02d", clock.hour,
  // clock.minute);

  // /* battery */
  // HAL::Power_Info_t power;
  // actStatusBar->Pull("Power", &power, sizeof(power));
  // lv_label_set_text_fmt(ui.battery.label, "%d", power.usage);
  lv_label_set_text_fmt(ui.battery.label, "%d", 20);

  // bool Is_BattCharging = power.isCharging;
  // lv_obj_t* contBatt = ui.battery.objUsage;
  // static bool Is_BattChargingAnimActive = false;
  // if(Is_BattCharging)
  // {
  //     if(!Is_BattChargingAnimActive)
  //     {
  //         StatusBar_AnimCreate(contBatt);
  //         Is_BattChargingAnimActive = true;
  //     }
  // }
  // else
  // {
  //     if(Is_BattChargingAnimActive)
  //     {
  //         lv_anim_del(contBatt, nullptr);
  //         StatusBar_ConBattSetOpa(contBatt, LV_OPA_COVER);
  //         Is_BattChargingAnimActive = false;
  //     }
  //     lv_coord_t height = lv_map(power.usage, 0, 100, 0, BATT_USAGE_HEIGHT);
  //     lv_obj_set_height(contBatt, height);
  // }
}

lv_obj_t* status_bar_create(lv_obj_t* par) {
  lv_obj_t* cont = lv_obj_create(par);
  // lv_obj_remove_style_all(cont);

  lv_obj_set_size(cont, LV_HOR_RES, STATUS_BAR_HEIGHT);
  lv_obj_set_y(cont, -STATUS_BAR_HEIGHT);

  /* style1 */
  lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(cont, lv_color_hex(0x333333), LV_STATE_DEFAULT);

  /* style2 */
  lv_obj_set_style_bg_opa(cont, LV_OPA_60, LV_STATE_USER_1);
  lv_obj_set_style_bg_color(cont, lv_color_black(), LV_STATE_USER_1);
  lv_obj_set_style_shadow_color(cont, lv_color_black(), LV_STATE_USER_1);
  lv_obj_set_style_shadow_width(cont, 10, LV_STATE_USER_1);

  static lv_style_transition_dsc_t tran;
  static const lv_style_prop_t prop[] = {LV_STYLE_BG_COLOR, LV_STYLE_OPA,
                                         LV_STYLE_PROP_INV};
  lv_style_transition_dsc_init(&tran, prop, lv_anim_path_ease_out, 200, 0,
                               NULL);
  lv_obj_set_style_transition(cont, &tran, LV_STATE_USER_1);

  ui.cont = cont;

  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_text_color(&style, lv_color_white());
  lv_style_set_text_font(&style, &font_bahnschrift_13);

  /* satellite */
  lv_obj_t* img = lv_img_create(cont);
  lv_img_set_src(img, &img_src_satellite);
  lv_obj_align(img, LV_ALIGN_LEFT_MID, 14, 0);
  ui.satellite.img = img;

  lv_obj_t* label = lv_label_create(cont);
  lv_obj_add_style(label, &style, 0);
  lv_obj_align_to(label, ui.satellite.img, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  lv_label_set_text(label, "0");
  ui.satellite.label = label;

  /* sd card */
  img = lv_img_create(cont);
  lv_img_set_src(img, &img_src_sd_card);
  lv_obj_align(img, LV_ALIGN_LEFT_MID, 50, -1);
  lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
  ui.imgSD = img;

  /* clock */
  label = lv_label_create(cont);
  lv_obj_add_style(label, &style, 0);
  lv_label_set_text(label, "00:00");
  lv_obj_center(label);
  ui.labelClock = label;

  /* recorder */
  label = lv_label_create(cont);
  lv_obj_add_style(label, &style, 0);
  lv_obj_align(label, LV_ALIGN_RIGHT_MID, -50, 0);
  lv_label_set_text(label, "");
  lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
  ui.labelRec = label;

  /* battery */
  img = lv_img_create(cont);
  lv_img_set_src(img, &img_src_battery);
  lv_obj_align(img, LV_ALIGN_RIGHT_MID, -30, 0);
  lv_img_t* img_ext = (lv_img_t*)img;
  lv_obj_set_size(img, img_ext->w, img_ext->h);
  ui.battery.img = img;

  lv_obj_t* obj = lv_obj_create(img);
  lv_obj_remove_style_all(obj);
  lv_obj_set_style_bg_color(obj, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_size(obj, BATT_USAGE_WIDTH, BATT_USAGE_HEIGHT);
  lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, -2);
  ui.battery.objUsage = obj;

  label = lv_label_create(cont);
  lv_obj_add_style(label, &style, 0);
  lv_obj_align_to(label, ui.battery.img, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  lv_label_set_text(label, "100%");
  ui.battery.label = label;

  status_bar_set_style(STYLE_TRANSP);

  lv_timer_t* timer = lv_timer_create(status_bar_update, 1000, NULL);
  lv_timer_ready(timer);

  return ui.cont;
}