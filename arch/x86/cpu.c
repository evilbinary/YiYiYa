/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../cpu.h"

extern boot_info_t* boot_info;

static inline unsigned long read_cr0(void) {
  unsigned long val;
  asm volatile("movl %%cr0, %0" : "=r"(val) : : "memory");
  return val;
}

static inline void write_cr0(unsigned long val) {
  asm volatile("movl %0, %%cr0" : : "r"(val) : "memory");
}

static inline unsigned long read_cr2(void) {
  unsigned long val;

  asm volatile("mov %%cr2,%0\n\t" : "=r"(val) : : "memory");
  return val;
}

static inline unsigned long read_cr3(void) {
  unsigned long val;

  asm volatile("mov %%cr3,%0\n\t" : "=r"(val) : : "memory");
  return val;
}

static inline unsigned long read_cr4(void) {
  unsigned long val;
  asm volatile("mov %%cr4,%0\n\t" : "=r"(val) : : "memory");
  return val;
}

static inline void wbinvd(void) { asm volatile("wbinvd" : : : "memory"); }

static inline void invd(void) { asm volatile("invd" : : : "memory"); }

static inline void invlpg(void* virt) {
  asm volatile("invlpg %0\n\t" : : "m"(*(uint8_t*)virt));
}

void cpu_enable_paging_pae(ulong cr3) {
  asm volatile("cli");
  asm __volatile__(
      "movl	%0, %%cr3\n"

      "movl	%%cr4, %%eax\n"
      "orl	$0x00000020, %%eax\n"
      "movl	%%eax, %%cr4\n"

      "movl	%%cr0, %%eax\n"
      "orl	$0x80000000, %%eax\n"
      "movl	%%eax, %%cr0\n"
      :
      : "r"(cr3)
      : "eax");
}

static inline void cpu_disable_paging_pae(void) {
  asm __volatile__(
      "movl	%%cr0, %%eax\n"
      "andl	$0x7fffffff, %%eax\n"
      "movl	%%eax, %%cr0\n"
      "movl	%%cr4, %%eax\n"
      "andl	$0xffffffdf, %%eax\n"
      "movl	%%eax, %%cr4\n"
      :
      :
      : "eax");
}

static inline __attribute__((always_inline)) void cpu_hlt(void) { asm("hlt"); }

static inline ulong cpu_get_sp(void) {
  ulong result;
  asm volatile("mov %%esp, %%eax" : "=a"(result));
  return result;
}

static inline ulong cpu_get_bp(void) {
  ulong result;
  asm volatile("mov %%ebp, %%eax" : "=a"(result));
  return result;
}

static inline u32 cpu_get_ip(void) {
  u32 result;
  asm volatile("movl 0x4(%%ebp), %%eax" : "=a"(result));
  // u32* p;
  // asm volatile("movl -0x4(%%esp), %%eax" : "=a"(result));
  // asm volatile("movl %%esp, %%eax" : "=a"(p));
  return result;
}

ulong cpu_get_cs(void) {
  ulong result;
  asm volatile("mov %%cs, %%eax" : "=a"(result));
  return result;
}

static inline void set_ldt(u16 tss) {
  asm volatile(
      "movw %0, %%ax\n"
      "ltr %%ax\n" ::"r"(tss));
}

void cpu_init() {
  unsigned long cr0 = read_cr0();
  kprintf("cpu init cr0=%x\n", cr0);

  u32 tss_base = (u32) & (boot_info->tss[0]);
  kprintf("tr base %x\n", tss_base);

  u64* gdt_addr = boot_info->gdt_base;
  kprintf("gdt base %x\n", gdt_addr);
  gdt_addr[GDT_ENTRY_32BIT_TSS] = GDT_ENTRY(tss_base, 0xfffff, 0xc089);  // 0x20

  // load gdt
  gdt_ptr_t gdt;
  gdt.limit = (boot_info->gdt_number * GDT_SIZE) - 1;
  gdt.base = (u32)boot_info->gdt_base;
  asm volatile("lgdtl %0\n" : : "m"(gdt));

  kprintf("idt base %x\n", boot_info->pdt_base);

  acpi_init();
}

void cpu_halt() { asm("hlt\n"); }

void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level) {
  if (context == NULL) {
    return;
  }
  tss_t* tss = &boot_info->tss[0];
  context->tss = tss;
  context->eip = entry;
  context->level = level;
  u32 cs, ds;
  if (level == 0) {
    cs = GDT_ENTRY_32BIT_CS * GDT_SIZE;
    ds = GDT_ENTRY_32BIT_DS * GDT_SIZE;
  } else if (level == 3) {
    cs = GDT_ENTRY_USER_32BIT_CS * GDT_SIZE | level;
    ds = GDT_ENTRY_USER_32BIT_DS * GDT_SIZE | level;
  } else {
    kprintf("not suppport level %d\n", level);
  }
  interrupt_context_t* c = stack0;
  c->ss = ds;          // ss
  c->esp = stack3;     // esp
  c->eflags = 0x0200;  // eflags
  c->cs = cs;          // cs
  c->eip = entry;      // eip 4

  c->no = 0;             // no  5
  c->code = 0;           // no  5
  c->eax = 0;            // eax 6
  c->ecx = 0;            // ecx 7
  c->edx = 0;            // edx 8
  c->ebx = 0;            // ebx 9
  c->esp_null = stack0;  // esp 10
  c->ebp = stack3;       // ebp 11
  c->esi = 0;            // esi 12
  c->edi = 0;            // edi 13
  c->ds = ds;            // ds  14
  c->es = ds;            // es  15
  c->fs = ds;            // fs  16
  c->gs = ds;            // gs    17

  context->esp0 = c;
  context->ss0 = GDT_ENTRY_32BIT_DS * GDT_SIZE;
  context->ds0 = GDT_ENTRY_32BIT_DS * GDT_SIZE;
  context->esp = stack3;
  context->ss = ds;
  context->ds = ds;

  ulong addr = (ulong)boot_info->pdt_base;
  context->page_dir = addr;
  context->kernel_page_dir = addr;

  if (tss->eip == 0 && tss->cr3 == 0) {
    tss->ss0 = context->ss0;
    tss->esp0 = context->esp0 + sizeof(interrupt_context_t);
    tss->eip = context->eip;
    tss->ss = ds;
    tss->ds = tss->es = tss->fs = tss->gs = ds;
    tss->cs = cs;
    tss->esp = context->esp;
    tss->cr3 = boot_info->pdt_base;
    set_ldt(GDT_ENTRY_32BIT_TSS * GDT_SIZE);
  }
}

int context_get_mode(context_t* context) {
  int mode = 0;
  if (context != NULL) {
  }
  return mode;
}

void cpu_backtrace(stack_frame_t* fp, void** buf, int size) {
  int i;
  for (i = 0; i < size && fp != NULL; fp = fp->prev, i++) {
    buf[i] = fp->return_addr;
  }
}

void context_dump(context_t* c) {
  kprintf("eip:     %x\n", c->eip);
  kprintf("esp0:    %x\n", c->esp0);
  kprintf("esp:     %x\n", c->esp);

  kprintf("page_dir: %x\n", c->page_dir);
  kprintf("kernel page_dir: %x\n", c->kernel_page_dir);

  if (c->esp0 != 0) {
    context_dump_interrupt(c->esp0);
  }
}

void context_dump_interrupt(interrupt_context_t* context) {
  u32 cr2, cr3;
  u32 ds, es, fs, gs;
  asm volatile("movl	%%cr2,	%%eax" : "=a"(cr2));
  asm volatile("movl %%cr3,	%%eax" : "=a"(cr3));
  asm volatile("movl %%ds,	%%eax" : "=a"(ds));
  asm volatile("movl %%es,	%%eax" : "=a"(es));
  asm volatile("movl %%fs,	%%eax" : "=a"(fs));
  asm volatile("movl %%gs,	%%eax" : "=a"(gs));
  kprintf("cs:\t%x\teip:\t%x\teflags:\t%x\n", context->cs, context->eip,
          context->eflags);
  kprintf("ss:\t%x\tesp:\t%x\n", context->ss, context->esp);
  // kprintf("old ss:\t%x\told esp:%x\n", old_ss, old_esp);
  kprintf("code:%x\tcr2:\t%x\tcr3:\t%x\n", context->no, cr2, cr3);
  kprintf("General Registers:\n----------------------------\n");
  kprintf("eax:\t%x\tebx:\t%x\n", context->eax, context->ebx);
  kprintf("ecx:\t%x\tedx:\t%x\n", context->ecx, context->edx);
  kprintf("esi:\t%x\tedi:\t%x\tebp:\t%x\n", context->esi, context->edi,
          context->ebp);
  kprintf("Segment Registers:\n----------------------------\n");
  kprintf("ds:\t%x\tes:\t%x\n", ds, es);
  kprintf("fs:\t%x\tgs:\t%x\n", fs, gs);
  // char* buf[10]; 
  // stack_frame_t fp;
  // fp.prev=context->eip;
  // fp.return_addr=context->eip;
  // cpu_backtrace(&fp,buf,10);
  // kprintf("backtrace:\n");
  // for(int i=0;i<10;i++){
  //   kprintf("   %x\n",buf[i]);
  // }
}

void context_clone(context_t* des, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {
  *des = *src;
  interrupt_context_t* c0 = stack0;
  interrupt_context_t* s0 = src->esp0;
  if (stack0 != NULL) {
    *c0 = *s0;
    c0->eflags = 0x0200;
  }
  if (stack3 != NULL) {
    des->esp = s0->esp;
  }
  des->esp0 = (u32)c0;

  des->page_dir = page_alloc_clone(src->page_dir);
}

void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;

  if (context == NULL) {
    context = current_context->esp0;
    context->esp = current_context->esp;
  } else {
    current_context->esp0 = context;
    current_context->esp = context->esp;
  }

  interrupt_context_t* c = next_context->esp0;

  tss_t* tss = next_context->tss;
  tss->esp0 = next_context->esp0 + sizeof(interrupt_context_t);
  tss->ss0 = next_context->ss0;
  tss->cr3 = next_context->page_dir;

  *current = next_context;
  context_switch_page(next_context->page_dir);
}

int TAS(volatile int* addr, int newval) {
  int result = newval;
  asm volatile("lock; xchg %0, %1"
               : "+m"(*addr), "=r"(result)
               : "1"(newval)
               : "cc");
  return result;
}



int cpu_get_number(){
  return boot_info->tss_number;
}

u32 cpu_get_id(){
  return acpi_get_id();
}

u32 cpu_get_index(int idx){
  return lapic_get_index(idx);
}

int cpu_init_id(u32 id){
  return lapic_send_init(id);
}

int cpu_start_id(u32 id){
  //start at 0x2000 at entry-point on boot init.c
  return lapic_send_start(id,2);
}

void cpu_delay(int n){
  for (int i = 0; i < 10000 * n; i++)
    ;
}