/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "cpu.h"
#include "../mm.h"
#include "../cpu.h"
#include "gpio.h"
#include "../display.h"

#define L1_PAGE_TABLE  (1 << 0)
#define L2_SMALL_PAGE (2 << 0)

#define L1_PXN (0<<2) //The Privileged execute-never bit
#define L1_NS (0<<3) // Non-secure bit
#define L1_SBZ (0<<4) //Should be Zero

#define L1_DOMAIN(n)  (n<<5)

#define L2_XN (0<<0) //The Execute-never bit
#define L2_CB (3<<2) //0b11 cache write-back
#define L2_NCNB (0<<2) 

#define L2_AP_ACCESS (3<<4)
#define L2_AP_RW     (0<<9) //read write any privilege level
#define L2_TEXT  (7<<6) 
#define L2_S     (1<<10)  //The Shareable bit
#define L2_NG    (1<<11) //The not global bit

#define L1_DESC      (L1_PAGE_TABLE |L1_PXN|L1_NS|L1_SBZ|L1_DOMAIN(0))
#define L2_DESC      (L2_XN|L2_SMALL_PAGE|L2_NCNB|L2_AP_ACCESS|L2_AP_RW|L2_TEXT|L2_S|L2_NG)


static mem_block_t* block_alloc_head = NULL;
static mem_block_t* block_alloc_tail = NULL;

static mem_block_t* block_available_tail = NULL;
static mem_block_t* block_available = NULL;

extern boot_info_t* boot_info;
static u32 count = 0;


u32 page_dir[4096] __attribute__((aligned(0x10000)));
u32 page_tab[256] __attribute__((aligned(0x10000)));
u32 page_tab1[256] __attribute__((aligned(0x10000)));

void map_page_on(page_dir_t* l1, u32 virtualaddr, u32 physaddr, u32 flags) {
  u32 l1_index=virtualaddr>>20;
  u32 l2_index=virtualaddr>>12 & 0xFF;
  u32* l2=(u32)l1[l1_index] & ~0xFFFF;
  // u32* l2=(u32*)( ((u32*)l1)[l1_index]);
  if (l2 == NULL) {
    l2 = mm_alloc_zero_align(sizeof(u32) * 256, 0x10000);
    l1[l1_index] = L1_DESC| (((u32)l2)>>10)<<10;
  }
  l2[l2_index] = L2_DESC| (physaddr<<12)>>12| flags;
}

void map_page(u32 virtualaddr, u32 physaddr, u32 flags) {
  map_page_on(boot_info->pdt_base,virtualaddr,physaddr,flags);
}


void mm_init() {
  kprintf("mm init\n\r");
  block_available=NULL;
  block_alloc_head=NULL;
  block_alloc_tail=NULL;
  block_available_tail=NULL;
  count=0;
  // mm init
  mm_alloc_init();
  mm_test();
  boot_info->pdt_base = page_dir;
  u32 address=0;
  
  memset(page_dir,0,4096);
  memset(page_tab,0,256);
  memset(page_tab1,0,256);


  page_dir[0]= ((u32)page_tab>>10)<<10 | L1_DESC;
  page_dir[1]= ((u32)page_tab1>>10)<<10 | L1_DESC;

  kprintf("start 0x%x ", address);
  for(int j =0; j < 256 ; j++){
      page_tab[j] = address | L2_DESC;
      address+=0x1000;
  }
  kprintf("- 0x%x\n\r", address);  // 0x200000 2GB
  for(int j =0; j < 256 ; j++){
      page_tab1[j] = address | L2_DESC;
      address+=0x1000;
  }

  // for(int j =0; j < 512 ; j++){
  //   map_page(address,address,0);
  //   address+=0x1000;
  // }
  kprintf("map gpio\n\r");
  //map gpio
#ifdef V3S
  address=boot_info->kernel_entry;
  int i;
  for(i=0;i< (boot_info->kernel_size/4096)+1;i++){
    map_page(address,address,0);
    address+=0x1000;
  }
#endif
  map_page(MMIO_BASE,MMIO_BASE,0);
  map_page(UART0_DR,UART0_DR,0);
  map_page(CORE0_TIMER_IRQCNTL,CORE0_TIMER_IRQCNTL,0);
  map_page(0x40010000,0x40010000,0);
  map_page(0x4010a000,0x4010a000,0);
  map_page(0x4010b000,0x4010b000,0);

  kprintf("map page end\n\r");
  
  // cpu_disable_page();
  // cpu_icache_disable();
  cp15_invalidate_icache();
  cpu_invalid_tlb();
  cpu_set_page(page_dir);

  // start_dump();

  cpu_set_domain(0xFFFFFFFF);
  kprintf("enable page\n\r");

  cpu_enable_page();  

  kprintf("paging pae scucess\n\r");
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
    //debug("=>block:%x type:%d size:%d star: %x end: %x\n",block,block->type,block->size,block->addr,block->addr+block->size);
  }
}

void* mm_alloc(size_t size) {
  mem_block_t* p = block_available;
  //debug("malloc count %d size %d\n",count,size);
  u32 pre_alloc_size = size + sizeof(mem_block_t);
  for (; p != NULL; p = p->next) {
    //debug("p=>:%x type:%d size:%x\n", p, p->type, p->size);
    if ((p->type != MEM_FREE)) {
      continue;
    }
    if ((pre_alloc_size) <= p->size) {
      //debug("p:%x pre_alloc_size:%d size:%d type:%d\n",p,pre_alloc_size,p->size,p->type);
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
