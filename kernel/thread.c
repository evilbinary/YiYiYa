/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "thread.h"

#include "fd.h"
#include "syscall.h"

thread_t* current_thread = NULL;
thread_t* head_thread = NULL;
thread_t* tail_thread = NULL;
u32 thread_ids = 0;
extern context_t* current_context;

thread_t* thread_create_level(void* entry, void* data, u32 level) {
  u32 size = THREAD_STACK_SIZE;
#ifdef NO_THREAD_STACK0
  u8* stack0 = NULL;
#else
  u8* stack0 = kmalloc(size);
#endif
  u8* stack3 = kmalloc_alignment(size, PAGE_SIZE);
  thread_t* thread =
      thread_create_ex(entry, stack0, stack3, data, size, level, 1);
  return thread;
}

thread_t* thread_create_name(char* name, void* entry, void* data) {
  thread_t* t = thread_create(entry, data);
  char* kname = kmalloc(kstrlen(name));
  kstrcpy(kname, name);
  t->name = kname;
  return t;
}

thread_t* thread_create_name_level(char* name, void* entry, void* data,
                                   u32 level) {
  thread_t* t = thread_create_level(entry, data, level);
  char* kname = kmalloc(kstrlen(name));
  kstrcpy(kname, name);
  t->name = kname;
  return t;
}

thread_t* thread_create(void* entry, void* data) {
  return thread_create_level(entry, data, USER_MODE);
}

thread_t* thread_create_ex_name(char* name, void* entry, u32* stack0,
                                u32* stack3, u32 size, void* data, u32 level,
                                u32 page) {
  thread_t* t =
      thread_create_ex(entry, stack0, stack3, size, data, level, page);
  char* kname = kmalloc(kstrlen(name));
  kstrcpy(kname, name);
  t->name = kname;
  return t;
}

thread_t* thread_create_ex(void* entry, u32* stack0, u32* stack3, u32 size,
                           void* data, u32 level, u32 page) {
  thread_t* thread = kmalloc(sizeof(thread_t));
  thread->lock = 0;
  thread->data = data;
  thread->fd_size = 40;
  thread->fd_number = 0;
  thread->fds = kmalloc(sizeof(fd_t) * thread->fd_size);

  // vfs
  thread->vfs = kmalloc(sizeof(vfs_t));
  // file description
  thread_fill_fd(thread);

  thread_init(thread, entry, stack0, stack3, size, level);
  if (page == 1) {
    thread->context.page_dir = page_alloc_clone(thread->context.page_dir);
  }

  return thread;
}

void thread_fill_fd(thread_t* thread) {
  thread->fds[STDIN] = fd_find(STDIN);
  thread->fds[STDOUT] = fd_find(STDOUT);
  thread->fds[STDERR] = fd_find(STDERR);
  // thread->fds[STDSELF]=3;
  for (int i = STDIN; i <= STDERR; i++) {
    if (thread->fds[STDIN] != NULL) {
      thread->fd_number++;
    }
  }
}

void thread_sleep(thread_t* thread) {
  thread->state = THREAD_SLEEP;
  schedule_next();
}

void thread_wait(thread_t* thread) {
#ifdef DEBUG_THREAD
  kprintf("thread %d wait==============> %d\n", current_thread->id, thread->id);
#endif
  acquire(&thread->lock);
  thread->state = THREAD_WAITING;
  release(&thread->lock);
  schedule_next();
}

void thread_wake(thread_t* thread) {
#ifdef DEBUG_THREAD
  kprintf("thread %d wake==============> %d\n", current_thread->id, thread->id);
#endif
  acquire(&thread->lock);
  thread->state = THREAD_RUNNING;
  release(&thread->lock);
  schedule_next();
}

void thread_init(thread_t* thread, void* entry, u32* stack0, u32* stack3,
                 u32 size, u32 level) {
  u8* stack0_top = stack0;
  stack0_top += size;
  u8* stack3_top = stack3;
  stack3_top += size;
  thread->stack0 = stack0;
  thread->stack3 = stack3;
  thread->id = thread_ids++;
  thread->next = NULL;
  thread->priority = 1;
  thread->counter = 0;
  thread->state = THREAD_CREATE;
  thread->stack0_top = stack0_top;
  thread->stack3_top = stack3_top;
  context_init(&thread->context, (u32*)entry, stack0_top, stack3_top, level);
}

void thread_add(thread_t* thread) {
  cpu_cli();
  if (head_thread == NULL) {
    head_thread = thread;
    tail_thread = thread;
  } else {
    tail_thread->next = thread;
    tail_thread = thread;
  }
  thread->state = THREAD_RUNABLE;
  if (current_thread == NULL) {
    if (head_thread == NULL) {
      kprintf("no thread please create a thread\n");
      cpu_halt();
    }
    current_thread = head_thread;
    current_context = &current_thread->context;
  }
}

void thread_remove(thread_t* thread) {
  thread_t* v = head_thread;
  thread->state = THREAD_STOPPED;
  thread->counter += 1000;

  thread_t* prev = v;
  if (head_thread == v) {
    head_thread = v->next;
    thread_destroy(thread);
    return;
  }
  for (; v; v = v->next) {
    if (v == thread) {
      prev->next = v->next;
      thread_destroy(thread);
      break;
    }
    prev = v;
  }
}

void thread_destroy(thread_t* thread) {
  if (thread == NULL) return;
  if (thread->stack0 != NULL) {
    // kfree(thread->stack0);
  }
  if (thread->stack3 != NULL) {
    // kfree(thread->stack3);
  }
  kfree(thread);
}

void thread_stop(thread_t* thread) {
  if (thread == NULL) return;
  thread->state = THREAD_STOPPED;
}

void thread_exit(thread_t* thread, int code) {
  if (thread == NULL) return;
  thread_stop(thread);
  thread->code = code;
  // todo relase more
  // schedule_next();
  // cpu_sti();
}

thread_t* thread_find(thread_t* thread) {
  thread_t* v = head_thread;
  for (; v; v = v->next) {
    if (v->next == thread) {
      return v;
    }
  }
  return NULL;
}

void thread_run(thread_t* thread) {
  if (thread->state == THREAD_CREATE) {
    thread_add(thread);
  }
  if (thread->state == THREAD_RUNABLE) {
    thread->state = THREAD_RUNNING;
  }
}

void thread_yield() {
  thread_t* current = thread_current;
  if (current == NULL) {
    return;
  }
  if (current->state == THREAD_RUNNING) {
    current->counter++;
    schedule_next();
  }
}

thread_t* thread_current() { return current_thread; }

thread_t* thread_clone(thread_t* thread, u32* vstack3, u32 size) {
  if (thread == NULL) {
    return NULL;
  }
  thread_t* copy = kmalloc(sizeof(thread_t));

  copy->id = thread_ids++;
  copy->next = NULL;
  copy->state = THREAD_CREATE;
  copy->priority = thread->priority;
  copy->counter = thread->counter;
  copy->vmm = thread->vmm;
  copy->data = thread->data;
  copy->pid = thread->id;
  copy->name = thread->name;

  // copy files
  copy->fd_size = thread->fd_size;
  copy->fd_number = thread->fd_number;
  copy->fds = kmalloc(sizeof(fd_t) * thread->fd_size);
  kmemmove(copy->fds, thread->fds, sizeof(fd_t) * thread->fd_size);

  // copy stack0
  u8* copy_stack0 = kmalloc(size);
  u8* copy_stack0_top = copy_stack0 + size;
  copy->stack0_top = copy_stack0_top;
  copy->stack0 = copy_stack0;
  // kmemmove((u32)stack0_top-size, (u32)thread->stack0_top-size, size);
  // kmemmove(stack0_top, thread->stack0_top, size);
  kmemmove(copy_stack0, thread->stack0, size);

  // copy stack3
  u8* copy_stack3 = kmalloc_alignment(size, PAGE_SIZE);
  u8* copy_stack3_top = copy_stack3 + size;
  copy->stack3_top = copy_stack3_top;
  copy->stack3 = copy_stack3;

  u8* thread_stack3_top = thread->stack3_top;
  if (vstack3 != NULL) {
    copy->stack3 = vstack3;
    copy->stack3_top = (u32)vstack3 + size;
  }

  // use phy copy direct
  kprintf("copy_stack0: %x copy_stack3: %x \n", copy_stack0, copy_stack3);
  context_clone(&copy->context, &thread->context, copy_stack0_top,
                copy_stack3_top, thread->stack0_top, thread_stack3_top);

  if (vstack3 != NULL) {
    // may ref by vmm area
    u32 offset = 0;
    for (int i = 0; i < (size / PAGE_SIZE + 1); i++) {
      u8* p_thread_stack3 = (u8*)thread->stack3 + offset;
      p_thread_stack3 =
          virtual_to_physic(thread->context.page_dir, p_thread_stack3);
      if (p_thread_stack3 == NULL) {
        kprintf("thread stack3 is null\n");
        continue;
      }
      // kprintf("copy_stack3 %x p_thread_stack3 %x thread->stack3
      // %x\n",copy_stack3,p_thread_stack3,thread->stack3 );
      kmemmove(copy_stack3 + offset, p_thread_stack3, PAGE_SIZE);
      map_page_on(copy->context.page_dir, vstack3 + offset,
                  copy_stack3 + offset, PAGE_P | PAGE_USU | PAGE_RWW);
      offset += PAGE_SIZE;
    }
  }

  return copy;
}

int thread_find_fd_name(thread_t* thread, u8* name) {
  if (thread->fd_number == 0) {
    thread_fill_fd(thread);
  }
  if (thread->fd_number > thread->fd_size) {
    kprintf("thread find fd name limit %d > %d\n", thread->fd_number,
            thread->fd_size);
    return -1;
  }
  for (int i = 0; i < thread->fd_number; i++) {
    fd_t* fd = thread->fds[i];
    if (kstrcmp(name, fd->name) == 0) {
      return i;
    }
  }
  return -1;
}

int thread_add_fd(thread_t* thread, fd_t* fd) {
  if (thread->fd_number > thread->fd_size) {
    kprintf("thread add fd limit\n");
    return -1;
  }
  thread->fds[thread->fd_number] = fd;
  return thread->fd_number++;
}

fd_t* thread_find_fd_id(thread_t* thread, u32 fd) {
  if (thread->fd_number > thread->fd_size) {
    kprintf("thread find number limit %d\n", fd);
    return NULL;
  }
  if (fd > thread->fd_number) {
    kprintf("thread find fd limit %d > %d\n", fd, thread->fd_number);
    return NULL;
  }
  return thread->fds[fd];
}

fd_t* thread_set_fd(thread_t* thread, u32 fd, fd_t* nfd) {
  if (thread->fd_number > thread->fd_size) {
    kprintf("thread set number limit %d\n", fd);
    return NULL;
  }
  if (fd > thread->fd_number) {
    kprintf("thread set fd limit %d\n", fd);
    return NULL;
  }
  return thread->fds[fd] = nfd;
}

void thread_dump_fd(thread_t* thread) {
  for (int i = 0; i < thread->fd_number; i++) {
    fd_t* fd = thread->fds[i];
    kprintf("tid:%d fd:%d id:%d ptr:%x name:%s\n", thread->id, i, fd->id, fd,
            fd->name);
  }
}

void thread_dump(thread_t* thread) {
  if (thread == NULL) return;
  kprintf("id       %d\n", thread->id);
  if (thread->name != NULL) {
    kprintf("name   %s\n", thread->name);
  }
  kprintf("priority %d\n", thread->priority);
  kprintf("counter  %d\n", thread->counter);
  kprintf("state    %d\n", thread->state);
  kprintf("stack0   %x\n", thread->stack0);
  kprintf("stack3   %x\n", thread->stack3);
  kprintf("pid      %d\n", thread->pid);
  kprintf("fd_num   %d\n", thread->fd_number);
  kprintf("code     %d\n", thread->code);
  kprintf("--context--\n");
  context_dump(&thread->context);
  kprintf("\n");
}

void thread_dumps() {
  char* state_str[] = {"create", "running", "runnable", "stopped",
                       "waitting"
                       "sleep"};
  char* str = "unkown";
  kprintf("\n--------------threads--------------\n");
  kprintf("id    pid    name                 state    counter\n");
  for (thread_t* p = head_thread; p != NULL; p = p->next) {
    if (p->state <= THREAD_SLEEP) {
      str = state_str[p->state];
    }
    kprintf("%-6d ", p->id);
    kprintf("%-6d ", p->pid);

    if (p->name != NULL) {
      kprintf("%-20s ", p->name);
    } else {
      kprintf("   ");
    }
    kprintf("%-8s %d\n", str, p->counter);
  }
}

void thread_run_all() {
  thread_t* v = head_thread;
  for (; v; v = v->next) {
    thread_run(v);
  }
}