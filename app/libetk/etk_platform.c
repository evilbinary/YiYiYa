#include "etk_platform.h"

#include "etk.h"
#include "etk_display.h"
#include "etk_global.h"
#include "etk_type.h"
#include "image.h"
#include "screen.h"
#include "sys/time.h"

screen_info_t *screen;
mouse_data_t mouse;
bitmap_t *curssor = NULL;
EtkRect last_rect;

size_t etk_get_relative_time(void) {
  struct timeval t_curr;
  gettimeofday(&t_curr, NULL);
  uint64_t inus = 1000 * 1000 * t_curr.tv_sec + t_curr.tv_usec;
  return (size_t)(inus / 1000);
}

Ret etk_platform_flush() {
  // screen_flush();
  EtkRect r;
  r.x = mouse.x;
  r.y = mouse.y;
  r.width = 4;
  r.height = 4;

  etk_default_wnd_manager_update_rect(etk_default_wnd_manager(), &last_rect);
  screen_fill_rect(mouse.x, mouse.y, 4, 4, 0x00ff00);
  last_rect=r;
  if (curssor != NULL) {
    screen_show_bitmap(mouse.x, mouse.y, 16, 16, curssor);
  }
}

Ret etk_platform_event_poll(EtkEvent *event) {
  event_t e;
  int ret = event_poll(&e);
  if (ret <= 0) return RET_CONTINUE;
  e.mouse.y = screen->height - e.mouse.y;
  switch (e.type) {
    case MOUSE_MOTION:
      // printf(".");
      event->type = ETK_EVENT_MOUSE_MOVE;
      event->u.mouse.x = e.mouse.x;
      event->u.mouse.y = e.mouse.y;
      break;
    case MOUSE_BUTTON_DOWN:
      //   printf("down\n");
      event->type = ETK_EVENT_MOUSE_DOWN;
      event->u.mouse.x = e.mouse.x;
      event->u.mouse.y = e.mouse.y;
      break;
    case MOUSE_BUTTON_UP:
      //   printf("up\n");
      event->type = ETK_EVENT_MOUSE_UP;
      event->u.mouse.x = e.mouse.x;
      event->u.mouse.y = e.mouse.y;
      break;

    case KEY_DOWN:
      event->type = ETK_EVENT_KEY_DOWN;
      event->u.key.code = e.key;
      break;
    case KEY_UP:
      event->type = ETK_EVENT_KEY_UP;
      event->u.key.code = e.key;
      break;
    default:
      break;
  }
  mouse = e.mouse;
  return RET_OK;
}

Ret etk_platform_update(EtkDisplay *display, EtkBitmap *bitmap, EtkRect *rect,
                        e32 xoffset, e32 yoffset) {
  e32 i, j;
  e32 x, y;
  u32 *p32;
  u16 *p16;
  EtkRect inter, rect0;
  if (bitmap->bitperpixel == 16) {
    u16 *lp = NULL;
    p16 = (u16 *)bitmap->bits;
    rect0.x = rect->x;
    rect0.y = rect->y;
    rect0.width = bitmap->w;
    rect0.height = bitmap->h;
    inter = etk_rect_intersect(rect0, *rect);

    for (j = yoffset, y = inter.y; y < (inter.y + inter.height); y++, j++) {
      lp = (u16 *)(screen->buffer) + y * display->width;
      if (y < 0 || y > (display->height - 1)) continue;
      for (i = xoffset, x = inter.x; x < (inter.x + inter.width); x++, i++) {
        // lcd_dot(x,y, p16[i + j * bitmap->w]);
        if (x < 0 || x > (display->width - 1)) continue;
        lp[x] = p16[i + j * bitmap->w];
      }
    }

  } else if (bitmap->bitperpixel == 32) {
    u32 *lp = NULL;
    p32 = (u32 *)bitmap->bits;
    rect0.x = rect->x;
    rect0.y = rect->y;
    rect0.width = bitmap->w;
    rect0.height = bitmap->h;
    inter = etk_rect_intersect(rect0, *rect);

    for (j = yoffset, y = inter.y; y < (inter.y + inter.height); y++, j++) {
      lp = (u32 *)(screen->buffer) + y * display->width;
      if (y < 0 || y > (display->height - 1)) continue;
      for (i = xoffset, x = inter.x; x < (inter.x + inter.width); x++, i++) {
        // lcd_dot(x,y, p16[i + j * bitmap->w]);
        if (x < 0 || x > (display->width - 1)) continue;
        lp[x] = p32[i + j * bitmap->w];
      }
    }
  }
}

void etk_platform_display_init() {
  etkglobal.display = (EtkDisplay *)ETK_MALLOC(sizeof(EtkDisplay));
  etkglobal.display->update = etk_platform_update;
  etkglobal.display->format = ETK_PIXEL_BGRA32;
  etkglobal.display->height = DISAPLAY_HEIGHT;
  etkglobal.display->width = DISAPLAY_WIDTH;
  screen_init();
  screen = screen_info();
  //   curssor= load_png("/dev/sda/normal.png");
}

int etk_platform_init(int argc, char **argv) {
  etk_platform_display_init();
  return 0;
}
void etk_platform_deinit(void) {}

u32 getpixel(screen_info_t *surface, e32 x, e32 y) {
  int bpp = surface->bpp;
  /* Here p is the address to the pixel we want to retrieve */
  u8 *p = (u8 *)surface->buffer + y * surface->width + x * bpp;

  switch (bpp) {
    case 1:
      return *p;

    case 2:
      return *(u16 *)p;

    case 3:
      // if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
      //     return p[0] << 16 | p[1] << 8 | p[2];
      // else
      return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
      return *(u32 *)p;

    default:
      return 0; /* shouldn't happen, but avoids warnings */
  }
}

void draw_line(u16 x1, u16 y1, u16 x2, u16 y2) {
  int t;
  int xerr = 0, yerr = 0, delta_x, delta_y, distance;
  int incx, incy, row, col;
  int color = rand() % 0xffffff;
  delta_x = x2 - x1;  //�����������
  delta_y = y2 - y1;
  row = x1;
  col = y1;
  if (delta_x > 0) {
    incx = 1;  //���õ�������
  } else if (delta_x == 0) {
    incx = 0;  //��ֱ��
  } else {
    incx = -1;
    delta_x = -delta_x;
  }
  if (delta_y > 0) {
    incy = 1;
  } else if (delta_y == 0) {
    incy = 0;  //ˮƽ��
  } else {
    incy = -1;
    delta_y = -delta_y;
  }
  if (delta_x > delta_y) {
    distance = delta_x;  //ѡȡ�����������
  } else {
    distance = delta_y;
  }

  for (t = 0; t <= distance + 1; t++) {  //�������
    put_pixel(screen, row, col, color);  //����
    xerr += delta_x;
    yerr += delta_y;
    if (xerr > distance) {
      xerr -= distance;
      row += incx;
    }
    if (yerr > distance) {
      yerr -= distance;
      col += incy;
    }
  }
}

__inline void put_pixel(screen_info_t *surface, int x, int y, u32 pixel) {
  /*if (x < 0 || y < 0 || x > (SCREEN_WIDTH - 1) || y > (SCREEN_HEIGHT - 1))
   return;*/
  int bpp = surface->bpp;
  /* Here p is the address to the pixel we want to set */
  u8 *p = (u8 *)surface->buffer + y * surface->width + x * bpp;
  if (surface->height <= y || surface->width <= x || x < 0 || y < 0) return;
  switch (bpp) {
    case 1:
      *p = pixel;
      break;

    case 2:
      *(u16 *)p = pixel;
      break;

    case 3:
      // if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
      // 	p[0] = (pixel >> 16) & 0xff;
      // 	p[1] = (pixel >> 8) & 0xff;
      // 	p[2] = pixel & 0xff;
      // } else {
      p[0] = pixel & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = (pixel >> 16) & 0xff;
      // }
      break;

    case 4:
      *(u32 *)p = pixel;
      break;
  }
}

void put_pixel16(screen_info_t *surface, int x, int y, u16 pixel) {
  // Convert the pixels to 32 bit
  u16 *pixels = (u16 *)surface->buffer;
  // Set the pixel
  if (surface->height <= y || surface->width <= x) return;
  pixels[(y * surface->width) + x] = pixel;
}
void put_pixel32(screen_info_t *surface, int x, int y, u32 pixel) {
  // Convert the pixels to 32 bit
  u32 *pixels = (u32 *)surface->buffer;
  // Set the pixel
  if (surface->height <= y || surface->width <= x || x < 0 || y < 0) return;
  pixels[(y * surface->width) + x] = pixel;
}
void draw_dot_line(screen_info_t *surface, int x, int y, u32 pixel) {
  if (x % 2 == 0 || y % 2 == 0) put_pixel(surface, x, y, pixel);
}
u32 get_pixel32(screen_info_t *surface, int x, int y) {
  // Convert the pixels to 32 bit
  u32 *pixels = (u32 *)surface->buffer;
  // Get the requested pixel
  return pixels[(y * surface->width) + x];
}

void draw_hline(screen_info_t *surface, int x, int y, int length, u32 color) {
  int z = x + length;
  for (; x <= z; x++) {
    put_pixel(surface, x, y, color);
  }
}
void draw_vline(screen_info_t *surface, int x, int y, int length, u32 color) {
  int z = y + length;
  for (; y <= z; y++) {
    put_pixel(surface, x, y, color);
  }
}
void draw_rectangle(screen_info_t *surface, int x, int y, int width, int height,
                    u32 color) {
  draw_hline(surface, x, y, width, color);
  // SDL_Flip(screen);
  draw_hline(surface, x, y + height, width, color);
  // SDL_Flip(screen);
  draw_vline(surface, x, y, height, color);
  // SDL_Flip(screen);
  draw_vline(surface, x + width, y, height, color);
  // SDL_Flip(screen);
}
void draw_dot_rectangle(screen_info_t *surface, int x, int y, int width,
                        int height, u32 color) {
  draw_dot_line(surface, x, y, color);
}

void draw_rectangle_rect(screen_info_t *surface, EtkRect rect, u32 color) {
  draw_rectangle(surface, rect.x, rect.y, rect.width, rect.height, color);
}
void fill_rectangle(screen_info_t *surface, int x, int y, int width, int height,
                    u32 color) {
  int z = y + height;
  for (; y <= z; y++) {
    draw_hline(surface, x, y, width, color);
  }
}

void draw_pixel(e32 x, e32 y, u32 pixel) { put_pixel(screen, x, y, pixel); }
void draw_rect(EtkRect rect, u32 color) {
  draw_rectangle_rect(screen, rect, color);
}

void draw_rects_minus(EtkRect *rects, e32 m, u32 color) {
  EtkRect *p = rects;
  EtkRect *t;
  int i = 0;
  while (p != NULL) {
    EtkRect t;
    t.height = p->height - m - m;
    t.width = p->width - m - m;
    t.x = p->x + m;
    t.y = p->y + m;
    draw_rectangle_rect(screen, t, color);
    p = p->next;
  }
}
void draw_rects(EtkRect *rects, u32 color) {
  EtkRect *p = rects;
  EtkRect *t;
  int i = 0;
  while (p != NULL) {
    draw_rectangle_rect(screen, *p, color);
    t = p;
    p = p->next;
  }
  // draw_rectangel_rect(screen,*t,YELLOW);
}
void fill_rects(EtkRect *rects) {
  EtkRect *p = rects;
  int i = 0;
  while (p != NULL) {
    fill_rectangle(screen, p->x, p->y, p->width, p->height, i);
    i += 0xff;
    p = p->next;
  }
}
void fill_rects_plus(EtkRect *rects, u32 color) {
  EtkRect *p = rects;
  while (p != NULL) {
    fill_rect_plus(*p, color);
    p = p->next;
  }
}
void fill_rect_plus(EtkRect rect, u32 color) {
  EtkRect *p = &rect;
  if (p == NULL) return;
  fill_rectangle(screen, p->x + 1, p->y + 1, p->width - 2, p->height - 2,
                 color);
  // SDL_Delay(100);
  // fill_rectangle(screen, p->x + 1, p->y + 1, p->width - 2, p->height - 2,
  // GRAY); SDL_Flip(screen);
}

void draw_region(EtkRegion *region, u32 color) {
  // if (region->iscover == 0) {
  //	draw_rects(region->rects, color);
  //} else {
  //	fill_rect_plus(region->rect, BLACK);
  //}
}