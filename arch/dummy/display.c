/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../display.h"
#include "../boot.h"
#include "../io.h"
#include "../lock.h"


extern boot_info_t* boot_info;

void display_init() {
  kprintf("Hello,YiYiYa OS\n");
  if (boot_info->disply.mode == 1) {
  }
}
