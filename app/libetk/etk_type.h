#ifndef	_ETK_TYPE_H
#define _ETK_TYPE_H

typedef char   e8;
typedef short  e16;
typedef int    e32;
typedef double e64;
typedef int  ebool;
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
// typedef double u64;


typedef void (*function_handl)();

struct point_t{
	e32 x;
	e32 y;
};
typedef struct point_t EtkPoint;
enum ret_t{
	RET_OK,
	RET_FAIL,
	RET_REMOVE,
	RET_CONTINUE,
	RET_FOUND,
	RET_NEXT,
	RET_QUIT,
	RET_IGNORED,
	RET_NO_TARGET,
	RET_OUT_OF_SPACE
};
typedef enum ret_t Ret;


typedef enum EtkPixelFormat_e
{
	ETK_PIXEL_NONE   = 0,
	ETK_PIXEL_BGR24  = 1,
	ETK_PIXEL_BGRA32 = 2,
	ETK_PIXEL_RGB565 = 3,
	ETK_PIXEL_RGB555 = 4
}EtkPixelFormat;

#define RED 0xff0000
#define GREEN  0x00ff00
#define BLUE 0x0000ff
#define WHITE 0xffffff
#define BLACK 0x000000
#define GRAY 0x808080
#define YELLOW 0xFFFF00
#define DIMGRAY 0x696969

#define CONRNISLK 0xFFF8DC
#define PALEGODENROD 0xEEE8AA
#define CRIMSON	0xDC143C

#define DARKRED 0x8b0000
#define FIREBRICK 0xb22222
#define CYAN	0xFF00FF
#define MAGENTA	0xFF00FF
#define BROWN	0xA52A2A
#define WHITESMOKE 0xF5F5F5
#define DARKGRAY 0xa9a9a9	
#define LIGHTGRAY	0xd3d3d3
#define LIGHTBLUE	0xadd8e6
#define LIGHTGREEN	0x90ee90
#define LIGHTCYAN	0xE1FFFF
#define BEIGE	0x6B8E23


#define ETK_MAX(x,y) (x)>(y)?(x):(y)
#define ETK_MIN(x,y) (x)<(y)?(x):(y)

#define true 1
#define false 0


#define TRUE 1
#define FALSE 0

#define NULL 0
typedef int Bool;
//typedef  unsigned int   size_t;

typedef void (*EtkDestroy)(void* user_data);
typedef Ret  (*EtkIdle)(void* user_data);
typedef Ret  (*EtkTimer)(void* user_data);
typedef int  (*EtkCompare)(const void* obj1, const void* obj2);
typedef Ret  (*EtkListener)(void* user_data, void* obj);


#define dbg_printf printf
//#define etk_logw(s) printf(s)
//#define etk_logw(s)
//#define etk_logw(s) printf(s)		//use in vc
#define etk_logw(fmt,args...) printf(fmt,##args)	//use in codeblock
#define return_if_fail(p)          if(!(p)) { /*lcd_printf("%s:%d "#p" failed.\n", __func__, __LINE__);*/ return;}
#define return_val_if_fail(p, val) if(!(p)) { /*lcd_printf("%s:%d "#p" failed.\n", __func__, __LINE__);*/ return (val);}

#endif
