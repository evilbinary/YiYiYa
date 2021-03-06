/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef MM_H
#define MM_H

#include "libs/include/types.h"
#include "boot.h"


#define NR_BLOCKS   30
#define MEM_FREE	1
#define MEM_USED	0x10
#define MEM_LOCK	3
#define MEM_SHARE	4
#define MEM_HEAD	5

#define PAGE_P   1
#define PAGE_RWR   0 //读执行
#define PAGE_RWW   2 //读/写/执行
#define PAGE_USS   0 //系统级
#define PAGE_USU   4 //用户级

typedef struct mem_block{
    u32 addr;
	u32 size;
	u32 type;
	struct mem_block * next;
} mem_block_t;


typedef u64* page_dir_t;

void mm_init();
void* mm_alloc(size_t size);
void mm_free(void* p);
void* mm_alloc_zero_align(size_t size,int alignment);
void map_page_on(page_dir_t* page,u32 virtualaddr, u32 physaddr, u32 flags);

#endif