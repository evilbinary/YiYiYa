#ifndef _INTTYPES_H
#define _INTTYPES_H

#define _Addr int
#define _Int64 long long
#define _Reg int

typedef unsigned int u_int;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed _Int64   int64_t;
typedef unsigned int uint32_t;

#ifndef STDINT_H
#define STDINT_H
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed _Int64   int64_t;
typedef unsigned int uint32_t;
#endif

typedef _Addr intptr_t;
typedef unsigned _Addr uintptr_t;

#endif
