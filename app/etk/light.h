#ifndef _LIGHT_H
#define _LIGHT_H

#include "etk_type.h"
#include "etk_widget.h"
EtkWidget *etk_light_create(e32 x,e32 y,e32 width,e32 height);
Ret etk_light_button_up_paint(EtkWidget* thiz);

#endif