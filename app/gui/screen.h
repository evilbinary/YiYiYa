/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef SCREEN_H
#define SCREEN_H

#include "ioctl.h"
#include "stdint.h"
#include "stdlib.h"

#define ASC_WIDTH 8
#define CHS_WIDTH 16
#define SPACING 2
#define CHAR_HEIGHT 16
#define ASC_NUM 256

struct point_t {
  i32 x;
  i32 y;
};

typedef struct mouse_data {
  u32 sate;
  i32 x;
  i32 y;
} mouse_data_t;

typedef struct framebuffer_info {
  u32 width;
  u32 height;
  u32 bpp;
  u32 mode;
  u32 *frambuffer;
  u32 framebuffer_count;
  u32 framebuffer_index;
  u32 framebuffer_length;
  u32 inited;
  u32* write;
  u32* flip_buffer;
} framebuffer_info_t;

typedef struct screen_info {
  int width;
  int height;
  int bpp;
  u32 *buffer;
  struct point_t cur;
  u8 *ASC;
  mouse_data_t mouse;
  int fd;
  framebuffer_info_t fb;
} screen_info_t;

#define IOC_MAGIC 'v'
#define IOC_INIT _IO(IOC_MAGIC, 0)
#define IOC_READ_REG _IOW(IOC_MAGIC, 1, int)
#define IOC_WRITE_REG _IOW(IOC_MAGIC, 2, int)
#define IOC_READ_FRAMBUFFER _IOW(IOC_MAGIC, 3, int)
#define IOC_READ_FRAMBUFFER_WIDTH _IOW(IOC_MAGIC, 4, int)
#define IOC_READ_FRAMBUFFER_HEIGHT _IOW(IOC_MAGIC, 5, int)
#define IOC_READ_FRAMBUFFER_BPP _IOW(IOC_MAGIC, 6, int)
#define IOC_FLUSH_FRAMBUFFER _IOW(IOC_MAGIC, 7, int)
#define IOC_READ_FRAMBUFFER_INFO _IOW(IOC_MAGIC, 8, int)

void screen_init();
screen_info_t *screen_info();
void screen_put_pixel(u32 x, u32 y, u32 c);
void screen_draw_poi32(i32 x, i32 y, i32 color);
i32 screen_get_poi32_color(i32 x, i32 y);
void screen_draw_vline(i32 x1, i32 x2, i32 y, i32 color);
void screen_draw_hline(i32 y1, i32 y2, i32 x, i32 color);
void screen_draw_line(u32 x1, u32 y1, u32 x2, u32 y2, u32 color);
void screen_clear_screen(void);
void screen_fill_rect(i32 x, i32 y, i32 w, i32 h, u32 color);
void screen_put_ascii(i32 x, i32 y, u8 ch, i32 color);

// 画横线函数
void screen_draw_x_line(i32 y, i32 x1, i32 x2, i32 color);
// 画竖线函数
void screen_draw_y_line(i32 x, i32 y1, i32 y2, i32 color);

// 画矩形函数
void screen_draw_rect(i32 x1, i32 y1, i32 x2, i32 y2, i32 color,
                      i32 dose_fill_it);

void screen_printf(i32 x, i32 y, char *fmt, ...);

void screen_draw_string(i32 x, i32 y, i8 *str);
void screen_draw_char(i32 x, i32 y, u16 ch);

void screen_draw_char_witdh_color(i32 x, i32 y, u16 ch, u32 frcolor,
                                  u32 bgcolor);
void screen_draw_string_with_color(i32 x, i32 y, i8 *str, u32 frcolor,
                                   u32 bgcolor);

// 色调转换函数，把 5:5:5 格式转换成标准的 5:6:5 格式
i32 screen_change_color_form_555_to_565(i32 color_form_555);

// 显示 pbmp 格式的图片
void screen_show_bmp_picture(i32 x, i32 y, void *bmp_addr, i32 mask_color,
                             i32 dose_use_mask_color);

#endif
