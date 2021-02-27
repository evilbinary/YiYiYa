/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "syscall.h"

#include "device.h"
#include "kernel/stdarg.h"
#include "thread.h"
#include "vfs.h"

fd_t open_list[256];
int open_number = 0;

fd_t* find_fd(int fd) {
  for (int i = 0; i < open_number; i++) {
    if (open_list[i].id == fd) {
      return &open_list[i];
    }
  }
  return NULL;
}

int sys_print(char* s) {
  kprintf("%s", s);
  return 0;
}

int sys_print_at(char* s, u32 x, u32 y) {
  set_cursor(x, y);
  kprintf("%s", s);
  return 0;
}

size_t sys_ioctl(int fd, u32 cmd, ...) {
  u32 ret = 0;
  fd_t* f = find_fd(fd);
  if (f == NULL) {
    kprintf("read not found fd %d\n", fd);
    return 0;
  }
  va_list args;
  va_start(args, cmd);
  vnode_t* node = f->data;
  ret = vioctl(node, cmd, args);
  va_end(args);
  return ret;
}

u32 sys_open(char* name, int attr) {
  //mm_dump();
  vnode_t* node = vfind(NULL, name);
  if (node == NULL) {
    kprintf("open %s failed \n", name);
    return -1;
  }
  char dup_name[256] = {0};
  kstrcpy(dup_name, name);
  char* token = kstrtok(dup_name, node->name);
  char* last;
  while (token != NULL) {
    last = token;
    token = kstrtok(NULL, node->name);
  }
  if (last[0] == '/') {
    last++;
  }
  vnode_t* file = vfind(node, last);
  if (file == NULL) {
    kprintf("find %s failed\n", token);
    return -1;
  }
  // u32 ret = vopen(file);
  if (open_number > 256) {
    kprintf("open limit \n");
    return -1;
  }
  open_list[open_number].id = open_number;
  open_list[open_number].type = DEVICE_TYPE_FILE;
  open_list[open_number].data = file;
  open_list[open_number].offset = 0;
  return open_list[open_number++].id;
}

void sys_close(int fd) {}

size_t sys_write(int fd, void* buf, size_t nbytes) {
  fd_t* f = find_fd(fd);
  if (f == NULL) {
    kprintf("write not found fd %d\n", fd);
    return 0;
  }
  vnode_t* node = f->data;
  u32 ret = vwrite(node, f->offset, nbytes, buf);
  return ret;
}
size_t sys_read(int fd, void* buf, size_t nbytes) {
  fd_t* f = find_fd(fd);
  if (f == NULL) {
    kprintf("read not found fd %d\n", fd);
    return 0;
  }
  vnode_t* node = f->data;
  u32 ret = vread(node, f->offset, nbytes, buf);
  return ret;
}

size_t dev_ioctl(int fd, u32 cmd, ...) {
  u32 ret = 0;
  device_t* dev = device_find(fd);
  if (dev == NULL) {
    return ret;
  }
  va_list args;
  va_start(args, cmd);
  ret = dev->ioctl(dev, cmd, args);
  va_end(args);
  return ret;
}

size_t dev_read(int fd, void* buf, size_t nbytes) {
  device_t* dev = device_find(fd);
  return dev->read(dev, buf, nbytes);
}

size_t dev_write(int fd, void* buf, size_t nbytes) {
  device_t* dev = device_find(fd);
  return dev->write(dev, buf, nbytes);
}

size_t sys_yeild(thread_t* thread) {
  if (thread != NULL) {
    thread->counter++;
    schedule();
  }
}

static void* syscall_table[SYSCALL_NUMBER] = {
    &sys_read, &sys_write, &sys_yeild, &sys_print, &sys_print_at, &sys_ioctl,
    &sys_open, &sys_close, &dev_read,  &dev_write, &dev_ioctl};

INTERRUPT_SERVICE
void syscall_handler() {
  interrupt_entering_code(ISR_SYSCALL, 0);
  interrupt_process(do_syscall);
  interrupt_exit();
}

extern context_t* current_context;
void syscall_init() { interrutp_regist(ISR_SYSCALL, syscall_handler); }

void* do_syscall(interrupt_context_t* context) {
  // kprintf("syscall %x\n",context.no);
  // current_context->esp0 = context;
  // current_context->esp = context->esp;

  if (context->eax >= 0 && context->eax < SYSCALL_NUMBER) {
    void* fn = syscall_table[context->eax];
    if (fn != NULL) {
      sys_fn_call((context), fn);
      return context->eax;
    } else {
      kprintf("syscall %x not found\n", context->eax);
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
  u32 ret = 0;
  asm volatile("int %1\n"
               : "=a"(ret)
               : "i"(ISR_SYSCALL), "0"(num), "b"(arg0), "c"(arg1), "d"(arg2));
  return ret;
}
