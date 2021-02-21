/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../mm.h"

#include "../cpu.h"

static mem_block_t* block_head = NULL;
static mem_block_t* block_tail = NULL;
static mem_block_t* blocks = NULL;

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
    page_dir_ptr = mm_alloc_zero_align(sizeof(u64) * 512,4096);
    page_dir_ptr_tab[pdpte_index] = ((u64)page_dir_ptr) | PAGE_P;
  }
  u64* page_tab_ptr = (u64)page_dir_ptr[pde_index] & ~0xFFF;
  if (page_tab_ptr == NULL) {
    page_tab_ptr = mm_alloc_zero_align(sizeof(u64) * 512,4096);
    page_dir_ptr[pde_index] = (u64)page_tab_ptr | flags;
  }
  page_tab_ptr[pte_index] = (u64)physaddr & ~0xFFF | flags;
  // page_tab_ptr[offset] = (u32)physaddr | (flags & 0xFFF);
}

void mm_test() {
  // map_page(0x90000,0x600000,3);
  // 0xfd000000
  // map_page(0x1ff000,0x400000,PAGE_P | PAGE_USU | PAGE_RWW);
  // map_page(0xfd000000, 0xfd000000, PAGE_P | PAGE_USU | PAGE_RWW);
}

void mm_init() {
  boot_info->pdt_base = (ulong)page_dir_ptr_tab;
  boot_info->page_type = 1;  // pae mode

  // set zero
  for (int i = 0; i < 4; i++) {
    page_dir_ptr_tab[i] = 0;
  }
  page_dir_ptr_tab[0] = (u32)page_dir | PAGE_P;

  page_dir[0] = (u32)page_tab | (PAGE_P | PAGE_USU | PAGE_RWW);
  unsigned int i, address = 0;
  kprintf("start 0x%x ", address);
  for (i = 0; i < 512; i++) {
    page_tab[i] = address | (PAGE_P | PAGE_USU | PAGE_RWW);
    address = address + 0x1000;
  }
  kprintf("- 0x%x\n", address);  // 0x200000 2GB

  // mm init
  mm_alloc_init();

  mm_test();
}

void mm_alloc_init() {
  memory_info_t* first_mem = (memory_info_t*)&boot_info->memory[0];
  mem_block_t* blocks = (mem_block_t*)first_mem->base + 0x10;
  u32 size = sizeof(mem_block_t) * boot_info->memory_number;
  for (int i = 0; i < boot_info->memory_number; i++) {
    memory_info_t* mem = (memory_info_t*)&boot_info->memory[i];
    if (mem->type != 1) {
      continue;
    }
    // skip
    u32 addr = mem->base;
    u32 len = mem->length;
    if (0x500 > mem->base && mem->base < 0x4000) {
      addr = 0x4000;
      len = mem->base - addr;
    } else if (mem->base > boot_info->kernel_base && mem->base < 0x200000) {
      addr = 0x200000;
      len = mem->base - addr;
    }
    blocks[i].addr = addr;
    blocks[i].size = len;
    blocks[i].type = MEM_FREE;
    blocks[i].next = NULL;
  }
  blocks[0].addr = blocks[0].addr + size;
  blocks[0].size = blocks[0].size - size;
  for (int i = 0; i < (boot_info->memory_number - 1); i++) {
    blocks[i].next = &blocks[i + 1];
    if (i > 0) {
      blocks[i].prev = &blocks[i - 1];
    } else {
      blocks[i].prev = NULL;
    }
  }
  block_head = blocks;
}


void* mm_alloc_zero_align(size_t size,int alignment) {
  int offset = alignment - 1 + sizeof(void*);
  void* addr = mm_alloc(size + offset);
  memset(addr, 0, size + offset);
  void** align = (void**)(((size_t)addr + offset) & ~(alignment - 1));
  align[-1] = addr;
  return align;
}

void mm_free_align(void* addr){
    void* real = ((void**)addr)[-1];
    mm_free(real);
}

void* mm_alloc(size_t size) {
  mem_block_t* p = block_head;
  for (; p != NULL; p = p->next) {
    if ((p->type == MEM_FREE)) {
      if (p->size == size) {
        p->type = MEM_USED;
        return p->addr;
      } else if ((size + sizeof(mem_block_t)) < p->size) {
        mem_block_t* b = (mem_block_t*)p->addr;
        u32 rest = p->size - size - sizeof(mem_block_t);
        b->size = size;
        b->addr = b + sizeof(mem_block_t);
        b->type = MEM_USED;
        mem_block_t* prev = p->prev;
        if (prev != NULL) {
          prev->next = b;
        } else {
          block_head = b;
        }
        p = b->addr + size;
        p->size = rest;
        p->addr = p + sizeof(mem_block_t);
        p->prev = b;
        p->type = MEM_FREE;
        b->next = p;
        return b->addr;
      }
    }
  }
  kprintf("erro malloc end");
  return NULL;
}

void mm_free(void* addr) {
  mem_block_t* p = block_head;
  for (; p != NULL; p = p->next) {
    if (p->addr == addr) {
      p->type = MEM_FREE;
      // todo merging mem
      break;
    }
  }
}

u32 mm_get_size(void* addr) {
  mem_block_t* p = block_head;
  for (; p != NULL; p = p->next) {
    if (p->addr == addr) {
      return p->size;
    }
  }
}
