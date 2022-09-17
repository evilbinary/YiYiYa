#ifndef _ETK_INPUT_METHOD_MANAGER_H
#define _ETK_INPUT_METHOD_MANAGER_H 

#include "etk_input_method.h"
#include "etk_widget.h"

#define ETK_INPUT_METHOD_MAX_NR 3


struct EtkInputMethodManager_t;
typedef struct EtkInputMethodManager_t EtkInputMethodManager;

EtkInputMethodManager *etk_default_input_method_manager();

EtkInputMethodManager* etk_input_method_manager_create(void);
e32  etk_input_method_manager_count(EtkInputMethodManager* thiz);
Ret  etk_input_method_manager_get(EtkInputMethodManager* thiz, e32 index, EtkInputMethod** im);
Ret  etk_input_method_manager_get_current(EtkInputMethodManager* thiz, EtkInputMethod** im);
Ret  etk_input_method_manager_set_current(EtkInputMethodManager* thiz, e32 index);
Ret  etk_input_method_manager_set_current_type(EtkInputMethodManager* thiz, EtkInputType type);

Ret  etk_input_method_manager_register(EtkInputMethodManager* thiz, EtkInputMethod* im);
Ret  etk_input_method_manager_unregister(EtkInputMethodManager* thiz, EtkInputMethod* im);
void etk_input_method_manager_destroy(EtkInputMethodManager* thiz);

Ret  etk_input_method_manager_focus_out(EtkInputMethodManager* thiz, EtkWidget* widget);
Ret  etk_input_method_manager_focus_ack_commit(EtkInputMethodManager* thiz);
Ret  etk_input_method_manager_focus_in(EtkInputMethodManager* thiz, EtkWidget* widget);



#endif

