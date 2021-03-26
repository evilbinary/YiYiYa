#ifndef _ETK_UTIL_H
#define _ETK_UTIL_H
#include "etk_bitmap.h"

EtkBitmap* load_bmp(const char *filename);
void read_file_to_buffer(char* buffer,char* filename,int size);
Ret etk_image_bmp_copy24(EtkBitmap* bitmap, int compress, const unsigned char* data);
Ret etk_image_bmp_copy32(EtkBitmap* bitmap, int compress, const unsigned char* data);

#endif