#include "etk_sources_manager.h"
#include "etk_source.h"
#include "etk_global.h"
#include "etk_platform.h"

EtkSourcesManager* etk_default_sources_manager(){
	return etkglobal.sources_manager;
}

EtkSourcesManager* etk_sources_manager_create(){
	EtkSourcesManager *thiz=(EtkSourcesManager*)ETK_MALLOC(sizeof(EtkSourcesManager));
	thiz->source_nr=0;
	thiz->need_refresh=0;
	thiz->sources=NULL;
	thiz->tail=NULL;
	return thiz;
}
Ret  etk_sources_manager_add(EtkSourcesManager* thiz, EtkSource* source){
	EtkSource *p=thiz->sources;
	if(p==NULL){
		thiz->sources=source;
		thiz->sources->next=NULL;
		thiz->sources->prev=NULL;
		thiz->tail=source;
	}else{
		thiz->tail->next=source;
		source->prev=thiz->tail;
		thiz->tail=source;
		source->next=NULL;
	}
	thiz->source_nr++;
	return RET_OK;
}
Ret  etk_sources_manager_remove(EtkSourcesManager* thiz, EtkSource* source){
	if(source->prev==NULL){//head
		thiz->sources=source->next;
		if(source->next!=NULL){
			source->next->prev=NULL;
		}
	}else{	
		source->prev->next=source->next;
		if(source->next!=NULL){
			source->next->prev=source->prev;
		}else{
			thiz->tail=source->prev;
		}
	}
	thiz->source_nr--;
	//ETK_FREE(source);
	return RET_OK;
}
void etk_sources_manager_destroy(EtkSourcesManager* thiz){
	EtkSource *p=thiz->sources;
	EtkSource *t;
	while(p!=NULL){
		t=p;
		p=p->next;
		ETK_FREE(t);
	}
}