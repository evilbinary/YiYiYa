#ifndef _ETK_DISPLAY_H
#define _ETK_DISPLAY_H

#include "etk_bitmap.h"
#include "etk_type.h"
#include "etk_rect.h"

struct EtkDisplay_t;
typedef struct EtkDisplay_t EtkDisplay;
typedef Ret (*EtkDisplayUpdate)(EtkDisplay *display,EtkBitmap* bitmap, EtkRect* rect, e32 xoffset,e32 yoffset);

struct EtkDisplay_t{
	u16 width;
	u16 height;
	EtkPixelFormat  format;
	EtkDisplayUpdate update;
};


Ret etk_display_update(EtkDisplay *thiz, EtkBitmap *bitmap, EtkRect *rect, e32 xoffset, e32 yoffset);
EtkDisplay* etk_get_display();
Ret etk_display_flush();

#endif