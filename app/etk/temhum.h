#ifndef _TEMANDHUM_H
#define _TEMANDHUM_H

#include "etk_widget.h"


EtkWidget *etk_temhum_create(e32 x,e32 y,e32 width,e32 height);
void etk_temhum_set_tem(EtkWidget *thiz,float tem);
float etk_temhum_get_tem(EtkWidget *thiz);
void etk_temhum_set_hum(EtkWidget *thiz,float hum);
float etk_temhum_get_hum(EtkWidget *thiz);

#endif