#ifndef _MALLOC_H
#define _MALLOC_H


#include "stddef.h"

void *malloc(unsigned long size);
void free(void *addr);
void * realloc (void *block,unsigned long size);



#endif
