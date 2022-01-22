#include "mine.h"
#include "time.h"


//#define MINE_DEBUG
#define RES_PATH(res) "/"res

//window title bar property
#define MINE_WINDOW_TITLE_HEIGHT				20
#define MINE_WINDOW_TITLE_CLOSE_HEIGHT	16
#define MINE_WINDOW_TITLE_CLOSE_WIDTH	16
#define MINE_WINDOW_TITLE_CLOSE_TOP			((MINE_WINDOW_TITLE_HEIGHT - MINE_WINDOW_TITLE_CLOSE_HEIGHT)/2)
#define MINE_WINDOW_TITLE_CLOSE_LEFT		MINE_WINDOW_TITLE_CLOSE_TOP
#define MINE_WINDOW_TITLE_CLOSE_BOTTOM	(MINE_WINDOW_TITLE_CLOSE_TOP + MINE_WINDOW_TITLE_CLOSE_HEIGHT)
#define MINE_WINDOW_TITLE_CLOSE_RIGHT		(MINE_WINDOW_TITLE_CLOSE_LEFT + MINE_WINDOW_TITLE_CLOSE_WIDTH)


//mine cell size
#define MINE_CELL_WIDTH		16
#define MINE_CELL_HEIGHT		16

//about mine scale
#define MINE_ROWS				mine->rows
#define MINE_COLS				mine->cols

//tool bar property
#define MINE_TOOL_HEIGHT_ALL		20
#define MINE_TOOL_CELL_WIDTH		16
#define MINE_TOOL_CELL_HEIGHT	16
#define MINE_TOOL_CELL_COUNT		6
#define MINE_TOOL_HEIGHT				MINE_TOOL_CELL_HEIGHT

//widow size
#define MINE_CLIENT_EDGE				0
#define MINE_WINDOW_WIDTH		((MINE_CELL_WIDTH * MINE_COLS) + MINE_CLIENT_EDGE)
#define MINE_WINDOW_HEIGHT		((MINE_WINDOW_TITLE_HEIGHT + MINE_TOOL_HEIGHT_ALL + (MINE_CELL_HEIGHT * MINE_ROWS))+MINE_CLIENT_EDGE)




//client a
#define MINE_WINDOW_CLIENT_WIDTH		(MINE_CELL_WIDTH * MINE_COLS)
#define MINE_WINDOW_CLIENT_HEIGHT		(MINE_CELL_HEIGHT * MINE_ROWS)
#define MINE_WINDOW_CLIENT_LEFT			(((MINE_WINDOW_WIDTH - MINE_WINDOW_CLIENT_WIDTH))/2)
#define MINE_WINDOW_CLIENT_TOP			((MINE_WINDOW_HEIGHT - MINE_WINDOW_TITLE_HEIGHT - MINE_TOOL_HEIGHT_ALL - MINE_WINDOW_CLIENT_HEIGHT )/2 + MINE_WINDOW_TITLE_HEIGHT + MINE_TOOL_HEIGHT_ALL)
#define MINE_WINDOW_CLIENT_RIGHT		(MINE_WINDOW_CLIENT_LEFT + MINE_WINDOW_CLIENT_WIDTH)
#define MINE_WINDOW_CLIENT_BOTTOM	(MINE_WINDOW_CLIENT_TOP + MINE_WINDOW_CLIENT_HEIGHT)

//toolbar property
#define MINE_TOOL_WIDTH_ALL	MINE_WINDOW_WIDTH
#define MINE_TOOL_WIDTH			(MINE_TOOL_CELL_WIDTH * MINE_TOOL_CELL_COUNT)
#define MINE_TOOL_TOP				(((MINE_TOOL_HEIGHT_ALL-MINE_TOOL_HEIGHT)/2) + MINE_WINDOW_TITLE_HEIGHT)
#define MINE_TOOL_LEFT				((MINE_TOOL_WIDTH_ALL - MINE_TOOL_WIDTH)/2)
#define MINE_TOOL_RIGHT			(MINE_TOOL_LEFT + MINE_TOOL_WIDTH)
#define MINE_TOOL_BOTTOM		(MINE_TOOL_TOP + MINE_TOOL_HEIGHT)


//position switch
//client
//client to window
#define MINE_CLIENT_X2WIN(_x)				((_x) + MINE_WINDOW_CLIENT_LEFT)
#define MINE_CLIENT_Y2WIN(_y)				((_y) + MINE_WINDOW_CLIENT_TOP)
//window to client
#define MINE_CLIENT_WIN2X(_x)				((_x) - MINE_WINDOW_CLIENT_LEFT)
#define MINE_CLIENT_WIN2Y(_y)				((_y) - MINE_WINDOW_CLIENT_TOP)

//tool bar
//client to window
#define MINE_TOOL_X2WIN(_x)				((_x) + MINE_TOOL_LEFT)
#define MINE_TOOL_Y2WIN(_y)				((_y) + MINE_TOOL_TOP)
//window to client
#define MINE_TOOL_WIN2X(_x)				((_x) - MINE_TOOL_LEFT)
#define MINE_TOOL_WIN2Y(_y)				((_y) - MINE_TOOL_TOP)

//get mine value
#ifdef WIN32
#define MINE_GET(_mine, _member, _y, _x)	(((_mine)->##_member##[((_mine)->cols * (_y) + (_x))]))
#else
#define MINE_GET(_mine, _member, _y, _x)	(((_mine)->_member[((_mine)->cols * (_y) + (_x))]))
#endif

//game status
#define MINE_STA_RUN		0	//running
#define MINE_STA_DIE		1	//death
#define MINE_STA_SUC		2	//success



//bmp resource
EtkBitmap *mine_bmp_bomb[3] = {NULL, NULL, NULL};
EtkBitmap *mine_bmp_sta[3] = {NULL, NULL, NULL};
EtkBitmap *mine_bmp_num[9];

EtkBitmap *mine_bmp_tool[6];

#define MINE_BMP_TOOL_SMILE		0
#define MINE_BMP_TOOL_CRY			1
#define MINE_BMP_TOOL_SUCCESS	2
#define MINE_BMP_TOOL_FLAG			3
#define MINE_BMP_TOOL_UKN			4
#define MINE_BMP_TOOL_CUR			5

#define MINE_TOOL_IX_FLAG			0
#define MINE_TOOL_IX_UKN			1
#define MINE_TOOL_IX_CUR			2
#define MINE_TOOL_IX_FACE			4

#define MINE_EMPTY			0
#define MINE_BOMB			1

#define MINE_BMP_STA					9
#define MINE_BMP_STA_CT			3
#define MINE_BMP_STA_BLANK		9
#define MINE_BMP_STA_FLAG		10
#define MINE_BMP_STA_UNK			11

#define MINE_BMP_BOMB				12
#define MINE_BMP_BOMB_CT		2
#define MINE_BMP_BOMB_NAT		12
#define MINE_BMP_BOMB_FOC		13

//when select cell
void mine_click_cell(mine_type* mine, unsigned short x, unsigned short y, char isuser)
{
	int i,j;
	if(mine->status != MINE_STA_RUN || x >= mine->cols || y >= mine->rows)return;
	if(	MINE_GET(mine, top, y, x) != MINE_BMP_STA_BLANK && 
		MINE_GET(mine, top, y, x) != MINE_BMP_STA_FLAG &&
		MINE_GET(mine, top, y, x) != MINE_BMP_STA_UNK)return;

	if(isuser)
	{
		if(mine->toolix == MINE_TOOL_IX_FLAG)
		{
			if(MINE_GET(mine, top, y, x) == MINE_BMP_STA_BLANK)
				MINE_GET(mine, top, y, x) = MINE_BMP_STA_FLAG;
			else
				MINE_GET(mine, top, y, x) = MINE_BMP_STA_BLANK;
			return;
		}
		else	if(mine->toolix == MINE_TOOL_IX_UKN)
		{
			if(MINE_GET(mine, top, y, x) == MINE_BMP_STA_BLANK)
				MINE_GET(mine, top, y, x) = MINE_BMP_STA_UNK;
			else
				MINE_GET(mine, top, y, x) = MINE_BMP_STA_BLANK;
			return;
		}
	}

	if(	MINE_GET(mine, top, y, x) != MINE_BMP_STA_BLANK && MINE_GET(mine, top, y, x) != MINE_BMP_STA_UNK)return;

	if(MINE_GET(mine, bottom, y, x) == MINE_EMPTY)	//is bomb
	{
		MINE_GET(mine, top, y, x) = MINE_GET(mine, middle, y, x);
		if(MINE_GET(mine, middle, y, x) == 0)
		{
			if(x > 0)mine_click_cell(mine, x-1, y, 0);	//left
			if(y > 0)mine_click_cell(mine, x, y-1, 0);	//top
			if(x < (MINE_COLS-1))mine_click_cell(mine, x+1, y, 0);	//right
			if(y < (MINE_ROWS-1))mine_click_cell(mine, x, y+1, 0);	//right
		}
		--mine->recount;
		if(mine->recount == 0)	//success
		{
			mine->status = MINE_STA_SUC;
			if(mine->bombcount < (mine->cols * mine->rows - 1))
				++mine->bombcount;
		}
	}
	else
	{
		if(isuser)	//fail
		{
			for(j = 0; j < MINE_ROWS; ++j){
				for(i = 0; i < MINE_COLS; ++i){
					if(MINE_GET(mine, bottom, j, i) == MINE_BOMB){
						MINE_GET(mine, top, j, i) = MINE_BMP_BOMB_NAT;
					}
				}
			}
			MINE_GET(mine, top, y, x) = MINE_BMP_BOMB_FOC;
			mine->status = MINE_STA_DIE;
		}
		else	//nothing to do
		{
			return;
		}
	}
}

//when select tool
void mine_click_tool(mine_type* mine, unsigned short ix){
	switch(ix){
	case MINE_TOOL_IX_FACE:{
		mine_rand_mine(mine);
		mine->status = MINE_STA_RUN;
		mine->toolix = MINE_TOOL_IX_CUR;
		break;
						   }
	case MINE_TOOL_IX_CUR:
	case MINE_TOOL_IX_FLAG:
	case MINE_TOOL_IX_UKN:{
		mine->toolix = ix;
						  }
	}


}
Ret etk_mine_on_mouse_event(EtkWidget* thiz, EtkEvent* event){
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	mine_type* mine = NULL;
	mine = (mine_type*)priv->data[0];
	if(event->type == ETK_EVENT_MOUSE_DOWN){
		EtkWindowClass* priv=(EtkWindowClass*)thiz->subclass;
		e32 x = event->u.mouse.x - thiz->rect.x;
		e32 y = event->u.mouse.y - thiz->rect.y;
		if(	x >= MINE_WINDOW_CLIENT_LEFT && x < MINE_WINDOW_CLIENT_RIGHT &&y >= MINE_WINDOW_CLIENT_TOP && y < MINE_WINDOW_CLIENT_BOTTOM){
			mine_click_cell(mine,MINE_CLIENT_WIN2X(x) / MINE_CELL_WIDTH,MINE_CLIENT_WIN2Y(y) / MINE_CELL_WIDTH, 1);
		}
		else if(x >= MINE_TOOL_LEFT && x < MINE_TOOL_RIGHT && y >= MINE_TOOL_TOP && y < MINE_TOOL_BOTTOM){
			mine_click_tool(mine,  MINE_TOOL_WIN2X(x) / MINE_TOOL_CELL_WIDTH);
		}
		mine_paint(thiz);

	}else if(event->type==ETK_EVENT_MOUSE_UP){
		EtkRect r;
		r.width=1;
		r.height=1;
		r.x=event->u.mouse.x-thiz->rect.x;
		r.y=event->u.mouse.y-thiz->rect.y;
		r=etk_rect_intersect(priv->close_rect,r);
		if(r.width>0){
			EtkEvent e;
			//printf("--etk_window_on_mouse_event\n");
			etk_event_init(&e, ETK_EVENT_WND_DESTROY);
			e.widget = thiz;
			etk_wnd_manager_dispatch_event(etk_default_wnd_manager(),&e);

		}
	}
	 
}

//event
Ret mine_event(EtkWidget* thiz, EtkEvent* event){
	int x, y;
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;
	Ret ret;
	EtkEvent e;
	EtkRect r;
	mine_type* mine = NULL;
	mine = (mine_type*)priv->data[0];

	if(event==NULL)
		return RET_OK;
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
		//dbg_printf("ETK_EVENT_SHOW\n");
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
	case ETK_EVENT_MOUSE_LONG_PRESS: 
		ret = etk_mine_on_mouse_event(thiz,event);
		break;
	case ETK_EVENT_KEY_DOWN:
	case ETK_EVENT_KEY_UP:
	case ETK_EVENT_KEY_LONG_PRESS:{
		ret = etk_window_on_key_event(thiz,event);
		break;
								  }
	default:
		break;
	}

	return RET_OK;
}

//create a random mine
void mine_rand_mine(mine_type* mine){
	int i, j,x,y;
	time_t tm = 0;
	srand(time(&tm));

	mine->recount = (MINE_ROWS * MINE_COLS) - mine->bombcount;
	for(j = 0; j < MINE_ROWS; ++j){
		for(i = 0; i < MINE_COLS; ++i){
			MINE_GET(mine, bottom, j, i) = MINE_EMPTY;
			MINE_GET(mine, top, j, i) = MINE_BMP_STA_BLANK;
		}
	}
	int count=0;
	for(i = 0; i < mine->bombcount; ++i){
		do{
			x = rand() % MINE_COLS;
			y = rand() % MINE_ROWS;
			count++;
			if(count>2000){
				printf("random not work\n");
				break;
			}
		}while(MINE_GET(mine, bottom, y, x) != MINE_EMPTY);
		MINE_GET(mine, bottom, y, x) = MINE_BOMB;
	}	
	//display middle
	for(j = 0; j < MINE_ROWS; ++j){
		for(i = 0; i < MINE_COLS; ++i){
			MINE_GET(mine, middle, j, i) = mine_bombcount(mine, i, j);
		}
	}

	mine->toolix = MINE_TOOL_IX_CUR;
}

//initialize mine
void mine_init_mine(mine_type* mine){
	int size;
	size = MINE_ROWS * MINE_COLS * sizeof(char);

	mine->top = (char*)ETK_MALLOC(size);
	mine->middle = (char*)ETK_MALLOC(size);
	mine->bottom = (char*)ETK_MALLOC(size);

	if(mine->bombcount > (MINE_ROWS * MINE_COLS)){
		mine->bombcount = (MINE_ROWS * MINE_COLS);
	}
	mine_rand_mine(mine);
}

int mine_bombcount(mine_type* mine, int x, int y){
	int ct = 0;
	if(x > 0 && MINE_GET(mine, bottom, y, x-1) == MINE_BOMB)++ct;	//left
	if(y > 0 && MINE_GET(mine, bottom, y-1, x) == MINE_BOMB)++ct;	//top
	if(x < (MINE_COLS-1) && MINE_GET(mine, bottom, y, x+1) == MINE_BOMB)++ct;	//right
	if(y < (MINE_ROWS-1) && MINE_GET(mine, bottom, y+1, x) == MINE_BOMB)++ct;	//right

	if(x > 0 && y > 0 && MINE_GET(mine, bottom, y-1, x-1) == MINE_BOMB)++ct;	//left top
	if(x > 0 && y < (MINE_ROWS-1) && MINE_GET(mine, bottom, y+1, x-1) == MINE_BOMB)++ct;	//left bottom
	if(y > 0 && x < (MINE_COLS-1)  && MINE_GET(mine, bottom, y-1, x+1) == MINE_BOMB)++ct;	//right top
	if(x < (MINE_COLS-1) && y < (MINE_ROWS-1) && MINE_GET(mine, bottom, y+1, x+1) == MINE_BOMB)++ct;	//right bottom

	return ct;
}

void mine_init_bmp(){
	if(mine_bmp_num[0] != NULL){
		return;
	}


	//number 0  to 8
	mine_bmp_num[0] = load_bmp(RES_PATH("num_0.bmp"));
	mine_bmp_num[1] = load_bmp(RES_PATH("num_1.bmp"));
	mine_bmp_num[2] = load_bmp(RES_PATH("num_2.bmp"));
	mine_bmp_num[3] = load_bmp(RES_PATH("num_3.bmp"));
	mine_bmp_num[4] = load_bmp(RES_PATH("num_4.bmp"));
	mine_bmp_num[5] = load_bmp(RES_PATH("num_5.bmp"));
	mine_bmp_num[6] = load_bmp(RES_PATH("num_6.bmp"));
	mine_bmp_num[7] = load_bmp(RES_PATH("num_7.bmp"));
	mine_bmp_num[8] = load_bmp(RES_PATH("num_8.bmp"));		//8

	//state
	mine_bmp_sta[0] = load_bmp(RES_PATH("sta_blank.bmp"));//9
	mine_bmp_sta[1] = load_bmp(RES_PATH("sta_flag.bmp"));		//10
	mine_bmp_sta[2] = load_bmp(RES_PATH("sta_unknown.bmp"));		//11

	//bomb
	mine_bmp_bomb[0] = load_bmp(RES_PATH("bomb_nat.bmp"));		//12
	mine_bmp_bomb[1] = load_bmp(RES_PATH("bomb_focused.bmp"));	//13

	//tool
	//face
	mine_bmp_tool[MINE_BMP_TOOL_SMILE] = load_bmp(RES_PATH("tool_smile.bmp"));
	mine_bmp_tool[MINE_BMP_TOOL_CRY] = load_bmp(RES_PATH("tool_cry.bmp"));
	mine_bmp_tool[MINE_BMP_TOOL_SUCCESS] = load_bmp(RES_PATH("tool_success.bmp"));
	mine_bmp_tool[MINE_BMP_TOOL_FLAG] = load_bmp(RES_PATH("tool_flag.bmp"));
	mine_bmp_tool[MINE_BMP_TOOL_UKN] = load_bmp(RES_PATH("tool_unknown.bmp"));
	mine_bmp_tool[MINE_BMP_TOOL_CUR] = load_bmp(RES_PATH("tool_cursor.bmp"));
}

//draw game clinet
void mine_draw_client(EtkWidget* thiz){
	int i, j;
	char c;
	EtkBitmap *bmp = NULL;
	EtkRect srec, drec;
	EtkCanvas *can = NULL;
	mine_type *mine = NULL;
	EtkWindowClass *priv=(EtkWindowClass *)thiz->subclass;
	mine = (mine_type*)priv->data[0];
	if(mine_bmp_num[0] == NULL){	//did't initialize bitmap
		mine_init_bmp();
		if(mine_bmp_num[0] == NULL){
			return;
		}
	}
	srec.x = 0;
	srec.y = 0;
	drec.height = srec.height = MINE_CELL_HEIGHT;
	drec.width = srec.width = MINE_CELL_WIDTH;

	can = thiz->canvas;

	for(j = 0; j < MINE_ROWS; ++j){
		for(i = 0; i < MINE_COLS; ++i){
			drec.x = MINE_CLIENT_X2WIN(i * MINE_CELL_WIDTH);
			drec.y = MINE_CLIENT_Y2WIN(j * MINE_CELL_HEIGHT);
			c = MINE_GET(mine, top, j, i) ;
			if(c>= 0 && c <= MINE_BMP_BOMB_FOC)
			{
				if( c >=0 && c <= 8)
					bmp = mine_bmp_num[c];
				else if(c >=9 && c <= 11){
					bmp = mine_bmp_sta[c-9];
				}
				else{
					bmp = mine_bmp_bomb[c-12];
				}
				etk_canvas_draw_bitmap(can, bmp, &srec, &drec, 0);
			}
			else
			{
				//printf("Error!\n");
			}
		}
	}



	//toolbar
	//face
	drec.height = srec.height = MINE_TOOL_CELL_HEIGHT;
	drec.width = srec.width = MINE_TOOL_CELL_WIDTH;
	drec.x = MINE_TOOL_X2WIN(MINE_TOOL_IX_FACE * MINE_TOOL_CELL_WIDTH);
	drec.y = MINE_TOOL_Y2WIN(0);
	if(mine->status == MINE_STA_RUN)
	{
		etk_canvas_draw_bitmap(can, mine_bmp_tool[MINE_BMP_TOOL_SMILE], &srec, &drec, 0);
	}
	else if(mine->status == MINE_STA_DIE)
	{
		etk_canvas_draw_bitmap(can, mine_bmp_tool[MINE_BMP_TOOL_CRY], &srec, &drec, 0);
	}
	else if(mine->status == MINE_STA_SUC)
	{
		etk_canvas_draw_bitmap(can, mine_bmp_tool[MINE_BMP_TOOL_SUCCESS], &srec, &drec, 0);
	}
	//flag
	drec.x = MINE_TOOL_X2WIN(MINE_TOOL_IX_FLAG * MINE_TOOL_CELL_WIDTH);
	etk_canvas_draw_bitmap(can, mine_bmp_tool[MINE_BMP_TOOL_FLAG], &srec, &drec, 0);
	//unknow
	drec.x = MINE_TOOL_X2WIN(MINE_TOOL_IX_UKN * MINE_TOOL_CELL_WIDTH);
	etk_canvas_draw_bitmap(can, mine_bmp_tool[MINE_BMP_TOOL_UKN], &srec, &drec, 0);
	//cursor
	drec.x = MINE_TOOL_X2WIN(MINE_TOOL_IX_CUR * MINE_TOOL_CELL_WIDTH);
	etk_canvas_draw_bitmap(can, mine_bmp_tool[MINE_BMP_TOOL_CUR], &srec, &drec, 0);
	//can->draw_rect(can, MINE_TOOL_LEFT, MINE_TOOL_TOP, MINE_TOOL_WIDTH,  MINE_TOOL_HEIGHT, 0xff0000);

	//draw select box
	can->draw_rect(can, MINE_TOOL_X2WIN(mine->toolix*MINE_TOOL_CELL_WIDTH), MINE_TOOL_Y2WIN(0), MINE_TOOL_CELL_WIDTH-1,  MINE_TOOL_CELL_HEIGHT-1, 0xff0000);
}


//redraw snake munch
Ret mine_paint(EtkWidget* thiz){	
	EtkRect rectitle;
	EtkColor coltitle;
	EtkBitmap *bmptitle=NULL;
	EtkWindowClass *priv=(EtkWindowClass*)thiz->subclass;

	if((thiz->type&ETK_WIDGET_WINDOW)){
		const char *text = NULL;
		rectitle.x = 0;
		rectitle.y = 0;
		rectitle.height = MINE_WINDOW_TITLE_HEIGHT;
		rectitle.width = thiz->rect.width;
		etk_color_init(&coltitle,0xffffff-0x8f8fcf,ETK_PIXEL_BGR24);
		bmptitle = etk_bitmap_create(rectitle.width, rectitle.height, 32, coltitle);	//code should be change to save mem

		//Draw close box
		etk_bitmap_draw_rect(bmptitle,
			MINE_WINDOW_TITLE_CLOSE_LEFT,
			MINE_WINDOW_TITLE_CLOSE_TOP,
			MINE_WINDOW_TITLE_CLOSE_WIDTH,
			MINE_WINDOW_TITLE_CLOSE_HEIGHT,
			0xcccccc);
		etk_bitmap_draw_line(bmptitle,	
			MINE_WINDOW_TITLE_CLOSE_LEFT,
			MINE_WINDOW_TITLE_CLOSE_TOP, 
			MINE_WINDOW_TITLE_CLOSE_RIGHT,
			MINE_WINDOW_TITLE_CLOSE_BOTTOM);
		etk_bitmap_draw_line(bmptitle,	
			MINE_WINDOW_TITLE_CLOSE_RIGHT,
			MINE_WINDOW_TITLE_CLOSE_TOP, 
			MINE_WINDOW_TITLE_CLOSE_LEFT,
			MINE_WINDOW_TITLE_CLOSE_BOTTOM);
		//title bar area
		etk_canvas_draw_bitmap(thiz->canvas, bmptitle, &rectitle, &rectitle, 0x00);

		etk_bitmap_destroy(bmptitle);
	}

	//client area
	mine_draw_client(thiz);

	//thiz->display->update(thiz->display,thiz->canvas->bitmap, &thiz->rect, thiz->rect.x-thiz->rect.x,thiz->rect.y-thiz->rect.y);
	//etk_default_wnd_manager_update_widget(etk_default_wnd_manager(),thiz);
	return RET_OK;
}



EtkWidget* etk_game_mine(int x, int y, int rows, int cols, int count){
	EtkWidget* thiz;
	EtkWindowClass *priv;
	mine_type* mine;
	mine = (mine_type*)ETK_MALLOC(sizeof(mine_type));
	mine->rows = rows;
	mine->cols = cols;
	mine->bombcount = count;
	mine->status = MINE_STA_RUN;
	mine_init_mine(mine);
	thiz=etk_window_create(x, y, MINE_WINDOW_WIDTH, MINE_WINDOW_HEIGHT,ETK_WIDGET_WINDOW);
	priv=(EtkWindowClass*)thiz->subclass;
	priv->has_head=0;
	priv->data[0] = mine;
	thiz->event = mine_event;
	thiz->paint = mine_paint;
	return thiz;
}

//destory mine
void etk_game_mine_destory(EtkWidget* minewin){
	if(minewin!=NULL){
		mine_type * mine;
		EtkWindowClass *priv=(EtkWindowClass*)minewin->subclass;
		mine = (mine_type*)priv->data[0];
		ETK_FREE(mine->bottom);
		ETK_FREE(mine->middle);
		ETK_FREE(mine->top);
		ETK_FREE(priv->data[0]);
	}
}