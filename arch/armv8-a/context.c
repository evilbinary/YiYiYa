/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "context.h"
#include "cpu.h"

int context_get_mode(context_t* context) {
  int mode = 0;
  
  return mode;
}

void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level) {
  if (context == NULL) {
    return;
  }
  
}

// #define DEBUG 1
void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;

}

void context_dump(context_t* c) {
  
}



void context_clone(context_t* des, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {
  
}
