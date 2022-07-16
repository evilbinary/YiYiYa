#ifndef _MATH_H
#define _MATH_H



#ifdef __cplusplus
extern "C" {
#endif



#include "stddef.h"
#include "float.h"
#include <stdint.h>


#define M_E			2.7182818284590452354
#define M_LOG2E		1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN2		0.69314718055994530942
#define M_LN10		2.30258509299404568402
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
#define M_PI_4		0.78539816339744830962
#define M_1_PI		0.31830988618379067154
#define M_2_PI		0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2		1.41421356237309504880
#define M_SQRT1_2	0.70710678118654752440


#define ABS(a)   (((a) < 0) ? -(a) : (a) )
#define SGN(a)   (((a) < 0) ? -1 : 1)


#define __moddi3(a,b) __builtin_moddi3(a,b)
#define __udivdi3(a,b) __builtin_udivdi3(a,b)
#define __divdi3(a,b) __builtin_divdi3(a,b)

#define asuint(f) ((union{float _f; uint32_t _i;}){f})._i
#define asfloat(i) ((union{uint32_t _i; float _f;}){i})._f
#define asuint64(f) ((union{double _f; uint64_t _i;}){f})._i
#define asdouble(i) ((union{uint64_t _i; double _f;}){i})._f


#if 100*__GNUC__+__GNUC_MINOR__ >= 303
#define NAN       __builtin_nanf("")
#define INFINITY  __builtin_inff()
#else
#define NAN       (0.0f/0.0f)
#define INFINITY  1e5000f
#endif

static inline float eval_as_float(float x)
{
	float y = x;
	return y;
}

static inline double eval_as_double(double x)
{
	double y = x;
	return y;
}

/* fp_barrier returns its input, but limits code transformations
   as if it had a side-effect (e.g. observable io) and returned
   an arbitrary value.  */

#ifndef fp_barrierf
#define fp_barrierf fp_barrierf
static inline float fp_barrierf(float x)
{
	volatile float y = x;
	return y;
}
#endif

#ifndef fp_barrier
#define fp_barrier fp_barrier
static inline double fp_barrier(double x)
{
	volatile double y = x;
	return y;
}
#endif

#ifndef fp_barrierl
#define fp_barrierl fp_barrierl
static inline long double fp_barrierl(long double x)
{
	volatile long double y = x;
	return y;
}
#endif


/* fp_force_eval ensures that the input value is computed when that's
   otherwise unused.  To prevent the constant folding of the input
   expression, an additional fp_barrier may be needed or a compilation
   mode that does so (e.g. -frounding-math in gcc). Then it can be
   used to evaluate an expression for its fenv side-effects only.   */

#ifndef fp_force_evalf
#define fp_force_evalf fp_force_evalf
static inline void fp_force_evalf(float x)
{
	volatile float y;
	y = x;
}
#endif

#ifndef fp_force_eval
#define fp_force_eval fp_force_eval
static inline void fp_force_eval(double x)
{
	volatile double y;
	y = x;
}
#endif


static __inline unsigned __FLOAT_BITS(float __f)
{
	union {float __f; unsigned __i;} __u;
	__u.__f = __f;
	return __u.__i;
}
static __inline unsigned long long __DOUBLE_BITS(double __f)
{
	union {double __f; unsigned long long __i;} __u;
	__u.__f = __f;
	return __u.__i;
}

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

int __fpclassify(double);
int __fpclassifyf(float);
int __fpclassifyl(long double);

#define fpclassify(x) ( \
	sizeof(x) == sizeof(float) ? __fpclassifyf(x) : \
	sizeof(x) == sizeof(double) ? __fpclassify(x) : \
	__fpclassify(x) )

#define isinf(x) ( \
	sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) == 0x7f800000 : \
	sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & -1ULL >> 1) == 0x7ffULL << 52 : \
	__fpclassify(x) == FP_INFINITE)

#define isnan(x) __builtin_isnan (x)

// #define isnan(x) ( \
// 	sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) > 0x7f800000 : \
// 	sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & -1ULL >> 1) > 0x7ffULL << 52 : \
// 	__fpclassify(x) == FP_NAN)

#define isnormal(x) ( \
	sizeof(x) == sizeof(float) ? ((__FLOAT_BITS(x) + 0x00800000) & 0x7fffffff) >= 0x01000000 : \
	sizeof(x) == sizeof(double) ? ((__DOUBLE_BITS(x) + (1ULL << 52)) & -1ULL >> 1) >= 1ULL << 53 : \
	__fpclassify(x) == FP_NORMAL)

#define isfinite(x) ( \
	sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) < 0x7f800000 : \
	sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & -1ULL >> 1) < 0x7ffULL << 52 : \
	__fpclassify(x) > FP_INFINITE)

#define signbit(x) ( \
	sizeof(x) == sizeof(float) ? (int)(__FLOAT_BITS(x) >> 31) : \
	sizeof(x) == sizeof(double) ? (int)(__DOUBLE_BITS(x) >> 63) : \
	(int)(__DOUBLE_BITS(x) >> 63))



/* Helps static branch prediction so hot path can be better optimized.  */
#ifdef __GNUC__
#define predict_true(x) __builtin_expect(!!(x), 1)
#define predict_false(x) __builtin_expect(x, 0)
#else
#define predict_true(x) (x)
#define predict_false(x) (x)
#endif



#if 100*__GNUC__+__GNUC_MINOR__ >= 303
#define NAN       __builtin_nanf("")
#define INFINITY  __builtin_inff()
#else
#define NAN       (0.0f/0.0f)
#define INFINITY  1e5000f
#endif

/* Support non-nearest rounding mode.  */
#define WANT_ROUNDING 1
/* Support signaling NaNs.  */
#define WANT_SNAN 0

// TODO: Fill in these values
#define	MAXFLOAT		__MAXFLOAT__

/// \todo Find a way to fix this up. It's GCC code..
# define __HUGE_VAL_bytes	{ 0, 0, 0, 0, 0, 0, 0xf0, 0x7f }
# define __huge_val_t	union { unsigned char __c[8]; double __d; }
# ifdef	__GNUC__
#  define HUGE_VAL	(__extension__ \
			 ((__huge_val_t) { __c: __HUGE_VAL_bytes }).__d)
# else	/* Not GCC.  */
static __huge_val_t __huge_val = { __HUGE_VAL_bytes };
#  define HUGE_VAL	(__huge_val.__d)
# endif	/* GCC.  */
// ---------------------------
									  



/* Get two 32 bit ints from a double */
#define EXTRACT_WORDS(hi, lo, d) \
  do {                           \
    union {                      \
      double f;                  \
      uint64_t i;                \
    } __u;                       \
    __u.f = (d);                 \
    (hi) = __u.i >> 32;          \
    (lo) = (uint32_t)__u.i;      \
  } while (0)

/* Get the more significant 32 bit int from a double */
#define GET_HIGH_WORD(hi, d) \
  do {                       \
    union {                  \
      double f;              \
      uint64_t i;            \
    } __u;                   \
    __u.f = (d);             \
    (hi) = __u.i >> 32;      \
  } while (0)

/* Get the less significant 32 bit int from a double */
#define GET_LOW_WORD(lo, d) \
  do {                      \
    union {                 \
      double f;             \
      uint64_t i;           \
    } __u;                  \
    __u.f = (d);            \
    (lo) = (uint32_t)__u.i; \
  } while (0)

/* Set a double from two 32 bit ints */
#define INSERT_WORDS(d, hi, lo)                      \
  do {                                               \
    union {                                          \
      double f;                                      \
      uint64_t i;                                    \
    } __u;                                           \
    __u.i = ((uint64_t)(hi) << 32) | (uint32_t)(lo); \
    (d) = __u.f;                                     \
  } while (0)

/* Set the more significant 32 bits of a double from an int */
#define SET_HIGH_WORD(d, hi)       \
  do {                             \
    union {                        \
      double f;                    \
      uint64_t i;                  \
    } __u;                         \
    __u.f = (d);                   \
    __u.i &= 0xffffffff;           \
    __u.i |= (uint64_t)(hi) << 32; \
    (d) = __u.f;                   \
  } while (0)

/* Set the less significant 32 bits of a double from an int */
#define SET_LOW_WORD(d, lo)         \
  do {                              \
    union {                         \
      double f;                     \
      uint64_t i;                   \
    } __u;                          \
    __u.f = (d);                    \
    __u.i &= 0xffffffff00000000ull; \
    __u.i |= (uint32_t)(lo);        \
    (d) = __u.f;                    \
  } while (0)

/* Get a 32 bit int from a float */
#define GET_FLOAT_WORD(w, d) \
  do {                       \
    union {                  \
      float f;               \
      uint32_t i;            \
    } __u;                   \
    __u.f = (d);             \
    (w) = __u.i;             \
  } while (0)

/* Set a float from a 32 bit int */
#define SET_FLOAT_WORD(d, w) \
  do {                       \
    union {                  \
      float f;               \
      uint32_t i;            \
    } __u;                   \
    __u.i = (w);             \
    (d) = __u.f;             \
  } while (0)

#define EPS DBL_EPSILON
#define FORCE_EVAL(x)                                   \
  do {                                                  \
    if (sizeof(x) == sizeof(float)) {                   \
      volatile float __x __attribute__((unused));       \
      __x = (x);                                        \
    } else if (sizeof(x) == sizeof(double)) {           \
      volatile double __x __attribute__((unused));      \
      __x = (x);                                        \
    } else {                                            \
      volatile long double __x __attribute__((unused)); \
      __x = (x);                                        \
    }                                                   \
  } while (0)



typedef float	float_t;
typedef double	double_t;


double acos(double);
double asin(double);
double atan(double);
double atan2(double, double);
double ceil(double);
double cos(double);
double cosh(double);
double exp(double);
double fabs(double);
double floor(double);
double fmod(double, double);
double frexp(double, int *);
double ldexp(double, int);
double log(double);
double log10(double);
double modf(double, double *);
double pow(double, double);
double sin(double);
double sinh(double);
double sqrt(double);
double tan(double);
double tanh(double);
double erf(double);
double erfc(double);
double gamma(double);
double hypot(double, double);
double j0(double);
double j1(double);
double jn(int, double);
double lgamma(double);
double y0(double);
double y1(double);
double yn(int, double);
int    isnan(double);
double acosh(double);
double asinh(double);
double atanh(double);
double cbrt(double);
double expm1(double);
int    ilogb(double);
double log1p(double);
double logb(double);
double nextafter(double, double);
double remainder(double, double);
double rint(double);
double scalb(double, double);

double scalbn(double, int);

double trunc(double x);
double log2(double x);

extern int signgam;

#ifdef __cplusplus
}
#endif
									  

#endif

