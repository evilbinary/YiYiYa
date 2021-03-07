/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "memory.h"

#include "thread.h"
void* kmalloc(size_t size) {
  void* addr = NULL;
  size = ((size + PAGE_SIZE) / PAGE_SIZE) * PAGE_SIZE;
  // size=((size+1024)/1024)*1024;
  addr = mm_alloc(size);
  if (addr == NULL) {
    kprintf("kmalloc error\n");
    return addr;
  }
  memset(addr, 0, size);
  return addr;
}

void kfree(void* ptr) { mm_free(ptr); }

void* kmalloc_alignment(size_t size, int alignment) {
  // size=((size+4096)/4096)*4096;
  return mm_alloc_zero_align(size, alignment);
}

void kfree_alignment(void* ptr) { mm_free_align(ptr); }

void* valloc(void* addr, size_t size) {
  thread_t* current = thread_current();
  if(size<PAGE_SIZE){
    size=PAGE_SIZE;
  }
  void* vaddr=addr;
  void* phy_addr=kmalloc(size);
  void* paddr = phy_addr;
  for (int i = 0; i < size / PAGE_SIZE; i++) {
    if (current != NULL) {
      map_page_on(current->context.page_dir, vaddr, paddr,
                  PAGE_P | PAGE_USU | PAGE_RWW);
    } else {
      map_page(vaddr, paddr, PAGE_P | PAGE_USU | PAGE_RWW);
    }
    kprintf("vmap vaddr:%x paddr:%x\n", vaddr, paddr);
    vaddr+=PAGE_SIZE;
    paddr+=PAGE_SIZE;
  }
  return addr;
}

void vfree(void* addr) {}

void map_2gb(u64* page_dir_ptr_tab) {
  u32 addr = 0;
  for (int i = 0; i < 0x200000 / PAGE_SIZE; i++) {
    map_page_on(page_dir_ptr_tab, addr, addr, PAGE_P | PAGE_USU | PAGE_RWW);
    addr += PAGE_SIZE;
  }
}

void map_alignment(void* page, void* vaddr, void* buf, u32 size) {
  u32 file_4k = PAGE_SIZE;
  if (size > PAGE_SIZE) {
    file_4k = size;
  }
  for (int i = 0; i < file_4k / PAGE_SIZE; i++) {
    kprintf("map_alignment vaddr:%x paddr:%x\n", vaddr, buf);
    map_page_on(page, (u32)vaddr, (u32)buf, PAGE_P | PAGE_USU | PAGE_RWW);
    vaddr += PAGE_SIZE;
    buf += PAGE_SIZE;
  }
}

void page_clone(u64* page, u64* page_dir_ptr_tab) {
  for (int pdpte_index = 0; pdpte_index < 4; pdpte_index++) {
    u64* page_dir_ptr = page[pdpte_index] & ~0xFFF;
    if (page_dir_ptr != NULL) {
      u64* new_page_dir_ptr = kmalloc_alignment(sizeof(u64) * 512, PAGE_SIZE);
      page_dir_ptr_tab[pdpte_index] =
          ((u64)new_page_dir_ptr) | PAGE_P | PAGE_USU | PAGE_RWW;
      for (int pde_index = 0; pde_index < 512; pde_index++) {
        u64* page_tab_ptr = (u64)page_dir_ptr[pde_index] & ~0xFFF;
        if (page_tab_ptr != NULL) {
          u64* new_page_tab_ptr =
              kmalloc_alignment(sizeof(u64) * 512, PAGE_SIZE);
          new_page_dir_ptr[pde_index] =
              ((u64)new_page_tab_ptr) | PAGE_P | PAGE_USU | PAGE_RWW;
          for (int pte_index = 0; pte_index < 512; pte_index++) {
            u64* page_tab_item = page_tab_ptr[pte_index] & ~0xFFF;
            if (page_tab_item != NULL) {
              new_page_tab_ptr[pte_index] = page_tab_ptr[pte_index];
              u32 virtualaddr = (pdpte_index & 0x03) << 30 |
                                (pde_index & 0x01FF) << 21 |
                                (pte_index & 0x01FF) << 12;
              // kprintf("map vir:%x phy:%x \n", virtualaddr,
              // new_page_tab_ptr[pte_index]& ~0xFFF);
            }
          }
        }
      }
    }
  }
}

void* virtual_to_physic(u64* page_dir_ptr_tab, void* vaddr){
  u32 pdpte_index = (u32)vaddr >> 30 & 0x03;
  u32 pde_index = (u32)vaddr >> 21 & 0x01FF;
  u32 pte_index = (u32)vaddr >> 12 & 0x01FF;
  u32 offset = (u32)vaddr & 0x0FFF;
  u64* page_dir_ptr = (u64)page_dir_ptr_tab[pdpte_index] & ~0xFFF;
  if (page_dir_ptr == NULL) {
    kprintf("page dir find errro\n");
    return NULL;
  }
  u64* page_tab_ptr = (u64)page_dir_ptr[pde_index] & ~0xFFF;
  if (page_tab_ptr == NULL) {
    kprintf("page tab find errro\n");
  }
  void* phyaddr=page_tab_ptr[pte_index]& ~0xFFF;
  return phyaddr;
}