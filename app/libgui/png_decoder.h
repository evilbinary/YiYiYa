#ifndef PNG_DECODER_H
#define PNG_DECODER_H

#include "screen.h"
#include "setjmp.h"
#include "stdio.h"
#include "stdlib.h"
#include "image.h"

bitmap_t* load_png(const char* filename);


#endif