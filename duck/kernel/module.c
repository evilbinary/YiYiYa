/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "module.h"

module_t* modules[MAX_MODULES];
u32 module_number=0;

void module_init() {
  module_number = 0;
}

void module_regist(module_t* mod) {
  if (module_number > MAX_MODULES) {
    kprintf("add module full %d\n", module_number);
    return;
  }
  modules[module_number++] = mod;
  if (mod->init != NULL) {
    void (*init)() = mod->init;
    init();
  }
}

void module_unregist(module_t* mod) {
  for (int i = 0; i < module_number; i++) {
    if (modules[i] == mod) {
      // todo xxx
      void (*exit)() = mod->exit;
      exit();
    }
  }
}

module_t* module_find(char* name) {
  for (int i = 0; i < module_number; i++) {
    if (kstrcmp(modules[i]->name, name) == 0) {
      return modules[i];
    }
  }
  return NULL;
}