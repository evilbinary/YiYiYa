#ifndef _FLOAT_H
#define _FLOAT_H

#if    __GNUC__==3&&__GNUC_MINOR__<=2
#define DBL_DIG         15                      /* # of decimal digits of precision */
#define DBL_EPSILON     2.2204460492503131e-016 /* smallest such that 1.0+DBL_EPSILON != 1.0 */
#define DBL_MANT_DIG    53                      /* # of bits in mantissa */
#define DBL_MAX         1.7976931348623158e+308 /* max value */
#define DBL_MAX_10_EXP  308                     /* max decimal exponent */
#define DBL_MAX_EXP     1024                    /* max binary exponent */
#define DBL_MIN         2.2250738585072014e-308 /* min positive value */
#define DBL_MIN_10_EXP  (-307)                  /* min decimal exponent */
#define DBL_MIN_EXP     (-1021)                 /* min binary exponent */
#define _DBL_RADIX      2                       /* exponent radix */
#define _DBL_ROUNDS     1                       /* addition rounding: near */

#define FLT_DIG         6                       /* # of decimal digits of precision */
#define FLT_EPSILON     1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */
#define FLT_GUARD       0
#define FLT_MANT_DIG    24                      /* # of bits in mantissa */
#define FLT_MAX         3.402823466e+38F        /* max value */
#define FLT_MAX_10_EXP  38                      /* max decimal exponent */
#define FLT_MAX_EXP     128                     /* max binary exponent */
#define FLT_MIN         1.175494351e-38F        /* min positive value */
#define FLT_MIN_10_EXP  (-37)                   /* min decimal exponent */
#define FLT_MIN_EXP     (-125)                  /* min binary exponent */
#define FLT_NORMALIZE   0
#define FLT_RADIX       2                       /* exponent radix */
#define FLT_ROUNDS      1                       /* addition rounding: near */

#ifndef _M_M68K
#define LDBL_DIG        DBL_DIG                 /* # of decimal digits of precision */
#define LDBL_EPSILON    DBL_EPSILON             /* smallest such that 1.0+LDBL_EPSILON != 1.0 */
#define LDBL_MANT_DIG   DBL_MANT_DIG            /* # of bits in mantissa */
#define LDBL_MAX        DBL_MAX                 /* max value */
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP          /* max decimal exponent */
#define LDBL_MAX_EXP    DBL_MAX_EXP             /* max binary exponent */
#define LDBL_MIN        DBL_MIN                 /* min positive value */
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP          /* min decimal exponent */
#define LDBL_MIN_EXP    DBL_MIN_EXP             /* min binary exponent */
#define _LDBL_RADIX     DBL_RADIX               /* exponent radix */
#define _LDBL_ROUNDS    DBL_ROUNDS              /* addition rounding: near */
#else
#define LDBL_DIG        18                                      /* # of decimal digits of precision */
#define LDBL_EPSILON    1.08420217248550443412e-019L            /* smallest such that 1.0+LDBL_EPSILON != 1.0 */
#define LDBL_MANT_DIG   64                                      /* # of bits in mantissa */
#define LDBL_MAX        1.189731495357231765e+4932L             /* max value */
#define LDBL_MAX_10_EXP 4932                                    /* max decimal exponent */
#define LDBL_MAX_EXP    16384                                   /* max binary exponent */
#define LDBL_MIN        3.3621031431120935063e-4932L            /* min positive value */
#define LDBL_MIN_10_EXP (-4931)                                 /* min decimal exponent */
#define LDBL_MIN_EXP    (-16381)                                /* min binary exponent */
#define _LDBL_RADIX     2                                       /* exponent radix */
#define _LDBL_ROUNDS    1                                       /* addition rounding: near */
#endif

#else
#define		FLT_RADIX 	__FLT_RADIX__
#define		FLT_ROUNDS		1

#define		FLT_DIG		__FLT_DIG__
#define		DBL_DIG		__DBL_DIG__
#define		LDBL_DIG		__LDBL_DIG__
#define		FLT_EPSILON		__FLT_EPSILON__
#define		DBL_EPSILON		__DBL_EPSILON__
#define		LDBL_EPSILON		__LDBL_EPSILON__
#define		FLT_MANT_DIG		__FLT_MANT_DIG__
#define		DBL_MANT_DIG		__DBL_MANT_DIG__
#define		LDBL_MANT_DIG		__LDBL_MANT_DIG__
#define		FLT_MAX		__FLT_MAX__
#define		DBL_MAX		__DBL_MAX__
#define		LDBL_MAX		__LDBL_MAX__
#define		FLT_MAX_EXP		__FLT_MAX_EXP__
#define		DBL_MAX_EXP		__DBL_MAX_EXP__
#define		LDBL_MAX_EXP		__LDBL_MAX_EXP__
#define		FLT_MIN		__FLT_MIN__
#define		DBL_MIN		__DBL_MIN__
#define		LDBL_MIN		__LDBL_MIN__
#define		FLT_MIN_EXP		__FLT_MIN_EXP__
#define		DBL_MIN_EXP		__DBL_MIN_EXP__
#define		LDBL_MIN_EXP		__LDBL_MIN_EXP__

#endif

#endif

