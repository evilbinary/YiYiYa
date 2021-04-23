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
u32 schedule_lock = 0;

context_t* current_context;

thread_t* schedule_get_next() {
  thread_t* next = NULL;
  thread_t* v = head_thread;
  // find next priority
  // if(current_thread->counter<0){
  //   current_thread->counter=0;
  // }
  for (; v != NULL; v = v->next) {
    //kprintf("v %d %d\n",v->id,v->state);
    if(v==current_thread) continue;
    if (v->state != THREAD_RUNNING) continue;
    if(next==NULL){
      next=v;
    }else if( v->counter <= next->counter ) {
      next = v;
    }
  }
  next->counter++;
  return next;
}

void schedule_next(){
  cpu_cli();
  thread_t* current=current_thread;
  // interrupt_context_t* interrupt_context=current_thread->context.esp0;
  // schedule(interrupt_context);
  for(;current!=current_thread;){
      cpu_sti();
  }
}

void schedule(interrupt_context_t* interrupt_context) {
  thread_t* next_thread = NULL;
  thread_t* prev_thread = NULL;
  next_thread = schedule_get_next();
  //kprintf("schedule next %d\n",next_thread->id);
  if (next_thread == NULL) {
    kprintf("schedule error next\n");
  }
  prev_thread = current_thread;
  current_thread = next_thread;
  context_switch(interrupt_context,&current_context,&next_thread->context);
}

void do_schedule(interrupt_context_t* interrupt_context) {
  schedule(interrupt_context);
  timer_ticks++;
  io_write8(0x20, 0x20);
}

INTERRUPT_SERVICE
void do_timer() {
  interrupt_entering_code(ISR_TIMER,0);
  interrupt_process(do_schedule);
  //interrupt_exit();
  interrupt_exit_context(current_context);
}

void schedule_init() {
  interrutp_regist(ISR_TIMER, do_timer);
  timer_init(1000);
}
