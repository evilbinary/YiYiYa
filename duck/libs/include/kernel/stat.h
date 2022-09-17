#ifndef	_SYS_STAT_H
#define	_SYS_STAT_H
#ifdef __cplusplus
extern "C" {
#endif


#include "time.h"
#include "type.h"


#define IOC_MAGIC 'f'
#define IOC_INIT _IO(IOC_MAGIC, 0)
#define IOC_READ_REG _IOW(IOC_MAGIC, 1, int)
#define IOC_WRITE_REG _IOW(IOC_MAGIC, 2, int)
#define IOC_STAT _IOW(IOC_MAGIC, 3, int)

struct stat {
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	unsigned long __pad;
	off_t st_size;
	blksize_t st_blksize;
	int __pad2;
	blkcnt_t st_blocks;
#if defined(__rtems__)
  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
#else
	time_t st_atim;
	time_t st_mtim;
	time_t st_ctim;
#endif
	unsigned __unused[2];
};


#define __NEED_dev_t
#define __NEED_ino_t
#define __NEED_mode_t
#define __NEED_nlink_t
#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_off_t
#define __NEED_time_t
#define __NEED_blksize_t
#define __NEED_blkcnt_t
#define __NEED_struct_timespec

#if defined(__rtems__)
#define st_atime st_atim.tv_sec
#define st_ctime st_ctim.tv_sec
#define st_mtime st_mtim.tv_sec
#endif

// #define st_atime st_atim.tv_sec
// #define st_mtime st_mtim.tv_sec
// #define st_ctime st_ctim.tv_sec

#define S_IFMT  0170000

#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFBLK 0060000
#define S_IFREG 0100000
#define S_IFIFO 0010000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

#define S_TYPEISMQ(buf)  0
#define S_TYPEISSEM(buf) 0
#define S_TYPEISSHM(buf) 0
#define S_TYPEISTMO(buf) 0

#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

#ifndef S_IRUSR
#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000
#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRWXU 0700
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IRWXG 0070
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001
#define S_IRWXO 0007
#endif

#define UTIME_NOW  0x3fffffff
#define UTIME_OMIT 0x3ffffffe

#ifdef __cplusplus
}
#endif
#endif


