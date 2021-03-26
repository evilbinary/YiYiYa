#ifndef _ETK_INPUT_METHOD_PINYIN_H
#define _ETK_INPUT_METHOD_PINYIN_H

#include "etk_input_method.h"


char* get_match_hz_string(char* pString);


EtkInputMethod *etk_input_method_pinyin_create();

Ret etk_input_method_pinyin_on_paint(EtkWidget* thiz);
Ret app_input_on_mouse_event(EtkWidget* thiz, EtkEvent* event);
Ret etk_input_method_pinyin_on_event(EtkWidget* thiz, EtkEvent* event);

Ret  etk_input_method_pinyin_focus_in(EtkInputMethod* thiz,EtkWidget* editor);
Ret  etk_input_method_pinyin_focus_out(EtkInputMethod* thiz);
Ret  etk_input_method_pinyin_set_type(EtkInputMethod* thiz, EtkInputType type);
Ret  etk_input_method_pinyin_handle_event(EtkInputMethod* thiz, EtkEvent* event);
void etk_input_method_pinyin_destroy(EtkInputMethod* thiz);
void etk_input_method_pinyin_ref(EtkInputMethod* thiz);
void etk_input_method_pinyin_unref(EtkInputMethod* thiz);

#endif