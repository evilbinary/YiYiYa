#include "etk_check_button.h"
#include "etk_wnd_manager.h"
#include "etk_group_box.h"
#include "etk_canvas_ex.h"

typedef struct PrivInfo_t{
	e32 checked;
	e32 is_radio;
	e32 data;
	EtkListener listener;
	void* listener_ctx;
}PrivInfo;

Ret etk_check_button_draw_check(EtkWidget *thiz,u32 fgcolor,u32 bgcolor){
	const char *text=etk_widget_get_text(thiz);
	EtkRect r=thiz->rect;
	if(!(thiz->type&ETK_WIDGET_SHARE_PARENT_CAVANS==0) ){
		EtkRect r0=thiz->rect;
		EtkWidget *p;
		for(p=thiz->parent;p!=NULL;){
			if((p->type&ETK_WIDGET_WINDOW)==ETK_WIDGET_WINDOW||((p->type&ETK_WIDGET_DESKTOP)==ETK_WIDGET_DESKTOP)){
				 
				if(etk_check_button_get_checked(thiz)==0){
					etk_canvas_fill_rect(thiz->canvas,r0.x,r0.y,r0.width/3,r0.height,bgcolor);//thiz->canvas->gc.bgcolor.color	
					etk_canvas_draw_rect(thiz->canvas,r0.x,r0.y,r0.width/3,r0.height,GRAY);
				}else if(etk_check_button_get_checked(thiz)==1){
					etk_canvas_fill_rect(thiz->canvas,r0.x,r0.y,r0.width/3,r0.height,LIGHTBLUE);//thiz->canvas->gc.bgcolor.color	
					etk_canvas_draw_rect(thiz->canvas,r0.x,r0.y,r0.width/3,r0.height,GRAY);
				}
				if(text!=NULL){
					etk_canvas_draw_string_with_color(thiz->canvas,r0.x+thiz->rect.width/3+5,r0.y+(thiz->rect.height>>2),text,fgcolor,bgcolor);
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
		etk_canvas_draw_rect(thiz->canvas,r.x,r.y,r.width-1,r.height-1,GRAY);
		if(text!=NULL){
			etk_canvas_draw_string_with_color(thiz->canvas,r.x+(thiz->rect.width>>2),r.y+(thiz->rect.height>>2),text,fgcolor,bgcolor);
		}
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&thiz->rect);
	}
	return RET_OK;
}
Ret etk_check_button_draw_radio(EtkWidget *thiz,u32 fgcolor,u32 bgcolor){
	 const char *text=etk_widget_get_text(thiz);
	EtkRect r=thiz->rect;
	if(!(thiz->type&ETK_WIDGET_SHARE_PARENT_CAVANS==0) ){
		EtkRect r0=thiz->rect;
		EtkWidget *p;
		for(p=thiz->parent;p!=NULL;){
			if((p->type&ETK_WIDGET_WINDOW)==ETK_WIDGET_WINDOW||((p->type&ETK_WIDGET_DESKTOP)==ETK_WIDGET_DESKTOP)){
				if(etk_check_button_get_checked(thiz)==0){
					etk_canvas_fill_circle(thiz->canvas,r0.x+r0.width/6,r0.y+r0.height/2,r0.height/2,bgcolor);
					etk_canvas_draw_circle(thiz->canvas,r0.x+r0.width/6,r0.y+r0.height/2,r0.height/2,GRAY);
				}else if(etk_check_button_get_checked(thiz)==1){
					etk_canvas_fill_circle(thiz->canvas,r0.x+r0.width/6,r0.y+r0.height/2,r0.height/2,LIGHTBLUE);
					etk_canvas_draw_circle(thiz->canvas,r0.x+r0.width/6,r0.y+r0.height/2,r0.height/2,GRAY);
				}
				if(text!=NULL){
					etk_canvas_draw_string_with_color(thiz->canvas,r0.x+thiz->rect.width/3+5,r0.y+(thiz->rect.height>>2),text,fgcolor,bgcolor);
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
		etk_canvas_draw_rect(thiz->canvas,r.x,r.y,r.width-1,r.height-1,GRAY);
		if(text!=NULL){
			etk_canvas_draw_string_with_color(thiz->canvas,r.x+(thiz->rect.width>>2),r.y+(thiz->rect.height>>2),text,fgcolor,bgcolor);
		}
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&thiz->rect);
	}
	return RET_OK;
}
Ret etk_check_button_draw(EtkWidget* thiz,u32 fgcolor,u32 bgcolor){
	if((thiz->type&ETK_WIDGET_CHECK_BUTTON)==ETK_WIDGET_CHECK_BUTTON){
		//printf("ETK_WIDGET_CHECK_BUTTON\n");
		return etk_check_button_draw_check(thiz,fgcolor,bgcolor);
	}else if((thiz->type&ETK_WIDGET_RADIO_BUTTON)==ETK_WIDGET_RADIO_BUTTON){
		//printf("ETK_WIDGET_RADIO_BUTTON\n");
		return etk_check_button_draw_radio(thiz,fgcolor,bgcolor);
	}else{
		//printf("else\n");
	}
	
}
Ret etk_check_button_on_paint(EtkWidget* thiz){
	thiz->visable=1;
	return etk_check_button_draw(thiz,BLACK,thiz->canvas->gc.bgcolor.color);
	 
}
Ret etk_check_button_on_event(EtkWidget* thiz,EtkEvent* event){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	EtkEvent e;
	//printf("etk_check_button_on_event\n");
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
		etk_check_button_draw(thiz,BLACK,WHITE);
		break;
	}
	case ETK_EVENT_MOUSE_UP:
		etk_check_button_check(thiz);
		etk_check_button_draw(thiz,BLACK,WHITE);
		if(priv->listener_ctx!=NULL)
			priv->listener(priv->listener_ctx,thiz);
		break;
	case ETK_EVENT_KEY_DOWN:
		break;
	case ETK_EVENT_KEY_UP:
		break;
	default:
		break;
	}
	return RET_OK;
}
Ret etk_check_button_set_clicked_listener(EtkWidget* thiz,EtkListener listener,void *ctx){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	priv->listener=listener;
	priv->listener_ctx=ctx;
}
Ret  etk_check_button_set_checked(EtkWidget* thiz, e32 checked){
	PrivInfo *priv;
	return_val_if_fail(thiz != NULL, RET_FAIL);
	priv=(PrivInfo*)thiz->subclass;
	if(priv->checked != checked){
		priv->checked = checked;
		etk_check_button_on_paint(thiz);
		//printf("check:%d\n",priv->checked);
			
	}
	return RET_OK;
}
Ret etk_check_button_check(EtkWidget* thiz){
	Ret ret = RET_OK;
	PrivInfo* priv;
	return_val_if_fail(thiz != NULL, RET_FAIL);
	priv=(PrivInfo*)thiz->subclass;
	if(priv->is_radio&&(thiz->parent->type&ETK_WIDGET_GROUP_BOX)==ETK_WIDGET_GROUP_BOX){
		ret = etk_group_box_set_checked(thiz->parent, thiz);
	}
	else{
		ret = etk_check_button_set_checked(thiz, !priv->checked);
	}

	return ret;
}

e32  etk_check_button_get_checked(EtkWidget* thiz){
	PrivInfo *priv;
	return_val_if_fail(thiz != NULL, RET_FAIL);
	priv=(PrivInfo*)thiz->subclass;
	return priv->checked;
}

EtkWidget* etk_check_button_create_ex(e32 x,e32 y,e32 width,e32 height,e32 attrib){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		priv->listener=NULL;
		priv->listener_ctx=NULL;
		priv->checked=0;
		if((attrib&ETK_WIDGET_RADIO_BUTTON)==ETK_WIDGET_RADIO_BUTTON){
			priv->is_radio=1;
		}else{
			priv->is_radio=0;
		}
		thiz->visable=1;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_BUTTON|attrib,0);
		thiz->event=etk_check_button_on_event;
		thiz->paint=etk_check_button_on_paint;
	}
	return thiz;
}
e32 etk_check_button_get_data(EtkWidget* thiz){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	return priv->data;
}
void etk_check_button_set_data(EtkWidget* thiz,e32 data){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	priv->data=data;
}

EtkWidget* etk_check_button_create(e32 x,e32 y,e32 width,e32 height){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		priv->listener=NULL;
		priv->listener_ctx=NULL;
		priv->checked=0;
		priv->is_radio=0;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_BUTTON|ETK_WIDGET_CHECK_BUTTON,0);
		thiz->event=etk_check_button_on_event;
		thiz->paint=etk_check_button_on_paint;
	}
	return thiz;
}
