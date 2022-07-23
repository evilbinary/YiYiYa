#ifndef _TYPE_H
#define _TYPE_H

#include "types.h"

#ifndef _HAVE_TIME_T
#define _HAVE_TIME_T
typedef	int32_t		time_t;
#endif


#ifndef _HAVE_BLKCNT_T
#define _HAVE_BLKCNT_T
typedef	int32_t		blkcnt_t;
#endif


#ifndef _HAVE_BLKSIZE_T
#define _HAVE_BLKSIZE_T
typedef	int32_t		blksize_t;
#endif


#ifndef _HAVE_CLOCK_T
#define _HAVE_CLOCK_T
typedef	int32_t		clock_t;
#endif


#ifndef _HAVE_CLOCKID_T
#define _HAVE_CLOCKID_T
typedef	int32_t		clockid_t;
#endif


#ifndef _HAVE_DEV_T
#define _HAVE_DEV_T
typedef	int32_t		dev_t;
#endif


#ifndef _HAVE_FSBLKCNT_T
#define _HAVE_FSBLKCNT_T
typedef	uint32_t		fsblkcnt_t;
#endif


#ifndef _HAVE_FSFILCNT_T
#define _HAVE_FSFILCNT_T
typedef	uint32_t		fsfilcnt_t;
#endif


#ifndef _HAVE_GID_T
#define _HAVE_GID_T
typedef	int32_t		gid_t;
#endif


#ifndef _HAVE_ID_T
#define _HAVE_ID_T
typedef	int32_t		id_t;
#endif


#ifndef _HAVE_INO_T
#define _HAVE_INO_T
typedef	uint32_t		ino_t;
#endif


#ifndef _HAVE_KEY_T
#define _HAVE_KEY_T
typedef	int32_t		key_t;
#endif


#ifndef _HAVE_MODE_T
#define _HAVE_MODE_T
typedef	int32_t		mode_t;
#endif


#ifndef _HAVE_NLINK_T
#define _HAVE_NLINK_T
typedef	int32_t		nlink_t;
#endif


#ifndef _HAVE_OFF_T
#define _HAVE_OFF_T
typedef	long int		off_t;
#endif


#ifndef _HAVE_PID_T
#define _HAVE_PID_T
typedef	int32_t		pid_t;
#endif


#ifndef _HAVE_PTHREAD_ATTR_T
#define _HAVE_PTHREAD_ATTR_T
typedef	int32_t		pthread_attr_t;
#endif


#ifndef _HAVE_PTHREAD_COND_T
#define _HAVE_PTHREAD_COND_T
typedef	int32_t		pthread_cond_t;
#endif


#ifndef _HAVE_PTHREAD_CONDATTR_T
#define _HAVE_PTHREAD_CONDATTR_T
typedef	int32_t		pthread_condattr_t;
#endif


#ifndef _HAVE_PTHREAD_KEY_T
#define _HAVE_PTHREAD_KEY_T
typedef	int32_t		pthread_key_t;
#endif


#ifndef _HAVE_PTHREAD_MUTEX_T
#define _HAVE_PTHREAD_MUTEX_T
typedef	int32_t		pthread_mutex_t;
#endif


#ifndef _HAVE_PTHREAD_MUTEXATTR_T
#define _HAVE_PTHREAD_MUTEXATTR_T
typedef	int32_t		pthread_mutexattr_t;
#endif


#ifndef _HAVE_PTHREAD_ONCE_T
#define _HAVE_PTHREAD_ONCE_T
typedef	int32_t		pthread_once_t;
#endif


#ifndef _HAVE_PTHREAD_RWLOCK_T
#define _HAVE_PTHREAD_RWLOCK_T
typedef	int32_t		pthread_rwlock_t;
#endif


#ifndef _HAVE_PTHREAD_RWLOCKATTR_T
#define _HAVE_PTHREAD_RWLOCKATTR_T
typedef	int32_t		pthread_rwlockattr_t;
#endif


#ifndef _HAVE_PTHREAD_T
#define _HAVE_PTHREAD_T
typedef	int32_t		pthread_t;
#endif


#ifndef _HAVE_SIZE_T
#define _HAVE_SIZE_T
#ifndef size_t
typedef	uint32_t		size_t;
#endif
#endif


#ifndef _HAVE_SSIZE_T
#define _HAVE_SSIZE_T
#ifndef ssize_t
typedef	int32_t		ssize_t;
#endif
#endif


#ifndef _HAVE_SUSECONDS_T
#define _HAVE_SUSECONDS_T
typedef	int32_t		suseconds_t;
#endif


#ifndef _HAVE_TIME_T
#define _HAVE_TIME_T
typedef	int32_t		time_t;
#endif


#ifndef _HAVE_TIMER_T
#define _HAVE_TIMER_T
typedef	int32_t		timer_t;
#endif


#ifndef _HAVE_UID_T
#define _HAVE_UID_T
typedef	int32_t		uid_t;
#endif


#ifndef _HAVE_USECONDS_T
#define _HAVE_USECONDS_T
typedef	uint32_t		useconds_t;
#endif



/* NOT PART OF THE STANDARD */
#ifndef _HAVE_QUAD_T
#define _HAVE_QUAD_T
typedef	long long		quad_t;
#endif


/* NOT PART OF THE STANDARD */
#ifndef _HAVE_U_QUAD_T
#define _HAVE_U_QUAD_T
typedef	unsigned long long		u_quad_t;
#endif

/* NOT PART OF THE STANDARD */
#ifndef _HAVE_U_LONG
#define _HAVE_U_LONG
typedef	unsigned long		u_long;
#endif

/* NOT PART OF THE STANDARD */
#ifndef _HAVE_U_INT32_T
#define _HAVE_U_INT32_T
typedef	int		u_int32_t;
#endif

/* NOT PART OF THE STANDARD */
#ifndef _HAVE_U_UINT32_T
#define _HAVE_U_UINT32_T
typedef	unsigned int		u_uint32_t;
#endif



typedef unsigned short sa_family_t;


#endif