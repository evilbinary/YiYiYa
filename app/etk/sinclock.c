#include "sinclock.h"
#include "time.h"


#define CLOCK_CLIENT_X2WIN(_x)	(_x+client.x)
#define CLOCK_CLIENT_Y2WIN(_y)	(_y+client.y)

#define CLOCK_POS_MIDLE_X		(client.width/2)
#define CLOCK_POS_MIDLE_Y		(client.height/2)
#define CLOCK_CENTER_POINT_SIZE	3

#define CLOCK_FRAME_MARGIN		5
#define CLOCK_FRAME_POINT		5
#define CLOCK_QUARTER_POINT		(CLOCK_FRAME_POINT<<2)

#define CLOCK_POINT_MARGIN		(CLOCK_FRAME_MARGIN+CLOCK_QUARTER_POINT+5)
#define CLOCK_POINT_DIF			20

#define CLOCK_MIN(_a, _b)		((_a)<(_b)?(_a):(_b))
#define CLOCK_MAX(_a, _b)		((_a)>(_b)?(_a):(_b))

const etc_clock_sintab[] = {100, 99, 97, 95, 91, 86, 80, 74, 66, 58, 50, 40, 30, 20, 10,  0, -10, -20, -30, -40, -50, -58, -66, -74, -80, -86, -91, -95, -97, -99, -100, -99, -97, -95, -91, -86, -80, -74, -66, -58, -49, -40, -30, -20, -10,  0, 10, 20, 30, 40, 50, 58, 66, 74, 80, 86, 91, 95, 97, 99};
const etc_clock_costab[] = { 0, 10, 20, 30, 40, 49, 58, 66, 74, 80, 86, 91, 95, 97, 99, 100, 99, 97, 95, 91, 86, 80, 74, 66, 58, 49, 40, 30, 20, 10,  0, -10, -20, -30, -40, -50, -58, -66, -74, -80, -86, -91, -95, -97, -99, -100, -99, -97, -95, -91, -86, -80, -74, -66, -58, -49, -40, -30, -20, -10};


static Ret etk_clock_event(EtkWidget* thiz, EtkEvent* event)
{
	EtkWindowClass *priv = NULL;
	printf("%d\n", (int)event->type);
	switch(event->type)
	{
		case ETK_EVENT_WND_DESTROY:
			printf("destroy!\n");
			priv=(EtkWindowClass*)thiz->subclass;
			etk_sources_manager_remove(	etk_default_sources_manager(), 
										(EtkSource*)priv->data[1]);
			break;
		default:
			break;
	}
	
	return etk_window_on_event(thiz, event);
}



static Ret etk_clock_draw(EtkWidget* thiz, unsigned char h, unsigned char m, unsigned char s)
{
	EtkWindowClass *priv;
	EtkRect client;
	e32 x;
	e32 y;
	e32 tx;
	e32 ty;
	e32 l;
	e32 i;

	h %= 12;

	priv=(EtkWindowClass*)thiz->subclass;
	client.x = priv->head_rect.x;
	client.y = priv->head_rect.y + priv->head_rect.height;
	client.width = thiz->rect.width - client.x*2 - 1;
	client.height = thiz->rect.height - client.y - 2;

	etk_canvas_fill_rect(thiz->canvas,client.x,client.y,client.width,client.height,WHITE);

	

	//draw clock frame
	for(i = 0; i < 60; ++i)
	{
		etk_canvas_draw_line(	thiz->canvas, 
								CLOCK_CLIENT_X2WIN(CLOCK_POS_MIDLE_X), 
								CLOCK_CLIENT_Y2WIN(CLOCK_POS_MIDLE_Y), 
								CLOCK_CLIENT_X2WIN(CLOCK_POS_MIDLE_X + CLOCK_CENTER_POINT_SIZE*etc_clock_costab[i]/100), 
								CLOCK_CLIENT_Y2WIN(CLOCK_POS_MIDLE_Y - CLOCK_CENTER_POINT_SIZE*etc_clock_sintab[i]/100));

		l = CLOCK_MIN(CLOCK_POS_MIDLE_X, CLOCK_POS_MIDLE_Y) - CLOCK_FRAME_MARGIN;
		x = CLOCK_POS_MIDLE_X + l*etc_clock_costab[i]/100;
		y = CLOCK_POS_MIDLE_Y - l*etc_clock_sintab[i]/100;
		if(i%5==0)
		{
			l -= CLOCK_QUARTER_POINT;
		}
		else
		{
			l -= CLOCK_FRAME_POINT;
		}
		tx = CLOCK_POS_MIDLE_X + l*etc_clock_costab[i]/100;
		ty = CLOCK_POS_MIDLE_Y - l*etc_clock_sintab[i]/100;
		etk_canvas_draw_line(	thiz->canvas, 
								CLOCK_CLIENT_X2WIN(x), 
								CLOCK_CLIENT_Y2WIN(y), 
								CLOCK_CLIENT_X2WIN(tx), 
								CLOCK_CLIENT_Y2WIN(ty));
		etk_canvas_draw_line(	thiz->canvas, 
								CLOCK_CLIENT_X2WIN(x+1), 
								CLOCK_CLIENT_Y2WIN(y), 
								CLOCK_CLIENT_X2WIN(tx+1), 
								CLOCK_CLIENT_Y2WIN(ty));
		etk_canvas_draw_line(	thiz->canvas, 
								CLOCK_CLIENT_X2WIN(x), 
								CLOCK_CLIENT_Y2WIN(y+1), 
								CLOCK_CLIENT_X2WIN(tx), 
								CLOCK_CLIENT_Y2WIN(ty+1));
		etk_canvas_draw_line(	thiz->canvas, 
								CLOCK_CLIENT_X2WIN(x), 
								CLOCK_CLIENT_Y2WIN(y), 
								CLOCK_CLIENT_X2WIN(tx), 
								CLOCK_CLIENT_Y2WIN(ty));
	}
	

	//draw second pointer
	l = CLOCK_MIN(CLOCK_POS_MIDLE_X, CLOCK_POS_MIDLE_Y) - CLOCK_POINT_MARGIN;
	x = CLOCK_POS_MIDLE_X + l*etc_clock_costab[s]/100;
	y = CLOCK_POS_MIDLE_Y - l*etc_clock_sintab[s]/100;
	etk_canvas_draw_line(	thiz->canvas, 
							CLOCK_CLIENT_X2WIN(CLOCK_POS_MIDLE_X), 
							CLOCK_CLIENT_Y2WIN(CLOCK_POS_MIDLE_Y), 
							CLOCK_CLIENT_X2WIN(x), 
							CLOCK_CLIENT_Y2WIN(y));


	etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&thiz->rect);

	//draw minute pointer
	l -= CLOCK_POINT_DIF;
	x = CLOCK_POS_MIDLE_X + l*etc_clock_costab[m]/100;
	y = CLOCK_POS_MIDLE_Y - l*etc_clock_sintab[m]/100;
	etk_canvas_draw_line(	thiz->canvas, 
							CLOCK_CLIENT_X2WIN(CLOCK_POS_MIDLE_X), 
							CLOCK_CLIENT_Y2WIN(CLOCK_POS_MIDLE_Y), 
							CLOCK_CLIENT_X2WIN(x), 
							CLOCK_CLIENT_Y2WIN(y));

	//draw hour pointer
	l -= CLOCK_POINT_DIF;
	x = CLOCK_POS_MIDLE_X + l*etc_clock_costab[h*60/12]/100;
	y = CLOCK_POS_MIDLE_Y - l*etc_clock_sintab[h*60/12]/100;
	etk_canvas_draw_line(	thiz->canvas, 
							CLOCK_CLIENT_X2WIN(CLOCK_POS_MIDLE_X), 
							CLOCK_CLIENT_Y2WIN(CLOCK_POS_MIDLE_Y), 
							CLOCK_CLIENT_X2WIN(x), 
							CLOCK_CLIENT_Y2WIN(y));


	etk_default_wnd_manager_update_rect(etk_default_wnd_manager(),&thiz->rect);

	//printf("time: %d:%d:%d\n", (int)h, (int)m, (int)s);

	return RET_OK;
}


Ret etk_clock_update_time_listener(void *data)
{
	time_t now = time(0);
	struct tm* t = localtime(&now);
	return etk_clock_draw((EtkWidget*)data, (unsigned char)t->tm_hour, (unsigned char)t->tm_min, (unsigned char)t->tm_sec);
}



EtkWidget* etk_app_clock(e32 x,e32 y,e32 width,e32 height)
{
	EtkWidget* thiz = NULL;
	EtkWindowClass *priv = NULL;
	thiz=etk_create_window(x, y, width, height, ETK_WIDGET_WINDOW);
	priv=(EtkWindowClass*)thiz->subclass;
	priv->has_head=1;
	etk_widget_set_text(thiz,"clock");
	thiz->event = etk_clock_event;
	priv->data[1] = etk_source_timer_create(500,etk_clock_update_time_listener,thiz);
	etk_sources_manager_add(	etk_default_sources_manager(), 
								(EtkSource*)priv->data[1]);
	return thiz;
}
