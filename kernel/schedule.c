/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "schedule.h"

#include "thread.h"

volatile u32 timer_ticks = 0;
extern thread_t* current_thread;
extern thread_t* head_thread;

context_t* current_context;

thread_t* schedule_next() {
  thread_t* next = NULL;
  thread_t* v = head_thread;
  // find next priority
  current_thread->wait--;
  next = current_thread;
  for (; v != NULL; v = v->next) {
    //if (v == current_thread) continue;
    if ((v->state == THREAD_RUNNING) && (v->wait >= next->wait)) {
      next = v;
    }
  }
  return next;
}

void schedule() {
  thread_t* next_thread = NULL;
  thread_t* prev_thread = NULL;
  next_thread = schedule_next();
  prev_thread = current_thread;
  current_thread = next_thread;
  context_switch(&current_context, &prev_thread->context,
                 &next_thread->context);
}

void do_schedule() {
  schedule();
  timer_ticks++;
  io_write8(0x20, 0x20);
}

INTERRUPT_SERVICE
void do_timer() {
  context_save(current_context);
  // debugger;
  do_schedule();
  context_restore(current_context);
}

void schedule_init() {
  interrutp_regist(ISR_TIMER, do_timer);
  timer_init(100);
  if (current_thread == NULL) {
    if (head_thread == NULL) {
      kprintf("no thread please create a thread\n");
      cpu_halt();
    }
    current_thread = head_thread;
    current_context = &current_thread->context;
  }
}
