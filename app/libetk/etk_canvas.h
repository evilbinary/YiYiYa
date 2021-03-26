#ifndef _ETK_CANVAS_H
#define _ETK_CANVAS_H
#include "etk_bitmap.h"
#include "etk_type.h"
#include "etk_gconf.h"
#include "etk_display.h"
#include "etk_rect.h"

struct EtkCanvas_t;
typedef struct EtkCanvas_t  EtkCanvas;

typedef Ret (*EtkCanvasDrawPixel)(EtkCanvas* thiz, e32 x,e32 y, u32 color);
typedef Ret (*EtkCanvasDrawLine)(EtkCanvas* thiz, e32 x1, e32 y1, e32 x2, e32 y2);
typedef Ret (*EtkCanvasClearRect)(EtkCanvas* thiz, e32 x, e32 y, e32 w, e32 h);
typedef Ret (*EtkCanvasDrawRect)(EtkCanvas* thiz, e32 x, e32 y, e32 width, e32 height, u32 color);
typedef Ret (*EtkCanvasDrawBitmap)(EtkCanvas* thiz, EtkBitmap* bitmap, EtkRect* s, EtkRect* d, u32 alpha);
typedef Ret (*EtkCanvasDrawString)(EtkCanvas* thiz, e32 x, e32 y, const char* str);
typedef Ret (*EtkCanvasPaint)(EtkDisplay *display,EtkCanvas *thiz,EtkRect *rect);

struct EtkCanvas_t{
	//property
	EtkGconf gc;
	EtkBitmap *bitmap;
	EtkRegion *region;
	//method
	EtkCanvasPaint paint;
	EtkCanvasDrawBitmap draw_bitmap;
	EtkCanvasDrawPixel draw_pixel;
	EtkCanvasDrawLine draw_line;
	EtkCanvasClearRect clear_rect;
	EtkCanvasDrawRect draw_rect;
	EtkCanvasDrawString draw_string;
};

EtkCanvas* etk_canvas_create_with_no_bitmap(e32 w, e32 h, EtkColor clear_color);
Ret etk_canvas_set_bitmap(EtkCanvas *thiz,EtkBitmap *bitmap);

EtkCanvas* etk_canvas_create(e32 w, e32 h, EtkColor clear_color);
Ret etk_canvas_paint(EtkDisplay *display,EtkCanvas *thiz,EtkRect *rect);
Ret etk_canvas_paint_rect(EtkDisplay *display,EtkCanvas *thiz,EtkRect *rect);
Ret etk_canvas_paint_global_rect(EtkDisplay *display,EtkCanvas *thiz,EtkRect *rect);

Ret etk_canvas_draw_pixel(EtkCanvas* thiz, e32 x,e32 y, u32 color);
Ret etk_canvas_draw_line(EtkCanvas* thiz, e32 x1, e32 y1, e32 x2, e32 y2);
Ret etk_canvas_clear_rect(EtkCanvas* thiz, e32 x, e32 y, e32 w, e32 h);
Ret etk_canvas_fill_rect(EtkCanvas* thiz,e32 x,e32 y,e32 width,e32 height,u32 color);

Ret etk_canvas_draw_rect(EtkCanvas* thiz, e32 x, e32 y, e32 width, e32 height, u32 color);
Ret etk_canvas_draw_bitmap(EtkCanvas* thiz, EtkBitmap* bitmap, EtkRect* s, EtkRect* d, u32 alpha);
Ret etk_canvas_draw_string_with_color(EtkCanvas* thiz, e32 x, e32 y, const char* str,u32 fg,u32 bg);
Ret etk_canvas_draw_string(EtkCanvas* thiz, e32 x, e32 y, const char* str);
Ret etk_canvas_hline(EtkCanvas *thiz,e32 x,e32 y,e32 length,u32 color);
Ret etk_canvas_vline(EtkCanvas *thiz,e32 x,e32 y,e32 length,u32 color);

Ret etk_canvas_draw_char(EtkCanvas* thiz,e32 x,e32 y,u32 ch);
Ret etk_canvas_draw_char_with_color(EtkCanvas* thiz,e32 x,e32 y,u32 ch,u32 fg,u32 bg);

#endif
