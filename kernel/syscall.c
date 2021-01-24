/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "syscall.h"

#include "device.h"
#include "thread.h"

static int print(char* s) {
  cpu_cli();
  kprintf("%s", s);
  cpu_sti();
  return 0;
}

static size_t write(int fd, void* buf, size_t nbytes) {
  // get fd
  fd_t filedesc;
  filedesc.id = fd;
  device_t* dev = device_find(filedesc.id);
  u32 ret = dev->write(&fd, buf, nbytes);
  return ret;
}
static size_t read(int fd, void* buf, size_t nbytes) {
  // get fd
  fd_t filedesc;
  filedesc.id = fd;
  device_t* dev = device_find(filedesc.id);
  u32 ret = dev->read(&fd, buf, nbytes);
  return ret;
}

static size_t yeild(thread_t* thread) {
  if (thread != NULL) {
    thread->wait -= 2;
    schedule();
  }
}

static void* syscall_table[SYSCALL_NUMBER] = {&read, &write, &yeild, &print};

INTERRUPT_SERVICE
void syscall_handler() {
  interrupt_entering_code(ISR_SYSCALL, 0);
  interrupt_process(do_syscall);
  interrupt_exit();
}

void syscall_init() { interrutp_regist(ISR_SYSCALL, syscall_handler); }

void* do_syscall(interrupt_context_t context) {
  // kprintf("syscall %x\n",context.no);
  if (context.eax >= 0 && context.eax < SYSCALL_NUMBER) {
    void* fn = syscall_table[context.eax];
    if (fn != NULL) {
      sys_fn_call((&context), fn);
      return context.eax;
    } else {
      kprintf("syscall %x no found\n", context.no);
    }
  }
  return NULL;
}

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
  u32 ret;
  asm volatile("int %1\n"
               : "=a"(ret)
               : "i"(ISR_SYSCALL), "0"(num), "b"(arg0), "c"(arg1), "d"(arg2));
  if (ret > 0) {
    int i = 0;
  }
  return ret;
}
