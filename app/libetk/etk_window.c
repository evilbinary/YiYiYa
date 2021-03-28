#include "etk_window.h"
#include "etk_platform.h"
#include "etk_global.h"
#include "etk_wnd_manager.h"
#include "etk_key.h"
#include "etk_stack.h"
#include "etk_rect.h"
#include "etk_theme.h"

//SLIST_HEAD(EtkLink_t,EtkLink);
STACK_TEMPLATE(EtkStack,void*);


EtkWidget* etk_create_window(e32 x,e32 y,e32 width,e32 height,e32 type){
	return etk_window_create(x,y,width,height,type);
}
EtkWidget* etk_window_create(e32 x,e32 y,e32 width,e32 height,e32 type){
	EtkWidget *thiz=(EtkWidget*)ETK_MALLOC(sizeof(EtkWidget));
	EtkWindowClass *priv;
	thiz->subclass=ETK_MALLOC(sizeof(EtkWindowClass));
	priv=(EtkWindowClass*)thiz->subclass;
	priv->close_rect.x=4;
	priv->close_rect.y=6;
	priv->close_rect.width=8;
	priv->close_rect.height=8;
	priv->head_rect.x=0;
	priv->head_rect.y=0;
	priv->head_rect.width=width;
	priv->head_rect.height=20;
	priv->has_head=1;
	priv->inner_child=NULL;
	priv->data[0]=NULL;
	priv->data[1]=NULL;
	priv->modal=NULL;
	
	etk_widget_init(thiz,x,y,width,height,type,0);
	thiz->event=etk_window_on_event;
	thiz->paint=etk_window_on_paint;
	thiz->destroy=etk_window_destroy;
	if(thiz->type&ETK_WIDGET_DESKTOP!=0){
		priv->has_head=0;
	}
	return thiz;
}

Bool etk_window_has_head(EtkWidget *thiz){
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	return priv->has_head;
}


Ret etk_window_on_paint_share_canvas_child(EtkWidget* thiz){
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	EtkWidget *child=(EtkWidget*)priv->inner_child;
	while(child!=NULL){
		if(child->visable==1){
			child->paint(child);
		}
		child=child->next;
	}
}
Ret etk_window_on_paint_head(EtkWidget* thiz){
	EtkColor color;
	EtkBitmap *h=NULL;
	const e8 *text=NULL;
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	e32 j;
	EtkRect r=priv->close_rect;
	EtkRect t=priv->head_rect;
	etk_color_init(&color,WINDOW_HEAD_COLOR,ETK_PIXEL_BGR24);
	h=thiz->canvas->bitmap;
	for(j=t.y;j<t.height;j++){
		etk_bitmap_hline(h,t.x,j,t.width,color.color);
	}
	
	
	etk_bitmap_draw_line_color(h,r.x,r.y,r.x+r.width,r.y+r.height,WINDOW_CLOSE_COLOR);
	etk_bitmap_draw_line_color(h,r.x,r.y+r.width,r.x+r.height,r.y,WINDOW_CLOSE_COLOR);

	// etk_bitmap_draw_rect(h,r.x-1,r.y-1,r.width+2,r.height+2,BLACK);
	// etk_bitmap_draw_rect(h,r.x+1,r.y+1,r.width-2,r.height-2,GRAY);
	

	// etk_bitmap_hline(h,r.x,r.y,r.width-1,WHITESMOKE);
	// etk_bitmap_vline(h,r.x,r.y,r.height,WHITESMOKE);
	// etk_bitmap_hline(h,r.x,r.y+r.height,r.width,GRAY);
	// etk_bitmap_vline(h,r.x+r.width,r.y,r.height,GRAY);
	

	// etk_canvas_hline(thiz->canvas,2,t.y+t.height-3,thiz->rect.width-5,GRAY);
	text=etk_widget_get_text(thiz);
	if(text!=NULL){
		etk_canvas_draw_string_with_color(thiz->canvas,25,3,text,WINDOW_TEXT_COLOR,color.color);
	}

	return RET_OK;
}
Ret etk_window_on_paint_head_with_color(EtkWidget* thiz,u32 fgcolor,u32 bgcolor){
	EtkBitmap *h=NULL;
	const e8 *text=NULL;
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	e32 j;
	EtkRect r=priv->close_rect;
	EtkRect t=priv->head_rect;
	h=thiz->canvas->bitmap;
	for(j=t.y;j<t.height;j++){
		etk_bitmap_hline(h,t.x,j,t.width,bgcolor);
	}
	
	etk_bitmap_draw_line_color(h,r.x,r.y,r.x+r.width,r.y+r.height,WINDOW_CLOSE_COLOR);
	etk_bitmap_draw_line_color(h,r.x,r.y+r.width,r.x+r.height,r.y,WINDOW_CLOSE_COLOR);
	
	// etk_bitmap_draw_line(h,r.x,r.y,r.x+r.width,r.y+r.height);
	// etk_bitmap_draw_line(h,r.x,r.y+r.width,r.x+r.height,r.y);

	// etk_bitmap_draw_rect(h,r.x-1,r.y-1,r.width+2,r.height+2,BLACK);
	// etk_bitmap_draw_rect(h,r.x+1,r.y+1,r.width-2,r.height-2,GRAY);
	

	// etk_bitmap_hline(h,r.x,r.y,r.width-1,WHITESMOKE);
	// etk_bitmap_vline(h,r.x,r.y,r.height,WHITESMOKE);
	// etk_bitmap_hline(h,r.x,r.y+r.height,r.width,GRAY);
	// etk_bitmap_vline(h,r.x+r.width,r.y,r.height,GRAY);
	

	// etk_canvas_hline(thiz->canvas,2,t.y+t.height-3,thiz->rect.width-5,GRAY);
	text=etk_widget_get_text(thiz);
	if(text!=NULL){
		etk_canvas_draw_string_with_color(thiz->canvas,25,3,text,fgcolor,bgcolor);
	}

	return RET_OK;
}

Ret etk_window_on_paint(EtkWidget* thiz){	
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	if((thiz->type&ETK_WIDGET_WINDOW)){
		if(priv->has_head==1){
			etk_window_on_paint_head(thiz);
			// etk_canvas_draw_rect(thiz->canvas,0,0,thiz->rect.width-1,thiz->rect.height-1,DIMGRAY);
			// etk_canvas_draw_rect(thiz->canvas,2,2,thiz->rect.width-5,thiz->rect.height-5,GRAY);

			// etk_canvas_hline(thiz->canvas,1,1,thiz->rect.width-3,WHITESMOKE);
			// etk_canvas_vline(thiz->canvas,1,1,thiz->rect.height-3,WHITESMOKE);

			// etk_canvas_hline(thiz->canvas,1,thiz->rect.height-2,thiz->rect.width-3,DIMGRAY);
			// etk_canvas_vline(thiz->canvas,thiz->rect.width-2,1,thiz->rect.height-3,DIMGRAY);
		}
	}
	

	etk_canvas_paint(thiz->display,thiz->canvas,&thiz->rect);
	etk_window_on_paint_share_canvas_child(thiz);
}
Ret etk_window_on_child_event(EtkWidget* thiz, EtkEvent* event){
	e32 x=event->u.mouse.x;
	e32 y=event->u.mouse.y;
	EtkWidget* w=thiz->chil;
	EtkWidget *r=NULL;
	EtkStack s;
	EtkRect cursor;
	struct STACK_NODE(EtkStack) *p=NULL;
	STACK_INIT(&s);
	w=thiz->chil;
	cursor.x=x;
	cursor.y=y;
	cursor.width=2;
	cursor.height=2;
	//printf("-(%d,%d)-\n",x,y);
	while(true){
		//visit w
		if(w==NULL){
			break;
		}//(w->type&ETK_WIDGET_WINDOW!=0)||
		if(w->visable==1&&is_rect_cross(w->rect,cursor)){
			r=w;
			
			/*if((w->type&ETK_WIDGET_WINDOW)!=0){
				etk_wnd_manager_set_grab_widget(etk_default_wnd_manager(),w);
				event->type=ETK_EVENT_ACTIVE;
				etk_wnd_manager_dispatch_event(etk_default_wnd_manager(),event);
			}*/
			//draw_rect(w,BLUE);
		}
		if(w->next!=NULL){
			STACK_NEW_NODE(EtkStack,p,w->next);
			STACK_PUSH(&s,p);
		}
		w=w->chil;
		if(w==NULL){
			if(STACK_IS_EMPTY(&s)){
				break;
			}else{

				STACK_POP(&s,p);
				w=(EtkWidget*)p->data;
				ETK_FREE(p);
			}
		}
	}
	if(r!=NULL){
		event->widget=r;
		r->event(r,event);
		etk_wnd_manager_dispatch_event(etk_default_wnd_manager(),event);
	}
}

Ret etk_window_on_event(EtkWidget* thiz, EtkEvent* event){
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	Ret ret;
	EtkEvent e;
	if(event==NULL)
		return RET_FAIL;
	if(priv->modal!=NULL){
		EtkRect r;
		r.width=1;
		r.height=1;
		r.x=event->u.mouse.x-thiz->rect.x;
		r.y=event->u.mouse.y-thiz->rect.y;
		r=etk_rect_intersect(priv->modal->rect,r);
		if(r.width>0){
			
			event->widget=priv->modal;
			etk_wnd_manager_default_dispatch_event(etk_get_wnd_manager(),event);
			etk_widget_event(priv->modal,event);
			
			
		}else{
			//beep(1);
			etk_window_on_paint_head_with_color(priv->modal,BLACK,WINDOW_HEAD_ACTIVE_COLOR);
			etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&priv->modal->rect);
			
			// etk_window_on_paint_head_with_color(priv->modal,WHITE,BLUE);
			// etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&priv->modal->rect);
			// etk_window_on_paint_head_with_color(priv->modal,BLACK,WHITE);
			// etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&priv->modal->rect);
			// etk_window_on_paint_head_with_color(priv->modal,WHITE,BLUE);
			// etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&priv->modal->rect);
			/*etk_window_on_paint_head_with_color(priv->modal,BLACK,WHITE);
			etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&priv->modal->rect);
			etk_window_on_paint_head_with_color(priv->modal,WHITE,BLUE);
			etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&priv->modal->rect);*/
			etk_event_init(&e,ETK_EVENT_ACTIVE);
			e.widget=priv->modal;
			etk_wnd_manager_default_dispatch_event(etk_get_wnd_manager(),&e);

		}

		return RET_OK;
	}
	//dbg_printf("==etk_window_on_event\n");
	switch(event->type){
	case ETK_EVENT_UPDATE:
		etk_widget_paint_all(thiz);
		//etk_window_update(thiz,&event->u.rect);
		break;
	case ETK_EVENT_MOVE_RESIZE:
		//dbg_printf("ETK_EVENT_MOVE_RESIZE\n");
		if(etk_widget_is_visible(thiz)){
			etk_wnd_manager_update(etk_default_wnd_manager());
		}
		break;
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
	case ETK_EVENT_MOUSE_UP:
	case ETK_EVENT_MOUSE_MOVE:
	case ETK_EVENT_MOUSE_LONG_PRESS:{
			ret = etk_window_on_mouse_event(thiz,event);
			break;
		}
	case ETK_EVENT_KEY_DOWN:
	case ETK_EVENT_KEY_UP:
	case ETK_EVENT_KEY_LONG_PRESS:{
			ret = etk_window_on_key_event(thiz,event);
			break;
		}
	default:
		break;
	}
	return etk_window_on_child_event(thiz,event);
}

Ret etk_window_on_key_event(EtkWidget* thiz, EtkEvent* event){
	//dbg_printf("--etk_window_on_key_event\n");
	//printf("key code:%x %d\n",event->u.key.code,event->u.key.code==ETK_KEY_SPACE);
}
Ret etk_window_on_mouse_event(EtkWidget* thiz, EtkEvent* event){
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	
	if(event->type==ETK_EVENT_MOUSE_UP){
		EtkRect r;
		EtkWidget *w;
		if(priv->has_head==1){
			r.width=1;
			r.height=1;
			r.x=event->u.mouse.x-thiz->rect.x;
			r.y=event->u.mouse.y-thiz->rect.y;
			r=etk_rect_intersect(priv->close_rect,r);
			if(r.width>0){	//check to close window on rect
				EtkEvent e;
				//printf("--etk_window_on_mouse_event\n");
				etk_event_init(&e, ETK_EVENT_WND_DESTROY);
				e.widget = thiz;
				etk_wnd_manager_dispatch_event(etk_default_wnd_manager(),&e);

			}
		}
		r.width=2;
		r.height=2;
		r.x=event->u.mouse.x-thiz->rect.x;
		r.y=event->u.mouse.y-thiz->rect.y;
	
		w=(EtkWidget*)priv->inner_child;
		while(w!=NULL){
			if(w->visable==1){
				if(is_rect_cross(w->rect,r)){
					EtkEvent e;
					e.u.mouse.x=event->u.mouse.x-thiz->rect.x;
					e.u.mouse.y=event->u.mouse.y-thiz->rect.y;
					etk_event_init(&e,ETK_EVENT_MOUSE_UP);
					e.widget=w;
					etk_widget_event(w,&e);
				}
			}
			w=w->next;
		}
		
	}else if(event->type==ETK_EVENT_MOUSE_DOWN){
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
					etk_event_init(&e,ETK_EVENT_MOUSE_DOWN);
					e.widget=w;
					e.u.mouse.x=event->u.mouse.x-thiz->rect.x;
					e.u.mouse.y=event->u.mouse.y-thiz->rect.y;
					etk_widget_event(w,&e);
				}
			}
			w=w->next;
		}
	}
}
EtkRect etk_window_get_head_rect(EtkWidget* thiz){
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	return priv->head_rect;
}
EtkRect etk_window_get_close_rect(EtkWidget* thiz){
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	return priv->close_rect;
}

Ret etk_window_update(EtkWidget* thiz, EtkRect* rect){

	//thiz->display->update(thiz->display,thiz->canvas->bitmap,rect,thiz->canvas->region->rect.x,thiz->canvas->region->rect.y);
}

void etk_window_append_child(EtkWidget* parent, EtkWidget* child){
	if(child->type&ETK_WIDGET_SHARE_PARENT_CAVANS){
		EtkWindowClass* priv=(EtkWindowClass*)parent->subclass;
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
void etk_window_set_all_visible(EtkWidget *thiz,e32 visible){
	EtkWindowClass* priv;
	if(thiz==NULL)
		return;
	thiz->visable=visible;
	if(etk_is_widget(thiz->type,ETK_WIDGET_WINDOW)|| etk_is_widget(thiz->type,ETK_WIDGET_DESKTOP)){
		EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
		EtkWidget *child=(EtkWidget*)priv->inner_child;
		etk_window_set_all_visible(child,visible);
	}
	etk_window_set_all_visible(thiz->chil,visible);
	etk_window_set_all_visible(thiz->next,visible);
}
void etk_window_set_visible(EtkWidget *thiz,e32 visible){
	EtkWidget *child=NULL;
	if(thiz==NULL)
		return;
	etk_widget_show(thiz,visible);
	if(etk_is_widget(thiz->type,ETK_WIDGET_WINDOW)||etk_is_widget(thiz->type,ETK_WIDGET_DESKTOP) ){
		EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
		EtkWidget *child=(EtkWidget*)priv->inner_child;
		etk_window_set_visible(child,visible);
	}
	child=thiz->chil;
	while(child!=NULL){
		etk_window_set_visible(child,visible);
		child=child->next;
	}
}

void etk_window_show_all(EtkWidget* thiz){
	if(thiz==NULL)
		return;
	
	etk_widget_show(thiz,1);
	if(etk_is_widget(thiz->type,ETK_WIDGET_WINDOW)!=0||etk_is_widget(thiz->type,ETK_WIDGET_DESKTOP)!=0){
		EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
		EtkWidget *child=(EtkWidget*)priv->inner_child;
		etk_widget_show_all(child);
	}
	etk_window_show_all(thiz->chil);
	etk_window_show_all(thiz->next);
}
void etk_window_show(EtkWidget *thiz){
	EtkWidget *child=NULL;
	if(thiz==NULL)
		return;
	etk_widget_show(thiz,1);
	if(etk_is_widget(thiz->type,ETK_WIDGET_WINDOW)||etk_is_widget(thiz->type,ETK_WIDGET_DESKTOP)){
		EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
		EtkWidget *child=(EtkWidget*)priv->inner_child;
		etk_widget_show_all(child);
	}
	child=thiz->chil;
	while(child!=NULL){
		etk_widget_show_all(child);
		child=child->next;
	}
	
}

void etk_window_hide_all(EtkWidget* thiz){
	if(thiz==NULL)
		return;
	
	etk_widget_show(thiz,0);
	if(etk_is_widget(thiz->type,ETK_WIDGET_WINDOW)!=0||etk_is_widget(thiz->type,ETK_WIDGET_DESKTOP)!=0){
		EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
		EtkWidget *child=(EtkWidget*)priv->inner_child;
		etk_window_hide_all(child);
	}
	etk_window_hide_all(thiz->chil);
	etk_window_hide_all(thiz->next);
}

void etk_window_destroy(EtkWidget *thiz){
	EtkWindowClass *subclass;
	return_if_fail(thiz!=NULL&&thiz->subclass!=NULL);
	subclass=(EtkWindowClass *)thiz->subclass;
	subclass->modal=NULL;
	ETK_FREE(thiz->subclass);
	ETK_FREE(thiz);

}