#ifndef _GAME_MINE
#define _GAME_MINE
#include "etk.h"


struct mine_t{
	
	char *top;			//for paint
	char *middle;	//for judge
	char *bottom;	//raw data
	
	int rows;
	int cols;
	int bombcount;
	int recount;
	int toolix;
	char status;
}mine_t;

typedef struct mine_t mine_type;

void mine_click_cell(mine_type* mine, unsigned short x, unsigned short y, char isuser);
void mine_click_tool(mine_type* mine, unsigned short ix);
Ret mine_event(EtkWidget* thiz, EtkEvent* event);
void mine_rand_mine(mine_type* mine);
void mine_init_mine(mine_type* mine);
int mine_bombcount(mine_type* mine, int x, int y);
void mine_init_bmp();
void mine_draw_client(EtkWidget* thiz);
Ret mine_paint(EtkWidget* thiz);

Ret etk_mine_on_mouse_event(EtkWidget* thiz, EtkEvent* event);

EtkWidget* etk_game_mine(int x, int y, int rows, int cols, int count);
void etk_game_mine_destory(EtkWidget* minewin);

#endif
