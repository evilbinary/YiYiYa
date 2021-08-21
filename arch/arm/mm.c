/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../mm.h"

#include "../cpu.h"
#include "../display.h"
#include "cpu.h"
#include "gpio.h"

#define L1_PAGE_TABLE (1 << 0)
#define L2_SMALL_PAGE (2 << 0)

#define L1_PXN (0 << 2)  // The Privileged execute-never bit
#define L1_NS (0 << 3)   // Non-secure bit
#define L1_SBZ (0 << 4)  // Should be Zero

#define L1_DOMAIN(n) (n << 5)

#define L2_XN (0 << 0)  // The Execute-never bit
#define L2_CB (3 << 2)  // 0b11 cache write-back
#define L2_NCNB (0 << 2)

#define L2_AP_ACCESS (3 << 4)
#define L2_AP_RW (0 << 9)  // read write any privilege level
#define L2_TEXT (7 << 6)
#define L2_S (1 << 10)   // The Shareable bit
#define L2_NG (1 << 11)  // The not global bit

#define L1_DESC (L1_PAGE_TABLE | L1_PXN | L1_NS | L1_SBZ | L1_DOMAIN(0))
#define L2_DESC \
  (L2_XN | L2_SMALL_PAGE | L2_NCNB | L2_AP_ACCESS | L2_AP_RW | L2_S | L2_NG)


extern boot_info_t* boot_info;
static u32 page_dir[4096] __attribute__((aligned(0x4000)));

void map_page_on(page_dir_t* l1, u32 virtualaddr, u32 physaddr, u32 flags) {
  u32 l1_index = virtualaddr >> 20;
  u32 l2_index = virtualaddr >> 12 & 0xFF;
  u32* l2 = ((u32)l1[l1_index]) & 0xFFFFFC00;
  if (l2 == NULL) {
    l2 = mm_alloc_zero_align(0x1000, 0x1000);
    kmemset(l2, 0, 0x1000);
    l1[l1_index] = (((u32)l2) & 0xFFFFFC00) | L1_DESC;
  }
  l2[l2_index] = (physaddr >> 12) << 12 | flags | L2_DESC;
}

void map_page(u32 virtualaddr, u32 physaddr, u32 flags) {
  map_page_on(boot_info->pdt_base, virtualaddr, physaddr, flags);
}

void mm_init_default() {
  mm_test();
  boot_info->pdt_base = page_dir;
  kmemset(page_dir, 0, 4096 * 4);

  u32 address = 0;
  kprintf("map %x - %x\n", address, 0x1000 * 1024*10);
  for (int j = 0; j < 1024*10; j++) {
    map_page(address, address, 0);
    address += 0x1000;
  }
  address = boot_info->kernel_entry;
  kprintf("map kernel %x ", address);
  int i;
  for (i = 0; i < (((u32)boot_info->kernel_size) / 0x1000 + 2); i++) {
    map_page(address, address, L2_TEXT);
    address += 0x1000;
  }
  kprintf("- %x\n", address);

  map_page(MMIO_BASE, MMIO_BASE, 0);
  map_page(UART0_DR, UART0_DR, 0);
  map_page(CORE0_TIMER_IRQCNTL, CORE0_TIMER_IRQCNTL, 0);
#ifdef V3S
  // memory
  address = 0x40000000;
  for (int i = 0; i < 0x700000 / 0x1000; i++) {
    map_page(address, address, 0);
    address += 0x1000;
  }
  map_page(0x01C20000, 0x01C20000, 0);
  map_page(0x01C28000, 0x01C28000, 0);
  // timer
  map_page(0x01C20C00, 0x01C20C00, 0);
  // gic
  map_page(0x01C81000, 0x01C81000, 0);
  map_page(0x01C82000, 0x01C82000, 0);

#endif

  kprintf("map page end\n");

  // cpu_disable_page();
  // cpu_icache_disable();
  cp15_invalidate_icache();
  cpu_invalid_tlb();

  cpu_set_domain(0x07070707);
  // cpu_set_domain(0xffffffff);
  // cpu_set_domain(0x55555555);
  cpu_set_page(page_dir);

  // start_dump();
  kprintf("enable page\n");

  cpu_enable_page();
  kprintf("paging pae scucess\n");
}

void mm_test() {
  // map_page(0x90000,0x600000,3);
  // u32* addr=mm_alloc(256);
  // *addr=0x123456;
}

void* virtual_to_physic(u64* page, void* vaddr) {
  void* phyaddr = NULL;
  u32* l1 = page;
  u32 l1_index = (u32)vaddr >> 20;
  u32 l2_index = (u32)vaddr >> 12 & 0xFF;
  u32* l2 = ((u32)l1[l1_index]) & 0xFFFFFC00;
  if (l2 != NULL) {
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
      page_dir_t* new_l2 = mm_alloc_zero_align(0x1000, 0x1000);
      kmemset(new_l2, 0, 0x1000);
      new_l1[l1_index] = (((u32)new_l2) & 0xFFFFFC00) | L1_DESC;
      // kprintf("%d %x\n", l1_index, (u32)l2>>10 );
      for (int l2_index = 0; l2_index < 512; l2_index++) {
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
  u32* page_dir_ptr_tab = kmalloc_alignment(sizeof(u32) * 4096, 0x4000);
  page_clone(kernel_page_dir, page_dir_ptr_tab);
  return page_dir_ptr_tab;
}

void unmap_page_on(page_dir_t* page, u32 virtualaddr) {
  u32* l1=page;
  u32 l1_index = virtualaddr >> 20;
  u32 l2_index = virtualaddr >> 12 & 0xFF;
  u32* l2 = ((u32)l1[l1_index]) & 0xFFFFFC00;
  if (l2 != NULL) {
    // l1[l1_index] = 0;
    l2[l2_index] = 0;
  }
}

// void* page_alloc_init(){
//   void* page_dir_ptr_tab = kmalloc_alignment(sizeof(u32) * 4096, 0x4000);
//   map_page_on(page_dir_ptr_tab, MMIO_BASE, MMIO_BASE, 0);
//   map_page_on(page_dir_ptr_tab,UART0_DR, UART0_DR, 0);
//   map_page_on(page_dir_ptr_tab,CORE0_TIMER_IRQCNTL, CORE0_TIMER_IRQCNTL, 0);
//   return page_dir_ptr_tab;
// }