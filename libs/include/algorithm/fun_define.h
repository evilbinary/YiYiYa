/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef FN_DEFINE_H
#define FN_DEFINE_H

#include "types.h"

#ifndef fn_malloc
#define fn_malloc kmalloc
#endif

#ifndef fn_free
#define fn_free kfree
#endif


#endif
