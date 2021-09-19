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
#define TTBCR_LPAE 1 << 31

/* data cache clean by MVA to PoC */
void dccmvac(unsigned long mva) {
  asm volatile("mcr p15, 0, %0, c7, c10, 1" : : "r"(mva) : "memory");
}

void cpu_icache_disable() { asm("mcr  p15, #0, r0, c7, c7, 0\n"); }

void cpu_invalid_tlb() {
  asm volatile("mcr p15, 0, %0, c8, c7, 0" : : "r"(0));
  asm volatile("mcr p15, 0, %0, c8, c6, 0" : : "r"(0));
  asm volatile("mcr p15, 0, %0, c8, c5, 0" : : "r"(0));
  asm volatile(
      "isb\n"
      "dsb\n");
}

void cp15_invalidate_icache(void) {
  asm volatile(
      "mov r0, #0\n"
      "mcr p15, 0, r0, c7, c5, 0\n"
      "dsb\n"
      :
      :
      : "r0", "memory");
}

u32 read_dfar() {
  u32 val = 0;
  asm volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(val));
  return val;
}

u32 read_dfsr() {
  u32 val = 0;
  asm volatile("mrc p15, 0, %0, c5, c0, 0" : "=r"(val));
  return val;
}

u32 read_pc() {
  u32 val = 0;
  asm volatile("ldr %0,[r15]" : "=r"(val));
  return val;
}

u32 read_ifsr() {
  u32 val = 0;
  asm volatile("mrc p15, 0, %0, c5, c0, 1" : "=r"(val));
  return val;
}

u32 read_fp() {
  u32 val = 0;
  asm volatile("mov %0,fp" : "=r"(val));
  return val;
}

static inline u32 read_ttbcr(void) {
  u32 val = 0;
  asm volatile("mrc p15, 0, %0, c2, c0, 2" : "=r"(val));
  return val;
}

static inline void write_ttbcr(u32 val) {
  asm volatile("mcr p15, 0, %0, c2, c0, 2" : : "r"(val) : "memory");
}

static inline void write_ttbr0(u32 val) {
  asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(val) : "memory");
}

static inline u32 read_ttbr0() {
  u32 val = 0;
  asm volatile("mrc p15, 0, %0, c2, c0, 0" : "=r"(val));
  return val;
}

static inline void write_ttbr1(u32 val) {
  asm volatile("mcr p15, 0, %0, c2, c0, 1" : : "r"(val) : "memory");
}

static inline u32 read_ttbr1() {
  u32 val = 0;
  asm volatile("mrc p15, 0, %0, c2, c0, 1" : "=r"(val));
  return val;
}

u32 cpu_set_domain(u32 val) {
  u32 old;
  asm volatile("mrc p15, 0, %0, c3, c0,0\n" : "=r"(old));
  asm volatile("mcr p15, 0, %0, c3, c0,0\n" : : "r"(val) : "memory");
  return old;
}

/* invalidate unified TLB by MVA and ASID */
void tlbimva(unsigned long mva) {
  asm volatile("mcr p15, 0, %0, c8, c7, 1" : : "r"(mva) : "memory");
}

void cpu_set_page(u32 page_table) {
  // set ttbcr0
  write_ttbr0(page_table);
  isb();
  write_ttbr1(page_table);
  isb();
  write_ttbcr(TTBCRN_16K);
  isb();
  dsb();
  // set all permission
  // cpu_set_domain(~0);
}

void cpu_disable_page() {
  u32 reg;
  // read mmu
  asm("mrc p15, 0, %0, c1, c0, 0" : "=r"(reg) : : "cc");
  reg &= ~0x1;
  asm volatile("mcr p15, 0, %0, c1, c0, #0" : : "r"(reg) : "cc");
}


void cpu_enable_smp_mode() {
  // Enable SMP mode for CPU0
  // asm volatile(
  //   "mrc p15, 1, r0, R1, C15\n" // Read CPUECTLR.
  //   "orr r0, r0, #1 << 6 \n" // Set SMPEN.
  //   "mcr p15, 1, R0, R1, C15"); // Write CPUECTLR.

  asm volatile(
      "mrc p15, 0, r0, c1, c0, 1\n"
      "orr r0, r0, #1 << 6\n"
      "mcr p15, 0, r0, c1, c0, 1\n");
}

static void cpu_enable_ca7_smp(void) {
  u32 val;

  /* Read MIDR */
  asm volatile("mrc p15, 0, %0, c0, c0, 0\n\t" : "=r"(val));
  val = (val >> 4);
  val &= 0xf;

  /* Only set the SMP for Cortex A7 */
  if (val == 0x7) {
    /* Read auxiliary control register */
    asm volatile("mrc p15, 0, %0, c1, c0, 1\n\t" : "=r"(val));

    if (val & (1 << 6)) return;

    /* Enable SMP */
    val |= (1 << 6);

    /* Write auxiliary control register */
    asm volatile("mcr p15, 0, %0, c1, c0, 1\n\t" : : "r"(val));

    dsb();
    isb();
  }
}


inline void cpu_invalidate_tlbs(void) {
  asm("mcr p15, 0, r0, c8, c7, 0\n"
      "mcr p15,0,0,c7,c10,4\n"
      :
      :
      : "r0", "memory");
}

static inline uint32_t get_ccsidr(void)
{
	uint32_t ccsidr;

	__asm__ __volatile__("mrc p15, 1, %0, c0, c0, 0" : "=r" (ccsidr));
	return ccsidr;
}

static inline void __v7_cache_flush_range(uint32_t start, uint32_t stop, uint32_t line)
{
	uint32_t mva;

	start &= ~(line - 1);
	if(stop & (line - 1))
		stop = (stop + line) & ~(line - 1);
	for(mva = start; mva < stop; mva = mva + line)
	{
		__asm__ __volatile__("mcr p15, 0, %0, c7, c14, 1" : : "r" (mva));
	}
}

/*
 * Flush range(clean & invalidate), affects the range [start, stop - 1]
 */
void cpu_cache_flush_range(unsigned long start, unsigned long stop)
{
	uint32_t ccsidr;
	uint32_t line;

	ccsidr = get_ccsidr();
	line = ((ccsidr & 0x7) >> 0) + 2;
	line += 2;
	line = 1 << line;
	__v7_cache_flush_range(start, stop, line);
	dsb();
}

static inline void __v7_cache_inv_range(uint32_t start, uint32_t stop, uint32_t line)
{
	uint32_t mva;

	start &= ~(line - 1);
	if(stop & (line - 1))
		stop = (stop + line) & ~(line - 1);
	for(mva = start; mva < stop; mva = mva + line)
	{
		__asm__ __volatile__("mcr p15, 0, %0, c7, c6, 1" : : "r" (mva));
	}
}
/*
 * Invalidate range, affects the range [start, stop - 1]
 */
void cache_inv_range(unsigned long start, unsigned long stop)
{
	uint32_t ccsidr;
	uint32_t line;

	ccsidr = get_ccsidr();
	line = ((ccsidr & 0x7) >> 0) + 2;
	line += 2;
	line = 1 << line;
	__v7_cache_inv_range(start, stop, line);
	dsb();
}



void mmu_inv_tlb(void)
{
	__asm__ __volatile__("mcr p15, 0, %0, c8, c7, 0" : : "r" (0));
	__asm__ __volatile__("mcr p15, 0, %0, c8, c6, 0" : : "r" (0));
	__asm__ __volatile__("mcr p15, 0, %0, c8, c5, 0" : : "r" (0));
	dsb();
	isb();
}

void cpu_enable_page() {
  cpu_enable_smp_mode();
  cache_inv_range(0, ~0);
  // mmu_inv_tlb();

  u32 reg;
  // read mmu
  asm("mrc p15, 0, %0, c1, c0, 0" : "=r"(reg) : : "cc");  // SCTLR
  reg |= 0x1;                                  // M enable mmu
  // reg|=(1<<29);//AFE
  // reg |= 1 << 28; //TEX remap enable.
  reg |= 1 << 12;  // Instruction cache enable:
  reg |= 1 << 2;   // Cache enable.
  reg |= 1 << 1;   // Alignment check enable.
  reg |= 1 << 11;  //Branch prediction enable
  asm volatile("mcr p15, 0, %0, c1, c0, #0" : : "r"(reg) : "cc");  // SCTLR
  dsb();
  isb();
}

void cpu_init() {
  // cpu_enable_smp_mode();
  // cpu_enable_ca7_smp();
}

void cpu_halt() {
  for (;;) {
    asm("wfi");
  };
}

int context_get_mode(context_t* context) {
  int mode = 0;
  if (context != NULL) {
    interrupt_context_t* c = context->esp0;
    if ((c->psr & 0x1F) == 0x10) {
      return 3;  // user mode
    }
  }
  return mode;
}

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
    cpsr.T = 0;  // arm
    cpsr.M = 0x10;
  } else {
    kprintf("not suppport level %d\n", level);
  }

  interrupt_context_t* user = stack0;
  kmemset(user, 0, sizeof(interrupt_context_t));
  user->lr = entry;  // r14
  user->lr += 4;
  user->psr = cpsr.val;
  user->r0 = 0;
  user->r1 = 0x00010001;
  user->r2 = 0x00020002;
  user->r3 = 0x00030003;
  user->r4 = 0x00040004;
  user->r5 = 0x00050006;
  user->r6 = 0x00060006;
  user->r7 = 0x00070007;
  user->r8 = 0x00080008;
  user->r9 = 0x00090009;
  user->r10 = 0x00100010;
  user->r11 = 0x00110011;  // fp
  user->r12 = 0x00120012;  // ip
  user->sp = stack3;       // r13
  user->lr0 = user->lr;
  context->esp = stack3;
  context->esp0 = stack0;

  ulong addr = (ulong)boot_info->pdt_base;
  context->kernel_page_dir = addr;
  context->page_dir = addr;
#ifdef PAGE_CLONE
  context->page_dir = page_alloc_clone(addr);
#endif
}

// #define DEBUG 1
void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;
#if DEBUG
  kprintf("-----switch dump current------\n");
  context_dump(current_context);
#endif
  current_context->esp0 = context;
  current_context->esp = context->sp;
  current_context->eip = context->lr;
  *current = next_context;
  context_switch_page(next_context->page_dir);
#if DEBUG
  kprintf("-----switch dump next------\n");
  context_dump(next_context);
  kprintf("\n");
#endif
}

void context_dump(context_t* c) {
  kprintf("ip:  %x\n", c->eip);
  kprintf("sp0: %x\n", c->esp0);
  kprintf("sp:  %x\n", c->esp);

  kprintf("page_dir: %x\n", c->page_dir);
  kprintf("kernel page_dir: %x\n", c->kernel_page_dir);
  kprintf("--interrupt context--\n");
  interrupt_context_t* context = c->esp0;
  context_dump_interrupt(context);
}

void context_dump_interrupt(interrupt_context_t* context) {
  kprintf("lr:  %x cpsr:%x\n", context->lr, context->psr);
  kprintf("sp:  %x\n", context->sp);
  kprintf("r0:  %x\n", context->r0);
  kprintf("r1:  %x\n", context->r1);
  kprintf("r2:  %x\n", context->r2);
  kprintf("r3:  %x\n", context->r3);
  kprintf("r4:  %x\n", context->r4);
  kprintf("r5:  %x\n", context->r5);
  kprintf("r6:  %x\n", context->r6);
  kprintf("r7:  %x\n", context->r7);
  kprintf("r8:  %x\n", context->r8);
  kprintf("r9:  %x\n", context->r9);
  kprintf("r10: %x\n", context->r10);
  kprintf("r11(fp): %x\n", context->r11);
  kprintf("r12(ip): %x\n", context->r12);
}

void context_clone(context_t* des, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {
  interrupt_context_t* d0 = stack0;
  interrupt_context_t* s0 = src->esp0;
#if DEBUG
  kprintf("------context clone dump src--------------\n");
  context_dump(src);
#endif
  des->eip = src->eip;
  des->level= src->level;
  if (stack0 != NULL) {
    kmemmove(d0, s0, sizeof(interrupt_context_t));
    des->esp0 = (u32)d0;
  }
  if (stack3 != NULL) {
    cpsr_t cpsr;
    cpsr.val = 0;
    cpsr.Z = 1;
    cpsr.C = 1;
    cpsr.A = 1;
    cpsr.I = 1;
    cpsr.F = 1;
    cpsr.T =0;

    cpsr.M = 0x13;
    d0->psr = cpsr.val;
    d0->sp=stack3;
    des->esp=stack3;
  }
  des->page_dir =src->page_dir;
  des->page_dir = page_alloc_clone(src->page_dir);
  des->kernel_page_dir = src->kernel_page_dir;
#if DEBUG
  kprintf("------context clone dump des--------------\n");
#endif
  context_dump(des);
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

void cpu_backtrace(void) {
  int topfp = read_fp();
  for (int i = 0; i < 10; i++) {
    u32 fp = *(((u32*)topfp) - 3);
    u32 sp = *(((u32*)topfp) - 2);
    u32 lr = *(((u32*)topfp) - 1);
    u32 pc = *(((u32*)topfp) - 0);
    if (i == 0) {
      kprintf("top frame %x\n", pc);
    }  // top frame
    if (fp != 0) {
      kprintf(" %x\n", lr);
    }  // middle frame
    else {
      kprintf("bottom frame %x\n", pc);
    }  // bottom frame, lr invalid
    if (fp == 0) break;
    topfp = fp;
  }
}
