#include "etk_bitmap.h"
#include "etk_platform.h"
#include "etk_color.h"
 

EtkBitmap* etk_bitmap_create(e32 w,e32 h,e32 bitperpixel,EtkColor color){
	u32 i;
	u32 *p32=NULL;
	u16 *p16=NULL;
	u32 length=w*(h+5);
	EtkBitmap *bitmap=(EtkBitmap*)ETK_MALLOC(sizeof(EtkBitmap));
	bitmap->bits=(void*)ETK_EX_MALLOC(length * (bitperpixel>>3));
	bitmap->w=w;
	bitmap->h=h;
	bitmap->length=length;
	bitmap->bitperpixel=bitperpixel;
	//printf("size:%d length:%d\n",length*bitperpixel,length);
	switch(bitperpixel){
	case 16:
		p16=(u16*)bitmap->bits;
		for(i=0;i<length;i++){
			*p16=(u16)color.color;
			p16++;
		}
		break;
	case 32:
		p32=(u32*)bitmap->bits;
		for(i=0;i<length;i++){
			*p32=color.color;
			p32++;
		}
		break;
	}
		
	return bitmap;
}
void etk_bitmap_destroy(EtkBitmap* bitmap){
	if(bitmap!=NULL){
		ETK_EX_FREE(bitmap->bits)
		ETK_FREE(bitmap);
	}
}

Ret etk_bitmap_draw_pixel(EtkBitmap* thiz, e32 x,e32 y, u32 color){
	//return_val_if_fail(thiz!=NULL,RET_FAIL);
	if(x>=0&&x<=thiz->w&&y>=0&&y<thiz->h){
	if(thiz->bitperpixel==16){
		u16 *p16=(u16*)( thiz->bits);
		
			p16[ (y *thiz->w) + x] = color;
	}else if( thiz->bitperpixel==32){
		u32 *p32=(u32*)( thiz->bits);
		p32[ (y *thiz->w) + x] = color;
	}
	}
	return RET_OK;
}
Ret etk_bitmap_hline(EtkBitmap *thiz,e32 x,e32 y,e32 length,u32 color){
	int z=x+length;
	for(;x<=z;x++){
		etk_bitmap_draw_pixel(thiz,x,y,color);
	}
	return RET_OK;
}
Ret etk_bitmap_vline(EtkBitmap *thiz,e32 x,e32 y,int length,u32 color){
	int z=y+length;
	for(;y<=z;y++){
		etk_bitmap_draw_pixel(thiz,x,y,color);
	}
	return RET_OK;
}
Ret etk_bitmap_draw_rect(EtkBitmap* thiz, e32 x, e32 y, e32 width, e32 height, u32 color){
	etk_bitmap_hline(thiz,x,y,width,color);
	etk_bitmap_hline(thiz,x,y+height,width,color);
	etk_bitmap_vline(thiz,x,y,height,color);
	etk_bitmap_vline(thiz,x+width,y,height,color);
	return RET_OK;
}

Ret etk_bitmap_draw_line(EtkBitmap* thiz, e32 x1, e32 y1, e32 x2, e32 y2){
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
		etk_bitmap_draw_pixel(thiz,row, col , GREEN);//����
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