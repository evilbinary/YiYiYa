/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../arch.h"

void ap_init(int cpu){
    cpu_init(cpu);
    interrupt_init(cpu);
}
