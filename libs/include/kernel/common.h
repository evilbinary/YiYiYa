/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef COMMON_H
#define COMMON_H

#include "libs/include/types.h"
#include "io.h"


#ifndef _HAVE_OFF_T
#define _HAVE_OFF_T
typedef	long int		off_t;
#endif


void* kmemcpy(void* /* restrict */ s1, const void* /* restrict */ s2, size_t n);
void* kmemset(void* s, int c, size_t n);



#endif