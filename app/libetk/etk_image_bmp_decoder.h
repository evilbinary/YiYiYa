#ifndef _ETK_IMAGE_BMP_DECODER_H
#define _ETK_IMAGE_BMP_DECODER_H
#include "etk_image_decoder.h"

EtkImageDecoder* etk_image_bmp_decoder_create(void);
EtkBitmap* etk_image_bmp_decoder(EtkImageDecoder *thiz,const e8 *filename);

#endif