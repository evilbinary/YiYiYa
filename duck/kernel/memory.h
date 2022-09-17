/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef MEMORY_H
#define MEMORY_H

#include "arch/arch.h"



#define MEMORY_FREE 0
#define MEMORY_USE 1
#define MEMORY_SHARE 2
#define MEMORY_HEAP 3
#define MEMORY_EXEC 4
#define MEMORY_DATA 5
#define MEMORY_STACK 6

#define MEMORY_HEAP_SIZE 4096*1024*100 //100m
#define MEMORY_EXEC_SIZE 4096*1024*100 //100m

#define KERNEL_POOL_NUM 20
#define USER_POOL_NUM 20

typedef struct vmemory_area{
    void* vaddr;
    void* vend;
    u32 size;
    u8 flags;
    struct vmemory_area* next; 
}vmemory_area_t;


#ifdef MALLOC_TRACE
#define kmalloc(size) kmalloc_trace(size, __FILE__, __LINE__, __FUNCTION__)
#define kmalloc_alignment(size,alignment) kmalloc_alignment_trace(size,alignment, __FILE__, __LINE__, __FUNCTION__)
#else
void* kmalloc(size_t size);
void* kmalloc_alignment(size_t size, int alignment);
#endif

void kfree(void* ptr);
void kfree_alignment(void* ptr);

void map_alignment(void* page, void* vaddr, void* buf, u32 size);


void* valloc(void* addr, size_t size);
void vfree(void* addr);

void kpool_init();
void* kpool_poll();
int kpool_put(void* e);

void use_kernel_page();
void use_user_page();

void* virtual_to_physic(u64* page_dir_ptr_tab, void* vaddr);

void memory_init();

vmemory_area_t* vmemory_area_create(void* addr,u32 size,u8 flags);
vmemory_area_t* vmemory_area_destroy(vmemory_area_t* area);

vmemory_area_t* vmemory_area_alloc(vmemory_area_t* areas,void* addr, u32 size);
vmemory_area_t* vmemory_area_find(vmemory_area_t* areas,void* addr, size_t size);
void vmemory_area_add(vmemory_area_t* areas,vmemory_area_t* area);
void vmemory_area_free(vmemory_area_t* area);


#endif