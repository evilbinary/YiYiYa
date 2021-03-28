#include <setjmp.h>
#include <stdlib.h>
#include "stdio.h"
#include "etk_image_png_decoder.h"

EtkBitmap* load_png(const char* filename) {
  EtkBitmap* bitmap = NULL;
  EtkColor bg;
  e32 i = 0;
  e32 quality = 50;
  FILE* infile;      /* source file */
  int row_stride;    /* physical row width in output buffer */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  // buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE,
  //                                     row_stride, 1);

  // etk_color_init(&bg, 0xffffff, etkglobal.display->format);
  // bitmap = etk_bitmap_create(cinfo.output_width, cinfo.output_height, 32, bg);
  // if (bitmap != NULL) {
  //   i = 0;
  //   int j = 0;
  //   while (cinfo.output_scanline < cinfo.output_height) {
  //     jpeg_read_scanlines(&cinfo, buffer, 1);

  //     put_scanline_to_bitmap(bitmap, (char*)buffer[0], i, cinfo.output_width);
  //     i += 4;
  //   }
  //   jpeg_finish_decompress(&cinfo);
  //   jpeg_destroy_decompress(&cinfo);
  //   fclose(infile);
  // } else {
  //   // destroy
  // }
  return bitmap;
}

EtkBitmap* etk_image_png_decoder_decode(EtkImageDecoder* thiz,
                                        const char* filename) {
  return load_png(filename);
}
void etk_image_png_decoder_destroy(EtkImageDecoder* thiz) {
  if (thiz != NULL) {
    ETK_FREE(thiz);
  }
}
EtkImageDecoder* etk_image_png_decoder_create(void) {
  EtkImageDecoder* thiz = (EtkImageDecoder*)ETK_MALLOC(sizeof(EtkImageDecoder));
  if (thiz != NULL) {
    thiz->decode = etk_image_png_decoder_decode;
    thiz->destroy = etk_image_png_decoder_destroy;
  }
  return thiz;
}
