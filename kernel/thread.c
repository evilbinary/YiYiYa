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

thread_t* thread_create(void* entry, void* data) {
  u32 size = PAGE_SIZE;
  u8* stack0 = kmalloc_alignment(size,4*8);
  u8* stack3 = kmalloc_alignment(size,4*8);
  thread_t* thread = thread_create_ex(entry, stack0, stack3, data, size);
  return thread;
}

thread_t* thread_create_ex(void* entry, u32* stack0, u32* stack3, u32 size,
                           void* data) {
  thread_t* thread = kmalloc(sizeof(thread_t));
  thread->lock=0;
  thread->data = data;
  thread->fd_size = 100;
  thread->fd_number = 0;
  thread->fds = kmalloc(sizeof(fd_t) * thread->fd_size);

  // file description
  thread->fds[STDIN] = fd_find(STDIN);
  thread->fds[STDOUT] = fd_find(STDOUT);
  thread->fds[STDERR] = fd_find(STDERR);
  // thread->fds[STDSELF]=3;
  thread->fd_number = 3;

  thread_init(thread, entry, stack0, stack3, size);
  return thread;
}

void thread_sleep(thread_t* thread) {
  thread->state = THREAD_SLEEP;
  schedule_next();
}

void thread_wait(thread_t* thread) {
#ifdef DEBUG_THREAD
  kprintf("thread %d wait==============> %d\n",current_thread->id,thread->id);
#endif
  acquire(&thread->lock);
  thread->state = THREAD_WAITING;
  release(&thread->lock);
  schedule_next();
}

void thread_wake(thread_t* thread) {
#ifdef DEBUG_THREAD
  kprintf("thread %d wake==============> %d\n",current_thread->id,thread->id);
#endif
  acquire(&thread->lock);
  thread->state = THREAD_RUNNING;
  release(&thread->lock);
  schedule_next();
}

void thread_init(thread_t* thread, void* entry, u32* stack0, u32* stack3,
                 u32 size) {
  u8* stack0_top = stack0;
  u8* stack3_top = stack3;
  stack3_top += size - 1024;
  thread->stack0 = stack0;
  thread->stack3 = stack3;
  thread->id = thread_ids++;
  thread->next = NULL;
  thread->priority = 1;
  thread->counter = 0;
  thread->state = THREAD_CREATE;
  thread->stack0 = stack0;
  thread->stack3 = stack3;
  thread->stack0_top = stack0_top;
  thread->stack3_top = stack3_top;
  context_init(&thread->context, (u32*)entry, stack0_top, stack3_top,
               USER_MODE);
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

void thread_exit(thread_t* thread,int code) {
  if (thread == NULL) return;
  thread_stop(thread);
  thread->code=code;
  // schedule_next();
  //cpu_sti();
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
  copy->fd_size = thread->fd_size;
  copy->fd_number = thread->fd_number;
  copy->fds = kmalloc(sizeof(fd_t) * thread->fd_size);

  kmemmove(copy->fds, thread->fds, sizeof(fd_t) * thread->fd_size);

  u8* stack0 = kmalloc(size);
  u8* stack3 = kmalloc_alignment(size, PAGE_SIZE);
  kmemmove(stack0, thread->stack0, size);

  u8* stack0_top = stack0 + size;
  u8* stack3_top = vstack3 + size;

  copy->stack0_top = stack0_top;
  copy->stack3_top = stack3_top;
  kprintf("stack0:%x stack3:%x \n", stack0, stack3);
  context_clone(&copy->context, &thread->context, stack0_top, stack3_top,
                thread->stack0_top, thread->stack3_top);
  copy->stack0 = stack0;
  copy->stack3 = vstack3;
  copy->id = thread_ids++;
  copy->next = NULL;
  copy->state = THREAD_CREATE;
  copy->priority = thread->priority;
  copy->counter = thread->counter;
  copy->vmm = thread->vmm;
  copy->data = thread->data;
  copy->context.page_dir = page_alloc_clone(thread->context.page_dir);

  // unmap_page_on(copy->context.page_dir,vstack3);

  // may ref by vmm area
  u32 thread_stack3 = thread->stack3;
  for (int i = 0; i < 1; i++) {
    u32 phy_thread_stack3 =
        virtual_to_physic(thread->context.page_dir, thread_stack3);
    kmemmove(stack3, phy_thread_stack3, size);
    map_page_on(copy->context.page_dir, vstack3, stack3,
                PAGE_P | PAGE_USU | PAGE_RWW);
    vstack3 += PAGE_SIZE;
    thread_stack3 += PAGE_SIZE;
    stack3 + PAGE_SIZE;
  }
  return copy;
}

int thread_find_fd_name(thread_t* thread, u8* name) {
  if (thread->fd_number > thread->fd_size) {
    kprintf("thread find fd name limit\n");
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
    kprintf("thread find fd limit %d\n", fd);
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

void thread_dump_fd(thread_t* thread){
   for (int i = 0; i < thread->fd_number; i++) {
    fd_t* fd = thread->fds[i];
    kprintf("tid:%d fd:%d id:%d ptr:%x name:%s\n",thread->id,i,fd->id,fd,fd->name);
  }
}

void thread_dumps(){
  //cpu_cli();
  kprintf("tid state counter\n");
  for(thread_t* p=head_thread;p!=NULL;p=p->next){
    kprintf("%d  %d %d\n",p->id,p->state,p->counter);
  }
}