#include "etk_group_box.h"
#include "etk_platform.h"
#include "etk_wnd_manager.h"
#include "etk_key.h"
#include "etk_stack.h"
#include "etk_rect.h"
#include "etk_check_button.h"

///SLIST_HEAD(EtkLink_t,EtkLink);
STACK_TEMPLATE(EtkStack,void*);


typedef struct PrivInfo_t{
	EtkBitmap* bg;
	void *inner_child;
}PrivInfo;

Ret etk_group_box_draw_rect_border(EtkWidget *thiz,u32 upper,u32 bottom){
	EtkRect r=thiz->rect;
	//ÉÏ
	etk_canvas_hline(thiz->canvas,r.x+2,r.y+10,r.width-4,upper);
	etk_canvas_draw_pixel(thiz->canvas,r.x+1,r.y+11,upper);
	etk_canvas_hline(thiz->canvas,r.x+2,r.y+11,r.width-4,bottom);
	//ÏÂ
	etk_canvas_hline(thiz->canvas,r.x+2,r.y+r.height-1,r.width-4,bottom);
	etk_canvas_hline(thiz->canvas,r.x+2,r.y+r.height-2,r.width-4,upper);
	//×ó
	etk_canvas_vline(thiz->canvas,r.x,r.y+10+2,r.height-11-5,upper);
	etk_canvas_vline(thiz->canvas,r.x+1,r.y+10+2,r.height-11-3,bottom);
	//ÓÒ
	etk_canvas_vline(thiz->canvas,r.x+r.width,r.y+10+2,r.height-11-5,upper);
	etk_canvas_vline(thiz->canvas,r.x+r.width-1,r.y+10+2,r.height-11-3,bottom);

	etk_canvas_draw_pixel(thiz->canvas,r.x+1,r.y+r.height-3,upper);
	etk_canvas_draw_pixel(thiz->canvas,r.x,r.y+r.height-3,bottom);
	etk_canvas_draw_pixel(thiz->canvas,r.x+r.width-1,r.y+11,upper);
	etk_canvas_draw_pixel(thiz->canvas,r.x+2+r.width-3,r.y+r.height-3,upper);
	etk_canvas_draw_pixel(thiz->canvas,r.x+2+r.width-2,r.y+r.height-3,bottom);
	return RET_OK;
}
Ret etk_group_box_on_paint_share_canvas_child(EtkWidget* thiz){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	EtkWidget *child=(EtkWidget*)priv->inner_child;
	while(child!=NULL){
		if(child->visable==1){
			child->paint(child);
		}
		child=child->next;
	}
}

Ret etk_group_box_on_paint(EtkWidget* thiz){
	const char *text=etk_widget_get_text(thiz);
	EtkRect r=thiz->rect;
	etk_group_box_on_paint_share_canvas_child(thiz);
	if(thiz->type&ETK_WIDGET_SHARE_PARENT_CAVANS){
		EtkRect r0=thiz->rect;
		EtkWidget *p;
		for(p=thiz->parent;p!=NULL;p=p->parent){
			r0.x+=(p->rect.x);
			r0.y+=(p->rect.y);
					//draw_rect(r0,RED);
			if(!(p->type&ETK_WIDGET_WINDOW==0)){
				break;
			}
			
		}

		//etk_canvas_draw_rect(thiz->canvas,r.x,r.y+10,r.width,r.height-11,GRAY);
		etk_group_box_draw_rect_border(thiz,GRAY,WHITESMOKE);
		if(text!=NULL){
			etk_canvas_draw_string_with_color(thiz->canvas,r.x+10,r.y,text,thiz->canvas->gc.fgcolor.color,thiz->canvas->gc.bgcolor.color);
		}
		r0.width++;
		
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&r0);
	}else{
		r.x=r.y=0;
		etk_group_box_draw_rect_border(thiz,DIMGRAY,GRAY);
		if(text!=NULL){
			etk_canvas_draw_string_with_color(thiz->canvas,r.x+10,r.y,text,thiz->canvas->gc.fgcolor.color,0xc9e7f4);
		}
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&r);
	}
	return RET_OK;
}

Ret etk_group_box_on_event(EtkWidget* thiz,EtkEvent* event){
	PrivInfo *priv=(PrivInfo*)thiz->subclass;
	EtkEvent e;
	

	switch(event->type){

	case ETK_EVENT_SHOW:
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
		EtkRect r;
		EtkWidget *w;
		r.width=2;
		r.height=2;
		r.x=event->u.mouse.x-thiz->rect.x;
		r.y=event->u.mouse.y-thiz->rect.y;
		//lcd_printf("r.x:%d r.y:%d\n",r.x,r.y);
		//lcd_printf("mx:%d my:%d\n",event->u.mouse.x,event->u.mouse.y);
		w=(EtkWidget*)priv->inner_child;
		
		while(w!=NULL){
			if(w->visable==1){
				//lcd_printf("etk_group_box_on_event\n");
				//draw_rect(w->rect,GREEN);
				//draw_rect(r,RED);
				if(is_rect_cross(w->rect,r)){
					EtkEvent e;
					etk_event_init(&e,ETK_EVENT_MOUSE_DOWN);
					e.widget=w;
					etk_widget_event(w,&e);
					//etk_group_box_update(thiz,&w->rect);
					
				}
			}
			w=w->next;
		}
		break;
	}
	case ETK_EVENT_MOUSE_UP:{
		EtkRect r;
		EtkWidget *w;
		r.width=2;
		r.height=2;
		r.x=event->u.mouse.x-thiz->rect.x;
		r.y=event->u.mouse.y-thiz->rect.y;
		w=(EtkWidget*)priv->inner_child;
		while(w!=NULL){
			if(w->visable==1){
				if(is_rect_cross(w->rect,r)){
					EtkEvent e;
					etk_event_init(&e,ETK_EVENT_MOUSE_UP);
					e.widget=w;
					etk_widget_event(w,&e);
					//etk_group_box_update(thiz,&w->rect);
				}
			}
			w=w->next;
		}
		
		break;
	}
	case ETK_EVENT_KEY_DOWN:
		break;
	case ETK_EVENT_KEY_UP:
		break;
	default:
		break;
	}
	return etk_window_on_child_event(thiz,event);
}
void etk_group_box_destroy(EtkWidget * thiz){

}

EtkWidget* etk_group_box_create_ex(e32 x,e32 y,e32 width,e32 height,e32 attrib){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_BUTTON|ETK_WIDGET_GROUP_BOX|attrib,0);
		thiz->event=etk_group_box_on_event;
		thiz->paint=etk_group_box_on_paint;
		priv->inner_child=NULL;
		priv->bg=NULL;
	}
	return thiz;
}

EtkWidget* etk_group_box_create(e32 x,e32 y,e32 width,e32 height){
	EtkWidget* thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	PrivInfo *priv;
	thiz->subclass=(PrivInfo*)ETK_MALLOC(sizeof(PrivInfo));
	if(thiz->subclass!=NULL){
		priv=(PrivInfo*)thiz->subclass;
		etk_widget_init(thiz,x,y,width,height,ETK_WIDGET_BUTTON|ETK_WIDGET_GROUP_BOX,0);
		thiz->event=etk_group_box_on_event;
		thiz->paint=etk_group_box_on_paint;
		priv->inner_child=NULL;
		priv->bg=NULL;
	}
	return thiz;
}
Ret etk_group_box_set_checked(EtkWidget *thiz,EtkWidget* radio){
	EtkWidget * iter=NULL;
	PrivInfo*priv;

	return_val_if_fail(thiz != NULL && radio != NULL, RET_FAIL);
	iter=thiz->chil;
	while(iter!=NULL){
		if((iter->type&ETK_WIDGET_RADIO_BUTTON)==ETK_WIDGET_RADIO_BUTTON)	
			etk_check_button_set_checked(iter,iter==radio);
		iter=iter->next;
	}
	priv=(PrivInfo*)thiz->subclass;
	iter=(EtkWidget*)priv->inner_child;
	while(iter!=NULL){
		if((iter->type&ETK_WIDGET_RADIO_BUTTON)==ETK_WIDGET_RADIO_BUTTON)
			etk_check_button_set_checked(iter,iter==radio);
		iter=iter->next;
	}
}
void etk_group_box_append_child(EtkWidget* parent, EtkWidget* child){
	if(child->type&ETK_WIDGET_SHARE_PARENT_CAVANS){
		PrivInfo* priv=(PrivInfo*)parent->subclass;
		child->canvas=parent->canvas;
		child->parent=parent;
		if(priv->inner_child==NULL){
			priv->inner_child=child;
			
		}else{
			EtkWidget *w=(EtkWidget*)priv->inner_child;
			etk_widget_append_next(w,child);
		}
	}else{
		etk_widget_append_child(parent,child);
	}
}
Ret etk_group_box_update(EtkWidget *thiz,EtkRect *rect){
	if(!(thiz->type&ETK_WIDGET_SHARE_CAVANS==0)){
		EtkRect r=thiz->parent->rect;
		r.x+=thiz->rect.x;
		r.y+=thiz->rect.y;
		r.x+=rect->x;
		r.y+=rect->y;
		r.width=rect->width;
		r.height=rect->height;
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&r);
		//fill_rect_plus(r,RED);
		//draw_rect(thiz->rect,RED);
		//draw_rect(thiz->parent->rect,RED);
	}else{
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),rect);
	}
}