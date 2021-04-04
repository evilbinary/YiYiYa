#include "etk_canvas.h"
#include "etk_gconf.h"
#include "etk_bitmap.h"
#include "etk_platform.h"
#include "etk_wnd_manager.h"
#include "etk_global.h"

EtkCanvas* etk_canvas_create(e32 w, e32 h, EtkColor clear_color){
	EtkCanvas* canvas = (EtkCanvas*)ETK_MALLOC(sizeof(EtkCanvas));
	e32 bits;
	//printf("clor format:%d\n",clear_color.format);
	if(canvas!= NULL){
		canvas->gc.bgcolor = clear_color;
		clear_color.color=0xffffff-clear_color.color;
		canvas->gc.fgcolor=clear_color;
		canvas->gc.font=etkglobal.font;
		if(clear_color.format==ETK_PIXEL_RGB565){
			bits=16;
		}else if(clear_color.format==ETK_PIXEL_RGB555){
			bits=16;
		}else if(clear_color.format==ETK_PIXEL_BGRA32){
			bits=32;
		}
		//printf("bits:%d\n",bits);
		canvas->region=etk_region_create();
		canvas->bitmap = etk_bitmap_create(w, h,bits,canvas->gc.bgcolor);
		canvas->paint=etk_canvas_paint;
		canvas->draw_pixel=etk_canvas_draw_pixel;
		canvas->clear_rect=etk_canvas_clear_rect;
		canvas->draw_rect=etk_canvas_draw_rect;
		canvas->draw_bitmap=etk_canvas_draw_bitmap;
		canvas->draw_string=etk_canvas_draw_string;
		canvas->draw_line=etk_canvas_draw_line;

	}
	return canvas;
}
EtkCanvas* etk_canvas_create_with_no_bitmap(e32 w, e32 h, EtkColor clear_color){
	EtkCanvas* canvas = (EtkCanvas*)ETK_ZALLOC(sizeof(EtkCanvas));
	u32 bits;
	if(canvas!= NULL){
		canvas->gc.bgcolor = clear_color;
		clear_color.color=0xffffff-clear_color.color;
		canvas->gc.fgcolor=clear_color;
		canvas->gc.font=etkglobal.font;
		if(clear_color.format==ETK_PIXEL_RGB565){
			bits=16;
		}else if(clear_color.format==ETK_PIXEL_RGB555){
			bits=16;
		}else if(clear_color.format==ETK_PIXEL_BGRA32){
			bits=32;
		}
		canvas->bitmap =NULL;
		canvas->region=etk_region_create();
		canvas->draw_pixel=etk_canvas_draw_pixel;
		canvas->clear_rect=etk_canvas_clear_rect;
		canvas->draw_rect=etk_canvas_draw_rect;
		canvas->draw_bitmap=etk_canvas_draw_bitmap;
		canvas->draw_string=etk_canvas_draw_string;
		canvas->draw_line=etk_canvas_draw_line;
	}
	return canvas;
}
Ret etk_canvas_set_bitmap(EtkCanvas *thiz,EtkBitmap *bitmap){
	thiz->bitmap=bitmap;
	return RET_OK;
}
Ret etk_canvas_paint(EtkDisplay *display,EtkCanvas *thiz,EtkRect *rect){

	return RET_OK;
}
Ret etk_canvas_paint_rect(EtkDisplay *display,EtkCanvas *thiz,EtkRect *rect){
	return RET_OK;
}

Ret etk_canvas_draw_pixel(EtkCanvas* thiz, e32 x,e32 y, u32 color){
	if(thiz->bitmap==NULL) return RET_FAIL;
	//return_val_if_fail(thiz!=NULL&&thiz->bitmap!=NULL,RET_FAIL);
	if(x>=0&&x<=thiz->bitmap->w&&y>=0&&y<thiz->bitmap->h){
		if(thiz->bitmap->bitperpixel==16){
			u16 *p16=(u16*)(thiz->bitmap->bits);
			p16[ (y *thiz->bitmap->w) + x] = color;
		}else if(thiz->bitmap->bitperpixel==32){
			u32 *p32=(u32*)(thiz->bitmap->bits);
			p32[ (y *thiz->bitmap->w) + x] = color;
		}
	}
	return RET_OK;
}
Ret etk_canvas_draw_line_with_color(EtkCanvas* thiz, e32 x1, e32 y1, e32 x2, e32 y2,u32 color){
	int t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, row, col;
	delta_x = x2 - x1; //������������
	delta_y = y2 - y1;
	row = x1;
	col = y1;
	if (delta_x > 0) {
		incx = 1; //���õ�������
	} else if (delta_x == 0) {
		incx = 0;//��ֱ��
	} else {
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0) {
		incy = 1;
	} else if (delta_y == 0) {
		incy = 0;//ˮƽ��
	} else {
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y) {
		distance = delta_x; //ѡȡ��������������
	} else {
		distance = delta_y;
	}
	for (t = 0; t <= distance + 1; t++){ //�������
		etk_canvas_draw_pixel(thiz,row, col , color);//����
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance) {
			xerr -= distance;
			row += incx;
		}
		if (yerr > distance) {
			yerr -= distance;
			col += incy;
		}
	}
	return RET_OK;
}
Ret etk_canvas_draw_line(EtkCanvas* thiz, e32 x1, e32 y1, e32 x2, e32 y2){
	int t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, row, col;
	delta_x = x2 - x1; //������������
	delta_y = y2 - y1;
	row = x1;
	col = y1;
	if (delta_x > 0) {
		incx = 1; //���õ�������
	} else if (delta_x == 0) {
		incx = 0;//��ֱ��
	} else {
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0) {
		incy = 1;
	} else if (delta_y == 0) {
		incy = 0;//ˮƽ��
	} else {
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y) {
		distance = delta_x; //ѡȡ��������������
	} else {
		distance = delta_y;
	}
	for (t = 0; t <= distance + 1; t++){ //�������
		etk_canvas_draw_pixel(thiz,row, col , BLACK);//����
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance) {
			xerr -= distance;
			row += incx;
		}
		if (yerr > distance) {
			yerr -= distance;
			col += incy;
		}
	}
	return RET_OK;
}
Ret etk_canvas_clear_rect(EtkCanvas* thiz, e32 x, e32 y, e32 w, e32 h){
	e32 i;
	for(i=0;i<h;i++){
		etk_canvas_hline(thiz,x,y+i,w,thiz->gc.bgcolor.color);
	}
	return RET_OK;
}
Ret etk_canvas_fill_rect(EtkCanvas* thiz,e32 x,e32 y,e32 width,e32 height,u32 color){
	e32 i;
	for(i=0;i<height;i++){
		etk_canvas_hline(thiz,x,y+i,width,color);
	}
	return RET_OK;
}
Ret etk_canvas_draw_rect(EtkCanvas* thiz, e32 x, e32 y, e32 width, e32 height, u32 color){
	etk_canvas_hline(thiz,x,y,width,color);
	etk_canvas_hline(thiz,x,y+height,width,color);
	etk_canvas_vline(thiz,x,y,height,color);
	etk_canvas_vline(thiz,x+width,y,height,color);
	return RET_OK;
}
Ret etk_canvas_draw_bitmap(EtkCanvas* thiz, EtkBitmap* bitmap, EtkRect* s, EtkRect* d, u32 alpha){
	e32 desi,desj,srci,srcj;
	return_val_if_fail(bitmap!=NULL&&s!=NULL&d!=NULL,RET_FAIL);
	if(thiz->bitmap->bitperpixel==16){
		u16 *p16des=NULL;
		u16 *p16src=NULL;
		e32 i,j;
		EtkRect brect,src,des;
		brect.x=0;brect.y=0;
		brect.width=bitmap->w;
		brect.height=bitmap->h;
		src=etk_rect_intersect(*s,brect);
		brect.width=thiz->bitmap->w;
		brect.height=thiz->bitmap->h;
		des=etk_rect_intersect(*d,brect);
	
		for(j=0;j<src.height&&j<des.height;j++){
			p16src=(u16*)(bitmap->bits)+(src.y+j)*bitmap->w+src.x;
			p16des=(u16*)(thiz->bitmap->bits)+(des.y+j)*thiz->bitmap->w+des.x;
			for(i=0;i<src.width&&i<des.width;i++){
				//draw_pixel(i,j,p32src1[i]);
				p16des[i]=p16src[i];
			}
		}
	}else if(thiz->bitmap->bitperpixel==32){
		u32 *p32des=NULL;
		u32 *p32src=NULL;
		e32 i,j;
		EtkRect brect,src,des;
		brect.x=0;brect.y=0;
		brect.width=bitmap->w;
		brect.height=bitmap->h;
		src=etk_rect_intersect(*s,brect);
		brect.width=thiz->bitmap->w;
		brect.height=thiz->bitmap->h;
		des=etk_rect_intersect(*d,brect);
	
		for(j=0;j<src.height&&j<des.height;j++){
			p32src=(u32*)(bitmap->bits)+(src.y+j)*bitmap->w+src.x;
			p32des=(u32*)(thiz->bitmap->bits)+(des.y+j)*thiz->bitmap->w+des.x;
			for(i=0;i<src.width&&i<des.width;i++){
				//draw_pixel(i,j,p32src1[i]);
				p32des[i]=p32src[i];
			}
		}

		/*for(srci=src.x,desi=des.x;desi<(des.x+src.width)&&srci<(src.x+src.width);desi++,srci++){
			for(srcj=src.y,desj=des.y;desj<(des.y+src.height)&&srcj<(src.y+src.height);desj++,srcj++){
				p32[ (desj *d->width) + desi] =p32src[srcj*s->width+srci];
			}
		}
*/
		/*for(srci=s->x,desi=d->x;desi<(d->x+s->width)&&srci<(s->x+s->width);desi++,srci++){
			for(srcj=s->y,desj=d->y;desj<(d->y+s->height)&&srcj<(s->y+s->height);desj++,srcj++){
				p32[ (desj *d->width) + desi] =p32src[srcj*s->width+srci];
			}
		}*/
		 
		
	}
	return RET_OK;
}

extern u8  ASCII[1536];
Ret etk_canvas_draw_char_with_color(EtkCanvas* thiz,e32 x,e32 y,u32 ch,u32 fg,u32 bg){
	u16 i, j, z, code;
	u8 *lp;
	EtkPoint cur;
	cur.x=x;
	cur.y=y;
	code=ch;
	if(code<0x20) return RET_OK;
	if(code<0x7f){	
		code=code-0x20;
		lp = ASCII + code * 16;
		for(i=0; i<16; i++){	
			z = *lp++;
			for(j=0; j<8; j++){
				// if(z&0x1){
				if(z & 0x80){	//this is fg
					etk_canvas_draw_pixel(thiz,cur.x+j,cur.y+i,fg);
				}else{	//this is bg
					etk_canvas_draw_pixel(thiz,cur.x+j,cur.y+i,bg);
				}
				z = z <<1;
				// z=z>>1;
			}
		}
	}else if(code>0xa1a1){
		code = code - 0xa1a1;
		if(code>0x0f00) code = code - 0x600;
		lp = ((u8 *)thiz->gc.font->data + ((code>>8)*94 + (code&0xff)) * 32);
		for(i=0; i<16; i++){
			z = *lp++;
			z = (z << 8) + *lp++;
			for(j=0; j<16; j++){
				if(z & 0x8000){
					etk_canvas_draw_pixel(thiz,cur.x+j,cur.y+i,fg);
				}
				else{
					etk_canvas_draw_pixel(thiz,cur.x+j,cur.y+i,bg);
				}
				z = z <<1;
			}
		}
		
	} 

}


Ret etk_canvas_draw_char(EtkCanvas* thiz,e32 x,e32 y,u32 ch){
	u16 i, j, z, code;
	u8 *lp;
	EtkPoint cur;
	cur.x=x;
	cur.y=y;
	code=ch;
	if(code<0x20) return RET_OK;
	if(code<0x7f){	
		code=code-0x20;
		lp = ASCII + code * 16;
		for(i=0; i<16; i++){	
			z = *lp++;
			for(j=0; j<8; j++){
				// if(z & 0x01){	//this is fg
				if(z & 0x80){	//this is fg
					etk_canvas_draw_pixel(thiz,cur.x+j,cur.y+i,thiz->gc.fgcolor.color);
				}else{	//this is bg
					etk_canvas_draw_pixel(thiz,cur.x+j,cur.y+i,thiz->gc.bgcolor.color);
				}
				z = z <<1;
				// z=z>>1;
			}
		}
	}else if(code>0xa1a1){
		code = code - 0xa1a1;
		if(code>0x0f00) code = code - 0x600;
		lp = ((u8 *)thiz->gc.font->data + ((code>>8)*94 + (code&0xff)) * 32);
		for(i=0; i<16; i++){
			z = *lp++;
			z = (z << 8) + *lp++;
			for(j=0; j<16; j++){
				if(z & 0x8000){
					etk_canvas_draw_pixel(thiz,cur.x+j,cur.y+i,thiz->gc.fgcolor.color);
				}
				else{
					etk_canvas_draw_pixel(thiz,cur.x+j,cur.y+i,thiz->gc.bgcolor.color);
				}
				z = z <<1;
			}
		}
		
	} 

}
Ret etk_canvas_draw_string_with_color(EtkCanvas* thiz, e32 x, e32 y, const char* str,u32 fg,u32 bg){
	/*u16 code;


	while(*str != '\0'){
		code = *str++;
		if(code > 0x80)
			code=(code<<8)+ *str++;
		etk_canvas_draw_char_with_color(thiz,x,y,code,fg,bg);
		if(code<0x7f){
			x+=8;
		}else if(code>0xa1a1){
			x+=thiz->gc.font->width;
		}
	} */
	u16 code;
	u8 *lp=(u8*)str;
	while(*lp != '\0'){
		code = *lp++;
		if(code<0x7f){
			etk_canvas_draw_char_with_color(thiz,x,y,code,fg,bg);
			x+=8;
		}else if(code > 0x80){
			code=(0xff00&(code<<8))|(0xff&(*lp++));
			etk_canvas_draw_char_with_color(thiz,x,y,code,fg,bg);
			x+=thiz->gc.font->width;
		}
	} 
	return RET_OK;
}
Ret etk_canvas_draw_string(EtkCanvas* thiz, e32 x, e32 y, const char* str){
	/*u16 code;
	while(*str != '\0'){
		code = *str++;
		if(code > 0x80)
			code=(code<<8)+ *str++;
		etk_canvas_draw_char(thiz,x,y,code);
		if(code<0x7f){
			x+=8;
		}else if(code>0xa1a1){
			x+=thiz->gc.font->width;
		}
	} */
	u16 code;
	u8 *lp=(u8*)str;
	while(*lp != '\0'){
		code = *lp++;
		if(code<0x7f){
			etk_canvas_draw_char(thiz,x,y,code);
			x+=8;
		}else if(code > 0x80){
			code=(0xff00&(code<<8))|(0xff&(*lp++));;
			etk_canvas_draw_char(thiz,x,y,code);
			x+=thiz->gc.font->width;
		}
	} 
	
}


Ret etk_canvas_hline(EtkCanvas *thiz,e32 x,e32 y,e32 length,u32 color){
	int z=x+length;
	for(;x<=z;x++){
		etk_canvas_draw_pixel(thiz,x,y,color);
	}
}
Ret etk_canvas_vline(EtkCanvas *thiz,e32 x,e32 y,e32 length,u32 color){
	int z=y+length;
	for(;y<=z;y++){
		etk_canvas_draw_pixel(thiz,x,y,color);
	}
}
