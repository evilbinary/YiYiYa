#include "../arch.h"

//多核cpu初始化
void ap_init(int cpu){
    cpu_init(cpu);
    interrupt_init(cpu);
}