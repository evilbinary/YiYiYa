#include <assert.h>
#include "renderer.h"
#include "screen.h"
// #include "atlas.inl"

#define BUFFER_SIZE 16384
#define color_rgba(color) (color.r<<16|color.g<<8|color.b|color.a<<24)

screen_info_t * screen;
void r_init(void) {
  screen_init();
  screen=screen_info();
}


static void flush(void) {
  
}


void r_draw_rect(mu_Rect rect, mu_Color color) {
  screen_fill_rect(rect.x, rect.y, rect.w, rect.h,color_rgba(color)) ;
}


void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) { 
  screen_draw_string_with_color(pos.x,pos.y,text,color_rgba(color), 0);
}


void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  // mu_Rect src = atlas[id];
  // int x = rect.x + (rect.w - src.w) / 2;
  // int y = rect.y + (rect.h - src.h) / 2;
  // push_quad(mu_rect(x, y, src.w, src.h), src, color);
}


int r_get_text_width(const char *text, int len) {
  int res = 0;
  // for (const char *p = text; *p && len--; p++) {
  //   if ((*p & 0xc0) == 0x80) { continue; }
  //   int chr = mu_min((unsigned char) *p, 127);
  //   res += atlas[ATLAS_FONT + chr].w;
  // }
  return res;
}


int r_get_text_height(void) {
  return 18;
}


void r_set_clip_rect(mu_Rect rect) {
  flush();
}


void r_clear(mu_Color clr) {
  flush();
}


void r_present(void) {
  flush();
  screen_flush();
}
