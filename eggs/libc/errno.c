#include "errno.h"

int errno	=	0;


/** Put in here for libm. Seems to want it. */
int *__errno_location(void)
{
	return &errno;
}

