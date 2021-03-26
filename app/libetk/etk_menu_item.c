#include "etk_menu_item.h"
#include "etk_platform.h"
#include "etk_wnd_manager.h"

typedef struct PrivInfo_t{
	EtkListener listener;
	void* listener_ctx;
}PrivInfo;

EtkWidget* etk_menu_item_create(e32 x,e32 y,e32 width,e32 height){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		priv->listener=NULL;
		priv->listener_ctx=NULL;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_MENU_ITEM,0);
		thiz->event=etk_menu_item_on_event;
		thiz->paint=etk_menu_item_on_paint;
	}
	return thiz;
}
EtkWidget* etk_create_menu_item(EtkWidget *parent,e32 x,e32 y,e32 width,e32 height){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		priv->listener=NULL;
		priv->listener_ctx=NULL;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_MENU_ITEM,0);
		thiz->event=etk_menu_item_on_event;
		thiz->paint=etk_menu_item_on_paint;
		etk_menu_append_item(parent,thiz);
		//thiz->rect.x+=parent->rect.x;
		//thiz->rect.y+=parent->rect.y;
		//thiz->visable=2;		//specil for menu item
		//etk_widget_append_next(parent,thiz);
	}
	return thiz;
}
Ret etk_menu_item_on_paint(EtkWidget* thiz){
	const char *text=etk_widget_get_text(thiz);
	if(text!=NULL){
		//printf("%s\n",text);
		etk_canvas_draw_string(thiz->canvas,(thiz->rect.width>>2)-strlen(text),(thiz->rect.height>>2)-2,text);
	}
	etk_canvas_paint(thiz->display,thiz->canvas,&thiz->rect);
}
Ret etk_menu_item_on_down_paint(EtkWidget *thiz,u32 fgcolor,u32 bgcolor){
	const char *text=etk_widget_get_text(thiz);
	etk_canvas_fill_rect(thiz->canvas,0,0,thiz->rect.width,thiz->rect.height,bgcolor);
	if(text!=NULL){
		//printf("%s\n",text);
		etk_canvas_draw_string_with_color(thiz->canvas,(thiz->rect.width>>2)-strlen(text),(thiz->rect.height>>2)-2,text,fgcolor,bgcolor);
	}
	
}
Ret etk_menu_item_on_event(EtkWidget* thiz,EtkEvent* event){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	EtkEvent e;
	//dbg_printf("etk_button_on_event");
	switch(event->type){
	case ETK_EVENT_SHOW:
		//printf("menu item ETK_EVENT_SHOW\n");
		etk_widget_paint(thiz);
		etk_event_init(&e, ETK_EVENT_SHOW);
		e.widget = thiz;
		etk_wnd_manager_dispatch_event(etk_default_wnd_manager(),&e);
		break;
	case ETK_EVENT_HIDE:
		
		etk_event_init(&e, ETK_EVENT_HIDE);
		e.widget = thiz;
		etk_wnd_manager_dispatch_event(etk_default_wnd_manager(), &e);
		break;
	case ETK_EVENT_MOUSE_DOWN:
		etk_menu_item_on_down_paint(thiz,BLACK,LIGHTGRAY);
		break;
	case ETK_EVENT_MOUSE_UP:
		etk_menu_item_on_down_paint(thiz,BLACK,thiz->canvas->gc.bgcolor.color);
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&thiz->rect);
		//thiz->canvas->draw_rect(thiz->canvas,thiz->rect.x+10,thiz->rect.y+10,thiz->rect.width-10,thiz->rect.height-10,BLUE);
		if(priv->listener_ctx!=NULL)
			priv->listener(priv->listener_ctx,thiz);
		break;
		break;
	case ETK_EVENT_KEY_DOWN:
		//printf("ETK_EVENT_KEY_DOWN\n");
		break;
	case ETK_EVENT_KEY_UP:
		//printf("ETK_EVENT_KEY_UP\n");
		break;
	default:
		break;
	}
	return RET_OK;
}
Ret etk_menu_item_set_clicked_listener(EtkWidget* thiz, EtkListener listener, void* ctx){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	priv->listener=listener;
	priv->listener_ctx=ctx;
	return RET_OK;
}