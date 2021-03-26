#ifndef _ETK_COLOR_H
#define _ETK_COLOR_H

#include "etk_type.h"
struct EtkColor_t;
typedef struct EtkColor_t EtkColor;
struct EtkColor_t{
	u32 color;
	EtkPixelFormat format;
};

void etk_color_init(EtkColor *thiz,u32 color,EtkPixelFormat format);


#endif