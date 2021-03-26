#ifndef _ETK_ENTRY_H
#define _ETK_ENTRY_H


#include "etk_widget.h"

EtkWidget* etk_entry_create(EtkWidget* parent, e32 x, e32 y, e32 width, e32 height);

const char* etk_entry_get_text(EtkWidget* thiz);
Ret etk_entry_set_text(EtkWidget* thiz, const char* text);
Ret etk_entry_insert_text(EtkWidget* thiz,const char* text);
Ret etk_entry_insert_char(EtkWidget* thiz, char c);

//Ret etk_entry_set_tips(EtkWidget* thiz, const char* tips);
//Ret etk_entry_set_input_type(EtkWidget* thiz, EtkInputType type);


#endif