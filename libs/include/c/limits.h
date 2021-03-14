#ifndef _LIMITS_H
#define _LIMITS_H		1





#ifndef __GNUC__

#ifndef __i386__
#error	Limits can not be set unknown platform. 
#endif

#define		CHAR_MIN 			(-128)
#define		SCHAR_MAX 			127
#define		SCHAR_MIN			(-128) 
#define		SHRT_MAX 			32767
#define		USHRT_MAX 			65535
#define		INT_MAX 			2147483647
#define		INT32_MAX 			2147483647
#define		LONG_MAX 			2147483647L
#define		LLONG_MAX			9223372036854775807LL
#define		LLONG_MIN			(-LLONG_MAX - 1)
#define		ULLONG_MAX			18446744073709551615ULL

#else

#define		SCHAR_MIN		__SCHAR_MIN__
#define		SCHAR_MAX		__SCHAR_MAX__
#define		CHAR_MIN		__CHAR_MIN__
#define		SHRT_MAX		__SHRT_MAX__
#define		USHRT_MAX		__USHRT_MAX__
#define		INT_MAX			__INT_MAX__
#define		INT32_MAX		INT_MAX
#define		LONG_MAX		__LONG_MAX__
#define		LLONG_MIN		__LLONG_MIN__
#define		LLONG_MAX		__LLONG_MAX__
#define		ULLONG_MAX		__ULLONG_MAX__

#endif



#define		CHAR_BIT			8 
#define		UCHAR_MAX			255 
#define		CHAR_MAX			127 
#define		MB_LEN_MAX			16
#define		ULONG_MAX 			4294967295UL
#define		LONG_MIN 			(-LONG_MAX - 1)
#define		INT_MIN 			(-INT_MAX - 1)
#define		INT32_MIN 			INT_MIN
#define		UINT_MAX			4294967295U 
#define		SIZE_MAX			UINT_MAX
#define		SHRT_MIN 			(-32768)



#define 	PATH_MAX			4096



#endif

