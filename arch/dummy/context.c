/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "context.h"

#include "cpu.h"

//获取上下文模式
int context_get_mode(context_t* context) {
  int mode = 0;

  return mode;
}

//上下文初始化
void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level) {
  if (context == NULL) {
    return;
  }
}

//上下文切换
void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;
}

//打印上下文信息
void context_dump(context_t* c) {}

//克隆上下文
void context_clone(context_t* des, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {}

//打印中断信息
void context_dump_interrupt(interrupt_context_t* context) {}