#ifndef _ETK_SOURCE_MANAGER_H
#define _ETK_SOURCE_MANAGER_H

#include "etk_type.h"
#include "etk_source.h"

struct EtkSourcesManager_t;
typedef struct EtkSourcesManager_t EtkSourcesManager;

struct EtkSourcesManager_t
{
	int source_nr;
	int need_refresh;
	EtkSource* tail;	//tail
	EtkSource* sources;	//head
};

EtkSourcesManager* etk_default_sources_manager();
EtkSourcesManager* etk_sources_manager_create();
Ret  etk_sources_manager_add(EtkSourcesManager* thiz, EtkSource* source);
Ret  etk_sources_manager_remove(EtkSourcesManager* thiz, EtkSource* source);
void etk_sources_manager_destroy(EtkSourcesManager* thiz);



#endif