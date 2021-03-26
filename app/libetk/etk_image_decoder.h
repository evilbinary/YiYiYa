#ifndef _ETK_IMAGE_DECODER_H
#define _ETK_IMAGE_DECODER_H
#include "etk_bitmap.h"

struct EtkImageDecoder_t;
typedef struct EtkImageDecoder_t EtkImageDecoder;

//typedef Ret       (*EtkImageDecoderMatch)(EtkImageDecoder* thiz, const char* filename);
typedef EtkBitmap* (*EtkImageDecoderDecode)(EtkImageDecoder* thiz, const char* filename);
typedef void      (*EtkImageDecoderDestroy)(EtkImageDecoder* thiz);

struct EtkImageDecoder_t{
	//EtkImageDecoderMatch   match;
	EtkImageDecoderDecode  decode;
	EtkImageDecoderDestroy destroy;
	void *priv;
};


//Ret etk_image_decoder_match(EtkImageDecoder* thiz, const char* filename);
EtkBitmap* etk_image_decoder_decode(EtkImageDecoder* thiz, const char* filename);
void etk_image_decoder_destroy(EtkImageDecoder* thiz);

#endif