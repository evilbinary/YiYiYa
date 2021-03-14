#ifndef JPEG_DECODER_H
#define JPEG_DECODER_H

#include "../jpeg/jpeglib.h"
#include "screen.h"
#include "setjmp.h"
#include "stdio.h"
#include "stdlib.h"

#define INPUT_BUF_SIZE 4096 /* choose an efficiently fread'able size */

struct my_error_mgr {
  struct jpeg_error_mgr pub; /* "public" fields */
  jmp_buf setjmp_buffer;     /* for return to caller */
};
typedef struct my_error_mgr* my_error_ptr;

typedef struct {
  struct jpeg_source_mgr pub; /* public fields */
  FILE* infile;               /* source stream */
  JOCTET* buffer;             /* start of buffer */
  boolean start_of_file;      /* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr* my_src_ptr;

void put_scanline_to_bitmap(bitmap_t* bitmap, char* scanline, int y,
                            int width);
void* bitmap_create(i32 w, i32 h, i32 bitperpixel, u32 color);
bitmap_t* load_jpeg(const char* filename);


#endif