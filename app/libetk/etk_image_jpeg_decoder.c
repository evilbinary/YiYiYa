#include "etk_image_jpeg_decoder.h"

#include <setjmp.h>
#include <stdlib.h>

#include "etk_global.h"
#include "jpeglib.h"
#include "jerror.h"

#define INPUT_BUF_SIZE 4096*5 /* choose an efficiently fread'able size */

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

void put_scanline_to_bitmap(EtkBitmap* bitmap, char* scanline, int y,
                            int width) {
  int i = 0;
  u8* pixels = (u8*)scanline;
  u8* dst = (u8*)bitmap->bits + y * width;
  while (i < width) {
    dst[0] = pixels[2];
    dst[1] = pixels[1];
    dst[2] = pixels[0];
    i += 1;
    dst += 4;
    pixels += 3;
  }
}
void my_error_exit(j_common_ptr cinfo) {
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr)cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message)(cinfo);

  char buffer[JMSG_LENGTH_MAX];
  /* Create the message */
  (*cinfo->err->format_message)(cinfo, buffer);
  printf("error %s code: %d\n", buffer, cinfo->err->msg_code);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

METHODDEF(void) init_source(j_decompress_ptr cinfo) {
  my_src_ptr src = (my_src_ptr)cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->start_of_file = TRUE;
}
METHODDEF(boolean) fill_input_buffer(j_decompress_ptr cinfo) {
  my_src_ptr src = (my_src_ptr)cinfo->src;
  size_t nbytes;

  nbytes = fread(src->buffer, sizeof(u8), INPUT_BUF_SIZE, src->infile);
  if (nbytes <= 0) {
    if (src->start_of_file) {
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    }
    // WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET)0xFF;
    src->buffer[1] = (JOCTET)JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;

  return TRUE;
}

METHODDEF(boolean) fill_mem_input_buffer(j_decompress_ptr cinfo) {
  static const JOCTET mybuffer[4] = {(JOCTET)0xFF, (JOCTET)JPEG_EOI, 0, 0};

  /* The whole JPEG data is expected to reside in the supplied memory
   * buffer, so any request for more data beyond the given buffer size
   * is treated as an error.
   */
  // WARNMS(cinfo, JWRN_JPEG_EOF);

  /* Insert a fake EOI marker */

  cinfo->src->next_input_byte = mybuffer;
  cinfo->src->bytes_in_buffer = 2;

  return TRUE;
}

METHODDEF(void) skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
  struct jpeg_source_mgr* src = cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0) {
    while (num_bytes > (long)src->bytes_in_buffer) {
      num_bytes -= (long)src->bytes_in_buffer;
      (void)(*src->fill_input_buffer)(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->next_input_byte += (size_t)num_bytes;
    src->bytes_in_buffer -= (size_t)num_bytes;
  }
}

METHODDEF(void) term_source(j_decompress_ptr cinfo) {
  /* no work necessary here */
}

GLOBAL(void) jpeg_stdio_src(j_decompress_ptr cinfo, FILE* infile) {
  my_src_ptr src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) { /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr*)(*cinfo->mem->alloc_small)(
        (j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(my_source_mgr));
    src = (my_src_ptr)cinfo->src;
    src->buffer = (JOCTET*)(*cinfo->mem->alloc_small)(
        (j_common_ptr)cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
  }

  src = (my_src_ptr)cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->infile = infile;
  src->pub.bytes_in_buffer = 0;    /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}

METHODDEF(void) init_mem_source(j_decompress_ptr cinfo) {
  /* no work necessary here */
}

/*
 * Prepare for input from a supplied memory buffer.
 * The buffer must contain the whole JPEG data.
 */

void jpeg_mem_src(j_decompress_ptr cinfo, unsigned char* inbuffer,
                  unsigned long insize) {
  struct jpeg_source_mgr* src;
  if (inbuffer == NULL || insize == 0) {
    // ERREXIT(cinfo, JERR_INPUT_EMPTY);
  }
  /* The source object is made permanent so that a series of JPEG images
   * can be read from the same buffer by calling jpeg_mem_src only before
   * the first one.
   */
  if (cinfo->src == NULL) { /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr*)(*cinfo->mem->alloc_small)(
        (j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
  }

  src = cinfo->src;
  src->init_source = init_mem_source;
  src->fill_input_buffer = fill_mem_input_buffer;
  src->skip_input_data = skip_input_data;
  src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->term_source = term_source;
  src->bytes_in_buffer = (size_t)insize;
  src->next_input_byte = (JOCTET*)inbuffer;
}

EtkBitmap* load_jpeg(const char* filename) {
  EtkBitmap* bitmap = NULL;
  EtkColor bg;
  e32 i = 0;
  e32 quality = 50;
  FILE* infile;      /* source file */
  JSAMPARRAY buffer; /* Output row buffer */
  int row_stride;    /* physical row width in output buffer */
  struct my_error_mgr jerr;
  struct jpeg_decompress_struct cinfo;
  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  /* Step 1: allocate and initialize JPEG decompression object */
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  jpeg_create_decompress(&cinfo);
  /* Step 2: specify data source (eg, a file) */
  jpeg_stdio_src(&cinfo, infile);
  /* Step 3: read file parameters with jpeg_read_header() */
  (void)jpeg_read_header(&cinfo, TRUE);
  /* Step 4: set parameters for decompression */
  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */
  /* Step 5: Start decompressor */
  jpeg_start_decompress(&cinfo);

  printf("output_width:%d cinfo.output_height:%d\n",cinfo.output_width,cinfo.output_height);
  printf("out_color_space:%d\n",cinfo.out_color_space);

  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /*printf("output_width:%d
  cinfo.output_height:%d\n",cinfo.output_width,cinfo.output_height);
  printf("out_color_space:%d\n",cinfo.out_color_space);*/

  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE,
                                      row_stride, 1);

  etk_color_init(&bg, 0xffffff, etkglobal.display->format);
  bitmap = etk_bitmap_create(cinfo.output_width, cinfo.output_height, 32, bg);
  if (bitmap != NULL) {
    i = 0;
    int j = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
      jpeg_read_scanlines(&cinfo, buffer, 1);
      // for(j=0;j<cinfo.output_width;j++){
      // 	printf("%x ",((char*)buffer[0])[j]);
      // }
      // printf("\n");
      put_scanline_to_bitmap(bitmap, (char*)buffer[0], i, cinfo.output_width);
      i += 4;
    }
    printf("\n\n\n");

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
  } else {
    // destroy
  }
  return bitmap;
}

EtkBitmap* load_jpeg_to_bitmap(EtkBitmap* bitmap, const char* filename) {
  e32 i = 0;
  e32 quality = 50;
  FILE* infile;      /* source file */
  JSAMPARRAY buffer; /* Output row buffer */
  int row_stride;    /* physical row width in output buffer */
  struct my_error_mgr jerr;
  struct jpeg_decompress_struct cinfo;
  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  /* Step 1: allocate and initialize JPEG decompression object */
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  jpeg_create_decompress(&cinfo);
  /* Step 2: specify data source (eg, a file) */
  jpeg_stdio_src(&cinfo, infile);
  /* Step 3: read file parameters with jpeg_read_header() */
  (void)jpeg_read_header(&cinfo, TRUE);
  /* Step 4: set parameters for decompression */
  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */
  /* Step 5: Start decompressor */
  jpeg_start_decompress(&cinfo);

  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /*printf("output_width:%d
  cinfo.output_height:%d\n",cinfo.output_width,cinfo.output_height);
  printf("out_color_space:%d\n",cinfo.out_color_space);*/

  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE,
                                      row_stride, 1);

  if (bitmap != NULL) {
    i = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
      jpeg_read_scanlines(&cinfo, buffer, 1);
      put_scanline_to_bitmap(bitmap, (char*)buffer[0], i, cinfo.output_width);
      i += 4;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
  } else {
    // destroy
  }
  return bitmap;
}

EtkBitmap* etk_image_jpeg_decoder_decode(EtkImageDecoder* thiz,
                                         const char* filename) {
  return load_jpeg(filename);
  // return load_jpeg1(filename);
}
void etk_image_jpeg_decoder_destroy(EtkImageDecoder* thiz) {
  if (thiz != NULL) {
    ETK_FREE(thiz);
  }
}
EtkImageDecoder* etk_image_jpeg_decoder_create(void) {
  EtkImageDecoder* thiz = (EtkImageDecoder*)ETK_MALLOC(sizeof(EtkImageDecoder));
  if (thiz != NULL) {
    thiz->decode = etk_image_jpeg_decoder_decode;
    thiz->destroy = etk_image_jpeg_decoder_destroy;
  }
  return thiz;
}
