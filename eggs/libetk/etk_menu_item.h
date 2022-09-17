#ifndef _ETK_MENU_ITEM_H
#define _ETK_MENU_ITEM_H

#include "etk_widget.h"
#include "etk_type.h"
#include "etk_menu.h"

EtkWidget* etk_menu_item_create(e32 x,e32 y,e32 width,e32 height);
EtkWidget* etk_create_menu_item(EtkWidget *parent,e32 x,e32 y,e32 width,e32 height);
Ret etk_menu_item_on_paint(EtkWidget* thiz);
Ret etk_menu_item_on_event(EtkWidget* thiz,EtkEvent* event);
Ret etk_menu_item_set_clicked_listener(EtkWidget* thiz, EtkListener listener, void* ctx);

#endif