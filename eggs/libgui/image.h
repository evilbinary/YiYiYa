#ifndef IMAGE_H
#define IMAGE_H

#include "types.h"

typedef struct bitmap {
  i32 w, h;
  i32 length;
  i32 bitperpixel;
  void* bits;
  i32 alpha_flag;
} bitmap_t;

void* bitmap_create(i32 w, i32 h, u32 bitperpixel, u32 color);


#endif