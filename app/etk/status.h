#ifndef _STATUS_H
#define _STATUS_H

#include "etk_type.h"
#include "etk_window.h"
#include "etk_widget.h"
#include "etk_platform.h"

typedef struct status_t {
	float smoke;
	int door_open;
	int door_count;
	int window_open;
	int window_count;
	int alarm;
}Status;

EtkWidget *etk_status_create(e32 x,e32 y,e32 width,e32 height);

void etk_status_set_alarm(EtkWidget* thiz,e32 value);
e32 etk_status_get_door_count(EtkWidget *thiz);
e32 etk_status_get_window_count(EtkWidget *thiz);
void etk_status_set_door_count(EtkWidget *thiz,e32 value);
void etk_status_set_window_count(EtkWidget *thiz,e32 value);
#endif