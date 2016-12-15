#include "apue.h"
#include<signal.h>
#include<errno.h>
/*#include<stdio.h>*/
/*#include<unistd.h>*/
/*void err_sys(const char* x) */
/*{ */
/*    perror(x); */
/*    exit(1); */
/*}*/
void pr_mask(const char *str){
	sigset_t sigset;
	int errno_save;
	errno_save = errno;
	sigaddset(&sigset,SIGINT);
	if(sigprocmask(SIG_BLOCK, NULL, &sigset) < 0){
		err_sys("sigprocmask error");
	}
	printf("%s", str);
	if(sigismember(&sigset, SIGINT)) printf("SIGINT ");
	if(sigismember(&sigset, SIGQUIT)) printf("SIGQUIT ");
	if(sigismember(&sigset, SIGUSR1)) printf("SIGUSR1 ");
	if(sigismember(&sigset, SIGALRM)) printf("SIGALRM ");
	printf("\n");
	errno = errno_save;
}
/*
 * =====================================================================================
 *
 *       Filename:  apue.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2009/06/23 16时31分19秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jack.zh(toserver@gmail.com) 
 *        Company:  
 *
 * =====================================================================================
 */

/*#include "apue.h"*/
#include <errno.h>
#include <stdarg.h>

static void err_doit(int, int, const char *, va_list);
static void sig_int(int signo)
{
	pr_mask("\nin signal handler of SIGINT: ");
}
void err_quit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

void err_sys(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

void err_msg(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}

void err_ret(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

static
void
err_doit(
		int errnoflag,
		int error,
		const char *fmt,
		va_list ap
		)
{
	char buf[MAXLINE];
	vsnprintf(buf, MAXLINE, fmt, ap);

	if (errnoflag)
	{
		snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s", strerror(error));
	}

	strcat(buf, "\n");
	fflush(stdout);
	fputs(buf, stderr);
	fflush(NULL);
}

int
main(void)
{
	sigset_t	newmask, oldmask, waitmask;

	pr_mask("program start: ");

	if (signal(SIGINT, sig_int) == SIG_ERR)
		err_sys("signal(SIGINT) error");
	sigemptyset(&waitmask);
	sigaddset(&waitmask, SIGUSR1);
	//sigaddset(&waitmask, SIGINT);	---------
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGINT);

	/*
	 * Block SIGINT and save current signal mask.
	 */
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		err_sys("SIG_BLOCK error");

	/*
	 * Critical region of code.
	 */
	pr_mask("in critical region: ");

	/*
	 * Pause, allowing all signals except SIGUSR1.
	 */
	if (sigsuspend(&waitmask) != -1)
		err_sys("sigsuspend error");

	pr_mask("after return from sigsuspend: ");

	/*
	 * Reset signal mask which unblocks SIGINT.
	 */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("SIG_SETMASK error");

	/*
	 * And continue processing ...
	 */
	pr_mask("program exit: ");

	exit(0);
}
