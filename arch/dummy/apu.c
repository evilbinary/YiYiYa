#include "../arch.h"

void ap_init(int cpu){
    cpu_init(cpu);
    interrupt_init(cpu);
}