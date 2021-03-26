#ifndef _ETK_APP_H
#define _ETK_APP_H

#include "etk_widget.h"

int etk_app_main( int argc, char* argv[] );
Ret etk_desktop_on_paint(EtkWidget* thiz);
Ret etk_desktop_on_event(EtkWidget* thiz, EtkEvent* event);



Ret menu0_listener(void *data,void *obj);
Ret menu0_item0_listener(void *data,void *obj);
Ret menu0_item1_listener(void *data,void *obj);
Ret menu0_item2_listener(void *data,void *obj);

Ret menu1_listener(void *data,void *obj);
Ret menu1_item0_listener(void *data,void *obj);
Ret menu1_item1_listener(void *data,void *obj);

Ret timer0_listener(void* data);
Ret timer1_listener(void* data);
Ret timer2_listener(void* data);

Ret button0_listener(void* user_data, void* obj);
Ret button1_listener(void* user_data, void* obj);
Ret button2_listener(void* user_data, void* obj);
Ret button3_listener(void* user_data, void* obj);
Ret button4_listener(void* user_data, void* obj);
Ret button5_listener(void* user_data, void* obj);
Ret button6_listener(void* user_data, void* obj);
Ret button7_listener(void* user_data, void* obj);
Ret button8_listener(void* user_data, void* obj);

#endif