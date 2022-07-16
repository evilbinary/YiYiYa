#ifndef _SIGNAL_H
#define _SIGNAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "unistd.h"

#define SIG_BLOCK     0
#define SIG_UNBLOCK   1
#define SIG_SETMASK   2

#define SI_ASYNCNL (-60)
#define SI_TKILL (-6)
#define SI_SIGIO (-5)
#define SI_ASYNCIO (-4)
#define SI_MESGQ (-3)
#define SI_TIMER (-2)
#define SI_QUEUE (-1)
#define SI_USER 0
#define SI_KERNEL 128

#define SA_NOCLDSTOP  1
#define SA_NOCLDWAIT  2
#define SA_SIGINFO    4
#define SA_ONSTACK    0x08000000
#define SA_RESTART    0x10000000
#define SA_NODEFER    0x40000000
#define SA_RESETHAND  0x80000000
#define SA_RESTORER   0x04000000

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


typedef struct {
#ifdef __SI_SWAP_ERRNO_CODE
	int si_signo, si_code, si_errno;
#else
	int si_signo, si_errno, si_code;
#endif
} siginfo_t;

struct sigaction {
	union {
		void (*sa_handler)(int);
		void (*sa_sigaction)(int, siginfo_t *, void *);
	} __sa_handler;
	sigset_t sa_mask;
	int sa_flags;
	void (*sa_restorer)(void);
};
#define sa_handler   __sa_handler.sa_handler
#define sa_sigaction __sa_handler.sa_sigaction

typedef int sig_atomic_t;

#define SIG_ERR  ((void (*)(int))-1)
#define SIG_DFL  ((void (*)(int)) 0)
#define SIG_IGN  ((void (*)(int)) 1)


#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE)
#define NSIG _NSIG
typedef void (*sig_t)(int);
#endif

// #ifdef _GNU_SOURCE
typedef void (*sighandler_t)(int);
void (*bsd_signal(int, void (*)(int)))(int);
int sigisemptyset(const sigset_t *);
int sigorset (sigset_t *, const sigset_t *, const sigset_t *);
int sigandset(sigset_t *, const sigset_t *, const sigset_t *);

#define SA_NOMASK SA_NODEFER
#define SA_ONESHOT SA_RESETHAND
// #endif		

int    raise(int);

int kill(pid_t, int);

#ifdef __cplusplus
}
#endif
		

#endif

