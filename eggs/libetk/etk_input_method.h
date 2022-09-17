#ifndef _ETK_INPUT_METHOD_H
#define _ETK_INPUT_METHOD_H

#include "etk_widget.h"
#include "etk_event.h"


typedef enum EtkInputType_t{
	ETK_INPUT_NORMAL= 0,
	ETK_INPUT_DIGIT,
	ETK_INPUT_ALPHA,
	ETK_INPUT_URL,
	ETK_INPUT_EMAIL,
	ETK_INPUT_ALL
}EtkInputType;

#define ETK_IM_MAX_CANDIDATES  20

struct EtkInputMethod_t;
typedef struct EtkInputMethod_t EtkInputMethod;

typedef Ret  (*EtkInputMethodFocusIn)(EtkInputMethod* thiz, EtkWidget* editor);
typedef Ret  (*EtkInputMethodFocusOut)(EtkInputMethod* thiz);
typedef Ret  (*EtkInputMethodSetType)(EtkInputMethod* thiz, EtkInputType type);
typedef Ret  (*EtkInputMethodHandleEvent)(EtkInputMethod* thiz,EtkEvent* event);
typedef void (*EtkInputMethodDestroy)(EtkInputMethod* thiz);

struct EtkInputMethod_t{
	const char* name;
	int ref;
	EtkInputMethodFocusIn     focus_in;
	EtkInputMethodFocusOut    focus_out;
	EtkInputMethodSetType     set_type;
	EtkInputMethodHandleEvent handle_event;
	EtkInputMethodDestroy     destroy;
	void *priv;
};

Ret  etk_input_method_focus_in(EtkInputMethod* thiz,EtkWidget* editor);
Ret  etk_input_method_focus_out(EtkInputMethod* thiz);
Ret  etk_input_method_set_type(EtkInputMethod* thiz, EtkInputType type);
Ret  etk_input_method_handle_event(EtkInputMethod* thiz, EtkEvent* event);
void etk_input_method_destroy(EtkInputMethod* thiz);
void etk_input_method_ref(EtkInputMethod* thiz);
void etk_input_method_unref(EtkInputMethod* thiz);

#endif

