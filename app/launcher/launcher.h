/**
 * @file launcher.h
 *
 */

#ifndef LAUNCHER_H
#define LAUNCHER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif

#include <assert.h>
/*********************
 *      DEFINES
 *********************/
/*Test  lvgl version*/
#if LV_VERSION_CHECK(8, 0, 0) == 0
#error "lv_demo: Wrong lvgl version"
#endif

#define M5PI_SCREEN_WIDTH  320
#define M5PI_SCREEN_HEIGHT 240
#define APPLICATION_ICON_SIZE 64
/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    PHOTOS = 0,
    SETTINGS,
    CAMERA,
    CALENDAR,
    MESSAGE,
    CALCULATOR,
    APP_STORE,
    MUSIC,
    CLOCK,
    HOME,
    NOTES,
    FACETIME,
    HEALTH,
    NES,
    MAIL,
    NEWS,
    MEASURE,
    SAFARI,
    WALLET,
    APPLE_TV,
    BOOKS,
    FILES,
    WEATHER,
    CLIPS,
    APP_LIST_CODE_MAX,
    LINUX = 97,
    PI,
    LIUBO,
}app_list_code_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void launcher_widgets(void);

/**********************
 *      MACROS
 **********************/
extern lv_obj_t *main_win;

extern lv_canvas_t *nes_canvas;
extern int nes_init(char *nes_file);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LAUNCHER_H*/
