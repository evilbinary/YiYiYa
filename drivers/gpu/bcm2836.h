#ifndef BCM2836_H
#define BCM2836_H

#include "kernel/kernel.h"

#include "mailbox.h"


#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8

#define COLORDEPTH 24
#define BYTES_PER_PIXEL COLORDEPTH/8

typedef struct pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pixel_t;


#endif
