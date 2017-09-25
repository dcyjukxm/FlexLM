/******************************************************************************

	    COPYRIGHT (c) 1994, 2003  by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/*	
 *	Module: $Id: jobtests.c,v 1.2 2003/01/13 22:55:14 kmaclean Exp $
 *
 *	Function:	jobtests()
 *
 *	Description: 	Tests license jobs.
 *
 *	D. Birns.
 *
 *	Last changed:  10/18/95
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lmclient.h"
#include "lm_attr.h"
#include "code.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "fcntl.h"
LM_DATA;
long start;
int pid;
int fd;
#define EXIT_SECONDS 25
#define KILL_SECONDS 20
int gotsig = 0;
void doexit(sig)
int sig; 
{
	if (((long)time(0) - start) != EXIT_SECONDS)
		printf("lmalarm(doexit) failed should be %d is %d\n",
			EXIT_SECONDS, time(0) - start);
	exit(0);
}

void catch(sig) 
int sig;
{
	gotsig = 1;
}
LM_HANDLE *handle1;
LM_HANDLE *handle2;
#define DUMMYJOBS 10
LM_HANDLE *dummyjobs[DUMMYJOBS];
main()
{
  int i;

	puts("------------------------------------------------------------");
	puts("Job Tests");
	puts("------------------------------------------------------------");
	tsputenv("LM_LICENSE_FILE=license.dat3:license.dat2");
	unlink("jobtests.log");
	if (!(pid = (int)fork())) 
	{ /*in child*/
		close(1);
		fd = open("jobtests.log", O_WRONLY|O_CREAT, 0666);
		execlp("lmgrd", "lmgrd", "-z", 0);
		exit(0);
	}
	sleep(7);
	lm_init("demo", &code, &lm_job);
	handle1 = lm_job;
	lm_init("demof", &codef, &handle2);
	lm_job = handle2;
	for (i=0;i<DUMMYJOBS;i++)
	{
		lm_init("demof", &codef, &dummyjobs[i]);
		lm_job = dummyjobs[i];
/* 	
 *	these lm_alarm calls should be removed with lm_free_job
 */
		lm_alarm(catch, 2000, 3000); 
	}
	for (i=0;i<DUMMYJOBS;i++)
		lm_free_job(dummyjobs[i]);


	lm_job = handle1;
	if (lm_checkout("f1", "1.0", 1, LM_CO_NOWAIT, &code, LM_DUP_NONE)) 
	{
		lm_perror("Can't checkout f1");
	}
	lm_set_attr(LM_A_CHECK_INTERVAL, 15);
	lm_job = handle2;
	if (lm_checkout("f6", "1.0", 1, LM_CO_NOWAIT, &codef, LM_DUP_NONE)) 
	{
		lm_perror("Can't checkout f6");
	}
	start = (long)time(0);
	if (!lm_alarm(catch, 0, 20000))
		perror ("lm_alarm failed");
	if (!lm_alarm(doexit, 0, 25000))
		perror ("lm_alarm failed");
	while (pause()) {
		if (gotsig) checkouts();
		gotsig = 0;
	}
	lm_free_job(handle1);
	lm_free_job(handle2);
	exit(0);
}
checkouts()
{
	int ret;
	int i;

	i = (time(0) - start);
	if (i != KILL_SECONDS)
		printf("lmalarm(catch) failed, got %d should be %d\n",
		i, KILL_SECONDS);
	lm_job = handle1;
	if (lm_checkin("f1",0))
	{
		lm_perror("Can't checkin f1");
	}
	lm_job = handle2;
	if (lm_checkin("f6",0))
	{
		lm_perror("Can't checkin f6");
	}
	if (ret = kill (pid, 15))
	{
		printf("kill lmgrd failed with errno %d\n", ret);
	}
}
