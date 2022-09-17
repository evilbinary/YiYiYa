#include "etk_type.h"
#include "etk_source_timer.h"
#include "etk_platform.h"

typedef struct PrivInfo_t{
	EtkTimer action;
	size_t interval;
	size_t next_time;
	void* user_data;
}PrivInfo;

static int etk_source_timer_check(EtkSource* thiz){
	PrivInfo *priv=(PrivInfo *)thiz->priv;
	int t = priv->next_time - etk_get_relative_time();
	t = t < 0 ? 0 : t;
	//printf("t=%d\n",t);
	return t;
}

static void etk_source_timer_calc_timer(PrivInfo* priv){
	priv->next_time = etk_get_relative_time() + priv->interval;
	//printf("etk_get_relative_time():%d priv->interval:%d nexttime:%d\n",etk_get_relative_time(),priv->interval,priv->next_time);
	return;
}

Ret etk_source_timer_reset(EtkSource* thiz){
	PrivInfo *priv;
	return_val_if_fail(thiz != NULL, RET_FAIL);
	priv=(PrivInfo *)thiz->priv;
	thiz->disable = 0;
	etk_source_timer_calc_timer(priv);

	return RET_OK;
}

Ret etk_source_timer_modify(EtkSource* thiz, int interval){
	PrivInfo *priv;
	return_val_if_fail(priv != NULL, RET_FAIL);
	priv=(PrivInfo *)thiz->priv;
	priv->interval = interval;
	etk_source_timer_calc_timer(priv);

	return RET_OK;
}

static Ret etk_source_timer_dispatch(EtkSource* thiz){
	Ret ret = RET_FAIL;
	PrivInfo *priv;
	//return_val_if_fail(thiz != NULL, RET_FAIL);
	priv=(PrivInfo *)thiz->priv;
	
	if(thiz->disable > 0){
		etk_source_timer_calc_timer(priv);
		return RET_OK;
	}

	ret = priv->action(priv->user_data);
	etk_source_timer_calc_timer(priv);
	return ret;
}

static void etk_source_timer_destroy(EtkSource* thiz){
	ETK_ZFREE(thiz, sizeof(EtkSource) + sizeof(PrivInfo));

	return;
}

EtkSource* etk_source_timer_create(int interval, EtkTimer action, void* user_data){
	EtkSource* thiz = NULL;
	return_val_if_fail(interval > 0 && action != NULL, NULL);

	thiz = (EtkSource*)ETK_MALLOC(sizeof(EtkSource));
	if(thiz != NULL){
		PrivInfo *priv;
		thiz->priv=ETK_MALLOC(sizeof(PrivInfo));
		priv=(PrivInfo *)thiz->priv;
		thiz->check    = etk_source_timer_check;
		thiz->dispatch = etk_source_timer_dispatch;
		thiz->destroy  = etk_source_timer_destroy;

		thiz->ref = 1;
		priv->interval  = interval;
		priv->user_data = user_data;
		priv->action    = action;
		etk_source_timer_calc_timer(priv);
	}

	return thiz;
}

