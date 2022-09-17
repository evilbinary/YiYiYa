/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef ARCH_MM_H
#define ARCH_MM_H

#include "boot.h"
#include "libs/include/types.h"

#ifdef ARM
  #ifdef ARMV7_A
    #include "armv7-a/mm.h"
  #elif defined(ARMV7)
    #include "armv7/mm.h"
  #elif defined(ARMV5)
    #include "armv5/mm.h"
  #elif defined(ARMV8_A)
    #include "armv8-a/mm.h"
  #else
    #error "no support arm"
  #endif
#elif defined(X86)
  #include "x86/mm.h"
#elif defined(LX6)
  #include "lx6/mm.h"
#else
#error "no support"
#endif

#define NR_BLOCKS 30
#define MEM_FREE 1
#define MEM_USED 0x10
#define MEM_LOCK 3
#define MEM_SHARE 4
#define MEM_HEAD 5

#define PAGE_SIZE 0x1000

typedef struct mem_block {
  u32 addr;
  u32 type;
  u32 size;
  struct mem_block* next;
} __attribute__((packed)) mem_block_t;

u32* page_alloc_clone(u32* page_dir);
void page_clone(u32* old_page, u32* new_page);


void mm_init();
void* mm_alloc(size_t size);
void mm_free(void* p);
void* mm_alloc_zero_align(size_t size, u32 alignment);
void mm_alloc_init();
void mm_dump_phy();
void mm_dump();

#endif