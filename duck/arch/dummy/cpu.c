/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "cpu.h"

#include "context.h"

extern boot_info_t* boot_info;
u32 cpus_id[MAX_CPU];

// cpu 初始化处理
void cpu_init() {
  for (int i = 0; i < MAX_CPU; i++) {
    cpus_id[i] = i;
  }
}

// cpu 停机处理
void cpu_halt() {
  for (;;) {
  };
}

//获取代码段
ulong cpu_get_cs(void) {
  ulong result;

  return result;
}

//锁实现
int cpu_tas(volatile int* addr, int newval) {
  int result = newval;

  return result;
}

// cpu 打印栈信息
void cpu_backtrace(void) {}

//获取cpu id
u32 cpu_get_id() { return 0; }

// cpu睡眠等待
void cpu_wait() {}

// cpu 获取cpu个数
int cpu_get_number() { return boot_info->tss_number; }

// cpu获取下标
u32 cpu_get_index(int idx) {
  if (idx < 0 || idx > cpu_get_number()) {
    kprintf("out of bound get cpu idx\n");
    return 0;
  }
  return cpus_id[idx];
}

// cpu 初始化
int cpu_init_id(u32 id) { return 0; }

//启动cpu
int cpu_start_id(u32 id, u32 entry) { return 0; }

// cpu 延迟
void cpu_delay(int n) {
  for (int i = 0; i < 10000 * n; i++)
    ;
}
