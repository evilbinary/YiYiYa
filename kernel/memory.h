/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef MEMORY_H
#define MEMORY_H

#include "arch/arch.h"

#define PAGE_SIZE 0x1000

#define MEMORY_FREE 0
#define MEMORY_USE 1
#define MEMORY_SHARE 2
#define MEMORY_HEAP 4
#define MEMORY_EXEC 8
#define MEMORY_DATA 7

#define MEMORY_CREATE_SIZE 4096*10 //40k
#define MEMORY_EXEC_SIZE 4096*100*1024 //400m

#define DATA_ADDR 0x800000
#define HEAP_ADDR 0x820000
#define EXEC_ADDR 0x200000

typedef struct vmemory_area{
    void* vaddr;
    u32 size;
    u8 flags;
    struct vmemory_area* next; 
}vmemory_area_t;

void* kmalloc(size_t size);
void kfree(void* ptr);
void* kmalloc_alignment(size_t size, int alignment);
void kfree_alignment(void* ptr);


void page_clone(u64* page, u64* page_dir_ptr_tab);
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