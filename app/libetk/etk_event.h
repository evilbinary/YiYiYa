#ifndef _ETK_EVENT_H
#define _ETK_EVENT_H

#include "etk_type.h"
#include "etk_rect.h"


typedef enum EtkEventType_e
{
	ETK_EVENT_NOP = 0,
	ETK_EVENT_IDLE,
	ETK_EVENT_INIT,
	ETK_EVENT_TIMER,
	ETK_EVENT_QUIT,
	ETK_EVENT_KEY_UP,
	ETK_EVENT_KEY_DOWN,
	ETK_EVENT_MOUSE_MOVE,
	ETK_EVENT_MOUSE_DOWN,
	ETK_EVENT_MOUSE_LONG_PRESS,
	ETK_EVENT_KEY_LONG_PRESS,
	ETK_EVENT_MOUSE_UP,
	ETK_EVENT_HIDE,
	ETK_EVENT_SHOW,
	ETK_EVENT_MAP,
	ETK_EVENT_UNMAP,
	ETK_EVENT_ACTIVE,
	ETK_EVENT_UPDATE,
	ETK_EVENT_MOVE,
	ETK_EVENT_RESIZE,
	ETK_EVENT_MOVE_RESIZE,
	ETK_EVENT_FOCUS_IN,
	ETK_EVENT_FOCUS_OUT,
	ETK_EVENT_ADD_CHILD,
	ETK_EVENT_IM_COMMIT,
	ETK_EVENT_IM_PREEDIT,
	ETK_EVENT_SET_TEXT,
	ETK_EVENT_GET_TEXT,
	ETK_EVENT_IM_ACT_COMMIT,
	ETK_EVENT_OS_IM_COMMIT,
	ETK_EVENT_REMOVE_CHILD,
	ETK_EVENT_WND_DESTROY,
	ETK_EVENT_RELAYOUT_WND,
	ETK_EVENT_ADD_SOURCE,
	ETK_EVENT_REMOVE_SOURCE,
	ETK_EVENT_THEME_CHANGED,
	ETK_EVENT_SCREEN_ROTATED,
	ETK_EVENT_DISABLE_CURSOR,
	ETK_EVENT_ENABLE_CURSOR,
	ETK_EVENT_OS_SCREEN_ROTATED,
	ETK_EVENT_LOCALE_CHANGED,
	ETK_EVENT_DISPLAY_CHANGED, 
	ETK_EVENT_TOP_WND_CHANGED,
	ETK_EVENT_WND_CONFIG_CHANGED,
	ETK_EVENT_TAB_PAGE_ACTIVATE,
	ETK_EVENT_TAB_PAGE_DEACTIVATE
}EtkEventType;

typedef struct EtkEvent_t{
	EtkEventType type;
	void* widget;
	u32 time;
	union{
		struct Idle_t{
			EtkIdle action;
			void* user_data;
		}idle;
		struct Timer_t{
			EtkTimer action;
			e32 interval;
			void* user_data;
		}timer;
		struct Key_t{
			int code;
		}key;
		struct Mouse_t{
			e32 press;
			e32 button;
			e32 x;
			e32 y;
		}mouse;
		EtkRect rect;
		void* extra;
	}u;
}EtkEvent;

typedef Ret (*EtkOnEvent)(void* user_data, EtkEvent* event);

Ret etk_event_init(EtkEvent* event,EtkEventType type);
Ret etk_event_dispatch();
Ret etk_event_queue(EtkEvent* event);

#endif