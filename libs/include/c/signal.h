#ifndef _SIGNAL_H
#define _SIGNAL_H

#define SIGABRT			1
#define SIGALRM			2
#define SIGFPE			3
#define SIGHUP			4
#define SIGILL			5
#define SIGINT			6
#define SIGKILL			7
#define SIGPIPE			8
#define SIGQUIT			9
#define SIGSEGV			10
#define SIGTERM			11
#define SIGUSR1			12
#define SIGUSR2			13
#define SIGCHLD			14
#define SIGCONT			15
#define SIGSTOP			16
#define SIGTSTP			17
#define SIGTTIN			18
#define SIGTTOU			19
#define SIGBUS			20
#define SIGPOLL			21
#define SIGPROF			22
#define SIGSYS			23
#define SIGTRAP			24
#define SIGURG			25
#define SIGVTALRM		26
#define SIGXCPU			27
#define SIGXFSZ			28


#ifndef _HAVE_SIGSET_T
#define _HAVE_SIGSET_T
typedef struct
{
	unsigned long bits[ (1024 / sizeof( unsigned long )) + 1 ];
} sigset_t;
#endif




#ifdef __cplusplus
extern "C" {
#endif
		

int    raise(int);


#ifdef __cplusplus
}
#endif
		

#endif

