#ifndef _ETK_SOURCE_H
#define _ETK_SOURCE_H

#include "etk_type.h"

struct EtkSource_t;
typedef struct EtkSource_t EtkSource;

typedef int  (*EtkSourceCheck)(EtkSource* thiz);
typedef Ret  (*EtkSourceDispatch)(EtkSource* thiz);
typedef void (*EtkSourceDestroy)(EtkSource* thiz);

struct EtkSource_t
{
	EtkSourceCheck    check;
	EtkSourceDispatch dispatch;
	EtkSourceDestroy  destroy;
	EtkSource* next;
	EtkSource* prev;

	int  ref;
	int  disable;
	void* priv;
};


Ret etk_source_disable(EtkSource* thiz);
Ret etk_source_enable(EtkSource* thiz);

int etk_source_get_fd(EtkSource* thiz);
int etk_source_check(EtkSource* thiz);
Ret etk_source_dispatch(EtkSource* thiz);
void etk_source_destroy(EtkSource* thiz);

void etk_set_source_default(EtkSource* source);
EtkSource* etk_get_default_source(void);

#endif