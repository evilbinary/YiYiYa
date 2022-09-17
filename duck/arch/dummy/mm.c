/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "arch/mm.h"
#include "arch/cpu.h"
#include "arch/display.h"
#include "cpu.h"

#define PAGE_DIR_NUMBER 4096 * 4

extern boot_info_t* boot_info;
static u32 page_dir[PAGE_DIR_NUMBER] __attribute__((aligned(0x4000)));

void map_page_on(page_dir_t* l1, u32 virtualaddr, u32 physaddr, u32 flags) {
  
}

void map_page(u32 virtualaddr, u32 physaddr, u32 flags) {
  map_page_on(boot_info->pdt_base, virtualaddr, physaddr, flags);
}

void mm_init_default() {
  // init map
  kprintf("map page end\n");
 
  //init page
  kprintf("enable page\n");

  kprintf("paging pae scucess\n");
}


void* virtual_to_physic(void* page, void* vaddr) {
  void* phyaddr = NULL;
 
  return phyaddr;
}

void page_clone(u32* old_page, u32* new_page) {
  // kprintf("page_clone:%x %x\n",old_page,new_page);
  if (old_page == NULL) {
    kprintf("page clone error old page null\n");
    return;
  }
  
}

u32* page_alloc_clone(u32* kernel_page_dir) {
  u32* page_dir_ptr_tab =
      mm_alloc_zero_align(sizeof(u32) * PAGE_DIR_NUMBER, 0x4000);
  page_clone(kernel_page_dir, page_dir_ptr_tab);
  return page_dir_ptr_tab;
}

void unmap_page_on(page_dir_t* page, u32 virtualaddr) {
  
}

