/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "thread.h"

#include "syscall.h"

thread_t* current_thread = NULL;
thread_t* head_thread = NULL;
thread_t* tail_thread = NULL;
u32 thread_ids = 0;
extern context_t* current_context;

thread_t* thread_create(void* entry) {
  thread_t* thread = kmalloc(sizeof(thread_t));
  int size = sizeof(u32) * 64;
  u32* stack0 = kmalloc(size);
  u32* stack3 = kmalloc(size);
  thread_init(thread, entry, stack0, stack3);
  return thread;
}

void thread_sleep(thread_t* thread) { thread->state == THREAD_SLEEP; }

void thread_wake(thread_t* thread) { thread->state == THREAD_RUNNING; }

void thread_init(thread_t* thread, void* entry, u32* stack0, u32* stack3) {
  thread->id = thread_ids++;
  thread->next = NULL;
  thread->priority = 1;
  thread->counter = 0;
  thread->state = THREAD_CREATE;
  context_init(&thread->context, (u32*)entry, stack0, stack3, USER_MODE);
}

void thread_add(thread_t* thread) {
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
  thread_t* t = NULL;
  for (; v; v = v->next) {
    if (v->next == thread) {
      t = v->next;
      v->next = t->next;
      t->state = THREAD_STOPPED;
      break;
      // kprintf("addr:%x state:%d p:%d",v,v->state,v->priority);
    }
  }
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
  if (current->state == THREAD_RUNNING) {
    syscall1(SYS_YIELD, current);
  }
}

thread_t* thread_current() { return current_thread; }