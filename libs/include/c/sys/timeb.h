#ifndef _SYS_TIMEB_H
#define _SYS_TIMEB_H

#include <inttypes.h>

#ifndef _HAVE_TIME_T
#define _HAVE_TIME_T
typedef	int32_t		time_t;
#endif



struct timeb
{
	time_t         time;		//  the seconds portion of the current time
	unsigned short millitm;		//  the milliseconds portion of the current time
	short          timezone;	//  the local timezone in minutes west of Greenwich
	short          dstflag;		//  TRUE if Daylight Savings Time is in effect
};




#ifdef __cplusplus
extern "C" {
#endif

int   ftime(struct timeb *);
		
#ifdef __cplusplus
}
#endif


#endif

