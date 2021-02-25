/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "kmalloc.h"

void* kmalloc(size_t size){
	void *addr=NULL;
	size=((size+4096)/4096)*4096;
	// size=((size+1024)/1024)*1024;
	addr= mm_alloc(size);
	if(addr==NULL){
		kprintf("kmalloc error\n");
		return addr;
	}
    memset(addr,0,size);
	return addr;
}

void kfree(void* ptr){
     mm_free(ptr);
}