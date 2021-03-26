#include "etk_image_bmp_decoder.h"
#include "etk_platform.h"
#include "etk_util.h"

EtkImageDecoder* etk_image_bmp_decoder_create(void){
	EtkImageDecoder* thiz=(EtkImageDecoder* )ETK_MALLOC(sizeof(EtkImageDecoder));
	thiz->decode=etk_image_bmp_decoder;
	return thiz;
}

EtkBitmap* etk_image_bmp_decoder(EtkImageDecoder *thiz,const e8 *filename){
	return load_bmp(filename);
}
void etk_image_bmp_decoder_destroy(EtkImageDecoder* thiz){
	if(thiz!=NULL){

		ETK_FREE(thiz);
	}
}


Ret etk_image_bmp_copy32(EtkBitmap* bitmap, int compress, const unsigned char* data){
	int x = 0;
	int y = 0;
	const unsigned char* src = data;
	int width = bitmap->w;
	int height =  bitmap->h;
	u8* bits =(u8*) bitmap->bits;
	for(y = 0; y < height; y++){
		for(x = 0; x < width; x++, bits+=4){
			bits[0] = src[0];
			bits[1]= src[1];	
			bits[2] = src[2];	
			bits[3] = 0xff;
			src += 4;
		}
		/*SDL_Flip(screen);
		SDL_Delay(50);*/
	}

	return RET_OK;
}
Ret etk_image_bmp_copy24(EtkBitmap* bitmap, int compress, const unsigned char* data){
	int x = 0;
	int y = 0;
	const unsigned char* src = data;
	const unsigned char* lsrc = data;
	int width = bitmap->w;
	int height = bitmap->h;
	int line_delta = (width * 3 + 3) & 0xfffffffc;
	u8 *bits=(u8 *)bitmap->bits;
	lsrc += line_delta * (height - 1);
	for(y = 0; y < height; y++){
		src = lsrc;
		for(x = 0; x < width; x++){
			//u32 *t=(u32*)bits;
			bits[0] = src[0];
			bits[1]= src[1];	
			bits[2] = src[2];	
			bits[3] = 0xff;
			//*t=rand()%0xffffff;
			//put_pixel(screen,x,y,*t);
			//put_pixel(screen,x,y,RED);
			src += 3;
			bits+=4;
		}
		lsrc -= line_delta;
	}
	// int x = 0;
	// int y = 0;
	// const unsigned char* src = data;
	// const unsigned char* lsrc = data;
	// int width =bitmap->w;
	// int height =bitmap->h;
	// u16 *bits=(u16 *)bitmap->bits;
	// int line_delta = (width * 3 + 3) & 0xfffffffc;
	// lsrc += line_delta * (height - 1);
	// for(y = 0; y < height; y++){
	// 	src = lsrc;
	// 	for(x = 0; x < width; x++){
	// 	 	//lcd_dot(x,y,RGB24_2_RGB555(src[2],src[1],src[0]));
	// 		*bits=RGB24_2_RGB555(src[0],src[1],src[2]);
	// 		bits+=1;
	// 		src += 3;
	// 	}
	// 	lsrc -= line_delta;
	// }
	return RET_OK;
}



EtkBitmap* load_bmp(const char *filename){
	EtkBitmap* bitmap=NULL;
	u16 bpp=0;
	u16 width;
	u16 height;
	u16 doffset;
	u32 size=1024*1024*2;
	char* buffer=(char*)ETK_MALLOC(size);
	unsigned char* src = NULL;
	unsigned char* data = NULL;
	int compress = 0;
	EtkColor color;
	int x = 0;
	int y = 0;
	u8* b;
	printf("load file %s\n",filename);
	read_file_to_buffer(buffer,filename,size);
	//print_buffer(buffer,1024);
	data=(unsigned char*)buffer;
	if(data[0] != 'B' || data[1] != 'M'){
		printf("not bmp %c %c\n",data[0],data[1]);
		return NULL;
	}
	//print_buffer(&data[0x1e1e0],16);
	doffset  = *(unsigned int*)(data + 0x000a);
	width    = *(unsigned int*)(data + 0x0012);
	height   = *(unsigned int*)(data + 0x0016);
	bpp      = *(unsigned short*)(data + 0x001c);
	compress = *(unsigned int*)(data + 0x001e);
	src      = data + doffset;
	etk_color_init(&color,rand()%0xffffff,ETK_PIXEL_BGRA32);
	bitmap=etk_bitmap_create(width,height,32,color);
	b=(u8*)bitmap->bits;
	printf("bpp:%d\n",bpp);
	switch(bpp){
	case 32:
			etk_image_bmp_copy32(bitmap, compress, src);
			break;
		case 24:
			etk_image_bmp_copy24(bitmap, compress, src);
			break;
		case 8:
			//etk_image_bmp_copy8(bitmap, compress, palette, src);
			break;
		case 4:
			//etk_image_bmp_copy4(bitmap, compress, palette, src);
			break;
		case 1:
			//etk_image_bmp_copy1(bitmap, compress, palette, src);
			break;
		default:
			break;
	}
	ETK_FREE(buffer);
	return bitmap;
}
