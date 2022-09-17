#include "etk_menu.h"
#include "etk_platform.h"
#include "etk_wnd_manager.h"

typedef struct PrivInfo_t{
	e32 item_count;
	EtkListener listener;
	void* listener_ctx;
}PrivInfo;


EtkWidget* etk_menu_create(e32 x,e32 y,e32 width,e32 height){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		priv->listener=NULL;
		priv->listener_ctx=NULL;
		priv->item_count=0;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_MENU,0);
		thiz->event=etk_menu_on_event;
		thiz->paint=etk_menu_on_paint;
	}
	return thiz;
}

EtkWidget* etk_create_menu(EtkWidget *parent,e32 x,e32 y,e32 width,e32 height){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		priv->listener=NULL;
		priv->listener_ctx=NULL;
		priv->item_count=0;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_MENU,0);
		thiz->event=etk_menu_on_event;
		thiz->paint=etk_menu_on_paint;
		thiz->rect.x+=parent->rect.x;
		thiz->rect.y+=parent->rect.y;
		etk_widget_append_child(parent,thiz);
	}
	return thiz;
}
Ret etk_menu_append_item(EtkWidget *thiz,EtkWidget *item){
	PrivInfo *priv;
	priv=(PrivInfo*)thiz->subclass;
	priv->item_count++;
	item->rect.y+=thiz->rect.y;
	item->visable=2;		//specil for menu item
	etk_widget_append_next(thiz,item);
	return RET_OK;
}

Ret etk_menu_on_paint(EtkWidget* thiz){
	const char *text=etk_widget_get_text(thiz);
	if(text!=NULL){
		//printf("%s\n",text);
		etk_canvas_draw_string(thiz->canvas,(thiz->rect.width>>2)-strlen(text),thiz->rect.height>>2,text);
	}
	//etk_canvas_paint(thiz->display,thiz->canvas,&thiz->rect);
	//printf("etk_menu_on_paint\n");
}

void etk_menu_on_select_paint(EtkWidget *thiz,u32 color){
	EtkColor c;
	e32 j;
	EtkBitmap *h=thiz->canvas->bitmap;
	const char *text=etk_widget_get_text(thiz);
	etk_color_init(&c,color,ETK_PIXEL_RGB555);	//RGB24_2_RGB555(0x4f,0x4f,0x4f)
	h=thiz->canvas->bitmap;
	for(j=0;j<thiz->rect.height;j++){
		etk_bitmap_hline(h,0,j,thiz->rect.width,c.color);
	}
	if(text!=NULL){
		etk_canvas_draw_string_with_color(thiz->canvas,(thiz->rect.width>>2)-strlen(text),thiz->rect.height>>2,text,BLACK,color);
	}
}


Ret etk_menu_on_event(EtkWidget* thiz,EtkEvent* event){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	EtkEvent e;
	EtkRect u;
	EtkWidget *p=thiz->next;
	e32 h=thiz->rect.height;
	EtkWidget *w;

	switch(event->type){
	case ETK_EVENT_SHOW:
		//printf("WINDOW ETK_EVENT_SHOW\n");
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
		//printf("menu down\n");
		if(p!=NULL&&p->visable!=1){
			e32 i=0;
			etk_menu_on_select_paint(thiz,LIGHTGRAY);
			u=thiz->rect;
			//draw_rect(u,GREEN);
			while(p!=NULL&&i<priv->item_count){
				if(p->visable!=1&&(p->type&ETK_WIDGET_MENU_ITEM)){
					u.height+=p->rect.height;
					p->visable=1;	
					//printf("i\n");
					i++;
				}
				p=p->next;
			}
			//draw_rect(u,RED);
			etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&u);
		}else if(p!=NULL&&p->visable==1){
			e32 i=0;
			etk_menu_on_select_paint(thiz,thiz->canvas->gc.bgcolor.color);
			u=thiz->rect;

			while(p!=NULL&&i<priv->item_count){
				if(p->visable==1&&(p->type&ETK_WIDGET_MENU_ITEM)){
					u.height+=p->rect.height;
					p->visable=3;
					i++;
				}
				p=p->next;
			}
			etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&u);
		}
		
		break;
	case ETK_EVENT_MOUSE_UP:
		//printf("menu up\n");
		if(priv->listener_ctx!=NULL)
			priv->listener(priv->listener_ctx,thiz);
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

Ret etk_menu_set_clicked_listener(EtkWidget* thiz, EtkListener listener, void* ctx){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	priv->listener=listener;
	priv->listener_ctx=ctx;
	return RET_OK;
}