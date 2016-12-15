#include "apue.h"
#include <errno.h>		/* for definition of errno */
#include <stdarg.h>		/* ISO C variable aruments */
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "log_to_stderr.h"

static void err_doit(int, int, const char *, va_list);

static void log_doit(int, int, const char *, va_list ap);

extern int log_to_stderr;

/**
 * Nonfatal error related to a system call
 * Print a message an return.
 */
void
err_ret(const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

/**
 * Fatal error related to a system call
 * Print a message and terminate.
 */
void
err_sys(const char* fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

/**
 * Fatal error unrelated to a system cal.
 * Error code passed as explict parameter.
 * Print a message and terminate.
 */
void
err_exit(int error, const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

/**
 * Fatal error related to a system call.
 * Print a message, dump core, and terminate.
 */
void
err_dump(const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	abort();	/* dump core and terminate */
	exit(1);	/* shouldn't get here */
}

/**
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void
err_msg(const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}

/**
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void
err_quit(const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

/**
 * Print a message and return to caller
 * Caller specifies "errnoflag".
 */
static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap){
	
	char	buf[MAXLINE];
	vsnprintf(buf, MAXLINE, fmt, ap);
	if (errnoflag){
		snprintf(buf + strlen(buf), MAXLINE-strlen(buf), ": %s", \
					strerror(error));
	}
	strcat(buf, "\n");
	fflush(stdout);		/* in case stdout and stderr are the same */
	fputs(buf, stderr); 
	fflush(NULL);		/* flushes all stdio output stream */
}

/**
 * Error routines for program that can run as a daemon
 */

/**
 * Caller must define and set this: nonzero if
 * interactive, zero if daemon.
 */
/* extern int log_to_stderr; */

/**
 * Initialize syslog(), if running as daemon.
 */
void
log_open(const char *ident, int option, int facility){
	if (log_to_stderr == 0){
		openlog(ident, option, facility);
	}
}

/**
 * Nonfatal error related to a system call.
 * Print a message with th system's errno value and return.
 */
void 
log_ret(const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	log_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
}

/**
 * Fatal  error related to a system call.
 * Print a message and terminate.
 */
void 
log_sys(const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	log_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(2);
}

/**
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void
log_msg(const char *fmt, ...){
	
	va_list		ap;

	va_start(ap, fmt);
	log_doit(0, LOG_ERR, fmt, ap);
	va_end(ap);
}

/**
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void 
log_quit(const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	log_doit(0, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(2);
}

/**
 * Print a message and return to caller.
 * Caller specifies "errnoflag" and "priority".
 */
static void
log_doit(int errnoflag, int priority, const char *fmt, va_list ap){
	int		errno_save;
	char	buf[MAXLINE];

	errno_save = errno;		/* value caller might want printed */
	vsnprintf(buf, MAXLINE, fmt, ap);
	if (errnoflag){
		snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s",\
					strerror(errno_save));
	}
	strcat(buf, "\n");
	if (log_to_stderr){
		fflush(stdout);
		fputs(buf, stderr);
		fflush(stderr);
	}else{
		syslog(priority, buf);
	}
}



void
pr_exit(int status)
{
	if (WIFEXITED(status))
		printf("normal termination, exit status = %d\n",
				WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		printf("abnormal termination, signal number = %d%s\n",
				WTERMSIG(status),
#ifdef	WCOREDUMP
				WCOREDUMP(status) ? " (core file generated)" : "");
#else
				"");
#endif
	else if (WIFSTOPPED(status))
		printf("child stopped, signal number = %d\n",
				WSTOPSIG(status));
}
/* Reliable version of signal(), using POSIX sigaction().  */
Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}

Sigfunc *
signal_intr(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
#ifdef	SA_INTERRUPT
	act.sa_flags |= SA_INTERRUPT;
#endif
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}

void
pr_mask(const char * str)
{
    sigset_t    sigset;
    int         errno_save;

    errno_save = errno;

    if (sigprocmask(0, NULL, &sigset) < 0)
        err_sys("sigpromask error");
    
    printf("%s", str);
    if (sigismember(&sigset, SIGINT))   printf("SIGINT ");
    if (sigismember(&sigset, SIGQUIT))  printf("SIGQUIT ");
    if (sigismember(&sigset, SIGUSR1))  printf("SIGUSER1 ");
    if (sigismember(&sigset, SIGALRM))  printf("SIGALRM ");
    
    /* remaining signals can go here */
    printf("\n");
    errno = errno_save;
}


static volatile sig_atomic_t sigflag; /* set nonzero by sig handler */
static sigset_t newmask, oldmask, zeromask;

static void
sig_usr(int signo)	/* one signal handler for SIGUSR1 and SIGUSR2 */
{
	sigflag = 1;
}

void
TELL_WAIT(void)
{
	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
		err_sys("signal(SIGUSR1) error");
	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
		err_sys("signal(SIGUSR2) error");
	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);

	/*
	 * Block SIGUSR1 and SIGUSR2, and save current signal mask.
	 */
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		err_sys("SIG_BLOCK error");
}

void
TELL_PARENT(pid_t pid)
{
	kill(pid, SIGUSR2);		/* tell parent we're done */
}

void
WAIT_PARENT(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);	/* and wait for parent */
	sigflag = 0;

	/*
	 * Reset signal mask to original value.
	 */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("SIG_SETMASK error");
}

void
TELL_CHILD(pid_t pid)
{
	kill(pid, SIGUSR1);			/* tell child we're done */
}

void
WAIT_CHILD(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);	/* and wait for child */
	sigflag = 0;

	/*
	 * Reset signal mask to original value.
	 */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("SIG_SETMASK error");
}

int
lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	struct flock	lock;

	lock.l_type = type;		/* F_RDLCK, F_WRLCK, F_UNLCK */
	lock.l_start = offset;	/* byte offset, relative to l_whence */
	lock.l_whence = whence;	/* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len;		/* #bytes (0 means to EOF) */

	return(fcntl(fd, cmd, &lock));
}

pid_t
lock_test(int fd, int type, off_t offset, int whence, off_t len)
{
	struct flock	lock;

	lock.l_type = type;		/* F_RDLCK or F_WRLCK */
	lock.l_start = offset;	/* byte offset, relative to l_whence */
	lock.l_whence = whence;	/* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len;		/* #bytes (0 means to EOF) */

	if (fcntl(fd, F_GETLK, &lock) < 0)
		err_sys("fcntl error");

	if (lock.l_type == F_UNLCK)
		return(0);		/* false, region isn't locked by another proc */
	return(lock.l_pid);	/* true, return pid of lock owner */
}

int
lockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return(fcntl(fd, F_SETLK, &fl));
}

void
set_fl(int fd, int flags) /* flags are file status flags to turn on */
{
	int		val;

	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
		err_sys("fcntl F_GETFL error");

	val |= flags;		/* turn on flags */

	if (fcntl(fd, F_SETFL, val) < 0)
		err_sys("fcntl F_SETFL error");
}


ssize_t             /* Write "n" bytes to a descriptor  */
writen(int fd, const void *ptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;

	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) < 0) {
			if (nleft == n)
				return(-1); /* error, return -1 */
			else
				break;      /* error, return amount written so far */
		} else if (nwritten == 0) {
			break;
		}
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n - nleft);      /* return >= 0 */
}

ssize_t             /* Read "n" bytes from a descriptor  */
readn(int fd, void *ptr, size_t n)
{
	size_t		nleft;
	ssize_t		nread;

	nleft = n;
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (nleft == n)
				return(-1); /* error, return -1 */
			else
				break;      /* error, return amount read so far */
		} else if (nread == 0) {
			break;          /* EOF */
		}
		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);      /* return >= 0 */
}

#define MAXSLEEP 128

int
connect_retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
{
	int nsec;

	/*
	 * Try to connect with exponential backoff.
	 */
	for (nsec = 1; nsec <= MAXSLEEP; nsec <<= 1) {
		if (connect(sockfd, addr, alen) == 0) {
			/*
			 * Connection accepted.
			 */
			return(0);
		}

		/*
		 * Delay before trying again.
		 */
		if (nsec <= MAXSLEEP/2)
			sleep(nsec);
	}
	return(-1);
}

int
initserver(int type, const struct sockaddr *addr, socklen_t alen,
  int qlen)
{
	int fd;
	int err = 0;

	if ((fd = socket(addr->sa_family, type, 0)) < 0)
		return(-1);
	if (bind(fd, addr, alen) < 0) {
		err = errno;
		goto errout;
	}
	if (type == SOCK_STREAM || type == SOCK_SEQPACKET) {
		if (listen(fd, qlen) < 0) {
			err = errno;
			goto errout;
		}
	}
	return(fd);

errout:
	close(fd);
	errno = err;
	return(-1);
}



