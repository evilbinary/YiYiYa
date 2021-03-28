#ifndef _SINCLOCK_H
#define _SINCLOCK_H
#include "etk.h"


static Ret etk_clock_event(EtkWidget* thiz, EtkEvent* event);
static Ret etk_clock_draw(EtkWidget* thiz, unsigned char h, unsigned char m, unsigned char s);
Ret etk_clock_update_time_listener(void *data);
EtkWidget* etk_app_clock();

EtkWidget* etk_app_clock(e32 x,e32 y,e32 width,e32 height);

















#endif
