/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../mm.h"

#include "../cpu.h"

static mem_block_t* block_alloc_head = NULL;
static mem_block_t* block_alloc_tail = NULL;

static mem_block_t* block_available_tail = NULL;
static mem_block_t* block_available = NULL;

extern boot_info_t* boot_info;

void map_page(void* physaddr, void* virtualaddr, u32 flags) {}

void mm_init() {
  // mm init
  mm_alloc_init();

  mm_test();
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
      kprintf("alloc count:%d: addr:%x size:%d\n", count, new_block->addr,
              new_block->size);
      // mm_dump();
      // mm_dump_print(block_available);
      return (void*)new_block->addr;
    }
  }
  kprintf("erro alloc count %d size %d kb\n",count, size / 1024);
  // mm_dump();
  for (;;)
    ;

  return NULL;
}

void* mm_alloc_zero_align(size_t size, int alignment) {
  void *p1, *p2;
  if ((p1 = (void*)mm_alloc(size + alignment + sizeof(size_t))) == NULL) {
    return NULL;
  }
  size_t addr = (size_t)p1 + alignment + sizeof(size_t);
  p2 = (void*)(addr - (addr % alignment));
  *((size_t*)p2 - 1) = (size_t)p1;
  return p2;
}

void mm_free_align(void* addr) {
  if (addr) {
    void* real = ((void**)addr)[-1];
    mm_free(real);
  }
  // int offset = *(((char*)addr) - 1);
  // mm_free(((char*)addr) - offset);
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


void mm_test() {
  // map_page(0x90000,0x600000,3);
  // u32* addr=mm_alloc(256);
  // *addr=0x123456;
}


void map_page_on(page_dir_t* page, u32 virtualaddr, u32 physaddr, u32 flags) {

  
}