#include "temhum.h"
#include "etk_window.h"
#include "etk_platform.h"
#include "etk_rect.h"
#include "etk_source.h"

struct temhum_t{
	float tem;
	float hum;
	EtkRect r0;
	EtkRect r1;
	float up;
};

static Ret etk_temhum_event(EtkWidget* thiz, EtkEvent* event){
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

static Ret etk_temhum_draw(EtkWidget *thiz){
	int i,j;
	EtkRect r;
	char buf[10];
	EtkWindowClass *priv=(EtkWindowClass *)thiz->subclass;
	struct temhum_t *temhum=(struct temhum_t*)priv->data[1];
	if(temhum->tem>=-10&&temhum->tem<=110){
		//draw tem
		r=temhum->r0;
		etk_canvas_fill_rect(thiz->canvas,r.x-5,r.y+r.height,15,10,RED);
		etk_canvas_draw_rect(thiz->canvas,r.x-5,r.y+r.height,15,10,BLACK);
		etk_canvas_draw_rect(thiz->canvas,r.x,r.y-5,r.width,r.height+5,BLACK);
		etk_canvas_fill_rect(thiz->canvas,r.x+1,r.height-temhum->tem*2+r.y-20,r.width-2,temhum->tem*2+22,RED);
		
		for(j=-10,i=r.height;i>=0;i--){
			if(i%20==0){
				sprintf(buf,"%d",j);
				j+=10;
				etk_canvas_hline(thiz->canvas,r.x+r.width,r.y+i,20,BLACK);
				etk_canvas_draw_string_with_color(thiz->canvas,r.x+r.width+22,r.y+i-5,buf,BLACK,WHITE);
			}else if(i%10==0){
				etk_canvas_hline(thiz->canvas,r.x+r.width,r.y+i,10,RED);
			}else {
				//etk_canvas_hline(thiz->canvas,r.x+r.width,r.y+i,10,GREEN);
			}
		}
		sprintf(buf,"梁業:%.1f業",temhum->tem);
		etk_canvas_draw_string_with_color(thiz->canvas,100,24,buf,BLACK,WHITE);
	}
	if(temhum->hum<=100){
		
		//draw hum
		r=temhum->r1;
		etk_canvas_fill_rect(thiz->canvas,r.x-5,r.y+r.height,15,10,BLUE);
		etk_canvas_draw_rect(thiz->canvas,r.x-5,r.y+r.height,15,10,BLACK);
		etk_canvas_draw_rect(thiz->canvas,r.x,r.y-5,r.width,r.height+5,BLACK);
		etk_canvas_fill_rect(thiz->canvas,r.x+1,r.height-temhum->hum*2+r.y,r.width-2,temhum->hum*2+2,BLUE);
		
		for(j=0,i=r.height;i>=0;i--){
			if(i%20==0){
				sprintf(buf,"%d%%",j);
				j+=10;
				etk_canvas_hline(thiz->canvas,r.x+r.width,r.y+i,20,BLACK);
				etk_canvas_draw_string_with_color(thiz->canvas,r.x+r.width+22,r.y+i-5,buf,BLACK,WHITE);
			}else if(i%10==0){
				etk_canvas_hline(thiz->canvas,r.x+r.width,r.y+i,10,RED);
			}else {
				//etk_canvas_hline(thiz->canvas,r.x+r.width,r.y+i,10,GREEN);
			}
		}
		sprintf(buf,"物業:%.1f%%",temhum->hum);
		etk_canvas_draw_string_with_color(thiz->canvas,100,44,buf,BLACK,WHITE);
	}	
}
static Ret etk_temhum_paint(EtkWidget *thiz){
	etk_temhum_draw(thiz);
	return etk_window_on_paint(thiz);
}

Ret etk_temhum_update_listener(void *data){
	EtkWidget *thiz=(EtkWidget*)data;
	EtkWindowClass *priv;
	struct temhum_t* temhum;
	priv=(EtkWindowClass*)thiz->subclass;
	temhum=priv->data[1];
	temhum->hum+=1*temhum->up;
	temhum->tem+=0.8*temhum->up;

	etk_temhum_draw(thiz);
	etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&thiz->rect);
	return RET_OK;
}

EtkWidget *etk_temhum_create(e32 x,e32 y,e32 width,e32 height){
	EtkWindowClass *priv;
	struct temhum_t* temhum;
	EtkSource *timer;
	EtkWidget *thiz=etk_create_window(x,y,width,height,ETK_WIDGET_WINDOW);
	thiz->event=etk_temhum_event;
	thiz->paint=etk_temhum_paint;
	etk_widget_set_text(thiz,"梁物業");
	priv=(EtkWindowClass*)thiz->subclass;
	priv->data[1]=(struct temhum_t*)ETK_MALLOC(sizeof(struct temhum_t));
	temhum=(struct temhum_t*)priv->data[1];
	temhum->tem=10.7;
	temhum->hum=67.6;
	temhum->r0.x=30;temhum->r0.y=30;temhum->r0.width=6;temhum->r0.height=240;
	temhum->r1.x=110;temhum->r1.y=70;temhum->r1.width=6;temhum->r1.height=200;
	temhum->up=0.1;
	timer=etk_source_timer_create(1000,etk_temhum_update_listener,thiz);
	etk_sources_manager_add(etk_default_sources_manager(),timer);
	return thiz;
}
void etk_temhum_set_tem(EtkWidget *thiz,float tem){
	EtkWindowClass *priv;
	struct temhum_t* temhum;
	priv=(EtkWindowClass*)thiz->subclass;
	temhum=(struct temhum_t*)priv->data[1];
	temhum->tem=tem;
}
void etk_temhum_set_hum(EtkWidget *thiz,float hum){
	EtkWindowClass *priv;
	struct temhum_t* temhum;
	priv=(EtkWindowClass*)thiz->subclass;
	temhum=(struct temhum_t*)priv->data[1];
	temhum->hum=hum;
}

float etk_temhum_get_tem(EtkWidget *thiz){
	EtkWindowClass *priv;
	struct temhum_t* temhum;
	priv=(EtkWindowClass*)thiz->subclass;
	temhum=(struct temhum_t*)priv->data[1];
	return temhum->tem;
}
float etk_temhum_get_hum(EtkWidget *thiz){
	EtkWindowClass *priv;
	struct temhum_t* temhum;
	priv=(EtkWindowClass*)thiz->subclass;
	temhum=(struct temhum_t*)priv->data[1];
	return temhum->hum;
}

