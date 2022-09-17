/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "cpu.h"
#include "context.h"
#include "gic2.h"

extern boot_info_t* boot_info;
u32 cpus_id[MAX_CPU];

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

static inline uint32_t get_ccsidr(void) {
  uint32_t ccsidr;

  __asm__ __volatile__("mrc p15, 1, %0, c0, c0, 0" : "=r"(ccsidr));
  return ccsidr;
}

static inline void __v7_cache_flush_range(uint32_t start, uint32_t stop,
                                          uint32_t line) {
  uint32_t mva;

  start &= ~(line - 1);
  if (stop & (line - 1)) stop = (stop + line) & ~(line - 1);
  for (mva = start; mva < stop; mva = mva + line) {
    __asm__ __volatile__("mcr p15, 0, %0, c7, c14, 1" : : "r"(mva));
  }
}

/*
 * Flush range(clean & invalidate), affects the range [start, stop - 1]
 */
void cpu_cache_flush_range(unsigned long start, unsigned long stop) {
  uint32_t ccsidr;
  uint32_t line;

  ccsidr = get_ccsidr();
  line = ((ccsidr & 0x7) >> 0) + 2;
  line += 2;
  line = 1 << line;
  __v7_cache_flush_range(start, stop, line);
  dsb();
}

static inline void __v7_cache_inv_range(uint32_t start, uint32_t stop,
                                        uint32_t line) {
  uint32_t mva;

  start &= ~(line - 1);
  if (stop & (line - 1)) stop = (stop + line) & ~(line - 1);
  for (mva = start; mva < stop; mva = mva + line) {
    __asm__ __volatile__("mcr p15, 0, %0, c7, c6, 1" : : "r"(mva));
  }
}
/*
 * Invalidate range, affects the range [start, stop - 1]
 */
void cache_inv_range(unsigned long start, unsigned long stop) {
  uint32_t ccsidr;
  uint32_t line;

  ccsidr = get_ccsidr();
  line = ((ccsidr & 0x7) >> 0) + 2;
  line += 2;
  line = 1 << line;
  __v7_cache_inv_range(start, stop, line);
  dsb();
}

void mmu_inv_tlb(void) {
  __asm__ __volatile__("mcr p15, 0, %0, c8, c7, 0" : : "r"(0));
  __asm__ __volatile__("mcr p15, 0, %0, c8, c6, 0" : : "r"(0));
  __asm__ __volatile__("mcr p15, 0, %0, c8, c5, 0" : : "r"(0));
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
  reg |= 0x1;                                             // M enable mmu
  // reg|=(1<<29);//AFE
  // reg |= 1 << 28; //TEX remap enable.
  reg |= 1 << 12;  // Instruction cache enable:
  reg |= 1 << 2;   // Cache enable.
  reg |= 1 << 1;   // Alignment check enable.
  reg |= 1 << 11;  // Branch prediction enable
  asm volatile("mcr p15, 0, %0, c1, c0, #0" : : "r"(reg) : "cc");  // SCTLR
  dsb();
  isb();
}

void cpu_init(int cpu) {
  // cpu_enable_smp_mode();
  // cpu_enable_ca7_smp();
  for (int i = 0; i < MAX_CPU; i++) {
    cpus_id[i] = i;
  }
}

void cpu_halt() {
  for (;;) {
    asm("wfi");
  };
}

void cpu_wait(){
  asm("wfi");
}

ulong cpu_get_cs(void) {
  ulong result;

  return result;
}

int cpu_tas(volatile int* addr, int newval) {
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

int cpu_get_number() { return boot_info->tss_number; }

u32 cpu_get_id() {
  int cpu = 0;
  __asm__ volatile("mrc p15, #0, %0, c0, c0, #5\n" : "=r"(cpu));
  return cpu&0xf;
}

u32 cpu_get_index(int idx) {
  if (idx < 0 || idx > cpu_get_number()) {
    kprintf("out of bound get cpu idx\n");
    return 0;
  }
  return cpus_id[idx];
}

int cpu_init_id(u32 id) {
  // kprintf("cpu init id %d\n", id);
  ipi_enable(id);
  return 0;
}

int cpu_start_id(u32 id, u32 entry) {
  // start at  at entry-point on boot init.c
  // kprintf("cpu start id %d entry: %x\n", id,entry);
  lcpu_send_start(id,entry);
  return 0;
}

void cpu_delay(int n) {
  for (int i = 0; i < 10000 * n; i++)
    ;
}