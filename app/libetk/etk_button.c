#include "etk_button.h"
#include "etk_platform.h"
#include "etk_widget.h"
#include "etk_wnd_manager.h"

typedef struct PrivInfo_t{
	EtkListener listener;
	void* listener_ctx;
	e32 data;
}PrivInfo;


EtkWidget* etk_button_create_ex(e32 x,e32 y,e32 width,e32 height,e32 attrib){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		priv->listener=NULL;
		priv->listener_ctx=NULL;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_BUTTON|attrib,0);
		thiz->event=etk_button_on_event;
		thiz->paint=etk_button_on_paint;
	}
	return thiz;
}

EtkWidget* etk_button_create(e32 x,e32 y,e32 width,e32 height){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		priv->listener=NULL;
		priv->listener_ctx=NULL;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_BUTTON,0);
		thiz->event=etk_button_on_event;
		thiz->paint=etk_button_on_paint;
	}
	return thiz;
}


EtkWidget* etk_create_button(EtkWidget *parent,e32 x,e32 y,e32 width,e32 height){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		priv->listener=NULL;
		priv->listener_ctx=NULL;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_BUTTON,0);
		thiz->event=etk_button_on_event;
		thiz->paint=etk_button_on_paint;
		thiz->rect.x+=parent->rect.x;
		thiz->rect.y+=parent->rect.y;
		etk_widget_append_child(parent,thiz);
	}
	return thiz;
}

Ret etk_button_draw(EtkWidget* thiz,u32 fgcolor,u32 bgcolor){
	const char *text=etk_widget_get_text(thiz);
	EtkRect r=thiz->rect;
	if(etk_is_widget(thiz->type,ETK_WIDGET_SHARE_PARENT_CAVANS) ){
		EtkRect r0=thiz->rect;
		EtkWidget *p;
		for(p=thiz->parent;p!=NULL;){
			if((p->type&ETK_WIDGET_WINDOW)==ETK_WIDGET_WINDOW||((p->type&ETK_WIDGET_DESKTOP)==ETK_WIDGET_DESKTOP)){
				//fill_rect_plus(r0,RED);
				etk_canvas_fill_rect(thiz->canvas,r0.x,r0.y,r0.width,r0.height,bgcolor);//thiz->canvas->gc.bgcolor.color	
				etk_canvas_draw_rect(thiz->canvas,r0.x,r0.y,r0.width,r0.height-1,GRAY);
				if(text!=NULL){
					etk_canvas_draw_string_with_color(thiz->canvas,r0.x+(thiz->rect.width>>2),r0.y+(thiz->rect.height>>2),text,fgcolor,bgcolor);
				}
				r0.x+=(p->rect.x);
				r0.y+=(p->rect.y);
				r0.width++;
				etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&r0);
				break;
			}
			
			r0.x+=(p->rect.x);
			r0.y+=(p->rect.y);	
			p=p->parent;
		}
 	 
	
	}else{
		r.x=r.y=0;
		etk_canvas_fill_rect(thiz->canvas,r.x,r.y,r.width,r.height,bgcolor);//thiz->canvas->gc.bgcolor.color
	
		if(text!=NULL){
			etk_canvas_draw_string_with_color(thiz->canvas,r.x+(thiz->rect.width>>2),r.y+(thiz->rect.height>>2),text,fgcolor,bgcolor);
		}
		etk_canvas_draw_rect(thiz->canvas,r.x,r.y,r.width-1,r.height-1,GRAY);
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&thiz->rect);
	}
	return RET_OK;
}
Ret etk_button_on_paint(EtkWidget* thiz){
	return etk_button_draw(thiz,BLACK,0xc9e7f4);
}

Ret etk_button_on_event(EtkWidget* thiz,EtkEvent* event){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	//dbg_printf("etk_button_on_event");
	EtkEvent e;
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
	case ETK_EVENT_MOUSE_DOWN:{
		//printf("ETK_EVENT_MOUSE_DOWNa\n");
		etk_button_draw(thiz,BLACK,GRAY);
		break;
	}
	case ETK_EVENT_MOUSE_UP:
		//printf("ETK_EVENT_MOUSE_UPa\n");
		if(priv->listener_ctx!=NULL)
			priv->listener(priv->listener_ctx,thiz);
		if(thiz->visable==1){
			etk_button_draw(thiz,BLACK,0xc9e7f4);
		}
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
Ret etk_button_set_clicked_listener(EtkWidget* thiz,EtkListener listener,void *ctx){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	priv->listener=listener;
	priv->listener_ctx=ctx;
}
void etk_button_destroy(EtkWidget* thiz){
	
}

e32 etk_button_get_data(EtkWidget* thiz){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	return priv->data;
}
void etk_button_set_data(EtkWidget* thiz,e32 data){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	priv->data=data;
}