/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef MEMORY_H
#define MEMORY_H

#include "arch/arch.h"

#define PAGE_SIZE 0x1000

void* kmalloc(size_t size);

void kfree(void* ptr);

void* kmalloc_alignment(size_t size, int alignment);
void kfree_alignment(void* ptr);


void page_clone(u64* page, u64* page_dir_ptr_tab);
void map_alignment(void* page, void* vaddr, void* buf, u32 size);


void* valloc(void* addr, size_t size);
void vfree(void* addr);

#endif