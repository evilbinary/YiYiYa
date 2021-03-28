#include "etk_source.h"

#include "etk_global.h"

typedef struct PrivInfo_t {
  EtkOnEvent on_event;
  void* user_data;
} PrivInfo;

Ret etk_source_disable(EtkSource* thiz) {
  thiz->disable++;

  return RET_OK;
}

Ret etk_source_enable(EtkSource* thiz) {
  if (thiz->disable > 0) {
    thiz->disable--;
  } else {
    thiz->disable = 0;
  }

  return RET_OK;
}

int etk_source_check(EtkSource* thiz) { return thiz->check(thiz); }
Ret etk_source_dispatch(EtkSource* thiz) { return thiz->dispatch(thiz); }
void etk_source_destroy(EtkSource* thiz) { thiz->destroy(thiz); }
void etk_set_source_default(EtkSource* source) {
  etkglobal.default_source = source;
}
EtkSource* etk_get_default_source(void) { return etkglobal.default_source; }


EtkSource* etk_default_source(void) { return etkglobal.default_source; }



Ret etk_source_default_dispatch(EtkSource* thiz) {
  EtkEvent event;
  PrivInfo* priv = (PrivInfo*)thiz->priv;
  int ret;
  if (priv == NULL) {
    return RET_NO_TARGET;
  }
  // dbg_printf("==etk_source_default_dispatch\n");
  switch (event.type) {
    case ETK_EVENT_NOP: {
      break;
    }
    case ETK_EVENT_ADD_SOURCE: {
      etk_sources_manager_add(etk_default_sources_manager(),
                              (EtkSource*)event.u.extra);
      break;
    }
    case ETK_EVENT_REMOVE_SOURCE: {
      etk_sources_manager_remove(etk_default_sources_manager(),
                                 (EtkSource*)event.u.extra);
      break;
    }
    default: {
      if (priv->on_event != NULL) {
        // dbg_printf("etk_source_default_dispatch on event\n");
        priv->on_event(priv->user_data, &event);
      }
      break;
    }
  }

  return RET_OK;
}

Ret etk_source_queue_event(EtkSource* thiz, EtkEvent* event) {
  e32 ret;
  PrivInfo* priv = (PrivInfo*)thiz->priv;
  etk_source_default_dispatch(thiz);
  return ret == sizeof(EtkEvent) ? RET_OK : RET_FAIL;
}

int etk_source_default_check(EtkSource* thiz) {
  PrivInfo* priv = (PrivInfo*)thiz->priv;
  return 0;
}

EtkSource* etk_source_create(EtkOnEvent on_event, void* user_data) {
  EtkSource* thiz = (EtkSource*)ETK_MALLOC(sizeof(EtkSource));
  if (thiz != NULL) {
    PrivInfo* priv;
    thiz->priv = (PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
    priv = (PrivInfo*)thiz->priv;
    thiz->check = etk_source_default_check;
    thiz->dispatch = etk_source_default_dispatch;
    thiz->destroy = etk_source_destroy;
    thiz->next = NULL;
    thiz->prev = NULL;
    thiz->disable = 0;

    thiz->ref = 1;
    priv->on_event = on_event;
    priv->user_data = user_data;
  }
  return thiz;
}
