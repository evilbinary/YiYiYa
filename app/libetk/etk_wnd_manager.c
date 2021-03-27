#include "etk_wnd_manager.h"
#include "etk_global.h"
#include "etk_type.h"
#include "etk_platform.h"
#include "etk_rect.h"
#include "etk_main_loop.h"
#include "etk_widget.h"
#include "etk_stack.h"
#include "queue.h"
#include "etk_window.h"

SLIST_HEAD(EtkLink_t,EtkLink);
STACK_TEMPLATE(EtkStack,void*);


typedef struct EtkWndManagerPriv_t
{
	int top;
	int pressed;
	int caplock;
	int shift_down;
	int dieing;
	EtkEvent pressed_event;
	EtkEvent last_event;
	EtkMainLoop* main_loop;
	EtkWidget* grab_widget;
	EtkWidget* focus_widget;
	EtkWidget* active_widget;
	EtkWidget* desktop;
	struct EtkLink_t link;
	//EtkSource* long_press_timer;
	//void* global_listeners_ctx[Etk_MAX_GLOBAL_LISTENER];
	//EtkListener global_listeners[Etk_MAX_GLOBAL_LISTENER];
	int disable_anim;
}EtkWndManagerPriv;

//void etk_wnd_manger_init(EtkWndManager* thiz){
//	thiz->add=etk_default_wnd_manager_add;
//	thiz->remove=etk_default_wnd_manager_remove;
//
//}
Ret etk_wnd_manager_add(EtkWndManager* thiz, EtkWidget* window){
	return thiz->add(thiz,window);
}
Ret etk_wnd_manager_remove(EtkWndManager *thiz,EtkWidget* window){
	return thiz->remove(thiz,window);
}
Ret etk_wnd_manager_update(EtkWndManager* thiz){
	return thiz->update(thiz);
}
void etk_wnd_manager_destroy(EtkWndManager* thiz){
	thiz->destroy(thiz);
}
Ret etk_wnd_manager_dispatch_event(EtkWndManager* thiz, EtkEvent* event){
	return thiz->dispatch_event(thiz,event);
}
Ret etk_wnd_manager_queue_event(EtkWndManager* thiz, EtkEvent* event){
	return thiz->queue_event(thiz,event);
}

EtkWidget* etk_wnd_manager_get_select_widget(EtkWndManager* thiz,e32 x,e32 y){
	EtkWidget *p=thiz->topwin;
	EtkRect cursor;
	EtkWidget *t=NULL;
	cursor.x=x;
	cursor.y=y;
	cursor.width=2;
	cursor.height=2;
	/*while(p!=NULL){
		t=p;
		p=p->next;
	}
	p=t;*/
	while(p!=NULL){
		if(p->visable==1&&is_rect_cross(p->rect,cursor)){
			return p;
		}
		p=p->prev;
	}
	return NULL;
}


//only for the active window who has a child 
EtkWidget* etk_wnd_manager_get_select_child(EtkWidget *parent,e32 x,e32 y){
	EtkWidget *p=parent->chil;
	EtkRect cursor;
	EtkWidget *t=NULL;
	cursor.x=x;
	cursor.y=y;
	cursor.width=2;
	cursor.height=2;
	while(p!=NULL){
		t=p;
		p=p->next;
	}
	p=t;
	while(p!=NULL){
		if(is_rect_cross(p->rect,cursor)){
			return p;
		}
		p=p->prev;
	}
	return NULL;
}
EtkWidget* etk_wnd_manager_get_select_next(EtkWidget *parent,e32 x,e32 y){
	EtkWidget *p=parent->next;
	EtkRect cursor;
	EtkWidget *t=NULL;
	cursor.x=x;
	cursor.y=y;
	cursor.width=2;
	cursor.height=2;
	/*while(p!=NULL){
		t=p;
		p=p->next;
	}
	p=t;*/
	while(p!=NULL){
		if(is_rect_cross(p->rect,cursor)){
			return p;
		}
		p=p->prev;
	}
	return NULL;
}
Ret etk_wnd_manager_set_focus_widget(EtkWndManager* thiz,EtkWidget* focus){
	EtkWndManagerPriv *priv=(EtkWndManagerPriv *)thiz->priv;
	priv->focus_widget=focus;
	return RET_OK;
}
EtkWidget * etk_wnd_manager_get_focus_widget(EtkWndManager* thiz){
	EtkWndManagerPriv *priv=(EtkWndManagerPriv *)thiz->priv;
	return priv->focus_widget;
}

Ret etk_wnd_manager_set_grab_widget(EtkWndManager* thiz,EtkWidget* grab){
	EtkWndManagerPriv *priv=(EtkWndManagerPriv *)thiz->priv;
	priv->grab_widget=grab;
	return RET_OK;
}
EtkWidget * etk_wnd_manager_get_grab_widget(EtkWndManager* thiz){
	EtkWndManagerPriv *priv=(EtkWndManagerPriv *)thiz->priv;
	return priv->grab_widget;
}
Ret etk_wnd_manager_set_active_widget(EtkWndManager* thiz,EtkWidget* active){
	EtkWndManagerPriv *priv=(EtkWndManagerPriv *)thiz->priv;
	EtkColor color;
	e32 j;
	EtkBitmap *h;
	if(active==NULL||priv->active_widget==active)
		return RET_OK;
	if(priv->active_widget!=NULL&&(priv->active_widget->type&ETK_WIDGET_WINDOW)){
		if(etk_window_has_head(priv->active_widget)==1){
			EtkRect t=etk_window_get_head_rect(priv->active_widget);
			etk_window_on_paint_head(priv->active_widget);
			//if(RET_FAIL==etk_default_wnd_manager_update_widget_rect(etk_default_wnd_manager(),priv->active_widget,&t)){
				etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&priv->active_widget->rect);
			//}
		}
	}

	priv->active_widget=active;
	
	if(active->type&ETK_WIDGET_WINDOW){
		if(etk_window_has_head(active)==1){
			etk_window_on_paint_head_with_color(priv->active_widget,WHITE,BLUE);
			//etk_default_wnd_manager_update_widget_rect(etk_default_wnd_manager(),priv->active_widget,&t);
		}
	}
	return RET_OK;
}

EtkWidget * etk_wnd_manager_get_active_widget(EtkWndManager* thiz){
	EtkWndManagerPriv *priv=(EtkWndManagerPriv *)thiz->priv;

	return priv->active_widget;
}

__inline Ret etk_wnd_manager_default_queue_event(EtkWndManager* thiz, EtkEvent* event){
	//update event time for timer
	if(event->time == 0){
		event->time = etk_get_relative_time();
	}
	//dbg_printf("xx etk_wnd_manager_default_queue_event\n");
	return etk_event_queue(event);
}


//default function
Ret etk_default_wnd_manager_add(EtkWndManager* thiz, EtkWidget* window){
	EtkWidget *p=thiz->windows;
	if(p==NULL){
		thiz->windows=window;
		thiz->topwin=window;
	}else{
		thiz->topwin->next=window;
		window->prev=thiz->topwin;
		thiz->topwin=window;
	}
	thiz->winnum++;
	return RET_OK;
}
Ret etk_default_wnd_manager_remove(EtkWndManager* thiz, EtkWidget* window){
	if(window->prev==NULL){//head
		//printf("remove head\n");
		if(window->parent==NULL){
			thiz->windows=window->next;
			if(window->next!=NULL){
				window->next->prev=NULL;
			}
		}/*else{
			window->parent->chil=window->next;
			if(window->next!=NULL){
				window->next->prev=NULL;
			}
		}*/
	}else{	
		window->prev->next=window->next;
		if(window->next!=NULL){
			window->next->prev=window->prev;
		}else{
			if(window->parent==NULL){
				thiz->topwin=window->prev;
			}else{
				//printf("null\n");
			}
		}
	}
	//free window code here
	window->next=NULL;
	window->prev=NULL;
	window->parent=NULL;
	
	return RET_OK;
}
Ret etk_default_wnd_manager_move_to(EtkWndManager* thiz, EtkWidget* window,e32 x,e32 y){
	EtkWidget* p;
	e32 dx,dy;
	p=window->chil;
	dx=x-window->rect.x;
	dy=y-window->rect.y;
	while(p!=NULL){

		//etk_default_wnd_manager_move_to(thiz,p,p->rect.x+dx,p->rect.y+dy);
		p->rect.x+=dx;
		p->rect.y+=dy;
		p=p->next;
	}
	/*p=window->next;
	while(p!=NULL){
		p->rect.x+=dx;
		p->rect.y+=dy;
		p=p->next;
	}*/

	window->rect.x=x;
	window->rect.y=y;
	return RET_OK;
}

EtkWndManager* etk_get_wnd_manager(){
	return etkglobal.wndManager;
}
EtkWndManager* etk_default_wnd_manager(){
	return etkglobal.wndManager;
}
Ret etk_default_wnd_manager_active(EtkWndManager* thiz,EtkWidget* window){
	EtkWndManagerPriv *priv;
	if(window==NULL)
		return RET_NO_TARGET;
	priv=(EtkWndManagerPriv *)thiz->priv;
	if(window->parent!=NULL&&(window->type&ETK_WIDGET_CHILD)){
		EtkWidget* p=window->parent;
		etk_widget_remove_child(p,window);
		etk_widget_append_child(p,window);
		//printf("active child\n");
	}else if((window->type&ETK_WIDGET_WINDOW)&&(window->type&ETK_WIDGET_DESKTOP)==0){
		etk_default_wnd_manager_remove(thiz,window);
		etk_default_wnd_manager_add(thiz,window);
		//printf("active \n");
	}
	etk_wnd_manager_set_active_widget(thiz,window);
	//priv->active_widget=window;
	return RET_OK;
}
void etk_default_wnd_manager_destroy(EtkWndManager* thiz){

}
Ret etk_default_wnd_manager_update(EtkWndManager* thiz){
	//update all screen with code here
	return RET_OK;
}
extern EtkWidget *etk_widget_clip_rect(EtkWidget *widget,EtkRect *rect);

Ret etk_default_wnd_manager_update_rect(EtkWndManager* thiz,EtkRect *rect){
	EtkWidget* w=thiz->windows;
	EtkStack s;
	struct STACK_NODE(EtkStack) *p=NULL;
	w=etk_widget_clip_rect(w,rect);
	STACK_INIT(&s);
	w=thiz->windows;
	while(true){
		//visit w
		if(w==NULL){
			break;
		}
		if(is_rect_cross(w->rect,*rect)){
			etk_default_wnd_manager_update_widget(thiz,w);
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

	return RET_OK;
}
Ret etk_default_wnd_manager_update_widget(EtkWndManager* thiz,EtkWidget *widget){
	EtkRect *r;
	if(widget==NULL){
		return RET_NO_TARGET;
	}else{
		if(widget->canvas->bitmap==NULL){
			return RET_NO_TARGET;
		}
	}
	if(1==(widget->type&ETK_WIDGET_NO_BITMAP)){
		return RET_OK;
	}
	if(widget->visable==1){
		r=widget->canvas->region->outputs;
		while(r!=NULL){
			widget->display->update(widget->display,widget->canvas->bitmap,r,r->x-widget->rect.x,r->y-widget->rect.y);
			r=r->next;
		}
	}
	return RET_OK;
}
Ret etk_default_wnd_manager_update_widget_rect(EtkWndManager* thiz,EtkWidget *widget,EtkRect *rect){
	EtkRect *r;
	if(widget==NULL){
		return RET_NO_TARGET;
	}else{
		if(widget->canvas->bitmap==NULL){
			return RET_NO_TARGET;
		}
	}
	if(1==(widget->type&ETK_WIDGET_NO_BITMAP)){
		return RET_OK;
	}
	if(widget->visable==1){
		EtkRect inter=*rect;
		inter.x+=widget->rect.x;
		inter.y+=widget->rect.y;
		r=widget->canvas->region->outputs;
		if(r==NULL)
			return RET_FAIL;
		while(r!=NULL){
			inter=etk_rect_intersect(*r,inter);
			if(inter.width>0)
				widget->display->update(widget->display,widget->canvas->bitmap,r,r->x-widget->rect.x,r->y-widget->rect.y);
			r=r->next;
		}
	}
	return RET_OK;
}

EtkWidget *etk_default_wnd_manager_get_desktop(EtkWndManager *thiz){
	EtkWndManagerPriv* priv=(EtkWndManagerPriv*)thiz->priv;
	return priv->desktop;
}
Ret etk_default_wnd_manager_set_desktop(EtkWndManager *thiz,EtkWidget *widget){
	EtkWndManagerPriv* priv=(EtkWndManagerPriv*)thiz->priv;
	priv->desktop=widget;
	return RET_OK;
}

EtkWndManager* etk_default_wnd_manager_create(EtkMainLoop* main_loop){
	EtkWndManager* thiz=NULL;
	thiz=(EtkWndManager*)ETK_MALLOC(sizeof(EtkWndManager));
	if(thiz!=NULL){
		EtkWndManagerPriv* priv;
		thiz->priv=ETK_MALLOC(sizeof(EtkWndManagerPriv));
		priv=(EtkWndManagerPriv*)thiz->priv;
		priv->main_loop = main_loop;
		priv->grab_widget=NULL;
		priv->focus_widget=NULL;
		priv->active_widget=NULL;
		priv->pressed=0;
		priv->pressed_event.widget=NULL;
		priv->last_event.widget=NULL;
		thiz->winnum=0;
		thiz->windows=NULL;
		thiz->topwin=NULL;
		thiz->add=etk_default_wnd_manager_add;
		thiz->remove=etk_default_wnd_manager_remove;
		thiz->destroy=etk_default_wnd_manager_destroy;

		thiz->update=etk_default_wnd_manager_update;
		thiz->dispatch_event=etk_wnd_manager_default_dispatch_event;
		thiz->queue_event=etk_wnd_manager_default_queue_event;
	}
	return thiz;
}


__inline Ret etk_wnd_manager_default_dispatch_event(EtkWndManager* thiz, EtkEvent* event){
	EtkWidget* widget=(EtkWidget*)event->widget;
	
	EtkWndManagerPriv *priv=(EtkWndManagerPriv *)thiz->priv;
	if(event==NULL)
		RET_NO_TARGET;
	switch(event->type){
	case ETK_EVENT_WND_DESTROY: 
		widget->visable=0;
		etk_default_wnd_manager_remove(thiz,widget);
		etk_default_wnd_manager_update_rect(thiz,&widget->rect);
		//widget->destroy(widget);
		//must free widget ,code here
		break;				 
	case ETK_EVENT_HIDE: 
		widget->visable=0;
		etk_default_wnd_manager_update_rect(thiz,&widget->rect);
		break;
	case ETK_EVENT_ACTIVE:
		etk_default_wnd_manager_active(thiz,widget);
		etk_default_wnd_manager_update_rect(thiz,&widget->rect);
		break;
	case ETK_EVENT_SHOW: 
		//printf("ETK_EVENT_SHOW\n");
		widget->visable=1;
		//draw_rect(widget->rect,RED);
		etk_default_wnd_manager_update_rect(thiz,&widget->rect);
		//etk_wnd_manager_update(thiz);
		break;
	case ETK_EVENT_MOVE:{
		EtkWidget *widget;
		if(event->widget!=NULL&&event->u.extra!=NULL){
			EtkRect r,lr;EtkPoint* p;
			widget=(EtkWidget*)event->widget;
			//dbg_printf("etk_default_wnd_manager_move_to\n");
			p=(EtkPoint*)event->u.extra;
			lr=widget->rect;
			//printf("mov x,y:%d,%d",p->x,p->y);
			etk_default_wnd_manager_move_to(etk_default_wnd_manager(),widget,p->x,p->y);
			r=etk_rect_min(widget->rect,lr);
			etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&r);
		}
		break;
	}
	case ETK_EVENT_RELAYOUT_WND: 
		//etk_wnd_manager_default_relayout(thiz);

		break;
	case ETK_EVENT_FOCUS_IN:
		etk_wnd_manager_set_focus_widget(etk_default_wnd_manager(),(EtkWidget*)event->widget);
		etk_widget_event((EtkWidget*)event->widget, event);
		break;
	case ETK_EVENT_FOCUS_OUT:
		etk_wnd_manager_set_focus_widget(etk_default_wnd_manager(),NULL);
		etk_widget_event((EtkWidget*)event->widget, event);
		break;
	case ETK_EVENT_KEY_DOWN:
	case ETK_EVENT_KEY_UP:{
		EtkWidget* active=etk_wnd_manager_get_active_widget(thiz);
		if(active!=NULL)
			etk_widget_event(active, event);
		break;
	}
	case ETK_EVENT_MOUSE_DOWN:{
		EtkWidget *select;
		if(event->widget==NULL){
			select=etk_wnd_manager_get_select_widget(etk_default_wnd_manager(),event->u.mouse.x,event->u.mouse.y);
			etk_widget_event(select, event);
		}else{
			select=(EtkWidget*)event->widget;
		}
		if(select!=NULL){
			EtkRect inter,rm;
			event->widget=select;

			if((select->type&ETK_WIDGET_DESKTOP)==0&&select->visable==1){
				etk_wnd_manager_set_grab_widget(etk_default_wnd_manager(),select);
				etk_default_wnd_manager_active(thiz,select);
				etk_default_wnd_manager_update_rect(thiz,&select->rect);
			
				rm.x=event->u.mouse.x-select->rect.x;
				rm.y=event->u.mouse.y-select->rect.y;
				rm.width=2;
				rm.height=2;
				inter=etk_rect_intersect(etk_window_get_head_rect(select),rm);
				if(inter.width>0){
					priv->pressed_event=*event;	
					priv->pressed=3;
				}else{
					priv->pressed_event=*event;	
					priv->pressed=1;
				}
				
			}
			if((select->type&ETK_WIDGET_ENTRY)!=ETK_WIDGET_ENTRY){
				if((select->type&ETK_WIDGET_INPUT_METHOD)==ETK_WIDGET_INPUT_METHOD){
				
				}else{
					event->widget=etk_wnd_manager_get_focus_widget(etk_default_wnd_manager());
					if(event->widget!=NULL){
						event->type=ETK_EVENT_FOCUS_OUT;
						etk_widget_event((EtkWidget*)event->widget, event);
						etk_wnd_manager_set_focus_widget(etk_default_wnd_manager(),NULL);
					}
				}
			}
		}else{
			priv->pressed_event=*event;	
			priv->pressed=1;
		}
		//printf("priv->pressed:%d\n",priv->pressed);	
		
		break;
	}
	case ETK_EVENT_MOUSE_UP:{
		EtkWidget *select;
		// priv->pressed=0;
		if(event->widget==NULL){
			select=etk_wnd_manager_get_select_widget(etk_default_wnd_manager(),event->u.mouse.x,event->u.mouse.y);
			etk_widget_event(select, event);
		}else{
			select=(EtkWidget*)event->widget;
		}
		if(select!=NULL){
			event->widget=select;;
			//etk_wnd_manager_set_grab_widget(etk_default_wnd_manager(),select);
			//etk_widget_event(select, event);
		}
		priv->pressed_event=*event;	
		break;
	}
	case ETK_EVENT_MOUSE_MOVE:{
		//printf("ETK_EVENT_MOUSE_MOVE\n");
		if(priv->pressed==3){
			EtkWidget *w=etk_wnd_manager_get_grab_widget(thiz);
			if(w!=NULL&&((w->type&ETK_WIDGET_WINDOW)||(w->type&ETK_WIDGET_CHILD))){
					int movx,movy;
					EtkRect r,lr;
					EtkEvent e;
					EtkPoint p;
					lr=w->rect;
					movx=event->u.mouse.x-priv->last_event.u.mouse.x+w->rect.x;
					movy=event->u.mouse.y-priv->last_event.u.mouse.y+w->rect.y;
					if(priv->last_event.u.mouse.x<0||priv->last_event.u.mouse.y<0)
						break;
					//printf("movx:%d movy:%d event:%d,%d last_event:%d,%d w:%d,%d\n",movx,movy,event->u.mouse.x,event->u.mouse.y,priv->last_event.u.mouse.x,priv->last_event.u.mouse.y,w->rect.x,w->rect.y);
					p.x=movx;
					p.y=movy;
					event->u.extra=&p;
					/*etk_default_wnd_manager_move_to(etk_default_wnd_manager(),w,movx,movy);
					r=etk_rect_min(w->rect,lr);
					etk_default_wnd_manager_update_rect(thiz,&r);*/	
					etk_event_init(&e,ETK_EVENT_MOVE);
					e.widget=w;
					e.u.extra=&p;
					//etk_widget_event(w, &e);
					etk_wnd_manager_default_dispatch_event(thiz,&e);
				
			}
		
		}

		break;
	}
	default:break;
	}
	priv->last_event=*event;
	return RET_OK;
}
