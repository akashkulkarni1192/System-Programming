#include	<signal.h>
#include	<unistd.h>
#include<stdio.h>
static void sig_alrm(int signo)
{
	printf("\nAlarm signal handler\n");
	/* nothing to do, just return to wake up the pause */
}

unsigned int sleep1(unsigned int nsecs)
{
	if(signal(SIGALRM, sig_alrm) == SIG_ERR)
		return(nsecs);
	alarm(nsecs);		/* start the timer */
	pause();		/* next caught signal wakes us up */
	return(alarm(0));	/* turn off timer, return unslept time */
}
int main(){
	printf("Alarm set for 5 seconds");
	printf("\nn=%d\n",sleep1(5));
	return 0;
}
