#include "etk_input_method.h"
#include "etk_widget.h"
#include "etk_type.h"

Ret  etk_input_method_focus_in(EtkInputMethod* thiz, EtkWidget* editor){
	return_val_if_fail(thiz != NULL && thiz->focus_in != NULL, RET_FAIL);
	return thiz->focus_in(thiz, editor);
}

Ret  etk_input_method_focus_out(EtkInputMethod* thiz){
	return_val_if_fail(thiz != NULL && thiz->focus_out != NULL, RET_FAIL);

	return thiz->focus_out(thiz);
}

Ret  etk_input_method_set_type(EtkInputMethod* thiz, EtkInputType type){
	return_val_if_fail(thiz != NULL && thiz->set_type != NULL, RET_FAIL);

	return thiz->set_type(thiz, type);
}

Ret  etk_input_method_handle_event(EtkInputMethod* thiz, EtkEvent* event){
	return_val_if_fail(thiz != NULL && thiz->handle_event != NULL, RET_FAIL);

	return thiz->handle_event(thiz, event);
}

void etk_input_method_destroy(EtkInputMethod* thiz){
	if(thiz != NULL && thiz->destroy != NULL){
		thiz->destroy(thiz);
	}

	return;
}

void etk_input_method_ref(EtkInputMethod* thiz){
	if(thiz != NULL){
		thiz->ref++;
	}

	return;
}

void etk_input_method_unref(EtkInputMethod* thiz){
	if(thiz != NULL){
		thiz->ref--;
		if(thiz->ref == 0){
			etk_input_method_destroy(thiz);	
		}
		else{
			//assert(thiz->ref > 0);
		}
	}
	return;
}

