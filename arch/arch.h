/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef ARCH_H
#define ARCH_H

#include "libs/include/types.h"
#include "boot.h"
#include "cpu.h"
#include "display.h"
#include "mm.h"
#include "libs/include/kernel/common.h"
#include "libs/include/kernel/elf.h"
#include "interrupt.h"
#include "lock.h"
#include "platform/platform.h"

void arch_init(boot_info_t* boot,int cpu_id);

#endif