#ifndef STDINT_H
#define STDINT_H

#ifndef _HAVE_SIZE_T
#define _HAVE_SIZE_T
typedef unsigned           size_t;
#endif

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned long      uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed long        int32_t;
typedef signed long long   int64_t;

typedef signed char	   s8;
typedef unsigned char      u8;

typedef signed short       s16;
typedef unsigned short     u16;

typedef signed int         s32;
typedef unsigned int       u32;

typedef signed long long   s64;
typedef unsigned long long u64;

#endif /* STDINT_H */
