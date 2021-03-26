#include "etk_canvas_ex.h"

u32 etk_mix_color2(u32 color0, u32 color1,u8 alpha) {
	/* Calc Color seperations for Fg color0 first */
	u8 R = (color0&0xff)*alpha;
	u8 G = (color0&0xff00)*alpha;
	u8 B = (color0&0xff0000)*alpha;
	/* Add Color seperations for Bg color1 */
	alpha = 15-alpha;
	R += (color1&0xff)*alpha;
	G += (color1&0xff00)*alpha;
	B += (color1&0xff0000)*alpha;
	R = (R/15)&0xff;
	G = (G/15)&0xff00;
	B = (B/15)&0xff0000;
	color0 = R+G+B;
	return color0;
}


Ret etk_canvas_draw_circle(EtkCanvas *thiz,e32 x0,e32 y0,e32 r,u32 color){
	int x =0;
	int y = r;
	int p = 1-r;
	etk_canvas_draw_pixel(thiz,x0+x,y0+y,color);
	etk_canvas_draw_pixel(thiz,x0-x,y0+y,color);
	etk_canvas_draw_pixel(thiz,x0+x,y0-y,color);
	etk_canvas_draw_pixel(thiz,x0-x,y0-y,color);
	
	etk_canvas_draw_pixel(thiz,x0+y,y0+x,color);
	etk_canvas_draw_pixel(thiz,x0-y,y0+x,color);
	etk_canvas_draw_pixel(thiz,x0+y,y0-x,color);
	etk_canvas_draw_pixel(thiz,x0-y,y0-x,color);
	while(x<y){
		x++;
		if(p<0)
			p+=2*x+1;
		else{
			y--;
			p+=2*(x-y)+1;
		}
		etk_canvas_draw_pixel(thiz,x0+x,y0+y,color);
		etk_canvas_draw_pixel(thiz,x0-x,y0+y,color);
		etk_canvas_draw_pixel(thiz,x0+x,y0-y,color);
		etk_canvas_draw_pixel(thiz,x0-x,y0-y,color);
		
		etk_canvas_draw_pixel(thiz,x0+y,y0+x,color);
		etk_canvas_draw_pixel(thiz,x0-y,y0+x,color);
		etk_canvas_draw_pixel(thiz,x0+y,y0-x,color);
		etk_canvas_draw_pixel(thiz,x0-y,y0-x,color);
	}
	
}
Ret etk_canvas_fill_circle(EtkCanvas *thiz,e32 x0,e32 y0,e32 r,u32 color){
	int i, x;
	int sqmax = r*r+r/2;
	int yMin, yMax;
	/* First step : find uppermost and lowermost coordinates */
	yMin = y0 - r;
	yMax = y0 + r;
	/* Draw top half */
	for (i=0, x=r; i<=r; i++) {
		int y = y0-i;
		if ((y >= yMin) && (y <= yMax)) {
			/* calc proper x-value */
			while ((i*i+x*x) >sqmax)
				--x;
			etk_canvas_hline (thiz,x0-x, y, x+x,color);
		}
	}
	/* Draw bottom half */
	for (i=0, x=r; i<=r; i++) {
		int y = y0 + i;
		if ((y >= yMin) && (y <= yMax)) {
			/* calc proper x-value */
			while ((i*i+x*x) >sqmax)
				--x;
			etk_canvas_hline(thiz,x0-x , y, x+x,color);
		}
	}
}

