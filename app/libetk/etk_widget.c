#include "etk_widget.h"
#include "etk_platform.h"
#include "etk_type.h"
#include "etk_canvas.h"
#include "etk_global.h"
#include "etk_wnd_manager.h"


struct EtkWidgetInfo_t{
	char* text;
	//e32 type;
	//e32 isvisble;
};

void etk_widget_init(EtkWidget *thiz,e32 x,e32 y,e32 width,e32 height,e32 type,e32 attr){
	EtkColor color;
	thiz->info=(EtkWidgetInfo*)ETK_MALLOC(sizeof(EtkWidgetInfo));
	if(thiz->info!=NULL){
		EtkWidgetInfo* info=(EtkWidgetInfo*)thiz->info;
		info->text=NULL;
		
		thiz->visable=0;
		thiz->type=type;
		thiz->next=NULL;
		thiz->prev=NULL;
		thiz->parent=NULL;
		thiz->chil=NULL;
		thiz->visable=0;
		thiz->rect.x=x;
		thiz->rect.y=y;
		thiz->rect.width=width;
		thiz->rect.height=height;
		thiz->rect.next=NULL;
		
		thiz->display=etkglobal.display;
		if(thiz->type&ETK_WIDGET_DESKTOP){
			etk_color_init(&color,WHITE,thiz->display->format);
		}else{
			etk_color_init(&color,WHITE,thiz->display->format);
		
		}
		
		thiz->event=etk_widget_event;

		//must be update downbelod code
		//printf("type :%x type&ETK_WIDGET_DESKTOP:%x\n",type,type&ETK_WIDGET_DESKTOP);
		if(type&ETK_WIDGET_WINDOW){
			if(type&ETK_WIDGET_CHILD){
				thiz->canvas=etk_canvas_create(width,height,color);
				thiz->canvas->paint=etk_canvas_paint;
				thiz->paint=etk_widget_paint;
			}else{
				thiz->canvas=etk_canvas_create(width,height,color);
				thiz->canvas->paint=etk_canvas_paint;
				thiz->paint=etk_widget_paint;
				etk_wnd_manager_add(etkglobal.wndManager ,thiz);
			}
		}else if(type&ETK_WIDGET_DESKTOP){
			if(type&ETK_WIDGET_NO_BITMAP){
				thiz->canvas=etk_canvas_create_with_no_bitmap(width,height,color);
				//thiz->canvas->bitmap=NULL;
			}else{
				thiz->canvas=etk_canvas_create(width,height,color);
			}
			thiz->canvas->paint=etk_canvas_paint;
			thiz->paint=etk_widget_paint;
			etk_wnd_manager_add(etkglobal.wndManager ,thiz);
			etk_default_wnd_manager_set_desktop(etkglobal.wndManager,thiz);
		}else if(type&ETK_WIDGET_CHILD){
			if(type&ETK_WIDGET_SHARE_CAVANS||type&ETK_WIDGET_SHARE_PARENT_CAVANS){
				//thiz->canvas=thiz->parent->canvas;
				thiz->canvas=NULL;
			}else if(type&ETK_WIDGET_OWN_CAVANS){
				if(type&ETK_WIDGET_NO_BITMAP){
					thiz->canvas=etk_canvas_create_with_no_bitmap(width,height,color);
					thiz->canvas->paint=etk_canvas_paint;
					thiz->paint=etk_widget_paint;
				}else{
					thiz->canvas=etk_canvas_create(width,height,color);
					thiz->canvas->paint=etk_canvas_paint;
					thiz->paint=etk_widget_paint;
				}
				thiz->canvas->paint=etk_canvas_paint;
				thiz->paint=etk_widget_paint;
			}else if(type&ETK_WIDGET_NO_CAVANS){
				thiz->canvas=NULL;
			}else{
				thiz->canvas=etk_canvas_create(width,height,color);
				thiz->canvas->paint=etk_canvas_paint;
			thiz->paint=etk_widget_paint;
			}
			
		}else if(type&ETK_WIDGET_BUTTON){
			if(type&ETK_WIDGET_NO_BITMAP){
				thiz->canvas=etk_canvas_create_with_no_bitmap(width,height,color);
				thiz->canvas->paint=etk_canvas_paint;
			}else{
				thiz->canvas=etk_canvas_create(width,height,color);
				thiz->canvas->paint=etk_canvas_paint;
			}
		}else if(type&ETK_WIDGET_MENU){
			thiz->canvas=etk_canvas_create(width,height,color);
			thiz->canvas->paint=etk_canvas_paint;
		}else if(type&ETK_WIDGET_MENU_ITEM){
			thiz->canvas=etk_canvas_create(width,height,color);
			thiz->canvas->paint=etk_canvas_paint;
		}else if(type&ETK_WIDGET_ENTRY){
			/*thiz->canvas=etk_canvas_create(width,height,color);
			thiz->canvas->paint=etk_canvas_paint;
			thiz->paint=etk_widget_paint;*/

			if(type&ETK_WIDGET_SHARE_CAVANS||type&ETK_WIDGET_SHARE_PARENT_CAVANS){
				//thiz->canvas=thiz->parent->canvas;
				thiz->canvas=NULL;
			}else if(type&ETK_WIDGET_OWN_CAVANS){
				if(type&ETK_WIDGET_NO_BITMAP){
					thiz->canvas=etk_canvas_create_with_no_bitmap(width,height,color);
					thiz->canvas->paint=etk_canvas_paint;
					thiz->paint=etk_widget_paint;
				}else{
					thiz->canvas=etk_canvas_create(width,height,color);
					thiz->canvas->paint=etk_canvas_paint;
					thiz->paint=etk_widget_paint;
				}
				thiz->canvas->paint=etk_canvas_paint;
				thiz->paint=etk_widget_paint;
			}else if(type&ETK_WIDGET_NO_CAVANS){
				thiz->canvas=NULL;
			}else{
				thiz->canvas=etk_canvas_create(width,height,color);
				thiz->canvas->paint=etk_canvas_paint;
				thiz->paint=etk_widget_paint;
			}
		}

	}
}
const char * etk_widget_get_text(EtkWidget* thiz){
	EtkWidgetInfo *info=(EtkWidgetInfo*)thiz->info;
	return info->text;
}
Ret etk_widget_set_text(EtkWidget* thiz,const char* text){
	EtkWidgetInfo *info=(EtkWidgetInfo*)thiz->info;
	if(info->text==NULL){
		info->text=ETK_EX_MALLOC(strlen(text)+50);
		memset(info->text,0,strlen(text)+50);
		strcpy((e8*)info->text,text);
	}else{
		strcpy((e8*)info->text,text);
		/*e32 l1=strlen(info->text);
		e32 l2=strlen(text);
		if(l2>l1){
			info->text=(e8*)ETK_REALLOC( ((void*)info->text),(l2));
		}
		strncpy((e8*)info->text,text,l2);*/
		
	}
	return RET_OK;
}
Ret etk_widget_paint_self(EtkWidget* thiz){
	thiz->paint(thiz);
	return RET_OK;
}
Ret etk_widget_paint_child(EtkWidget* thiz){
	EtkWidget *p=thiz->chil;
	while(p!=NULL){
		p->paint(p);
		etk_widget_paint_child(p);
		p=p->next;
	}
	return RET_OK;
}
Ret etk_widget_paint_all(EtkWidget* thiz){
	thiz->paint(thiz->chil);
	thiz->paint(thiz->next);
	thiz->paint(thiz);

	return RET_OK;
}
Ret etk_widget_paint(EtkWidget* thiz){
	//if(thiz->visable==1){
		thiz->paint(thiz);
	//}
	return RET_OK;
}
Ret etk_widget_update_rect(EtkWidget* thiz,EtkRect *rect){
	if(thiz->type&ETK_WIDGET_SHARE_PARENT_CAVANS){			
		EtkRect r=thiz->rect;
		r.x+=thiz->parent->rect.x;
		r.y+=thiz->parent->rect.y;
		//fill_rect_plus(r,rand()%0xffffff);
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&r);
		
	}else{
		EtkRect inter=etk_rect_intersect(thiz->rect,*rect);
		return etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&inter);
	}
	
}
Ret etk_widget_update(EtkWidget* thiz){
	return etk_default_wnd_manager_update_widget(etk_default_wnd_manager(),thiz);
}

Ret etk_widget_event(EtkWidget* thiz, EtkEvent* event){
	return_val_if_fail(thiz!=NULL&&event!=NULL,RET_FAIL);
	return thiz->event(thiz,event);
}
e32 etk_widget_is_visible(EtkWidget* thiz){
	return thiz->visable;
}

void etk_widget_show(EtkWidget* thiz, int visible){
	EtkEvent event;
	if(thiz->visable ==1){
		if(visible==1){
			return;
		}else if(visible==0){
			thiz->visable = visible;
			etk_event_init(&event,ETK_EVENT_HIDE);
			event.widget=thiz;
			etk_widget_event(thiz, &event);
			return;
		}
		
	}else if(thiz->visable==0){
		thiz->visable = visible;
		event.widget=thiz;
		//printf("=etk_widget_show%x\n",thiz);
		etk_event_init(&event, visible ? ETK_EVENT_SHOW : ETK_EVENT_HIDE);
		etk_widget_event(thiz, &event);
	}else{//for menu item visable =3 or other.
		//printf("v3\n");
		etk_widget_paint(thiz);
	}

}

void etk_widget_show_all(EtkWidget* thiz){
	if(thiz==NULL)
		return;
	etk_widget_show(thiz,1);
	etk_widget_show_all(thiz->chil);
	etk_widget_show_all(thiz->next);
}

void etk_widget_append_child(EtkWidget* parent, EtkWidget* child){
	if(parent->chil==NULL){
		parent->chil=child;
		child->parent=parent;
		child->next=NULL;
		child->prev=NULL;
		if(child->type&ETK_WIDGET_SHARE_PARENT_CAVANS){
			child->canvas=parent->canvas;
		}
	}else{
		etk_widget_append(parent->chil,child);
		child->parent=parent;
	}
}

void etk_widget_append_next(EtkWidget* parent, EtkWidget* next){
	if(parent->next==NULL){
		parent->next=next;
		next->next=NULL;
		next->prev=parent;
		//next->parent=parent;
		//printf("appen parent->next\n");
	}else{
		//printf("etk_widget_append\n");
		etk_widget_append(parent,next);
	}
}

void etk_widget_add_child(EtkWidget* thiz, EtkWidget* child){
	etk_widget_append_child(thiz,child);
}
void etk_widget_remove_child(EtkWidget* thiz,EtkWidget* child){
	if(thiz->chil==child){//head
		thiz->chil=child->next;
		if(child->next!=NULL){
			child->next->prev=NULL;
		}
	}else{
		child->prev->next=child->next;
		if(child->next!=NULL){
			child->next->prev=child->prev;
		}else{
			
		}
	}
	child->next=NULL;
	child->prev=NULL;
	child->parent=NULL;
}
void etk_widget_append(EtkWidget* thiz, EtkWidget* widget){
	EtkWidget* iter = thiz;
	for(; iter->next != NULL; iter = iter->next);
	iter->next = widget;
	widget->prev = iter;
	
	return;
}
