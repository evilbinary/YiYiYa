#include "etk_event.h"

#include "etk_global.h"

Ret etk_event_init(EtkEvent* event, EtkEventType type) {
  if (event != NULL) {
    // memset(event, 0x00, sizeof(EtkEvent));
    event->type = type;
  }
  return RET_OK;
}

Ret etk_event_dispatch() {
  EtkEvent event;
  etk_platform_event_poll(&event);
  etk_wnd_manager_queue_event(etk_default_wnd_manager(), &event);
  // etkglobal.wndManager->dispatch_event(etkglobal.wndManager,&event);
  return RET_OK;
}

Ret etk_event_queue(EtkEvent* event) {
  return etk_wnd_manager_default_dispatch_event(etk_default_wnd_manager(),event);
}
