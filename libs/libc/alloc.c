#include "alloca.h"
#include <stddef.h>


void *alloca(size_t t){
    return __builtin_alloca(t);
}
