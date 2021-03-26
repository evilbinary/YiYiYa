#ifndef _ETK_MENU_H
#define _ETK_MENU_H

#include "etk_widget.h"
#include "etk_type.h"

EtkWidget* etk_menu_create(e32 x,e32 y,e32 width,e32 height);
EtkWidget* etk_create_menu(EtkWidget *parent,e32 x,e32 y,e32 width,e32 height);
Ret etk_menu_on_paint(EtkWidget* thiz);
Ret etk_menu_on_event(EtkWidget* thiz,EtkEvent* event);
Ret etk_menu_set_clicked_listener(EtkWidget* thiz, EtkListener listener, void* ctx);
Ret etk_menu_append_item(EtkWidget *thiz,EtkWidget *item);
#endif