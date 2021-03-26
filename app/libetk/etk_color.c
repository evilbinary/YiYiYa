#include "etk_color.h"



void etk_color_init(EtkColor *thiz,u32 color,EtkPixelFormat format){
	thiz->color=color;
	thiz->format=format;
}