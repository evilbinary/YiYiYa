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
  write_ttbcr(TTBCRN_4K);
  isb();
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

void cpu_enable_page() {
  u32 reg;
  // read mmu
  asm("mrc p15, 0, %0, c1, c0, 0" : "=r"(reg) : : "cc");  // SCTLR
  reg |= 0x1;                                             // M enable mmu
  // reg|=(1<<29);//AFE
  reg |= 1 << 12;  // Instruction cache enable:
  reg |= 1 << 2;   // Cache enable.
  reg |= 1 << 4;   // Data Cache
  reg |= 1 << 1;   // Alignment check enable.
  asm volatile("mcr p15, 0, %0, c1, c0, #0" : : "r"(reg) : "cc");  // SCTLR
  dsb();
  isb();
}

inline void cpu_invalidate_tlbs(void) {
  asm("mcr p15, 0, r0, c8, c7, 0\n"
      "mcr p15,0,0,c7,c10,4\n"
      :
      :
      : "r0", "memory");
}

void cpu_init() {}

void cpu_halt() {
  for (;;) {
    asm("wfi");
  };
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
}

#define DEBUG 0
void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;
  interrupt_context_t* c = current_context->esp0;
#if DEBUG
  kprintf("\n=>lr:%x sp:%x lr:%x sp:%x fp:%x irq=> lr:%x sp:%x fp:%x\n",
          current_context->eip, current_context->esp, c->lr, c->sp, c->r11,
          context->lr, context->sp, context->r11);
#endif
  current_context->esp0 = context;
  current_context->esp = context->sp;
  current_context->eip = context->lr;
  *current = next_context;
  context_switch_page(next_context->page_dir);
#if DEBUG
  c = next_context->esp0;
  kprintf("  lr:%x sp:%x irq=> lr:%x sp:%x  fp:%x\n", next_context->eip,
          next_context->esp, c->lr, c->sp, c->r11);
#endif
}

void context_clone(context_t* des, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {
  *des = *src;
  interrupt_context_t* d0 = stack0;
  interrupt_context_t* s0 = src->esp0;

  // s0->lr=0x4000a48+4;

  kprintf("context_clone====>lr:%x sp:%x  lr:%x sp:%x  fp:%x\n", src->eip, src->esp,
          s0->lr, s0->sp, s0->r11);

  if (stack0 != NULL) {
    kmemmove(d0, s0, sizeof(interrupt_context_t));
    des->esp0 = (u32)d0;
  }
  if (stack3 != NULL) {
    cpsr_t cpsr;
    cpsr.val = 0;
    cpsr.M = 0x10;
    d0->psr = cpsr.val;
    des->esp = d0->sp;
  }
  // d0->lr-=4;
  
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
