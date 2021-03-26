#include "etk_platform.h"

#include "etk.h"
#include "etk_display.h"
#include "etk_global.h"
#include "etk_type.h"
#include "screen.h"
#include "sys/time.h"

screen_info_t *screen;

size_t etk_get_relative_time(void) {
  struct timeval t_curr;
  gettimeofday(&t_curr, NULL);
  uint64_t inus = 1000 * 1000 * t_curr.tv_sec + t_curr.tv_usec;
  return (size_t)(inus / 1000);
}

Ret etk_platform_flush(){
 
  screen_flush();
}

Ret etk_platform_event_poll(EtkEvent *event) {
  event_t e;
  int ret=event_poll(&e);
  if(ret<=0) return RET_CONTINUE;
  
  switch (e.type) {
    case MOUSE_MOTION:
      event->type = ETK_EVENT_MOUSE_MOVE;
      event->u.mouse.x = e.mouse.x;
      event->u.mouse.y = e.mouse.y;

      screen_fill_rect(e.mouse.x, screen->height - e.mouse.y, 6, 6,
                     0x00ff00);
      break;
    case MOUSE_BUTTON_DOWN:
      event->type = ETK_EVENT_MOUSE_DOWN;
      event->u.mouse.x = e.mouse.x;
      event->u.mouse.y = e.mouse.y;
      break;
    case MOUSE_BUTTON_UP:
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
}

int etk_platform_init(int argc, char **argv) {
  etk_platform_display_init();
  return 0;
}
void etk_platform_deinit(void) {}
