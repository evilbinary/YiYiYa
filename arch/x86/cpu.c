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

void set_ldt(u16 tss) {
  asm volatile(
      "movw %0, %%ax\n"
      "ltr %%ax\n" ::"r"(tss));
}

void cpu_init(int cpu) {
  unsigned long cr0 = read_cr0();
  kprintf("cpu %d init cr0=%x\n",cpu, cr0);

  u32 tss_base = (u32) & (boot_info->tss[cpu]);
  kprintf("tr base %x\n", tss_base);

  u64* gdt_addr = boot_info->gdt_base;
  kprintf("gdt base %x\n", gdt_addr);
  gdt_addr[GDT_ENTRY_32BIT_TSS] = GDT_ENTRY(tss_base, 0xfffff, 0xc089);  // 0x20

  // load gdt
  gdt_ptr_t gdt;
  gdt.limit = (boot_info->gdt_number * GDT_SIZE) - 1;
  gdt.base = (u32)boot_info->gdt_base;
  asm volatile("lgdtl %0\n" : : "m"(gdt));
  
  kprintf("idt base %x\n", boot_info->idt_base);

}

void cpu_halt() { asm("hlt\n"); }

void cpu_wait(){
  asm("hlt\n");
}

int cpu_tas(volatile int* addr, int newval) {
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

int cpu_start_id(u32 id,u32 entry){
  //start at 0x2000 at entry-point on boot init.h config
  return lapic_send_start(id,entry>>12);
}


void cpu_deplay_init(int hz){
  int val = 0;
  unsigned int divisor = 1193182/hz;
  val = (io_read8(0x61) & 0xfd) | 0x1;
  io_write8(0x61, val);
  io_write8(0x43, 0xb2);
  io_write8(0x42,(unsigned char)(divisor & 0xFF));		// LSB
  io_read8(0x60);
	io_write8(0x42,(unsigned char)(divisor >> 8) & 0xFF); // MSB

}

void cpu_delay_sleep(){
  int val = 0;
  val = (io_read8(0x61) & 0xfe);
  io_write8(0x61, val); // gate low
  val = val | 1;
  io_write8(0x61, val); // gate high
  while((io_read8(0x61) & 0x20) == 0){}
}

void cpu_delay(int n){
  for (int i = 0; i < 10000 * n; i++)
    ;
}