/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "schedule.h"

volatile u32 timer_ticks[MAX_CPU] = {0};
lock_t schedule_lock;
context_t* current_context[MAX_CPU] = {NULL};

thread_t* schedule_get_next() {
  thread_t* current = thread_current();
  thread_t* next = NULL;
  thread_t* v = thread_head();
  // find next priority
  // if(current_thread->counter<0){
  //   current_thread->counter=0;
  // }
  for (; v != NULL; v = v->next) {
    // kprintf("v %d %d\n",v->id,v->state);
    if (v == current) continue;
    if (v->state != THREAD_RUNNING) continue;
    if (next == NULL) {
      next = v;
    } else if (v->counter <= next->counter) {
      next = v;
    }
  }
  if (next == NULL) {
    next = thread_head();
  }
  next->counter++;
  return next;
}

void schedule_next() {
  cpu_cli();
  thread_t* current = thread_current();
  // interrupt_context_t* interrupt_context=current_thread->context.esp0;
  // schedule(interrupt_context);
  for (; current != current;) {
    cpu_sti();
  }
}

thread_t* schedule(interrupt_context_t* interrupt_context) {
  int cpu = cpu_get_id();
  thread_t* next_thread = NULL;
  thread_t* prev_thread = NULL;
  thread_t* current_thread = thread_current();
  next_thread = schedule_get_next();
  // kprintf("schedule next %s %d\n",next_thread->name,next_thread->id);
  if (next_thread == NULL) {
    kprintf("schedule error next\n");
    return;
  }
  current_context[cpu] = &current_thread->context;
  prev_thread = current_thread;
  current_thread = next_thread;
  context_switch(interrupt_context, &current_context[cpu],
                 &next_thread->context);
  thread_set_current(current_thread);
  return current_thread;
}

void do_schedule(interrupt_context_t* interrupt_context) {
  lock_acquire(&schedule_lock);
  int cpu = cpu_get_id();
  if (cpu > 0) {
    // kprintf("do do_schedule with %d\n",cpu);
  }
  thread_t* next = schedule(interrupt_context);
  timer_ticks[cpu]++;
  lock_release(&schedule_lock);
  timer_end();

}

INTERRUPT_SERVICE
void do_timer() {
  interrupt_entering_code(ISR_TIMER, 0);
  interrupt_process(do_schedule);
  interrupt_exit_context(current_context[cpu_get_id()]);
}

void schedule_init() {
  interrutp_regist(ISR_TIMER, do_timer);
  timer_init(1000);
  lock_init(&schedule_lock);
}
