#include "etk_main_loop.h"
#include "etk_global.h"

EtkMainLoop* etk_main_loop_create() {
  EtkMainLoop* thiz = (EtkMainLoop*)ETK_MALLOC(sizeof(EtkMainLoop));
  if (thiz != NULL) {
    thiz->running = 0;
  }
  return thiz;
}
Ret etk_main_loop_run(EtkMainLoop* thiz) {
  thiz->running = 1;
  while (thiz->running) {
    if (etk_event_dispatch() != RET_OK) {
    }
    etk_display_flush();
  }
}

EtkMainLoop* etk_default_main_loop(void) { return etkglobal.main_loop; }

Ret etk_main_loop_quit(EtkMainLoop* thiz) {
  EtkEvent event;
  etk_event_init(&event, ETK_EVENT_NOP);
  thiz->running = 0;
  return etk_event_queue(&event);
}

void etk_main_loop_destroy(EtkMainLoop* thiz) {
  if (thiz != NULL) {
    ETK_FREE(thiz);
  }
}