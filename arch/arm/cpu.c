/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../cpu.h"
#include "context.h"
#include "cpu.h"

extern boot_info_t* boot_info;

// 31:14-N -> va[29:20] va[19:12] 
#define TTBCRN_4K 0b010
#define TTBCRN_16K 0b000
#define TTBCR_LPAE 1<<31


/* data cache clean by MVA to PoC */
void dccmvac(unsigned long mva)
{
	asm volatile ("mcr p15, 0, %0, c7, c10, 1" : : "r" (mva) : "memory");
}

void cpu_icache_disable(){
  asm("mcr  p15, #0, r0, c7, c7, 0\n" );
}

void cpu_invalid_tlb(){
	asm volatile ("mcr p15, 0, %0, c8, c7, 0" : : "r" (0));
	asm volatile ("mcr p15, 0, %0, c8, c6, 0" : : "r" (0));
	asm volatile ("mcr p15, 0, %0, c8, c5, 0" : : "r" (0));
  asm volatile("isb\n"
    "dsb\n"
  );
}

void cp15_invalidate_icache(void)
{
  asm volatile(
      "mov r0, #0\n"
      "mcr p15, 0, r0, c7, c5, 0\n"
      "dsb\n"
      :
      :
      : "r0", "memory"
    );
}


u32 read_dfar(){
  u32 val = 0;
    asm volatile("mrc p15, 0, %0, c6, c0, 0"
                 : "=r"(val));
    return val;
}

u32 read_dfsr(){
  u32 val = 0;
    asm volatile("mrc p15, 0, %0, c5, c0, 0"
                 : "=r"(val));
    return val;
}


static inline u32 read_ttbcr(void) {
    u32 val = 0;
    asm volatile("mrc p15, 0, %0, c2, c0, 2"
                 : "=r"(val));
    return val;
}

static inline void write_ttbcr(u32 val) {
    asm volatile("mcr p15, 0, %0, c2, c0, 2"
                 :
                 : "r"(val)
                 : "memory");
}

static inline void write_ttbr0(u32 val) {
    asm volatile("mcr p15, 0, %0, c2, c0, 0"
                 :
                 : "r"(val)
                 : "memory");
}

static inline u32 read_ttbr0() {
    u32 val = 0;
    asm volatile("mrc p15, 0, %0, c2, c0, 0"
                 : "=r"(val));
    return val;
}

static inline void write_ttbr1(u32 val) {
    asm volatile("mcr p15, 0, %0, c2, c0, 1"
                 :
                 : "r"(val)
                 : "memory");
}

static inline u32 read_ttbr1() {
    u32 val = 0;
    asm volatile("mrc p15, 0, %0, c2, c0, 1"
                 : "=r"(val));
    return val;
}

u32 cpu_set_domain(u32 val){
    u32 old;
    asm volatile ("mrc p15, 0, %0, c3, c0,0\n" : "=r" (old));
    asm volatile ("mcr p15, 0, %0, c3, c0,0\n" : :"r" (val) : "memory");
    return old;
}


/* invalidate unified TLB by MVA and ASID */
void tlbimva(unsigned long mva)
{
	asm volatile ("mcr p15, 0, %0, c8, c7, 1" : : "r" (mva) : "memory");
}


void cpu_set_page(u32 page_table){
  //set ttbcr0
   write_ttbr0(page_table);
   isb();
   write_ttbr1(page_table);
   isb();
   write_ttbcr(TTBCRN_4K);
   isb();
  //set all permission
  //cpu_set_domain(~0);
}

void cpu_disable_page(){
  u32 reg;
  // read mmu 
  asm("mrc p15, 0, %0, c1, c0, 0" : "=r" (reg) : : "cc");
  reg&=~0x1;
  asm volatile("mcr p15, 0, %0, c1, c0, #0" : : "r" (reg) : "cc");
}

void cpu_enable_page(){
  u32 reg;
  // read mmu 
  asm("mrc p15, 0, %0, c1, c0, 0" : "=r" (reg) : : "cc");//SCTLR 
  reg|=0x1; //M enable mmu
  //reg|=1<<29;//AFE
  reg|=1<<12; //Instruction cache enable:
  reg|=1<<2;//Cache enable.
  reg|=1<<1;//Alignment check enable.
  asm volatile("mcr p15, 0, %0, c1, c0, #0" : : "r" (reg) : "cc");//SCTLR
  //asm("b 0");

  dsb();
  isb();
}

inline void cpu_invalidate_tlbs(void){
  asm (
      "mcr p15, 0, r0, c8, c7, 0\n"
      "mcr p15,0,0,c7,c10,4\n"
      :
      :
      : "r0", "memory"
    );
}


void cpu_init() {
  
}

void cpu_halt() { asm("wfi"); }

void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level) {
  if (context == NULL) {
    return;
  }
  context->tss = NULL;
  context->eip = entry;
  context->level = level;
  context->esp0 = stack0;
  u32 cs, ds;
  cpsr_t cpsr;
  cpsr.val = 0;
  if (level == 0) {
    // kernel mode
    cpsr.Z = 1;
    cpsr.C = 1;
    cpsr.A = 1;
    cpsr.I = 0;
    cpsr.F = 1;
    cpsr.M = 0x1f;
    interrupt_context_t* c = stack0;
  } else if (level == 3) {
    cpsr.I = 0;
    cpsr.F = 0;
    cpsr.T=0; //arm
    cpsr.M = 0x10;
  } else {
    kprintf("not suppport level %d\n", level);
  }

  interrupt_context_t* user = stack3;
  memset(user, 0, sizeof(interrupt_context_t));
  user->lr = entry;
  user->psr= cpsr.val;
  user->r0 = 0;
  user->r1 = 0x11111111;
  user->r2 = 0x22222222;
  user->r3 = 0x33333333;
  user->r4 = 0x44444444;
  user->r5 = 0x55555555;
  user->r6 = 0x66666666;
  user->r7 = 0x77777777;
  user->r8 = 0x88888888;
  user->r9 = 0x99999999;
  user->r10 = 0x101010;
  user->r11 = 0x111111;
  context->esp = stack3;
}

void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;
  if (context == NULL) {
    // context = current_context->esp0;
    context = current_context->esp;
  } else {
    // current_context->esp0 = context;
    current_context->esp = context;
  }
  *current = next_context;
}

void context_clone(context_t* context, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {
  *context = *src;
}

ulong cpu_get_cs(void) {
  ulong result;

  return result;
}

int TAS(volatile int* addr, int newval) {
  int result = newval;
  // asm volatile("lock; xchg %0, %1"
  //              : "+m"(*addr), "=r"(result)
  //              : "1"(newval)
  //              : "cc");
  return result;
}

// void context_restore(context_t* context){
//   cpu_sti();
//   interrupt_exit_context(context);
// }