#include <sys/timeb.h>



int   ftime(struct timeb *fb )
{
	int rc;
	time_t sec;
	time_t milli;

	/*if ( os_system_time( &sec, &milli, &rc ) == NOTIMPLEMENTED )
			os_freakout();

	if ( rc != 0 ) return -1;

		fb->time = sec;
		fb->millitm = milli;

		/// \todo Complete timezone and dstflag correctly.
		fb->timezone = 0;
		fb->dstflag  = 0;
	*/
	return 0;
}


		 
