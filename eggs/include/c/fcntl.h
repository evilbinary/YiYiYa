#ifndef	_FCNTL_H
#define	_FCNTL_H

#include "types.h"
#include "sys/types.h"

#define	F_DUPFD					0
#define	F_GETFD					1
#define	F_SETFD					2
#define	F_GETFL					3
#define	F_SETFL					4
#define	F_GETLK					5
#define	F_SETLK					6
#define	F_SETLKW				7


#define	FD_CLOEXEC				1


#define	F_RDLCK			0
#define	F_WRLCK			1
#define	F_UNLCK			2


#define	O_CREAT			00000100
#define	O_EXCL			00000200
#define	O_NOCTTY		00000400
#define	O_TRUNC			00001000


#define	O_APPEND		00002000
#define	O_NONBLOCK		00004000
#define	O_NDELAY		O_NONBLOCK
#define	O_SYNC			00010000
#define	O_RSYNC			O_SYNC
#define	O_DSYNC			O_SYNC


#define	O_ACCMODE		(O_RDONLY|O_RDWR|O_WRONLY)

#define	O_RDONLY		00	
#define	O_WRONLY		01
#define	O_RDWR			02

#define O_LARGEFILE		0100000


// #if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define AT_NO_AUTOMOUNT 0x800
#define AT_EMPTY_PATH 0x1000
#define AT_STATX_SYNC_TYPE 0x6000
#define AT_STATX_SYNC_AS_STAT 0x0000
#define AT_STATX_FORCE_SYNC 0x2000
#define AT_STATX_DONT_SYNC 0x4000
#define AT_RECURSIVE 0x8000

#define FAPPEND O_APPEND
#define FFSYNC O_SYNC
#define FASYNC O_ASYNC
#define FNONBLOCK O_NONBLOCK
#define FNDELAY O_NDELAY

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_ULOCK 0
#define F_LOCK  1
#define F_TLOCK 2
#define F_TEST  3

#define F_SETLEASE	1024
#define F_GETLEASE	1025
#define F_NOTIFY	1026
#define F_CANCELLK	1029
#define F_SETPIPE_SZ	1031
#define F_GETPIPE_SZ	1032
#define F_ADD_SEALS	1033
#define F_GET_SEALS	1034

#define F_SEAL_SEAL	0x0001
#define F_SEAL_SHRINK	0x0002
#define F_SEAL_GROW	0x0004
#define F_SEAL_WRITE	0x0008
#define F_SEAL_FUTURE_WRITE	0x0010

#define F_GET_RW_HINT		1035
#define F_SET_RW_HINT		1036
#define F_GET_FILE_RW_HINT	1037
#define F_SET_FILE_RW_HINT	1038

#define RWF_WRITE_LIFE_NOT_SET	0
#define RWH_WRITE_LIFE_NONE	1
#define RWH_WRITE_LIFE_SHORT	2
#define RWH_WRITE_LIFE_MEDIUM	3
#define RWH_WRITE_LIFE_LONG	4
#define RWH_WRITE_LIFE_EXTREME	5

#define DN_ACCESS	0x00000001
#define DN_MODIFY	0x00000002
#define DN_CREATE	0x00000004
#define DN_DELETE	0x00000008
#define DN_RENAME	0x00000010
#define DN_ATTRIB	0x00000020
#define DN_MULTISHOT	0x80000000

int lockf(int, int, off_t);
// #endif

#define AT_FDCWD (-100)
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR 0x200
#define AT_SYMLINK_FOLLOW 0x400
#define AT_EACCESS 0x200


struct flock {
	short l_type;
	short l_whence;
	off_t l_start;
	off_t l_len;
	pid_t l_pid;
};


int creat(const char *, u32);
int fcntl(int, int, ...);
int open(const char *, int, ...);


#endif
