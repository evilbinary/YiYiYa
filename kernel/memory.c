/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "memory.h"

#include "algorithm/queue_pool.h"
#include "thread.h"

queue_pool_t* kernel_pool;
extern context_t* current_context;

void memory_init() { kpool_init(); }

void* kmalloc(size_t size) {
  use_kernel_page();
  void* addr = NULL;
  size = ((size + PAGE_SIZE) / PAGE_SIZE) * PAGE_SIZE;
  // size=((size+1024)/1024)*1024;
  addr = mm_alloc(size);
  if (addr == NULL) {
    kprintf("kmalloc error\n");
    return addr;
  }
  memset(addr, 0, size);
  use_user_page();
  return addr;
}

void kfree(void* ptr) {
  // use_kernel_page();
  //mm_free(ptr);
  // use_user_page();
}

void* kmalloc_alignment(size_t size, int alignment) {
  // size=((size+4096)/4096)*4096;
  // use_kernel_page();
  void* addr = mm_alloc_zero_align(size, alignment);
  // use_user_page();
  return addr;
}

void kfree_alignment(void* ptr) {
  // use_kernel_page();
  mm_free_align(ptr);
  // use_user_page();
}

// alloc physic right now on virtual
void* valloc(void* addr, size_t size) {
  thread_t* current = thread_current();
  if (size < PAGE_SIZE) {
    size = PAGE_SIZE;
  }
  void* vaddr = addr;
  void* phy_addr = kmalloc_alignment(size,PAGE_SIZE);
  void* paddr = phy_addr;
  for (int i = 0; i < size / PAGE_SIZE; i++) {
    if (current != NULL) {
      map_page_on(current->context.page_dir, vaddr, paddr,
                  PAGE_P | PAGE_USU | PAGE_RWW);
    } else {
      map_page(vaddr, paddr, PAGE_P | PAGE_USU | PAGE_RWW);
    }
    kprintf("vmap vaddr:%x paddr:%x\n", vaddr, paddr);
    vaddr += PAGE_SIZE;
    paddr += PAGE_SIZE;
  }
  return addr;
}

// rease
void vfree(void* addr) {
  if (addr == NULL) return;
  thread_t* current = thread_current();
  void* phy = virtual_to_physic(current->context.page_dir, addr);
  //unmap_page_on(current->context.page_dir, addr);
  if(phy!=NULL){
    kfree(phy);
  }
}

void map_2gb(u64* page_dir_ptr_tab, u32 attr) {
  u32 addr = 0;
  for (int i = 0; i < 0x200000 / PAGE_SIZE; i++) {
    map_page_on(page_dir_ptr_tab, addr, addr, attr);
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

void page_clone_user(u64* page, u64* page_dir_ptr_tab) {
  use_kernel_page();
  page_clone(page, page_dir_ptr_tab);
  //unmap_mem_block(page_dir_ptr_tab, 0x200000);
  use_user_page();
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
    //kprintf("page tab find errro\n");
    return NULL;
  }
  void* phyaddr = page_tab_ptr[pte_index] & ~0xFFF;
  return phyaddr;
}

void kpool_init() { kernel_pool = queue_pool_create(PAGE_SIZE, 256); }

int kpool_put(void* e) { return queue_pool_put(kernel_pool, e); }

void* kpool_poll() {
  void* e = queue_pool_poll(kernel_pool);
  if (e == NULL) {
    kprintf("kpool poll is null\n");
  }
  return e;
}

void use_kernel_page() {
  if (current_context != NULL && cpu_cpl() == KERNEL_MODE) {
    current_context->tss->cr3 = current_context->kernel_page_dir;
    context_switch_page(current_context->kernel_page_dir);
  }
}

void use_user_page() {
  if (current_context != NULL && cpu_cpl() == KERNEL_MODE) {
    current_context->tss->cr3 = current_context->page_dir;
    context_switch_page(current_context->page_dir);
  }
}

void vmemory_area_free(vmemory_area_t* area) {
  if (area == NULL) return;
  u32 vaddr = area->vaddr;
  for (int i = 0; i < area->size / PAGE_SIZE; i++) {
    u32 phyaddr = virtual_to_physic(current_context->page_dir, vaddr);
    kfree(phyaddr);
    map_page_on(current_context->page_dir, vaddr, vaddr, 0);
    vaddr += PAGE_SIZE;
  }
  area->flags = MEMORY_FREE;
}

// alloc by page fault
vmemory_area_t* vmemory_area_alloc(vmemory_area_t* areas, void* addr,
                                   u32 size) {
  vmemory_area_t* area = vmemory_area_find(areas, addr, size);
  if (area != NULL) {
    return area;
  }
  area = vmemory_area_create(addr, size, 0);
  vmemory_area_add(areas, area);
  return area;
}

vmemory_area_t* vmemory_area_create(void* addr, u32 size, u8 flags) {
  vmemory_area_t* area = kmalloc(sizeof(vmemory_area_t));
  area->size = size;
  area->next = NULL;
  area->vaddr = addr;
  area->flags = flags;
  return area;
}

void vmemory_area_add(vmemory_area_t* areas, vmemory_area_t* area) {
  vmemory_area_t* p = areas;
  for (; p->next != NULL; p = p->next) {
  }
  p->next = area;
}

vmemory_area_t* vmemory_area_find(vmemory_area_t* areas, void* addr,
                                  size_t size) {
  vmemory_area_t* p = areas;
  for (; p != NULL; p = p->next) {
    if ((addr >= p->vaddr) && (addr <= (p->vaddr + (p->size) * PAGE_SIZE))) {
      return p;
    }
  }
  return NULL;
}

vmemory_area_t* vmemory_area_destroy(vmemory_area_t* area) {}

void* vmemory_area_find_flag(vmemory_area_t* areas, u8 flags) {
  vmemory_area_t* p = areas;
  for (; p != NULL; p = p->next) {
    if (p->flags & ~flags == flags) {
      return p;
    }
  }
  return NULL;
}