/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "cpu.h"
#include "../mm.h"


extern boot_info_t* boot_info;
extern mem_block_t* block_available;

u64 page_dir_ptr_tab[4] __attribute__((aligned(0x20)));
u64 page_dir[512] __attribute__((aligned(0x1000)));
u64 page_tab[512] __attribute__((aligned(0x1000)));

void map_page(u32 virtualaddr, u32 physaddr, u32 flags) {
  u32 pdpte_index = (u32)virtualaddr >> 30 & 0x03;
  u32 pde_index = (u32)virtualaddr >> 21 & 0x01FF;
  u32 pte_index = (u32)virtualaddr >> 12 & 0x01FF;
  u32 offset = (u32)virtualaddr & 0x0FFF;
  // kprintf("pdpte_index=>%d pde_index=%d  pte_index=%d\n", pdpte_index,
  //         pde_index, pte_index);

  u64* page_dir_ptr = (u64)page_dir_ptr_tab[pdpte_index] & ~0xFFF;
  if (page_dir_ptr == NULL) {
    page_dir_ptr = mm_alloc_zero_align(sizeof(u64) * 512, 0x1000);
    page_dir_ptr_tab[pdpte_index] = ((u64)page_dir_ptr) | PAGE_P;
  }
  u64* page_tab_ptr = (u64)page_dir_ptr[pde_index] & ~0xFFF;
  if (page_tab_ptr == NULL) {
    page_tab_ptr = mm_alloc_zero_align(sizeof(u64) * 512, 0x1000);
    page_dir_ptr[pde_index] = (u64)page_tab_ptr | flags;
  }
  page_tab_ptr[pte_index] = (u64)physaddr & ~0xFFF | flags;
  // page_tab_ptr[offset] = (u32)physaddr | (flags & 0xFFF);
}

void map_page_on(page_dir_t* page, u32 virtualaddr, u32 physaddr, u32 flags) {
  u32 pdpte_index = (u32)virtualaddr >> 30 & 0x03;
  u32 pde_index = (u32)virtualaddr >> 21 & 0x01FF;
  u32 pte_index = (u32)virtualaddr >> 12 & 0x01FF;
  u32 offset = (u32)virtualaddr & 0x0FFF;
  // kprintf("pdpte_index=>%d pde_index=%d  pte_index=%d\n", pdpte_index,
  //         pde_index, pte_index);

  u64* page_dir_ptr = (u64)page[pdpte_index] & ~0xFFF;
  if (page_dir_ptr == NULL) {
    page_dir_ptr = mm_alloc_zero_align(sizeof(u64) * 512, 0x1000);
    page[pdpte_index] = ((u64)page_dir_ptr) | PAGE_P;
  }
  u64* page_tab_ptr = (u64)page_dir_ptr[pde_index] & ~0xFFF;
  if (page_tab_ptr == NULL) {
    page_tab_ptr = mm_alloc_zero_align(sizeof(u64) * 512, 0x1000);
    page_dir_ptr[pde_index] = (u64)page_tab_ptr | flags;
  }
  page_tab_ptr[pte_index] = (u64)physaddr & ~0xFFF | flags;
  // page_tab_ptr[offset] = (u32)physaddr | (flags & 0xFFF);
}

void unmap_page_on(page_dir_t* page, u32 virtualaddr) {
  u32 pdpte_index = (u32)virtualaddr >> 30 & 0x03;
  u32 pde_index = (u32)virtualaddr >> 21 & 0x01FF;
  u32 pte_index = (u32)virtualaddr >> 12 & 0x01FF;
  u32 offset = (u32)virtualaddr & 0x0FFF;
  // kprintf("pdpte_index=>%d pde_index=%d  pte_index=%d\n", pdpte_index,
  //         pde_index, pte_index);

  u64* page_dir_ptr = (u64)page[pdpte_index] & ~0xFFF;
  if (page_dir_ptr != NULL) {
    page[pdpte_index] = 0;
  }
  u64* page_tab_ptr = (u64)page_dir_ptr[pde_index] & ~0xFFF;
  if (page_tab_ptr != NULL) {
    page_dir_ptr[pde_index] = 0;
  }
  if (page_tab_ptr != NULL) {
    page_tab_ptr[pte_index] = 0;
  }
  if (page_dir_ptr != NULL) {
    mm_free_align(page_dir_ptr);
  }
  if (page_tab_ptr != NULL) {
    mm_free_align(page_tab_ptr);
  }
}

void mm_test() {
  // map_page(0x90000,0x600000,3);
  // 0xfd000000
  // map_page(0x1ff000,0x400000,PAGE_P | PAGE_USU | PAGE_RWW);
  // map_page(0xfd000000, 0xfd000000, PAGE_P | PAGE_USU | PAGE_RWW);

  // u32 addr = 0x100000;
  // for (int i = 0; i < 100; i++) {
  //   map_page(addr, addr, PAGE_P | PAGE_USU | PAGE_RWW);
  //   addr += 0x1000;
  // }
}

void map_mem_block(u32 addr) {
  mem_block_t* p = block_available;
  for (; p != NULL; p = p->next) {
    if (p->addr > addr) {
      // map_page(p->addr, p->addr, PAGE_P | PAGE_USU | PAGE_RWW);
      u32 address = p->addr;
      for (int i = 0; i < p->size / 0x1000; i++) {
        map_page(address, address, PAGE_P | PAGE_USU | PAGE_RWW);
        // kprintf("map addr %x %x\n", address, address);
        address += 0x1000;
      }
    }
  }
}

void unmap_mem_block(u32* page, u32 addr) {
  for (int i = 0; i < boot_info->memory_number; i++) {
    memory_info_t* mem = (memory_info_t*)&boot_info->memory[i];
    if (mem->type != 1) {  // normal ram
      continue;
    }
    if (mem->base > addr) {
      u32 address = mem->base;
      for (int i = 0; i < mem->length / 0x1000; i++) {
        unmap_page_on(page, address);
        kprintf("unmap addr %x %x\n", address, address);
        address += 0x1000;
      }
    }
  }
}

void mm_init_default() {
  boot_info->pdt_base = (ulong)page_dir_ptr_tab;
  boot_info->page_type = 1;  // pae mode

  // set zero
  for (int i = 0; i < 4; i++) {
    page_dir_ptr_tab[i] = 0;
  }
  page_dir_ptr_tab[0] = (u32)page_dir | PAGE_P;

  page_dir[0] = (u32)page_tab | (PAGE_P | PAGE_USU | PAGE_RWW);
  // map 0-0x200000 2GB
  unsigned int i, address = 0;
  kprintf("start 0x%x ", address);
  for (i = 0; i < 512; i++) {
    page_tab[i] = address | (PAGE_P | PAGE_USU | PAGE_RWW);
    address = address + 0x1000;
  }
  kprintf("- 0x%x\n", address);  // 0x200000 2GB

  // map > 4GB addr
  map_mem_block(address);

  if (boot_info->pdt_base != NULL) {
    ulong addr = (ulong)boot_info->pdt_base;
    cpu_enable_paging_pae(addr);
    kprintf("paging pae scucess\n");
  }

}

void* virtual_to_physic(u64* page_dir_ptr_tab, void* vaddr) {
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
    // kprintf("page tab find errro\n");
    return NULL;
  }
  void* phyaddr = page_tab_ptr[pte_index] & ~0xFFF;
  // kprintf("virtual_to_physic %x\n",phyaddr);
  return phyaddr;
}

void page_clone(u32* old_page, u32* new_page) {
  u64* page=old_page;
  u64* page_dir_ptr_tab=new_page;
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


u32* page_alloc_clone(u32* old_page_dir){
  u32* page_dir_ptr_tab = kmalloc_alignment(sizeof(u64) * 4, 0x1000);
  page_clone(old_page_dir, page_dir_ptr_tab);
  return page_dir_ptr_tab;
}