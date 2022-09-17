#ifndef ETK_WND_MANAGER_H
#define ETK_WND_MANAGER_H

#include "etk_type.h"
#include "etk_widget.h"
#include "etk_event.h"
#include "etk_main_loop.h"

struct EtkWndManager_t;
typedef struct EtkWndManager_t EtkWndManager;

typedef Ret (*EtkWndManagerAdd)(EtkWndManager* thiz, EtkWidget* window);
typedef Ret (*EtkWndManagerRemove)(EtkWndManager* thiz, EtkWidget* window);
typedef Ret (*EtkWndManagerUpdate)(EtkWndManager* thiz);
typedef void (*EtkWndManagerDestroy)(EtkWndManager* thiz);
typedef Ret  (*EtkWndManagerDispatchEvent)(EtkWndManager* thiz, EtkEvent* event);
typedef Ret  (*EtkWndManagerQueueEvent)(EtkWndManager* thiz, EtkEvent* event);

struct EtkWndManager_t{
	//method
	EtkWndManagerAdd add;
	EtkWndManagerRemove remove;
	EtkWndManagerUpdate update;
	EtkWndManagerDestroy destroy;
	EtkWndManagerDispatchEvent dispatch_event;
	EtkWndManagerQueueEvent queue_event;
	//property
	EtkWidget *windows;
	EtkWidget *topwin;

	e32 winnum;
	void *priv;
};

void etk_wnd_manger_init(EtkWndManager* thiz);
Ret etk_wnd_manager_add(EtkWndManager* thiz, EtkWidget* window);
Ret etk_wnd_manager_remove(EtkWndManager* thiz, EtkWidget* window);
Ret etk_wnd_manager_update(EtkWndManager* thiz);
void etk_wnd_manager_destroy(EtkWndManager* thiz);
Ret etk_wnd_manager_dispatch_event(EtkWndManager* thiz, EtkEvent* event);
Ret etk_wnd_manager_queue_event(EtkWndManager* thiz, EtkEvent* event);

Ret etk_wnd_manager_set_grab_widget(EtkWndManager* thiz,EtkWidget* grab);
EtkWidget * etk_wnd_manager_get_grab_widget(EtkWndManager* thiz);

Ret etk_wnd_manager_set_focus_widget(EtkWndManager* thiz,EtkWidget* focus);
EtkWidget * etk_wnd_manager_get_focus_widget(EtkWndManager* thiz);

Ret etk_wnd_manager_set_active_widget(EtkWndManager* thiz,EtkWidget* active);
EtkWidget * etk_wnd_manager_get_active_widget(EtkWndManager* thiz);

EtkWidget* etk_wnd_manager_get_select_widget(EtkWndManager* thiz,e32 x,e32 y);
EtkWidget* etk_wnd_manager_get_select_child(EtkWidget *parent,e32 x,e32 y);
EtkWidget* etk_wnd_manager_get_select_next(EtkWidget *parent,e32 x,e32 y);

EtkWndManager* etk_get_wnd_manager();
EtkWndManager* etk_default_wnd_manager();
Ret etk_default_wnd_manager_add(EtkWndManager* thiz, EtkWidget* window);
Ret etk_default_wnd_manager_remove(EtkWndManager* thiz, EtkWidget* window);
Ret etk_default_wnd_manager_move_to(EtkWndManager* thiz, EtkWidget* window,e32 x,e32 y);
Ret etk_default_wnd_manager_active(EtkWndManager* thiz,EtkWidget* window);
void etk_default_wnd_manager_destroy(EtkWndManager* thiz);
Ret etk_default_wnd_manager_update(EtkWndManager* thiz);
Ret etk_default_wnd_manager_update_widget(EtkWndManager* thiz,EtkWidget *widget);
Ret etk_default_wnd_manager_update_rect(EtkWndManager* thiz,EtkRect *rect);
Ret etk_default_wnd_manager_update_widget_rect(EtkWndManager* thiz,EtkWidget *widget,EtkRect *rect);

EtkWndManager* etk_default_wnd_manager_create(EtkMainLoop* main_loop);
EtkWidget *etk_default_wnd_manager_get_desktop(EtkWndManager *thiz);
Ret etk_default_wnd_manager_set_desktop(EtkWndManager *thiz,EtkWidget *widget);

Ret etk_wnd_manager_default_queue_event(EtkWndManager* thiz, EtkEvent* event);
Ret etk_wnd_manager_default_dispatch_event(EtkWndManager* thiz, EtkEvent* event);

#endif