#ifndef _ETK_PLATFORM_H
#define _ETK_PLATFORM_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "etk_event.h"

#ifdef WIN32
#include "etk_win32.h"
#elif defined LINUX
#include "etk_linux.h"
#elif defined LINUX
#incldue "etk_ucosii.h"
#else

#endif

#if defined(SDL2)|| defined(SDL)
#include "etk_display_sdl.h"
#include "etk_source_sdl.h"
#elif defined( LCD)
#incldue "etk_display_lcd.h"
#include "etk_source_touch.h"
#endif


#define ETK_MALLOC(s) malloc(s)
#define ETK_REALLOC(p, s)  realloc(p,s)
#define ETK_CALLOC(s)      calloc(s)
#define ETK_FREE(p)        if(p) {free(p); p = NULL;}

#define ETK_NEW(s)	(*s) malloc(sizeof(s))
#define ETK_DELETE(s)	free(s)

#define ETK_ZALLOC(s)      malloc(s)
#define ETK_ZFREE(p, size) if(p) {memset((p), 0x00, (size)); free(p); p = NULL;}


#define ETK_EX_MALLOC(s) malloc(s);
#define ETK_EX_FREE(p)  free(p);


size_t etk_get_relative_time(void);
Ret etk_platform_event_poll(EtkEvent *event);
int etk_platform_init(int argc, char** argv);
void etk_platform_deinit(void);

#endif