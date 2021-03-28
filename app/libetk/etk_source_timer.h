#ifndef ETK_SOURCE_TIMER_H
#define ETK_SOURCE_TIMER_H
#include "etk_source.h"


EtkSource* etk_source_timer_create(int interval, EtkTimer action, void* user_data);
Ret etk_source_timer_reset(EtkSource* thiz);
Ret etk_source_timer_modify(EtkSource* thiz, int interval);

#endif

