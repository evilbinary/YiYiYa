#include "image.h"

void* bitmap_create(i32 w, i32 h, u32 bitperpixel, u32 color) {
  u32 i;
  u32* p32 = NULL;
  u16* p16 = NULL;
  u32 length = w * (h + 5);
  bitmap_t* bitmap = (bitmap_t*)malloc(sizeof(bitmap_t));
  bitmap->bits = (void*)malloc(length * (bitperpixel >> 3));
  bitmap->w = w;
  bitmap->h = h;
  bitmap->length = length;
  bitmap->bitperpixel = bitperpixel;
  printf("bitperpixel:%d size:%d length:%d\n",bitperpixel,length*(bitperpixel>>3),length);
  if(color==0) return bitmap;
  switch (bitperpixel) {
    case 16:
      p16 = (u16*)bitmap->bits;
      for (i = 0; i < length; i++) {
        *p16 = (u16)color;
        p16++;
      }
      break;
    case 32:
      p32 = (u32*)bitmap->bits;
      for (i = 0; i < length; i++) {
        *p32 = color;
        p32++;
      }
      break;
  }

  return bitmap;
}