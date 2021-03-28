#ifndef _ETK_CHECK_BUTTON_H
#define _ETK_CHECK_BUTTON_H

#include "etk_type.h"
#include "etk_widget.h"
	
EtkWidget* etk_check_button_create(e32 x,e32 y,e32 width,e32 height);
EtkWidget* etk_check_button_create_ex(e32 x,e32 y,e32 width,e32 height,e32 attrib);
Ret etk_check_button_check(EtkWidget* thiz);
e32 etk_check_button_get_data(EtkWidget* thiz);
void etk_check_button_set_data(EtkWidget* thiz,e32 data);

#endif