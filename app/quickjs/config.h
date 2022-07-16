#ifndef config_h
#define config_h


#define CONFIG_VERSION "QuickJS version 2021-03-27"
// #define CONFIG_BIGNUM 1

#undef CONFIG_ATOMICS
#define EMSCRIPTEN 1
// #define _WIN32 0

#define _GNU_SOURCE 1

#include "sys/select.h"
#include "alloca.h"


#endif /* config_h */