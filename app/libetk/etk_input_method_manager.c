#include "etk_input_method_manager.h"
#include "etk_input_method.h"
#include "etk_type.h"
#include "etk_widget.h"
#include "etk_platform.h"
#include "ekt_input_method_pinyin.h"
#include "etk_global.h"



struct EtkInputMethodManager_t{
	e32 nr;
	e32 current;
	EtkWidget* widget;
	EtkInputMethod* methods[ETK_INPUT_METHOD_MAX_NR+1];
};

EtkInputMethodManager* etk_input_method_manager_create(void){
	EtkInputMethodManager* thiz = (EtkInputMethodManager*)ETK_MALLOC(sizeof(EtkInputMethodManager));
	if(thiz != NULL){
		thiz->nr=0;
		thiz->current = 0;
	}
	return thiz;
}

EtkInputMethodManager *etk_default_input_method_manager(){
	return etkglobal.input_method_manager;
}

e32  etk_input_method_manager_count(EtkInputMethodManager* thiz){
	return thiz != NULL ? thiz->nr : 0;
}

Ret  etk_input_method_manager_get(EtkInputMethodManager* thiz, e32 index, EtkInputMethod** im){
	return_val_if_fail(thiz != NULL && im != NULL, RET_FAIL);
	if(index >= thiz->nr){
		*im = NULL;
		return RET_FAIL;
	}
	*im = thiz->methods[index];
	return RET_OK;
}

Ret  etk_input_method_manager_get_current(EtkInputMethodManager* thiz, EtkInputMethod** im){
	return etk_input_method_manager_get(thiz, thiz->current, im);
}

Ret  etk_input_method_manager_set_current(EtkInputMethodManager* thiz, e32 index){
	return_val_if_fail(thiz != NULL, RET_FAIL);

	thiz->current = index;

	return RET_OK;
}

Ret  etk_input_method_manager_set_current_type(EtkInputMethodManager* thiz, EtkInputType type){
	return_val_if_fail(thiz != NULL, RET_FAIL);

	if(thiz->current < thiz->nr){
		etk_input_method_set_type(thiz->methods[thiz->current], type);
	}
	return RET_OK;
}

Ret  etk_input_method_manager_register(EtkInputMethodManager* thiz, EtkInputMethod* im){
	return_val_if_fail(thiz != NULL && im != NULL && thiz->nr < ETK_INPUT_METHOD_MAX_NR, RET_FAIL);

	thiz->methods[thiz->nr] = im;
	thiz->nr++;

	return RET_OK;
}

Ret  etk_input_method_manager_unregister(EtkInputMethodManager* thiz, EtkInputMethod* im){
	e32 i = 0;
	return_val_if_fail(thiz != NULL && im != NULL, RET_FAIL);
	for(i = 0; i < thiz->nr; i++){
		if(thiz->methods[i] == im){
			etk_input_method_destroy(thiz->methods[i]);
			thiz->methods[i] = NULL;
			break;
		}
	}
	
	for(; i < thiz->nr; i++){
		thiz->methods[i] = thiz->methods[i+1];
	}

	return RET_OK;
}

void etk_input_method_manager_destroy(EtkInputMethodManager* thiz){
	e32 i = 0;
	if(thiz != NULL){
		for(i = 0; i < thiz->nr; i++){
			etk_input_method_destroy(thiz->methods[i]);
			thiz->methods[i] = NULL;
		}
		ETK_ZFREE(thiz, sizeof(EtkInputMethodManager));
	}

	return;
}

Ret  etk_input_method_manager_focus_in(EtkInputMethodManager* thiz, EtkWidget* widget){
	return_val_if_fail(thiz != NULL, RET_FAIL);

	if(thiz->current < thiz->nr){
		if(etk_input_method_focus_in(thiz->methods[thiz->current], widget) == RET_OK){
			thiz->widget = widget;
		}
	}

	return RET_OK;
}

Ret  etk_input_method_manager_focus_out(EtkInputMethodManager* thiz, EtkWidget* widget){
	return_val_if_fail(thiz != NULL, RET_FAIL);

	if(thiz->current < thiz->nr && thiz->widget == widget){
		thiz->widget = NULL;
		etk_input_method_focus_out(thiz->methods[thiz->current]);
	}

	return RET_OK;
}

Ret  etk_input_method_manager_focus_ack_commit(EtkInputMethodManager* thiz){
	return_val_if_fail(thiz != NULL, RET_FAIL);

	if(thiz->current < thiz->nr){
		EtkEvent event;
		etk_event_init(&event, ETK_EVENT_IM_ACT_COMMIT);
		etk_input_method_handle_event(thiz->methods[thiz->current], &event);
	}

	return RET_OK;
}
