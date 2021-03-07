#ifndef _MATH_H
#define _MATH_H

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
									  

#ifdef __cplusplus
extern "C" {
#endif
									  
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


extern int signgam;

#ifdef __cplusplus
}
#endif
									  

#endif

