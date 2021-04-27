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
    printf("not png file\n");
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
  nc = png_get_channels(png_ptr, info_ptr);
  w = png_get_image_width(png_ptr, info_ptr);
  h = png_get_image_height(png_ptr, info_ptr);
  bpc = png_get_bit_depth(png_ptr, info_ptr);
  printf(
      "PNG Image: width = %u, height = %u, %u b/channel in %u channels. "
      "Dumping data.\n",
      w, h, bpc, nc);

  png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
  bitmap_t* bitmap = NULL;
  int size;
  int pos = 0;
  int temp;
  bitmap = bitmap_create(w, h, bpc * 4, 0xffffff);
  if (bitmap == NULL) {
    printf("bitmap_create failed\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    fclose(fp);
    return NULL;
  }
  u8* p = bitmap->bits;
  png_bytep buff = p;
  switch (color_type) {
    case PNG_COLOR_TYPE_RGB_ALPHA:
      bitmap->alpha_flag = 1;
      for (u32 y = 0; y < h; ++y) {
        for (u32 x = 0; x < w * 4;) {
          *p++ = row_pointers[y][x++];  // red
          *p++ = row_pointers[y][x++];  // green
          *p++ = row_pointers[y][x++];  // blue
          *p++ = row_pointers[y][x++];  // alpha
        }
      }
      break;
    case PNG_COLOR_TYPE_RGB:
      bitmap->alpha_flag = 0;
      for (u32 y = 0; y < h; y++) {
        for (u32 x = 0; x < w; x++) {
          buff[y * w + 3 * x + 0] = row_pointers[y][3 * x + 0];
          buff[y * w + 3 * x + 1] = row_pointers[y][3 * x + 1];
          buff[y * w + 3 * x + 2] = row_pointers[y][3 * x + 2];
        }
      }
      break;
      /* 其它色彩类型的图像就不读了 */
    default:
      fclose(fp);
      png_destroy_read_struct(&png_ptr, &info_ptr, 0);
      printf("default color_type:close\n");
      return 0;
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, 0);
  fclose(fp);
  return bitmap;
}