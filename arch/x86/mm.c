/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "cpu.h"
#include "../mm.h"


static mem_block_t* block_alloc_head = NULL;
static mem_block_t* block_alloc_tail = NULL;

static mem_block_t* block_available_tail = NULL;
static mem_block_t* block_available = NULL;

extern boot_info_t* boot_info;

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

void mm_dump_phy() {
  for (int i = 0; i < boot_info->memory_number; i++) {
    memory_sinfo_t* m = (memory_sinfo_t*)&boot_info->memory[i];
    kprintf("base:%x %x lenght:%x %x type:%d\n", m->baseh, m->basel, m->lengthh,
            m->lengthl, m->type);
  }
  kprintf("total memory %dm\n", boot_info->total_memory / 1024 / 1024);
}

void mm_init() {
  mm_dump_phy();
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

  // mm init
  mm_alloc_init();
  // map > 4GB addr
  map_mem_block(address);
  // move to new addr
  // boot_info_t *new_boot=kmalloc(sizeof(boot_info_t));
  // *new_boot=*boot_info;
  // boot_info=new_boot;

  if (boot_info->pdt_base != NULL) {
    ulong addr = (ulong)boot_info->pdt_base;
    cpu_enable_paging_pae(addr);
    kprintf("paging pae scucess\n");
  }

}

void mm_alloc_init() {
  memory_info_t* first_mem = (memory_info_t*)&boot_info->memory[0];
  u32 size = sizeof(mem_block_t) * boot_info->memory_number;
  u32 pos = 0;
  for (int i = 0; i < boot_info->memory_number; i++) {
    memory_info_t* mem = (memory_info_t*)&boot_info->memory[i];
    if (mem->type != 1) {  // normal ram
      continue;
    }
    // skip
    u32 addr = mem->base;
    u32 len = mem->length;
    u32 boot_end = 0x11000;  //((u32)boot_info+1024*4);
    u32 kernel_start = boot_info->kernel_base;
    u32 kernel_end = kernel_start + boot_info->kernel_size;
    if (boot_info >= mem->base && mem->base < boot_end) {
      addr = boot_end;
      len = len - (addr - mem->base);
    } else if (kernel_start >= mem->base && mem->base < kernel_end) {
      addr = kernel_end;
      len = len - (addr - mem->base);
    }
    mem_block_t* block = addr;
    block->addr = (u32)block + sizeof(mem_block_t);
    block->size = len - sizeof(mem_block_t);
    block->type = MEM_FREE;
    block->next = NULL;
    if (block_available == NULL) {
      block_available = block;
      block_available_tail = block;
    } else {
      block_available_tail->next = block;
      block_available_tail = block;
    }
  }
}

void* mm_alloc(size_t size) {
  mem_block_t* p = block_available;
  u32 static count = 0;
  u32 pre_alloc_size = size + sizeof(mem_block_t);
  for (; p != NULL; p = p->next) {
    // kprintf("p:%x type:%d size:%x\n", p, p->type, p->size);
    if ((p->type != MEM_FREE)) {
      continue;
    }
    if ((pre_alloc_size) <= p->size) {
      // kprintf("p:%x pre_alloc_size:%d size:%d
      // type:%d\n",p,pre_alloc_size,p->size,p->type);
      mem_block_t* new_block = (mem_block_t*)p->addr;
      if (new_block == NULL) continue;
      p->addr += pre_alloc_size;
      p->size -= pre_alloc_size;

      new_block->addr = (u32)new_block + sizeof(mem_block_t);
      new_block->size = size;
      new_block->next = NULL;
      new_block->type = MEM_USED;

      if (block_alloc_head == NULL) {
        block_alloc_head = new_block;
        block_alloc_tail = new_block;
      } else {
        block_alloc_tail->next = new_block;
        block_alloc_tail = new_block;
      }
      count++;
      // kprintf("alloc count:%d: addr:%x size:%d\n", count, new_block->addr,
      //         new_block->size);
      // mm_dump();
      // mm_dump_print(block_available);
      return (void*)new_block->addr;
    }
  }
  kprintf("erro alloc size %d kb\n", size / 1024);
  // mm_dump();
  for (;;)
    ;

  return NULL;
}

void* mm_alloc_zero_align(size_t size, u32 alignment) {
  void *p1, *p2;
  if ((p1 = (void*)mm_alloc(size + alignment + sizeof(size_t))) == NULL) {
    return NULL;
  }
  size_t addr = (size_t)p1 + alignment + sizeof(size_t);
  p2 = (void*)(addr - (addr % alignment));
  *((size_t*)p2 - 1) = (size_t)p1;
  return p2;

  // int offset = alignment + sizeof(void*);
  // void* addr = mm_alloc(size + offset);
  // addr+=1;
  // memset(addr, 0, size + offset);
  // void** align = (void**)(((size_t)addr + offset-1) & ~(alignment - 1));
  // align[-1] = addr;
  // return align;

  // size_t request_size = size + alignment;
  // char* buf = (char*)mm_alloc(request_size);
  // memset(buf, 0, request_size);
  // size_t remainder = ((size_t)buf) % alignment;
  // size_t offset = alignment - remainder;
  // char* ret = buf + (unsigned char)offset;
  // *(unsigned char*)(ret - 1) = offset;
  // return (void*)ret;
}

void mm_free_align(void* addr) {
  if (addr) {
    void* real = ((void**)addr)[-1];
    mm_free(real);
  }
  // int offset = *(((char*)addr) - 1);
  // mm_free(((char*)addr) - offset);
}

void mm_dump_print(mem_block_t* p) {
  u32 use = 0;
  u32 free = 0;
  for (; p != NULL; p = p->next) {
    if ((p->type == MEM_FREE)) {
      kprintf("free %x %d\n", p->addr, p->size);
      free += p->size;
    } else {
      kprintf("use %x %d\n", p->addr, p->size);
      use += p->size;
    }
  }
  kprintf("------------\n");
  kprintf("total ");
  if (use > 0) {
    kprintf(" use: %dkb ", use / 1024);
  }
  if (free > 0) {
    kprintf(" free: %dkb ", free / 1024);
  }
  kprintf("\n");
}

void mm_dump() {
  kprintf("dump memory\n");
  mm_dump_print(block_alloc_head);
  kprintf("------------\n");
  mm_dump_print(block_available);
  kprintf("dump end\n\n");
}

void mm_free(void* addr) {
  if (addr == NULL) return;
  mem_block_t* block = (mem_block_t*)((u32)addr);
  if (block->addr == 0) {
    kprintf("mm free error %x\n", addr);
    return;
  }
  block->next = NULL;
  block->type = MEM_FREE;
  block_available_tail->next = block;
  block_available_tail = block;
}

u32 mm_get_size(void* addr) {
  mem_block_t* p = block_alloc_head;
  for (; p != NULL; p = p->next) {
    if (p->addr == addr) {
      return p->size;
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
    // kprintf("page tab find errro\n");
    return NULL;
  }
  void* phyaddr = page_tab_ptr[pte_index] & ~0xFFF;
  return phyaddr;
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


u32* page_alloc_clone(u32* kernel_page_dir){
  u32* page_dir_ptr_tab = kmalloc_alignment(sizeof(u64) * 4, 0x1000);
  page_clone(kernel_page_dir, page_dir_ptr_tab);
  return page_dir_ptr_tab;
}