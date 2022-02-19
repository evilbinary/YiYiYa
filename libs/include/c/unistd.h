#ifndef _UNISTD_H
#define _UNISTD_H			1


#include <inttypes.h>
#include <sys/types.h>
#include "stdio.h"
#include "fcntl.h"

#define PAGE_SIZE 1024*4

#define STDIN_FILENO		0
#define	STDOUT_FILENO		1
#define	STDERR_FILENO		2

#ifndef _HAVE_UID_T
#define _HAVE_UID_T
typedef	int32_t		uid_t;
#endif


#ifndef _HAVE_GID_T
#define _HAVE_GID_T
typedef	int32_t		gid_t;
#endif

#ifndef _HAVE_PID_T
#define _HAVE_PID_T
typedef	int32_t		pid_t;
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


#ifndef _HAVE_USECONDS_T
#define _HAVE_USECONDS_T
typedef	uint32_t		useconds_t;
#endif






#define R_OK		4
#define W_OK		2
#define X_OK		1
#define F_OK		0


#ifdef __cplusplus
extern "C" {
#endif


int          access(const char *, int);
unsigned int alarm(unsigned int);
int          brk(void *);
int          chdir(const char *);
int          chroot(const char *);  // LEGACY
int          chown(const char *, uid_t, gid_t);
int          close(int);
size_t       confstr(int, char *, size_t);
char        *crypt(const char *, const char *);
char        *ctermid(char *);
char        *cuserid(char *s);  // LEGACY
int          dup(int);
int          dup2(int, int);
void         encrypt(char[64], int);
int          execl(const char *, const char *, ...);
int          execle(const char *, const char *, ...);
int          execlp(const char *, const char *, ...);
int          execv(const char *, char *const []);
int          execve(const char *, char *const [], char *const []);
int          execvp(const char *, char *const []);
void        _exit(int);
int          fchown(int, uid_t, gid_t);
int          fchdir(int);
int          fdatasync(int);
pid_t        fork(void);
long int     fpathconf(int, int);
int          fsync(int);
int          ftruncate(int, off_t);
char        *getcwd(char *, size_t);
int          getdtablesize(void);  // LEGACY
gid_t        getegid(void);
uid_t        geteuid(void);
gid_t        getgid(void);
int          getgroups(int, gid_t []);
long         gethostid(void);
char        *getlogin(void);
int          getlogin_r(char *, size_t);
int          getopt(int, char * const [], const char *);
int          getpagesize(void);  // LEGACY
char        *getpass(const char *);  // LEGACY
pid_t        getpgid(pid_t);
pid_t        getpgrp(void);
pid_t        getpid(void);
pid_t        getppid(void);
pid_t        getsid(pid_t);
uid_t        getuid(void);
char        *getwd(char *);
int          isatty(int);
int          lchown(const char *, uid_t, gid_t);
int          link(const char *, const char *);
int          lockf(int, int, off_t);
off_t        lseek(int, off_t, int);
int          nice(int);
long int     pathconf(const char *, int);
int          pause(void);
int          pipe(int [2]);
ssize_t      pread(int, void *, size_t, off_t);
int          pthread_atfork(void (*)(void), void (*)(void), void(*)(void));
ssize_t      pwrite(int, const void *, size_t, off_t);
ssize_t      read(int, void *, size_t);
int          readlink(const char *, char *, size_t);
int          rmdir(const char *);
void        *sbrk(intptr_t);
int          setgid(gid_t);
int          setpgid(pid_t, pid_t);
pid_t        setpgrp(void);
int          setregid(gid_t, gid_t);
int          setreuid(uid_t, uid_t);
pid_t        setsid(void);
int          setuid(uid_t);
unsigned int sleep(unsigned int);
void         swab(const void *, void *, ssize_t);
int          symlink(const char *, const char *);
void         sync(void);
long int     sysconf(int);
pid_t        tcgetpgrp(int);
int          tcsetpgrp(int, pid_t);
int          truncate(const char *, off_t);
char        *ttyname(int);
int          ttyname_r(int, char *, size_t);
useconds_t   ualarm(useconds_t, useconds_t);
int          unlink(const char *);
int          usleep(useconds_t);
pid_t        vfork(void);
ssize_t      write(int, const void *, size_t);


#if defined(_LARGEFILE64_SOURCE) || defined(_GNU_SOURCE)
#define lseek64 lseek
#define pread64 pread
#define pwrite64 pwrite
#define truncate64 truncate
#define ftruncate64 ftruncate
#define lockf64 lockf
#define off64_t off_t
#endif

extern char   *optarg;
extern int    optind, opterr, optopt;


extern char **environ;
		
#ifdef __cplusplus
}
#endif



#endif

