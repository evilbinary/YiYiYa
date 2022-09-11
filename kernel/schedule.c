/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "schedule.h"

u32 timer_ticks[MAX_CPU] = {0};
lock_t schedule_lock;

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

void schedule_next(){

}

void schedule_sleep(u32 nsec) {
  thread_t* current = thread_current();
  thread_sleep(current,nsec/SCHEDULE_FREQUENCY*1000);
}

void schedule_state(int cpu){
  thread_t* v = thread_head();
  for (; v != NULL; v = v->next) {
    if(v->sleep_counter<0){
      thread_wake(v);
    }
    if(v->state==THREAD_SLEEP){
      u32 ticks=timer_ticks[cpu];
      v->sleep_counter-= ticks-v->counter;
    }
  }
}


void* do_schedule(interrupt_context_t* interrupt_context) {
  int cpu = cpu_get_id();
  thread_t* next_thread = NULL;
  thread_t* prev_thread = NULL;
  thread_t* current_thread = thread_current();
  schedule_state(cpu);
  next_thread = schedule_get_next();
  // kprintf("schedule next %s %d\n",next_thread->name,next_thread->id);
  if (next_thread == NULL) {
    kprintf("schedule error next\n");
    return NULL;
  }
  context_t* c = &current_thread->context;
  prev_thread = current_thread;
  current_thread = next_thread;
  context_switch(interrupt_context, &c, &next_thread->context);
  thread_set_current(next_thread);
  timer_ticks[cpu]++;
  timer_end();
  return c->esp0;
}

INTERRUPT_SERVICE
void do_timer() {
  interrupt_entering_code(ISR_TIMER, 0);
  interrupt_process(do_schedule);
  interrupt_exit_ret();
}

void schedule_init() {
  lock_init(&schedule_lock);
  interrutp_regist(ISR_TIMER, do_timer);
  timer_init(SCHEDULE_FREQUENCY);
}
