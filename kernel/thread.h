/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef THREAD_H
#define THREAD_H

#include "arch/arch.h"

#define THREAD_CREATE	0
#define THREAD_RUNNING	1
#define THREAD_RUNABLE	2
#define THREAD_STOPPED	3
#define THREAD_WAITING  4
#define THREAD_SLEEP	5
#define THREAD_UNINTERRUPTIBLE 15

typedef struct thread_s{
    u32 id;
    context_t context;
    int priority;
    int wait;
    int state;
    struct thread_s * next;
}thread_t;


thread_t * thread_create();

void thread_sleep(thread_t * thread);

void thread_wake(thread_t * thread);

void thread_add(thread_t* thread);

void thread_remove(thread_t * thread);

void thread_run(thread_t* thread);

void thread_yield();
thread_t* thread_current();

#endif