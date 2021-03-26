#ifndef _ETK_BITMAP_H
#define _ETK_BITMAP_H
#include "etk_type.h"
#include "etk_color.h"

typedef struct EtkBitmap_t{
	e32 w,h;
	e32 length;
	e32 bitperpixel;
	void* bits;
}EtkBitmap;

EtkBitmap* etk_bitmap_create(e32 w,e32 h,e32 bitperpixel,EtkColor color);
void etk_bitmap_destroy(EtkBitmap* bitmap);

Ret etk_bitmap_draw_pixel(EtkBitmap* thiz, e32 x,e32 y, u32 color);
Ret etk_bitmap_hline(EtkBitmap *thiz,e32 x,e32 y,e32 length,u32 color);
Ret etk_bitmap_vline(EtkBitmap *thiz,int x,int y,int length,u32 color);
Ret etk_bitmap_draw_rect(EtkBitmap* thiz, e32 x, e32 y, e32 width, e32 height, u32 color);
Ret etk_bitmap_draw_line(EtkBitmap* thiz, e32 x1, e32 y1, e32 x2, e32 y2);

#define RGB24_2_RGB565(r,g,b) (u16)((((r) << 8) & 0xF800) |(((g) << 3) & 0x7E0)|(((b) >> 3)))
#define RGB24_2_RGB555(r,g,b) (u16)((((r) << 7) & 0x7c00) |(((g) << 2) & 0x3E0)|(((b) >> 3)))

#endif