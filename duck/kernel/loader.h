/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef LOADER_H
#define LOADER_H


#include "config.h"

#ifdef X86 
#define EXEC_ADDR  0x200000
#define STACK_ADDR 0x8000000
#define HEAP_ADDR  0x8200000

#elif defined(ARM)
#define EXEC_ADDR  0x71000000
#define STACK_ADDR 0x70000000
#define HEAP_ADDR  0x70100000
#else
#define EXEC_ADDR  0x71000000
#define STACK_ADDR 0x70000000
#define HEAP_ADDR  0x70100000
#endif

#ifndef MAX_PHDR 
#define MAX_PHDR 12
#endif

#ifndef MAX_SHDR 
#define MAX_SHDR 30
#endif

typedef struct exec{
    char* filename;
    int argc;
    char** argv;
    char **envp;
}exec_t;

typedef int (*entry_fn)(int, char**,char**);

void run_elf_thread();

#endif