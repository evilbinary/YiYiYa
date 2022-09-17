#ifndef _ETK_GCONF_H
#define _ETK_GCONF_H

#include "etk_font.h"
#include "etk_color.h"

typedef struct EtkGconf_t EtkGconf;
struct EtkGconf_t{
	 EtkFont  *font;
	 EtkColor fgcolor;
	 EtkColor bgcolor;
};


#endif