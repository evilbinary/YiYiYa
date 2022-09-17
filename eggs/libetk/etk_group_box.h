#ifndef _ETK_GROUP_BOX_H
#define _ETK_GROUP_BOX_H

#include "etk_type.h"
#include "etk_widget.h"

EtkWidget* etk_group_box_create_ex(e32 x,e32 y,e32 width,e32 height,e32 attrib);
EtkWidget* etk_group_box_create(e32 x,e32 y,e32 width,e32 height);
Ret etk_group_box_set_checked(EtkWidget *thiz,EtkWidget* widget);
void etk_group_box_append_child(EtkWidget* parent, EtkWidget* child);
Ret etk_group_box_update(EtkWidget *thiz,EtkRect *rect);


#endif