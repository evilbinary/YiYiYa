#include "png_decoder.h"
#include <errno.h>
#include "image.h"
#include "png.h"

bitmap_t* load_png(const char* filename) {
  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    printf("error opening file '%s' (error %u: %s).\n", filename, errno,
           strerror(errno));
    return 1;
  }
  char header[8];
  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8) != 0) {
    return NULL;
  }
  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return NULL;
  }
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  unsigned int h, w, i, j, k, bpc, nc, color_type;

  color_type = png_get_color_type(png_ptr, info_ptr);

  printf(
      "PNG Image: width = %u, height = %u, %u b/channel in %u channels. "
      "Dumping data.\n",
      w = png_get_image_width(png_ptr, info_ptr),
      h = png_get_image_height(png_ptr, info_ptr),
      bpc = png_get_bit_depth(png_ptr, info_ptr),
      nc = png_get_channels(png_ptr, info_ptr));

  png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
  bitmap_t* bitmap = NULL;
  int size;
  int pos = 0;
  int temp;
  bitmap = bitmap_create(w, h, nc * (sizeof(u8)), 0xffffff);
  if (bitmap == NULL) {
    printf("bitmap_create failed\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    fclose(fp);
    return NULL;
  }
  u8* p = bitmap->bits;
  if (nc == 4 || color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
    bitmap->alpha_flag = 1;
    temp = nc - 1;
    for (i = 0; i < bitmap->h; i++) {
      for (j = 0; j < bitmap->w * 4; j += 4) {
        for (k = temp; k >= 0; k--) {
          p[pos++] = row_pointers[i][j + k];
        }
      }
    }
  } else if (nc == 3 ||
             color_type == PNG_COLOR_TYPE_RGB) {
    bitmap->alpha_flag = 0;
    temp = (3 * bitmap->w);
    for (i = 0; i < bitmap->h; i++) {
      for (j = 0; j < temp; j += 3) {
        p[pos++] = row_pointers[i][j + 2];
        p[pos++] = row_pointers[i][j + 1];
        p[pos++] = row_pointers[i][j + 0];
      }
    }
  } else {
    printf("error not support channel\n");
    return NULL;
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, 0);
  fclose(fp);
  return bitmap;
}