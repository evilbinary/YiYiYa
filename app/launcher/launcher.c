/**
 * @file launcher.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "launcher.h"

/*********************
 *      DEFINES
 *********************/

#define SCREEN_WITH 480
#define SCREEN_HEIGHT 272

/**********************
 *      TYPEDEFS
 **********************/
lv_obj_t * main_win;
lv_canvas_t *nes_canvas;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_app_icon_list(void);

static void dock_menu_event_handler(lv_event_t *e);
static void desktop_tbv_event_handler(lv_event_t *e);
static void back_btn_event_handler(lv_event_t *e);
static void event_handler(lv_event_t *e);
/**********************
 *  STATIC VARIABLES
 **********************/

static lv_obj_t * desktop_tbv;
static lv_style_t desktop_style;

// desktop_tbv tabview
static lv_obj_t *desktop_left;
static lv_obj_t *desktop_main;
static lv_obj_t *desktop_right;

static lv_obj_t *desktop_tabview_list[] = {
    &desktop_left, &desktop_main, &desktop_right
};

// application icon
static lv_obj_t *photos_imgbtn;
static lv_obj_t *setting_imgbtn;
static lv_obj_t *camera_imgbtn;
static lv_obj_t *calendar_imgbtn;
static lv_obj_t *message_imgbtn;
static lv_obj_t *calculator_imgbtn;
static lv_obj_t *app_store_imgbtn;
static lv_obj_t *music_imgbtn;

static lv_obj_t *clock_imgbtn;
static lv_obj_t *home_imgbtn;
static lv_obj_t *notes_imgbtn;
static lv_obj_t *facetime_imgbtn;
static lv_obj_t *health_imgbtn;
static lv_obj_t *shortcuts_imgbtn;
static lv_obj_t *mail_imgbtn;
static lv_obj_t *news_imgbtn;

static lv_obj_t *measure_imgbtn;
static lv_obj_t *safari_imgbtn;
static lv_obj_t *wallet_imgbtn;
static lv_obj_t *apple_tv_imgbtn;
static lv_obj_t *books_imgbtn;
static lv_obj_t *files_imgbtn;
static lv_obj_t *weather_imgbtn;
static lv_obj_t *clips_imgbtn;

static lv_obj_t *app_wigets_list[] = {
    &photos_imgbtn, &setting_imgbtn, &camera_imgbtn, &calendar_imgbtn,
    &message_imgbtn, &calculator_imgbtn, &app_store_imgbtn, &music_imgbtn,

    &clock_imgbtn, &home_imgbtn, &notes_imgbtn, &facetime_imgbtn,
    &health_imgbtn, &shortcuts_imgbtn, &mail_imgbtn, &news_imgbtn,

    &measure_imgbtn, &safari_imgbtn, &wallet_imgbtn, &apple_tv_imgbtn,
    &books_imgbtn, &files_imgbtn, &weather_imgbtn, &clips_imgbtn
};

// dock menu
static lv_obj_t *pi_menu_btn;
static lv_obj_t *linux_menu_btn;
static lv_obj_t *evilbinary_menu_btn;

static lv_obj_t *dock_menu_list[] = {
    &linux_menu_btn, &pi_menu_btn, &evilbinary_menu_btn
};

// tab pos led
static lv_obj_t *tab_pos_0;
static lv_obj_t *tab_pos_1;
static lv_obj_t *tab_pos_2;
static lv_style_t led_style;
static lv_color_t led_color;

static lv_obj_t *tab_pos_led_list[] = {
    &tab_pos_0, &tab_pos_1, &tab_pos_2
};

static lv_obj_t * back_btn;

// menu icons
LV_IMG_DECLARE(photos);
LV_IMG_DECLARE(settings);
LV_IMG_DECLARE(camera);
LV_IMG_DECLARE(calendar);
LV_IMG_DECLARE(message);
LV_IMG_DECLARE(calculator);
LV_IMG_DECLARE(appstore);
LV_IMG_DECLARE(music);
LV_IMG_DECLARE(clocks);
LV_IMG_DECLARE(home);
LV_IMG_DECLARE(notes);
LV_IMG_DECLARE(facetime);
LV_IMG_DECLARE(health);
LV_IMG_DECLARE(nes_menu);
LV_IMG_DECLARE(mail);
LV_IMG_DECLARE(news);
LV_IMG_DECLARE(measure);
LV_IMG_DECLARE(safari);
LV_IMG_DECLARE(wallet);
LV_IMG_DECLARE(appletv);
LV_IMG_DECLARE(books);
LV_IMG_DECLARE(files);
LV_IMG_DECLARE(weather);
LV_IMG_DECLARE(clips);

static const lv_img_dsc_t *app_icon_list[] = {
    &photos, &settings, &camera, &calendar,
    &message, &calculator, &appstore, &music,

    &clocks, &home, &notes, &facetime,
    &health, &nes_menu, &mail, &news,

    &measure, &safari, &wallet, &appletv,
    &books, &files, &weather, &clips
};

LV_IMG_DECLARE(linux_menu);
LV_IMG_DECLARE(pi_menu);
LV_IMG_DECLARE(duck_menu);
static const lv_img_dsc_t *dock_icon_list[] = {
    &linux_menu, &pi_menu,&duck_menu
};

lv_obj_t * app_win;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void launcher_widgets(void)
{
    lv_style_init(&desktop_style);
    LV_IMG_DECLARE(background_dock);
    lv_style_set_bg_img_src(&desktop_style, &background_dock);
    lv_style_set_bg_opa(&desktop_style, 100);

    led_color.ch.red = 255;
    led_color.ch.green = 255;
    led_color.ch.blue = 255;
#if (LV_COLOR_DEPTH == 32)
    led_color.ch.alpha = 0;
#endif
    lv_style_init(&led_style);
    lv_style_set_shadow_opa(&led_style, 0);
    lv_style_set_bg_color(&led_style, led_color);
    lv_style_set_bg_opa(&led_style, 100);

    main_win = lv_scr_act();
    desktop_tbv = lv_tabview_create(main_win, LV_DIR_TOP, 0);
    lv_obj_add_event_cb(desktop_tbv, desktop_tbv_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(desktop_tbv, SCREEN_WITH, SCREEN_HEIGHT);
    lv_obj_add_style(desktop_tbv, &desktop_style, 0);  // background

    create_app_icon_list();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void create_app_icon_list(void)
{
    int16_t app_x_offset = 0, app_y_offset = 0, dock_offset = -76, led_offset = -15, count = 0;

    for (size_t pages = 0; pages < 3; pages++) {
        desktop_tabview_list[pages] = lv_tabview_add_tab(desktop_tbv, "tabview");
        app_y_offset = -8;
        for (size_t y = 0; y < 2; y++) {
            app_x_offset = 0;
            for (size_t x = 0; x < 4; x ++) {
                app_wigets_list[count] = lv_imgbtn_create(desktop_tabview_list[pages]);
                lv_obj_set_width(app_wigets_list[count], APPLICATION_ICON_SIZE);
                lv_obj_set_height(app_wigets_list[count], APPLICATION_ICON_SIZE);
                lv_imgbtn_set_src(app_wigets_list[count], LV_IMGBTN_STATE_PRESSED, NULL, app_icon_list[count], NULL);
                lv_imgbtn_set_src(app_wigets_list[count], LV_IMGBTN_STATE_RELEASED, NULL, app_icon_list[count], NULL);
                lv_obj_align(app_wigets_list[count], LV_ALIGN_TOP_LEFT, app_x_offset, app_y_offset);
                lv_obj_set_user_data(app_wigets_list[count], (uint16_t)count);
                lv_obj_add_event_cb(app_wigets_list[count], event_handler, LV_EVENT_LONG_PRESSED, NULL);
                app_x_offset += 76;  // 64 + 12
                count += 1;
            }
            app_y_offset += 76;  // 64 + 12
        }

        tab_pos_led_list[pages] = lv_led_create(main_win);
        lv_led_set_color(tab_pos_led_list[pages], led_color);
        lv_obj_add_style(tab_pos_led_list[pages], &led_style, 0);
        lv_led_set_brightness(tab_pos_led_list[pages], 5);
        lv_obj_set_width(tab_pos_led_list[pages], 5);
        lv_obj_set_height(tab_pos_led_list[pages], 5);
        lv_obj_align(tab_pos_led_list[pages], LV_ALIGN_BOTTOM_MID, led_offset, -80);
        led_offset += 15;

        dock_menu_list[pages] = lv_imgbtn_create(main_win);
        lv_obj_set_width(dock_menu_list[pages], APPLICATION_ICON_SIZE);
        lv_obj_set_height(dock_menu_list[pages], APPLICATION_ICON_SIZE);
        lv_imgbtn_set_src(dock_menu_list[pages], LV_IMGBTN_STATE_PRESSED, NULL, dock_icon_list[pages], NULL);
        lv_imgbtn_set_src(dock_menu_list[pages], LV_IMGBTN_STATE_RELEASED, NULL, dock_icon_list[pages], NULL);
        lv_obj_add_event_cb(dock_menu_list[pages], dock_menu_event_handler, LV_EVENT_LONG_PRESSED, NULL);
        lv_obj_align(dock_menu_list[pages], LV_ALIGN_BOTTOM_MID, dock_offset, -5);
        dock_offset += 76;
    }
    lv_tabview_set_act(desktop_tbv, 0, LV_ANIM_OFF);
    lv_led_on(tab_pos_led_list[0]);
}


static void create_back_btn(lv_obj_t *win, lv_event_cb_t *event_cb, void *user_data)
{
    back_btn = lv_imgbtn_create(win);
    lv_obj_set_width(back_btn, 25);
    lv_obj_set_height(back_btn, 25);
    lv_obj_set_user_data(back_btn, user_data);
    LV_IMG_DECLARE(backhome);
    lv_imgbtn_set_src(back_btn, LV_IMGBTN_STATE_PRESSED, NULL, &backhome, NULL);
    lv_imgbtn_set_src(back_btn, LV_IMGBTN_STATE_RELEASED, NULL, &backhome, NULL);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_event_cb(back_btn, event_cb, LV_EVENT_PRESSED, NULL);
}

static void dock_menu_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    if (obj == dock_menu_list[0])
    {
        static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(SCREEN_WITH, SCREEN_HEIGHT)];
        app_win = lv_canvas_create(main_win);
        lv_obj_set_size(app_win, SCREEN_WITH, SCREEN_HEIGHT);
        lv_canvas_set_buffer(app_win, cbuf, SCREEN_WITH, SCREEN_HEIGHT, LV_IMG_CF_TRUE_COLOR);
        lv_canvas_fill_bg(app_win, lv_color_make(0xFF, 0xFF, 0xFF), 0);
        lv_obj_t * qr = lv_qrcode_create(app_win, 120, lv_color_make(255, 255, 255), lv_color_make(0, 0, 0));
        lv_qrcode_update(qr, "https://www.kernel.org/", strlen("https://www.kernel.org/"));
        lv_obj_align(qr, LV_ALIGN_CENTER, 0, -20);

        lv_obj_t * label = lv_label_create(app_win);
        lv_label_set_text(label, "Linux Kernel");
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -50);
        create_back_btn(main_win, back_btn_event_handler, LINUX);
    }
    else if (obj == dock_menu_list[1])
    {
        static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(SCREEN_WITH, SCREEN_HEIGHT)];
        app_win = lv_canvas_create(main_win);
        lv_obj_set_size(app_win, SCREEN_WITH, SCREEN_HEIGHT);
        lv_canvas_set_buffer(app_win, cbuf, SCREEN_WITH, SCREEN_HEIGHT, LV_IMG_CF_TRUE_COLOR);
        lv_canvas_fill_bg(app_win, lv_color_make(0xFF, 0xFF, 0xFF), 0);
        lv_obj_t * qr = lv_qrcode_create(app_win, 150, lv_color_make(255, 255, 255), lv_color_make(0, 0, 0));
        lv_qrcode_update(qr, "https://github.com/evilbinary/YiYiYa", strlen("https://github.com/evilbinary/YiYiYa"));
        lv_obj_align(qr, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t * label = lv_label_create(app_win);
        lv_label_set_text(label, "YiYiYa Project Github Repo");
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -25);
        create_back_btn(main_win, back_btn_event_handler, PI);
    }
    else if (obj == dock_menu_list[2])
    {
        static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(SCREEN_WITH, SCREEN_HEIGHT)];
        app_win = lv_canvas_create(main_win);
        lv_obj_set_size(app_win, SCREEN_WITH, SCREEN_HEIGHT);
        lv_canvas_set_buffer(app_win, cbuf, SCREEN_WITH, SCREEN_HEIGHT, LV_IMG_CF_TRUE_COLOR);
        lv_canvas_fill_bg(app_win, lv_color_make(0xFF, 0xFF, 0xFF), 0);
        lv_obj_t * qr = lv_qrcode_create(app_win, 150, lv_color_make(255, 255, 255), lv_color_make(0, 0, 0));
        lv_qrcode_update(qr, "https://space.bilibili.com/28582277", strlen("https://space.bilibili.com/28582277"));
        lv_obj_align(qr, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t * label = lv_label_create(app_win);
        lv_label_set_text(label, "Subscribe me!");
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -25);
        create_back_btn(main_win, back_btn_event_handler, LIUBO);
    }
}

static void desktop_tbv_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED){
        switch (lv_tabview_get_tab_act(obj))
        {
            case 0:
                lv_led_on(tab_pos_led_list[0]);
                lv_led_off(tab_pos_led_list[1]);
                lv_led_off(tab_pos_led_list[2]);
                break;
            case 1:
                lv_led_on(tab_pos_led_list[1]);
                lv_led_off(tab_pos_led_list[0]);
                lv_led_off(tab_pos_led_list[2]);
                break;
            case 2:
                lv_led_on(tab_pos_led_list[2]);
                lv_led_off(tab_pos_led_list[0]);
                lv_led_off(tab_pos_led_list[1]);
                break;
            default:
                break;
        } 
    }
}

static void back_btn_event_handler(lv_event_t *e)
{
    // lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_current_target(e);

    switch ((app_list_code_t)obj->user_data)
    {
        case PHOTOS:
        case SETTINGS:
        case CAMERA:
        case CALENDAR:
        case MESSAGE:
        case CALCULATOR:
        case APP_STORE:
        case MUSIC:
        case CLOCK:
        case HOME:
        case NOTES:
        case FACETIME:
        case HEALTH:
        case MAIL:
        case NEWS:
        case MEASURE:
        case SAFARI:
        case WALLET:
        case APPLE_TV:
        case BOOKS:
        case FILES:
        case WEATHER:
        case CLIPS:
        case LINUX:
        case PI:
        case LIUBO:
            lv_obj_del(app_win);   // delete app win
            lv_obj_del(obj);       // delete back btn
            break;
        case NES:
            // assert(nes_tid);
            // pthread_cancel(nes_tid);  // exit nes thread
            // pthread_join(nes_tid, NULL);
            lv_obj_del(nes_canvas);   // delete nes_canvas
            lv_obj_del(obj);
            break;
        default:
            break;
    }
}

static void event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);

    if((app_list_code_t)obj->user_data > APP_LIST_CODE_MAX)
    {
        return;
    }

    switch ((app_list_code_t)obj->user_data) {
        case PHOTOS:
        case SETTINGS:
        case CAMERA:
        case CALENDAR:
        case MESSAGE:
        case CALCULATOR:
        case APP_STORE:
        case CLOCK:
        case HOME:
        case NOTES:
        case FACETIME:
        case HEALTH:
        case MAIL:
        case NEWS:
        case MEASURE:
        case SAFARI:
        case WALLET:
        case APPLE_TV:
        case BOOKS:
        case FILES:
        case WEATHER:
        case CLIPS:
            {
                static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(SCREEN_WITH, SCREEN_HEIGHT)];
                app_win = lv_canvas_create(main_win);
                lv_obj_set_size(app_win, SCREEN_WITH, SCREEN_HEIGHT);
                lv_canvas_set_buffer(app_win, cbuf, SCREEN_WITH, SCREEN_HEIGHT, LV_IMG_CF_TRUE_COLOR);
                lv_canvas_fill_bg(app_win, lv_color_make(0xFF, 0xFF, 0xFF), 0);

                lv_obj_t * img = lv_img_create(app_win);
                LV_IMG_DECLARE(under_construction)
                lv_img_set_src(img, &under_construction);
                lv_obj_align(img, LV_ALIGN_CENTER, 0, -10);

                lv_obj_t * label = lv_label_create(app_win);
                lv_label_set_text(label, "Under construction");
                lv_obj_align(label, LV_ALIGN_CENTER, 0, 40);
                create_back_btn(main_win, back_btn_event_handler, obj->user_data);
            }
            break;
        case MUSIC:
            app_win = lv_win_create(main_win, 0);
            lv_obj_set_size(app_win, SCREEN_WITH, SCREEN_HEIGHT);
            // lv_demo_music(app_win);

            create_back_btn(main_win, &back_btn_event_handler, obj->user_data);
            break;
        case NES:
            nes_canvas = lv_canvas_create(main_win);
            lv_obj_align(main_win, LV_ALIGN_TOP_MID, 0, 0);
            create_back_btn(main_win, &back_btn_event_handler, obj->user_data);
            break;
        default: break;
    }
}
