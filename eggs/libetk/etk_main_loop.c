#include "etk_main_loop.h"

#include "etk_global.h"

EtkMainLoop* etk_main_loop_create(EtkSourcesManager* sources_manager) {
  EtkMainLoop* thiz = (EtkMainLoop*)ETK_MALLOC(sizeof(EtkMainLoop));
  if (thiz != NULL) {
    thiz->running = 0;
    thiz->sources_manager = sources_manager;
  }
  return thiz;
}
Ret etk_main_loop_run(EtkMainLoop* thiz) {
  e32 wait_time = 3600 * 10000;
  e32 source_wait_time = 0;
  EtkSource* source;
  thiz->running = 1;

  while (thiz->running) {
    if (etk_event_dispatch() != RET_OK) {
    }
    for (source = thiz->sources_manager->sources; source != NULL;
         source = source->next) {
      source_wait_time = etk_source_check(source);
      if (source_wait_time >= 0 && source_wait_time < wait_time) {
        wait_time = source_wait_time;
      }

      if (source->disable > 0) {
        etk_sources_manager_remove(thiz->sources_manager, source);
        dbg_printf("etk_sources_manager_remove11\n");
        continue;
      }
      if ((source_wait_time = etk_source_check(source)) == 0) {
        // printf("source_xxxx_time:%d\n",source_wait_time);
        // dbg_printf("etk_source_check\n");
        if (etk_source_dispatch(source) != RET_OK) {
          dbg_printf("etk_sources_manager_remove33\n");
          etk_sources_manager_remove(thiz->sources_manager, source);
        }
      }
    }
    etk_display_flush();
  }
}

EtkMainLoop* etk_default_main_loop(void) { return etkglobal.main_loop; }

void etk_main_loop_destroy(EtkMainLoop* thiz) {
  if (thiz != NULL) {
    ETK_FREE(thiz);
  }
}

Ret etk_main_loop_quit(EtkMainLoop* thiz) {
  EtkEvent event;
  etk_event_init(&event, ETK_EVENT_QUIT);
  thiz->running = 0;
  etk_event_queue(&event);
  return etk_source_queue_event(etk_default_source(), &event);
}
Ret etk_main_loop_add_source(EtkMainLoop* thiz, EtkSource* source) {
  EtkEvent event;
  etk_event_init(&event, ETK_EVENT_ADD_SOURCE);
  event.u.extra = source;
  etk_source_enable(source);
  return etk_source_queue_event(etk_default_source(), &event);
}
Ret etk_main_loop_remove_source(EtkMainLoop* thiz, EtkSource* source) {
  EtkEvent event;
  etk_event_init(&event, ETK_EVENT_REMOVE_SOURCE);
  event.u.extra = source;
  etk_source_enable(source);
  return etk_source_queue_event(etk_default_source(), &event);
}