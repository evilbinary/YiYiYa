#ifndef _ETK_FONT_H
#define _ETK_FONT_H

#include "etk_type.h"

struct EtkFont_t;
typedef struct EtkFont_t EtkFont;

struct EtkFont_t{
	e32 width;
	e32 height;
	u32 code;
	void *data;
};

EtkFont *etk_font_default_create(e8 *filename);

#endif