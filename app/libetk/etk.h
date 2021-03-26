#ifndef _ETK_H
#define _ETK_H


#include "etk_type.h"
#include "etk_widget.h"
#include "etk_window.h"
#include "etk_wnd_manager.h"
#include "etk_image_decoder.h"


#define DISAPLAY_HEIGHT 768
#define DISAPLAY_WIDTH 1024

void etk_init_all(void);
Ret etk_init(int argc,char* argv[]);
Ret etk_run(void);
int etk_exit(void);

#endif