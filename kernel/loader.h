/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef LOADER_H
#define LOADER_H



#ifdef X86 
#define EXEC_ADDR  0x200000
#define STACK_ADDR 0x8000000
#define HEAP_ADDR  0x8200000

#elif defined(V3S)
#define EXEC_ADDR  0x50000000
#define STACK_ADDR 0x60000000
#define HEAP_ADDR  0x61000000
#else
#define EXEC_ADDR  0x4000000
#define STACK_ADDR 0x8000000
#define HEAP_ADDR  0x8200000
#endif

#define MAX_PHDR 10
#define MAX_SHDR 30

typedef struct exec{
    char filename[256];
    int argc;
    char** argv;
}exec_t;

typedef int (*entry_fn)(int, char**);

void run_elf_thread();

#endif