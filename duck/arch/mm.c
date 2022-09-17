#include "mm.h"

mem_block_t* block_alloc_head = NULL;
mem_block_t* block_alloc_tail = NULL;

mem_block_t* block_available_tail = NULL;
mem_block_t* block_available = NULL;
static u32 count = 0;

extern boot_info_t* boot_info;

void mm_init() {
  kprintf("mm init\n");
  block_available = NULL;
  block_alloc_head = NULL;
  block_alloc_tail = NULL;
  block_available_tail = NULL;
  count = 0;

  kprintf("phy dump\n");
  mm_dump_phy();
  kprintf("alloc init\n");
  // mm init
  mm_alloc_init();
  kprintf("mm init default\n");
  mm_init_default();
}

int is_line_intersect(int a1, int a2, int b1, int b2) {
  return !(b2 < a1 || b1 > a2);
}

void mm_add_block(u32 addr, u32 len) {
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
  kprintf("=>block:%x type:%d size:%d start: %x end:%x\n", block, block->type,
          block->size, block->addr, block->addr + block->size);
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
    // mm_add_block(addr, len);
    u32 kernel_start = boot_info->kernel_base;
    u32 kernel_end = kernel_start + boot_info->kernel_size;
    if (is_line_intersect(addr, addr + len, kernel_start, kernel_end)) {
      int a1 = addr;
      int a2 = addr + len;
      int b1 = kernel_start;
      int b2 = kernel_end;
      if (b1 > a1) {
        addr = a1;
        len = b1 - a1;
        mm_add_block(addr, len);
      }
      if (b2 < a2) {
        addr = b2;
        len = a2 - b2;
        mm_add_block(addr, len);
      }
    } else {
      mm_add_block(addr, len);
    }
  }
}

void mm_dump_phy() {
  for (int i = 0; i < boot_info->memory_number; i++) {
    memory_sinfo_t* m = (memory_sinfo_t*)&boot_info->memory[i];
    kprintf("base:%x %x lenght:%x %x type:%d\n", m->baseh, m->basel, m->lengthh,
            m->lengthl, m->type);
  }
  kprintf("total memory %dm %dk\n", boot_info->total_memory / 1024 / 1024,boot_info->total_memory/1024);
}
#define debug 

void* mm_alloc(size_t size) {
  mem_block_t* p = block_available;
  debug("malloc count %d size %d\n",count,size);
  u32 pre_alloc_size = size + sizeof(mem_block_t);
  pre_alloc_size = (pre_alloc_size + 8) & ~0x3;
  for (; p != NULL; p = p->next) {
    debug("p=>:%x type:%d size:%x\n", p, p->type, p->size);
    if ((p->type != MEM_FREE)) {
      continue;
    }
    // debug("p2=>:%x type:%d size:%x\n", p, p->type, p->size);
    if ((pre_alloc_size) <= p->size) {
      // debug("p:%x pre_alloc_size:%d size:%d
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
      //kprintf("alloc count:%d: addr:%x size:%d\n", count, new_block->addr,new_block->size);
      if (new_block->addr == 0) {
        mm_dump();
      }
      // cpu_backtrace();
      // mm_dump_print(block_available);
      return (void*)new_block->addr;
    }
  }
  kprintf("erro alloc count %d size %d kb\n", count, size / 1024);
  mm_dump();
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
  kmemset(p2,0,size);
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
  kprintf("total ");
  if (use >= 0) {
    kprintf(" use: %dkb %dmb", use / 1024,use/1024/1024);
  }
  if (free >= 0) {
    kprintf(" free: %dkb %dmb", free / 1024,free/1024/1024);
  }
  kprintf("\n");
}

void mm_dump() {
  kprintf("dump memory\n");
  kprintf("---dump alloc-------\n");
  mm_dump_print(block_alloc_head);

  kprintf("---dump available---\n");
  mm_dump_print(block_available);
  kprintf("dump end\n\n");
}

void mm_free(void* addr) {
  debug("free %x\n",addr);
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