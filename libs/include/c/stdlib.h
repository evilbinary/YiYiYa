/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef STDLIB_H
#define STDLIB_H
#include "stdint.h"

void itoa(char* buf, int base, int d);
void abort();
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
double    strtod(const char *, char **);

#endif