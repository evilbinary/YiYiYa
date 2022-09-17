/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "arch/arch.h"

void exception_init();

void divide_error();

void debug_exception();

void breakpoint();

void nmi();

void overflow();

void bounds_check();

void invalid_opcode();

void cop_not_avalid();

void double_fault();

void overrun();

void invalid_tss();

void seg_not_present();

void stack_exception();

void general_protection();

void page_fault();

void reversed();

void coprocessor_error();

#endif