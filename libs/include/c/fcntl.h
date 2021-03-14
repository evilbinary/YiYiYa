#ifndef	_FCNTL_H
#define	_FCNTL_H

#include "types.h"

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

int creat(const char *, u32);
int fcntl(int, int, ...);
int open(const char *, int, ...);


#endif
