/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "arch/mm.h"

#include "arch/cpu.h"
#include "arch/display.h"
#include "cpu.h"
#include "gpio.h"

#define PAGE_DIR_NUMBER 1024 *1

extern boot_info_t* boot_info;
static u32 page_dir[PAGE_DIR_NUMBER] __attribute__((aligned(0x100)));

void map_page_on(page_dir_t* l1, u32 virtualaddr, u32 physaddr, u32 flags) {
  u32 l1_index = virtualaddr >> 20;
  u32 l2_index = virtualaddr >> 12 & 0xFF;
  u32* l2 = ((u32)l1[l1_index]) & 0xFFFFFC00;
  if (l2 == NULL) {
    l2 = mm_alloc_zero_align(0x1000, 0x1000);
    kmemset(l2, 0, 0x1000);
    l1[l1_index] = (((u32)l2) & 0xFFFFFC00) | L1_DESC;
  }
  l2[l2_index] = ((physaddr >> 12) << 12) | L2_DESC | flags;
}

void map_page(u32 virtualaddr, u32 physaddr, u32 flags) {
  map_page_on(boot_info->pdt_base, virtualaddr, physaddr, flags);
}

void mm_init_default() {
  kprintf("mem init default\n");
  // mm_test();
  // boot_info->pdt_base = page_dir;
  // kmemset(page_dir, 0, 4096 * 8);

  // u32 address = 0;
  // kprintf("map %x - %x\n", address, 0x1000 * 1024 * 10);
  // for (int j = 0; j < 1024 * 10; j++) {
  //   map_page(address, address, 0);
  //   address += 0x1000;
  // }
  // address = boot_info->kernel_entry;
  // kprintf("map kernel %x ", address);
  // int i;
  // for (i = 0; i < (((u32)boot_info->kernel_size) / 0x1000 + 6); i++) {
  //   map_page(address, address, L2_TEXT_1 | L2_CB);
  //   address += 0x1000;
  // }
  // kprintf("- %x\n", address);
  
  // kprintf("map page end\n");

  // // cpu_disable_page();
  // // cpu_icache_disable();
  // cp15_invalidate_icache();
  // cpu_invalid_tlb();

  // cpu_set_domain(0x07070707);
  // // cpu_set_domain(0xffffffff);
  // // cpu_set_domain(0x55555555);
  // cpu_set_page(page_dir);

  // // start_dump();
  // kprintf("enable page\n");

  // cpu_enable_page();
  // kprintf("paging pae scucess\n");
}

void mm_test() {
  // map_page(0x90000,0x600000,3);
  // u32* addr=mm_alloc(256);
  // *addr=0x123456;
    // kprintf("===============+>\n");
    // u32 *p = 0x1c2ac0c;
    // *p = 1 << 6;
    // kprintf("p=%x\n", *p);
}

void* virtual_to_physic(void* page, void* vaddr) {
  void* phyaddr = NULL;
  u32* l1 = page;
  u32 l1_index = (u32)vaddr >> 20;
  u32 l2_index = (u32)vaddr >> 12 & 0xFF;
  u32* l2 = ((u32)l1[l1_index]) & 0xFFFFFC00;
  if (l2 != NULL) {
    // kprintf("l2 %x\n",l2);
    phyaddr = (l2[l2_index] >> 12) << 12;
  }
  // kprintf("virtual_to_physic vaddr %x paddr %x\n",vaddr,phyaddr);
  return phyaddr;
}

void page_clone(u32* old_page, u32* new_page) {
  // kprintf("page_clone:%x %x\n",old_page,new_page);
  if (old_page == NULL) {
    kprintf("page clone error old page null\n");
    return;
  }
  u32* l1 = old_page;
  u32* new_l1 = new_page;
  // kprintf("page clone %x to %x\n",old_page,new_page);
  for (int l1_index = 0; l1_index < 4096; l1_index++) {
    u32* l2 = ((u32)l1[l1_index]) & 0xFFFFFC00;
    if (l2 != NULL) {
      page_dir_t* new_l2 = mm_alloc_zero_align(256 * sizeof(u32), 0x1000);
      new_l1[l1_index] = (((u32)new_l2) & 0xFFFFFC00) | L1_DESC;
      // kprintf("%d %x\n", l1_index, (u32)l2>>10 );
      for (int l2_index = 0; l2_index < 256; l2_index++) {
        u32* addr = l2[l2_index] >> 12;
        if (addr != NULL || l1_index == 0) {
          new_l2[l2_index] = l2[l2_index];
          // kprintf("  %d %x\n", l2_index, addr);
        }
      }
    }
  }
}

u32* page_alloc_clone(u32* kernel_page_dir) {
  u32* page_dir_ptr_tab =
      mm_alloc_zero_align(sizeof(u32) * PAGE_DIR_NUMBER, 0x4000);
  page_clone(kernel_page_dir, page_dir_ptr_tab);
  return page_dir_ptr_tab;
}

void unmap_page_on(page_dir_t* page, u32 virtualaddr) {
  u32* l1 = page;
  u32 l1_index = virtualaddr >> 20;
  u32 l2_index = virtualaddr >> 12 & 0xFF;
  u32* l2 = ((u32)l1[l1_index]) & 0xFFFFFC00;
  if (l2 != NULL) {
    // l1[l1_index] = 0;
    l2[l2_index] = 0;
  }
}

u32* page_alloc() {
  u32* page_dir_ptr_tab =
      mm_alloc_zero_align(sizeof(u32) * PAGE_DIR_NUMBER, 0x4000);
  return page_dir_ptr_tab;
}
// void* page_alloc_init(){
//   void* page_dir_ptr_tab = kmalloc_alignment(sizeof(u32) * 4096, 0x4000);
//   map_page_on(page_dir_ptr_tab, MMIO_BASE, MMIO_BASE, 0);
//   map_page_on(page_dir_ptr_tab,UART0_DR, UART0_DR, 0);
//   map_page_on(page_dir_ptr_tab,CORE0_TIMER_IRQCNTL, CORE0_TIMER_IRQCNTL, 0);
//   return page_dir_ptr_tab;
// }