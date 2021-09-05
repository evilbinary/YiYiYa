/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef THREAD_H
#define THREAD_H

#include "arch/arch.h"
#include "memory.h"
#include "device.h"
#include "fd.h"

#define THREAD_CREATE	0
#define THREAD_RUNNING	1
#define THREAD_RUNABLE	2
#define THREAD_STOPPED	3
#define THREAD_WAITING  4
#define THREAD_SLEEP	5
#define THREAD_UNINTERRUPTIBLE 15

#define KERNEL_THREAD_STACK_SIZE 1024*2
#define THREAD_STACK_SIZE PAGE_SIZE

typedef struct thread {
    u32 id;
    context_t context;
    int priority;
    int counter;
    int state;
    void* stack0;
    void* stack3;
    void* stack0_top;
    void* stack3_top;
    struct thread_s * next;
    void* data;
    vmemory_area_t* vmm;
    u32 pid;
    u32** fds;
    u32 fd_size;
    u32 fd_number;
    u32 lock;
    u32 code;
}thread_t;


thread_t * thread_create(void* entry,void* data);
thread_t* thread_create_ex(void* entry, u32* stack0, u32* stack3, u32 size,
                           void* data);

void thread_sleep(thread_t * thread);

void thread_wake(thread_t * thread);

void thread_add(thread_t* thread);

void thread_remove(thread_t * thread);

void thread_run(thread_t* thread);
void thread_stop(thread_t* thread);
void thread_exit(thread_t* thread, int code);

void thread_yield();
thread_t* thread_current();
thread_t* thread_clone(thread_t* thread, u32* vstack3, u32 size);
int thread_add_fd(thread_t* thread,fd_t* fd);
fd_t* thread_find_fd_id(thread_t* thread,u32 fd);
int thread_find_fd_name(thread_t* thread, u8* name);
fd_t* thread_set_fd(thread_t* thread, u32 fd,fd_t* nfd);

#endif