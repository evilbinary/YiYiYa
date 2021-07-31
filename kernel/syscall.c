/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "syscall.h"

extern context_t* current_context;
static void* syscall_table[SYSCALL_NUMBER] = {
    &sys_read,  &sys_write,  &sys_yeild,  &sys_print,   &sys_print_at,
    &sys_ioctl, &sys_open,   &sys_close,  &dev_read,    &dev_write,
    &dev_ioctl, &sys_exec,   &sys_test,   &sys_exit,    &sys_vmap,
    &sys_vumap, &sys_seek,   &sys_valloc, &sys_vfree,   &sys_vheap,
    &sys_fork,  &sys_pipe,   &sys_getpid, &sys_getppid, &sys_dup,
    &sys_dup2,  &sys_readdir};

INTERRUPT_SERVICE
void syscall_handler() {
  interrupt_entering_code(ISR_SYSCALL, 0);
  interrupt_process(do_syscall);
  interrupt_exit();
  // interrupt_exit_context(current_context);
}

void syscall_init() { interrutp_regist(ISR_SYSCALL, syscall_handler); }

void* do_syscall(interrupt_context_t* context) {
  // kprintf("syscall %x\n",context->no);
  // current_context->esp0 = context;
  // current_context->esp = context->esp;
  if (context_fn(context) >= 0 && context_fn(context) < SYSCALL_NUMBER) {
    void* fn = syscall_table[context_fn(context)];
    if (fn != NULL) {
      // kprintf("syscall fn:%d r0:%x r1:%x r2:%x r3:%x\n",context->r7,context->r0,context->r1,context->r2,context->r3);
      sys_fn_call((context), fn);
      // kprintf(" ret=%x\n",context_ret(context));
      return context_ret(context);
    } else {
      kprintf("syscall %x not found\n", context_fn(context));
    }
  }
  return NULL;
}

#ifdef ARM
void* syscall0(u32 num) {
  int ret;
  asm volatile(
      "mov r7,%1 \n\t"
      "svc 0x0\n\t"
      "mov %0,r0\n\t"
      : "=r"(ret)
      : "r"(num)
      :"r0", "r1", "r2", "r3", "r4", "r7");
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
      :"r0", "r1", "r2", "r3", "r4", "r7");
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
      :"r0", "r1", "r2", "r3", "r4", "r7");
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
      :"r0", "r1", "r2", "r3", "r4", "r7");
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
      :);
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
      :);
  return ret;
}

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
#endif