#include "syscall.h"
#ifdef ARM

#ifdef ARMV7

void* syscall0(u32 num) {
  int ret;
  // asm volatile(
  //     "mov r7,%1 \n\t"
  //     "svc 0x0\n\t"
  //     "mov %0,r0\n\t"
  //     : "=r"(ret)
  //     : "r"(num)
  //     : "r0", "r7");
  return ret;
}

void* syscall1(u32 num, void* arg0) {
  int ret;
  // asm volatile(
  //     "mov r7,%1 \n\t"
  //     "mov r0,%2 \n\t"
  //     "svc 0x0\n\t"
  //     "mov %0,r0\n\t"
  //     : "=r"(ret)
  //     : "r"(num), "r"(arg0)
  //     : "r0", "r1", "r2", "r3", "r4", "r7");
  return ret;
}
void* syscall2(u32 num, void* arg0, void* arg1) {
  int ret;
  // asm volatile(
  //     "mov r7,%1 \n\t"
  //     "mov r0,%2 \n\t"
  //     "mov r1,%3 \n\t"
  //     "swi 0x0\n\t"
  //     "mov %0,r0\n\t"
  //     : "=r"(ret)
  //     : "r"(num), "r"(arg0), "r"(arg1)
  //     : "r0", "r1", "r2", "r3", "r4", "r7", "memory");
  return ret;
}
void* syscall3(u32 num, void* arg0, void* arg1, void* arg2) {
  u32 ret = 0;
  // asm volatile(
  //     "mov r7,%1 \n\t"
  //     "mov r0,%2 \n\t"
  //     "mov r1,%3 \n\t"
  //     "mov r2,%4 \n\t"
  //     "svc 0x0\n\t"
  //     "mov %0,r0\n\t"
  //     : "=r"(ret)
  //     : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2)
  //     : "r0", "r1", "r2", "r3", "r4", "r7", "memory");
  return ret;
}

void* syscall4(u32 num, void* arg0, void* arg1, void* arg2, void* arg3) {
  u32 ret = 0;
  // asm volatile(
  //     "mov r7,%1 \n\t"
  //     "mov r0,%2 \n\t"
  //     "mov r1,%3 \n\t"
  //     "mov r2,%4 \n\t"
  //     "mov r3,%5 \n\t"
  //     "svc 0x0\n\t"
  //     "mov %0,r0\n\t"
  //     : "=r"(ret)
  //     : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3)
  //     : "r0", "r1", "r2", "r3", "r4", "r7", "memory");
  return ret;
}

void* syscall5(u32 num, void* arg0, void* arg1, void* arg2, void* arg3,
               void* arg4) {
  u32 ret = 0;
  // asm volatile(
  //     "mov r7,%1 \n\t"
  //     "mov r0,%2 \n\t"
  //     "mov r1,%3 \n\t"
  //     "mov r2,%4 \n\t"
  //     "mov r3,%5 \n\t"
  //     "mov r4,%6 \n\t"
  //     "svc 0x0\n\t"
  //     "mov %0,r0\n\t"
  //     : "=r"(ret)
  //     : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
  //     : "r0", "r1", "r2", "r3", "r4", "r7", "memory");
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
      : "r0", "r7");
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
      : "r0", "r1", "r2", "r3", "r4", "r7", "memory");
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
      : "r0", "r1", "r2", "r3", "r4", "r7", "memory");
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
      : "r0", "r1", "r2", "r3", "r4", "r7", "memory");
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
      : "r0", "r1", "r2", "r3", "r4", "r7", "memory");
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
#endif