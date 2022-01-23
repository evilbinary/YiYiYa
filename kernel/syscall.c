/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "syscall.h"

extern context_t* current_context;
static void* syscall_table[SYSCALL_NUMBER] = {0};

INTERRUPT_SERVICE
void syscall_handler() {
  interrupt_entering_code(ISR_SYSCALL, 0);
  interrupt_process(do_syscall);
  interrupt_exit();
  // interrupt_exit_context(current_context);
}

void syscall_init() {
  interrutp_regist(ISR_SYSCALL, syscall_handler);
  syscall_table[SYS_READ] = &sys_read;
  syscall_table[SYS_WRITE] = &sys_write;
  syscall_table[SYS_YIELD] = &sys_yeild;
  syscall_table[SYS_PRINT] = &sys_print;
  syscall_table[SYS_PRINT_AT] = &sys_print_at;
  syscall_table[SYS_IOCTL] = &sys_ioctl;
  syscall_table[SYS_OPEN] = &sys_open;
  syscall_table[SYS_CLOSE] = &sys_close;
  syscall_table[SYS_DEV_READ] = &dev_read;
  syscall_table[SYS_DEV_WRITE] = &dev_write;
  syscall_table[SYS_DEV_IOCTL] = &dev_ioctl;
  syscall_table[SYS_EXEC] = &sys_exec;
  syscall_table[SYS_TEST] = &sys_test;
  syscall_table[SYS_EXIT] = &sys_exit;
  syscall_table[SYS_MAP] = &sys_vmap;
  syscall_table[SYS_UMAP] = &sys_vumap;
  syscall_table[SYS_SEEK] = &sys_seek;
  syscall_table[SYS_VALLOC] = &sys_valloc;
  syscall_table[SYS_VFREE] = &sys_vfree;
  syscall_table[SYS_VHEAP] = &sys_vheap;
  syscall_table[SYS_FORK] = &sys_fork;
  syscall_table[SYS_PIPE] = &sys_pipe;
  syscall_table[SYS_GETPID] = &sys_getpid;
  syscall_table[SYS_GETPPID] = &sys_getppid;
  syscall_table[SYS_DUP] = &sys_dup;
  syscall_table[SYS_DUP2] = &sys_dup2;
  syscall_table[SYS_READDIR] = &sys_readdir;
  syscall_table[SYS_BRK] = &sys_brk;
  syscall_table[SYS_READV] = &sys_readv;
  syscall_table[SYS_WRITEV] = &sys_writev;
  syscall_table[SYS_CHDIR] = &sys_chdir;
  syscall_table[SYS_MMAP2] = &sys_mmap2;
  syscall_table[SYS_MPROTECT] = &sys_mprotect;
  syscall_table[SYS_RT_SIGPROCMASK] = &sys_rt_sigprocmask;

  syscall_table[SYS_ALARM] = &sys_alarm;
  syscall_table[SYS_UNLINK] = &sys_unlink;
  syscall_table[SYS_RENAME] = &sys_rename;

  syscall_table[SYS_RENAME] = &sys_rename;

  syscall_table[SYS_SET_THREAD_AREA]=&sys_set_thread_area;
  syscall_table[SYS_DUMPS] = &sys_dumps;

  syscall_table[SYS_GETDENTS64] = &sys_getdents64;
  syscall_table[SYS_MUNMAP] = &sys_munmap;

  syscall_table[SYS_FCNT64] = &sys_fcntl64;


  
}

void* do_syscall(interrupt_context_t* context) {
  // kprintf("syscall %d\n", context_fn(context));
  if (context_fn(context) >= 0 && context_fn(context) < SYSCALL_NUMBER) {
    void* fn = syscall_table[context_fn(context)];
    if (fn != NULL) {
      // kprintf("syscall fn:%d r0:%x r1:%x r2:%x
      // r3:%x\n",context->r7,context->r0,context->r1,context->r2,context->r3);
      sys_fn_call((context), fn);
      // kprintf(" ret=%x\n",context_ret(context));
      return context_ret(context);
    } else {
      kprintf("syscall %d not found\n", context_fn(context));
    }
  }
  return NULL;
}

#ifdef ARM

#ifdef ARMV7
void* syscall0(u32 num) {
  int ret;
  asm volatile(
      "mov r6,%1 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num)
      : "r0", "r6");
  return ret;
}

void* syscall1(u32 num, void* arg0) {
  int ret;
  asm volatile(
      "mov r6,%1 \n\t"
      "mov r0,%2 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0)
      : "r0", "r1", "r2", "r3", "r4", "r6");
  return ret;
}
void* syscall2(u32 num, void* arg0, void* arg1) {
  int ret;
  asm volatile(
      "mov r6,%1 \n\t"
      "mov r0,%2 \n\t"
      "mov r1,%3 \n\t"
      "swi 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0), "r"(arg1)
      : "r0", "r1", "r2", "r3", "r4", "r6", "memory");
  return ret;
}
void* syscall3(u32 num, void* arg0, void* arg1, void* arg2) {
  u32 ret = 0;
  asm volatile(
      "mov r6,%1 \n\t"
      "mov r0,%2 \n\t"
      "mov r1,%3 \n\t"
      "mov r2,%4 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2)
      : "r0", "r1", "r2", "r3", "r4", "r6", "memory");
  return ret;
}

void* syscall4(u32 num, void* arg0, void* arg1, void* arg2, void* arg3) {
  u32 ret = 0;
  asm volatile(
      "mov r6,%1 \n\t"
      "mov r0,%2 \n\t"
      "mov r1,%3 \n\t"
      "mov r2,%4 \n\t"
      "mov r3,%5 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3)
      : "r0", "r1", "r2", "r3", "r4", "r6", "memory");
  return ret;
}

void* syscall5(u32 num, void* arg0, void* arg1, void* arg2, void* arg3,
               void* arg4) {
  u32 ret = 0;
  asm volatile(
      "mov r6,%1 \n\t"
      "mov r0,%2 \n\t"
      "mov r1,%3 \n\t"
      "mov r2,%4 \n\t"
      "mov r3,%5 \n\t"
      "mov r4,%6 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
      : "r0", "r1", "r2", "r3", "r4", "r6", "memory");
      return ret;
}
#else
void* syscall0(u32 num) {
  int ret;
  asm volatile(
      "mov r7,%1 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num)
      : "r0", "r1", "r2", "r3", "r4", "r7");
  return ret;
}

void* syscall1(u32 num, void* arg0) {
  int ret;
  asm volatile(
      "mov r7,%1 \n\t"
      "mov r0,%2 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0)
      : "r0", "r1", "r2", "r3", "r4", "r7");
  return ret;
}
void* syscall2(u32 num, void* arg0, void* arg1) {
  int ret;
  asm volatile(
      "mov r7,%1 \n\t"
      "mov r0,%2 \n\t"
      "mov r1,%3 \n\t"
      "swi 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0), "r"(arg1)
      : "r0", "r1", "r2", "r3", "r4", "r7");
  return ret;
}
void* syscall3(u32 num, void* arg0, void* arg1, void* arg2) {
  u32 ret = 0;
  asm volatile(
      "mov r7,%1 \n\t"
      "mov r0,%2 \n\t"
      "mov r1,%3 \n\t"
      "mov r2,%4 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2)
      : "r0", "r1", "r2", "r3", "r4", "r7");
  return ret;
}

void* syscall4(u32 num, void* arg0, void* arg1, void* arg2, void* arg3) {
  u32 ret = 0;
  asm volatile(
      "mov r7,%1 \n\t"
      "mov r0,%2 \n\t"
      "mov r1,%3 \n\t"
      "mov r2,%4 \n\t"
      "mov r3,%5 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3)
      : "r0", "r1", "r2", "r3", "r4", "r7");
  return ret;
}

void* syscall5(u32 num, void* arg0, void* arg1, void* arg2, void* arg3,
               void* arg4) {
  u32 ret = 0;
  asm volatile(
      "mov r7,%1 \n\t"
      "mov r0,%2 \n\t"
      "mov r1,%3 \n\t"
      "mov r2,%4 \n\t"
      "mov r3,%5 \n\t"
      "mov r4,%6 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
      : "r0", "r1", "r2", "r3", "r4", "r7");
  return ret;
}
#endif

#elif defined(X86)
void* syscall0(u32 num) {
  int ret;
  asm("int %1" : "=a"(ret) : "i"(ISR_SYSCALL), "0"(num));
  return ret;
}

void* syscall1(u32 num, void* arg0) {
  int ret;
  asm volatile("int %1\n" : "=a"(ret) : "i"(ISR_SYSCALL), "0"(num), "b"(arg0));
  return ret;
}
void* syscall2(u32 num, void* arg0, void* arg1) {
  int ret;
  asm volatile("int %1\n"
               : "=a"(ret)
               : "i"(ISR_SYSCALL), "0"(num), "b"(arg0), "c"(arg1));
  return ret;
}
void* syscall3(u32 num, void* arg0, void* arg1, void* arg2) {
  u32 ret = 0;
  asm volatile("int %1\n"
               : "=a"(ret)
               : "i"(ISR_SYSCALL), "0"(num), "b"(arg0), "c"(arg1), "d"(arg2));
  return ret;
}

void* syscall4(u32 num, void* arg0, void* arg1, void* arg2, void* arg3) {
  u32 ret = 0;
  asm volatile("int %1\n"
               : "=a"(ret)
               : "i"(ISR_SYSCALL), "0"(num), "b"(arg0), "c"(arg1), "d"(arg2),
                 "S"(arg3));
  return ret;
}

void* syscall5(u32 num, void* arg0, void* arg1, void* arg2, void* arg3,
               void* arg4) {
  u32 ret = 0;
  asm volatile("int %1\n"
               : "=a"(ret)
               : "i"(ISR_SYSCALL), "0"(num), "b"(arg0), "c"(arg1), "d"(arg2),
                 "S"(arg3), "D"(arg4));
  return ret;
}
#elif defined(XTENSA)
void* syscall0(u32 num) {
  int ret;

  return ret;
}

void* syscall1(u32 num, void* arg0) {
  int ret;

  return ret;
}
void* syscall2(u32 num, void* arg0, void* arg1) {
  int ret;

  return ret;
}
void* syscall3(u32 num, void* arg0, void* arg1, void* arg2) {
  u32 ret = 0;

  return ret;
}

void* syscall4(u32 num, void* arg0, void* arg1, void* arg2, void* arg3) {
  u32 ret = 0;

  return ret;
}

void* syscall5(u32 num, void* arg0, void* arg1, void* arg2, void* arg3,
               void* arg4) {
  u32 ret = 0;

  return ret;
}

#endif