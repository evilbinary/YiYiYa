/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "cpu.h"

#include "context.h"

extern boot_info_t* boot_info;
u32 cpus_id[MAX_CPU];

void cpu_init() {
  for (int i = 0; i < MAX_CPU; i++) {
    cpus_id[i] = i;
  }
}

void cpu_halt() { asm("hlt\n"); }

ulong cpu_get_cs(void) {
  ulong result;

  return result;
}

int cpu_tas(volatile int* addr, int newval) {
  int result = newval;

  return result;
}

void cpu_set_page(u32 page_table) {}

void cpu_backtrace(void) {}

u32 cpu_get_id() { return 0; }

void cpu_wait() { asm("hlt\n"); }

int cpu_get_number() { return boot_info->tss_number; }

u32 cpu_get_index(int idx) {
  if (idx < 0 || idx > cpu_get_number()) {
    kprintf("out of bound get cpu idx\n");
    return 0;
  }
  return cpus_id[idx];
}

int cpu_init_id(u32 id) { return 0; }

int cpu_start_id(u32 id, u32 entry) { return 0; }

void cpu_delay(int n) {
  for (int i = 0; i < 10000 * n; i++)
    ;
}