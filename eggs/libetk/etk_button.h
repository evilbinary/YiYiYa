#ifndef _ETK_BUTTON_H
#define _ETK_BUTTON_H

#include "etk_widget.h"

EtkWidget* etk_button_create_ex(e32 x,e32 y,e32 width,e32 height,e32 attrib);
EtkWidget* etk_button_create(e32 x,e32 y,e32 width,e32 height);
EtkWidget* etk_create_button(EtkWidget *parent,e32 x,e32 y,e32 width,e32 height);


Ret etk_button_on_paint(EtkWidget* thiz);
Ret etk_button_on_event(EtkWidget* thiz,EtkEvent* event);
Ret etk_button_set_clicked_listener(EtkWidget* thiz,EtkListener listener,void *ctx);
void etk_button_destroy(EtkWidget* thiz);

e32 etk_button_get_data(EtkWidget* thiz);
void etk_button_set_data(EtkWidget* thiz,e32 data);

#endif