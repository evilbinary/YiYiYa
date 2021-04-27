#include "light.h"
#include "etk_window.h"
#include "etk_widget.h"
#include "etk_platform.h"
#include "etk_rect.h"
#include "etk_source.h"
#include "etk_event.h"
#include "etk_canvas_ex.h"
#define LIGHT_MAX_NR	11

typedef struct light_t{
	EtkWidget *buttons[15];
	int i;
}Light;

typedef struct PrivInfo_t{
	EtkListener listener;
	void* listener_ctx;
	e32 data;
}PrivInfo;


static Ret etk_light_event(EtkWidget* thiz, EtkEvent* event){
	EtkWindowClass *priv = NULL;
	//printf("%d\n", (int)event->type);
	switch(event->type)
	{
	case ETK_EVENT_WND_DESTROY:
		//printf("destroy!\n");
		priv=(EtkWindowClass*)thiz->subclass;
		
		break;
	default:
		break;
	}
	
	return etk_window_on_event(thiz, event);
}

static Ret etk_light_draw(EtkWidget *thiz){
	int i,j;
	EtkRect r;
	char buf[10];
	EtkWindowClass *priv=(EtkWindowClass *)thiz->subclass;
 
	
}
static Ret etk_light_paint(EtkWidget *thiz){
	//etk_light_draw(thiz);
	return etk_window_on_paint(thiz);
}

Ret etk_light_update_listener(void *data){
	 
	return RET_OK;
}
Ret etk_light_button_listener(void* user_data, void* obj){
	EtkWidget *thiz=obj;
	EtkWindowClass*	priv=(EtkWindowClass*)thiz->parent->subclass;
	Light *light=((Light*)priv->data[1]);
	if(light->buttons[LIGHT_MAX_NR]==thiz){
		e32 i;
		for(i=0;i<=LIGHT_MAX_NR;i++){
			EtkWidget *button=light->buttons[i];
			etk_button_set_data(button,etk_button_get_data(light->buttons[LIGHT_MAX_NR])^1);
			etk_light_button_up_paint(button);
		}
		//printf("set \n");
	}else{
		//printf("addr:%x\n",thiz);
		//printf("addr[0] :%x\n",light->buttons[0]);
		etk_button_set_data(thiz,etk_button_get_data(thiz)^1);
	}
	
	
	return RET_OK;
}
Ret etk_light_button_down_paint(EtkWidget* thiz){
	const char *text=etk_widget_get_text(thiz);
	EtkCanvas *canvas;
	EtkRect r=thiz->rect;
	if(thiz->type&ETK_WIDGET_SHARE_PARENT_CAVANS){
		 etk_canvas_fill_rect(thiz->canvas,r.x,r.y,r.width,r.height,0x4cbdf0);
		if(text!=NULL){
			etk_canvas_draw_string_with_color(thiz->canvas,r.x+(thiz->rect.width>>2),r.y+(thiz->rect.height>>2)-4,text,thiz->canvas->gc.fgcolor.color,0x4cbdf0);
		}
		etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&r);
	}else{
		r.x=r.y=0;
		etk_canvas_fill_rect(thiz->canvas,r.x,r.y,r.width,r.height,RED);
		if(text!=NULL){
			etk_canvas_draw_string_with_color(thiz->canvas,r.x+(thiz->rect.width>>2),r.y+(thiz->rect.height>>2)-4,text,thiz->canvas->gc.fgcolor.color,RED);
		}
		etk_default_wnd_manager_update_widget(etk_default_wnd_manager(),thiz);
	}
	
	if(etk_button_get_data(thiz)==1){
		etk_canvas_fill_circle(thiz->canvas,thiz->rect.x+20,thiz->rect.y+34,5,GREEN);
	}else{
		etk_canvas_fill_circle(thiz->canvas,thiz->rect.x+20,thiz->rect.y+34,5,RED);
	}
	return RET_OK;
}
Ret etk_light_button_up_paint(EtkWidget* thiz){
	const char *text=etk_widget_get_text(thiz);
	EtkRect r=thiz->rect;
	EtkRect r0=thiz->rect;
	if(thiz->type&ETK_WIDGET_SHARE_PARENT_CAVANS){
		r0.x+=(thiz->parent->rect.x);
		r0.y+=(thiz->parent->rect.y);
		r0.width+=1;
	}else{
		r.x=r.y=0;
	}
	etk_canvas_fill_rect(thiz->canvas,r.x,r.y,r.width,r.height,0xc9e7f4);//thiz->canvas->gc.bgcolor.color
	if(text!=NULL){
		etk_canvas_draw_string_with_color(thiz->canvas,r.x+(thiz->rect.width>>2),r.y+(thiz->rect.height>>2)-5,text,thiz->canvas->gc.fgcolor.color,0xc9e7f4);
	}
	if(etk_button_get_data(thiz)==1){
		etk_canvas_fill_circle(thiz->canvas,thiz->rect.x+20,thiz->rect.y+34,5,GREEN);
	}else{
		etk_canvas_fill_circle(thiz->canvas,thiz->rect.x+20,thiz->rect.y+34,5,RED);
	}

 
	//draw_rect(r,RED);
	etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&r0);
	//etk_default_wnd_manager_update_widget(etk_default_wnd_manager(),thiz);
	return RET_OK;
}

Ret etk_light_button_on_paint(EtkWidget* thiz){
	thiz->visable=1;
	etk_light_button_up_paint(thiz);
	if(etk_button_get_data(thiz)==1){
			etk_canvas_fill_circle(thiz->canvas,thiz->rect.x+20,thiz->rect.y+34,5,GREEN);
	}else{
		etk_canvas_fill_circle(thiz->canvas,thiz->rect.x+20,thiz->rect.y+34,5,RED);
	}
}

Ret etk_light_button_on_event(EtkWidget* thiz,EtkEvent* event){
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
		etk_light_button_down_paint(thiz);
	
		break;
	}
	case ETK_EVENT_MOUSE_UP:
		//printf("ETK_EVENT_MOUSE_UPa\n");
		
		if(priv->listener_ctx!=NULL)
			priv->listener(priv->listener_ctx,thiz);
		etk_light_button_up_paint(thiz);
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


EtkWidget *etk_light_create(e32 x,e32 y,e32 width,e32 height){
	EtkWindowClass *priv;
	Light *light;
	e32 xsize,ysize,xspan,yspan,startx,starty,wx,wy;
	e32 i;
	e8 button_name[LIGHT_MAX_NR+1][10]={"走廊","餐厅","客厅","厨房","浴室","主卧","次卧","客卧","书房","阳台","夜灯","ONOFF"};

	EtkWidget *thiz=etk_create_window(x,y,width,height,ETK_WIDGET_WINDOW);
	thiz->event=etk_light_event;
	thiz->paint=etk_light_paint;
	priv=(EtkWindowClass*)thiz->subclass;
	priv->data[1]=ETK_MALLOC(sizeof(Light));
	light=((Light*)priv->data[1]);
	startx=10;starty=30;xsize=52;ysize=42;xspan=3;yspan=5;
	wx=startx;wy=starty;
	for(i=0;i<=LIGHT_MAX_NR;i++){
		light->buttons[i]=etk_button_create_ex(wx,wy,xsize,ysize,ETK_WIDGET_CHILD|ETK_WIDGET_SHARE_PARENT_CAVANS); 
		light->buttons[i]->visable=1;
		etk_window_append_child(thiz,light->buttons[i]);
		light->buttons[i]->parent=thiz;
		etk_widget_set_text(light->buttons[i],button_name[i]);
		etk_button_set_data(light->buttons[i],0);
		light->buttons[i]->event=etk_light_button_on_event;
		light->buttons[i]->paint=etk_light_button_on_paint;
		etk_button_set_clicked_listener(light->buttons[i],etk_light_button_listener,light->buttons[i]);
		wx=wx+xsize+xspan;
			//printf("addr[%d] :%x\n",i,light->buttons[i]);
		if((i+1)%4==0){
			wy=wy+ysize+yspan;
			wx=startx;
		}
	}
	etk_widget_set_text(thiz,"灯光控制");

	return thiz;
}

