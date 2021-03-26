#ifndef _ETK_MAIN_LOOP_H
#define _ETK_MAIN_LOOP_H

#include "etk_type.h"
#include "etk_platform.h"

struct EtkMainLoop_t;
typedef struct EtkMainLoop_t EtkMainLoop;

struct EtkMainLoop_t {
  int running;
};

EtkMainLoop* etk_main_loop_create();
Ret etk_main_loop_run(EtkMainLoop* thiz);
Ret etk_main_loop_quit(EtkMainLoop* thiz);
void etk_main_loop_destroy(EtkMainLoop* thiz);

EtkMainLoop* etk_default_main_loop(void);

#endif
