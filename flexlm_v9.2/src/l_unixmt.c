/******************************************************************************

	    COPYRIGHT (c) 2002, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_unixmt.c,v 1.30 2003/01/14 21:47:17 kmaclean Exp $
 *
 *
 *	D. Birns
 *	4/01
 *
 */
#if COMMENT
	Overview
	________
	Multi-threaded heartbeats work similarly to windows.
	This is only available where reliable pthreads exists, which
	is all "modern" unix systems.  pthreads standard stabilized around
	1996.

	Summary
	________
	There is a single extra thread created per job to handle heartbeats.
	There is a mutex that is used to guarantee that the job is only
	used by a API lc_xxx call, *or* by the heartbeat thread.
	The heartbeat runs forever until there is a "join" used by the
	parent.

	mutexes
	_______
	We must guarantee that the main thread *or* the heartbeat thread
	have exclusize access to the job internals.
	This is done by honoring the following rules:
	Rule 1:
		LM_API_ERR_CATCH and LM_API_RETURN must be used to begin and 
		return from all lc_xxx functions.  
	Rule 2: 
		No lc_xxx functions can call another lc_xxx functions.
	
	To implement rule 2, if you need to call an lc_xxx functions, make
	a new function with the same name as lc_xxx, but l_xxx.  Have lc_xxx
	call this new function.  And have the LM_API_xxx calls occur only
	in the lc_xxx function, not the l_xxx.
	
	Cleaning up
	___________

	To exit, l_mt_free() is called from the "main" thread.  This function
	signals the heartbeat thread, which then exits when it gets the
	signal.

	The heartbeat thread will be in one of 2 states when it is signalled.
		1) waiting 
		2) checking licenses
	If it s waiting, which will usually be the case, then it returns
	immediately, and there is no delay.
	If it is checking licenses, then this completes before it returns.
	In some cases, this could take considerable time, possibly more than
	a minute.
	
	
#endif

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_attr.h"
#include "lgetattr.h"
#include "l_timers.h"
#include "lm_comm.h"
#include <stdio.h>
#include <errno.h>
#include <time.h>
#ifndef NO_UIO_H
#include <sys/types.h>
#include <sys/uio.h>
#endif
#include "l_prot.h"
#include "l_fdata.h"
#ifdef LM_UNIX_MT
#define _POSIX_C_SOURCE 199506L
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
typedef  struct _mt_info {
	pthread_attr_t attr;
	pthread_t id;
	pthread_mutex_t mutex; /* used by LM_API_xxx */
	pthread_mutex_t mutex2; /* not used as mutex, but rather as a way
				   of signalling the heartbeat thread to
				   exit */
	pthread_cond_t cond;	/* same as mutex2 */
	int interval;
	FP_PTRARG funcptr;

  }  MT_INFO;
static void 		l_mt_heartbeat		(LM_HANDLE *, int, FP_PTRARG );
static void * 		l_mt_child		(void *handle);

/*
 *	l_timer_mt_add:  This fits into the l_timers.c scheme.
 *	This "timer" is added, which simply means that the heartbeat
 *	thread will be added.  This is done with l_mt_heartbeat()
 */
LM_TIMER 
l_timer_mt_add(
LM_HANDLE *job,			/* Current license job */
int timer_type, 		/* REAL or VIRTUAL */
int interval,	 		/* if -1, a no-op -- nothing done */
FP_PTRARG funcptr,		/* function to call */
int check_retry_status, 	/* indicates who's using this signal */
int mseconds_till_next_call)	/* if 0, it's a no-op -- never gets called,
				     and UNSET is set instead */
{
	if ((check_retry_status == LM_TIMER_CHECK) && 
			(job->options->flags & LM_OPTFLAG_MT_HEARTBEAT))
	{
		l_mt_heartbeat(job, interval, funcptr);
		if (job->mt_info) return (LM_TIMER)1;
		else job->options->flags &= ~LM_OPTFLAG_MT_HEARTBEAT;
	}
	return l_timer_add(job, timer_type, interval, funcptr, 
		check_retry_status, mseconds_till_next_call);
}
/*
 *	This changes the heartbeat's wakeup interval
 */
void
l_timer_mt_change(job, t_id, timer_type, interval, funcptr, check_retry_status, 
				mseconds_till_next_call)
LM_HANDLE *job;		/* Current license job */
LM_TIMER t_id;
int 	timer_type;
int 	interval;
FP_PTRARG funcptr;
int	check_retry_status;
int	mseconds_till_next_call;
{
	if (job->mt_info)
	{
	  MT_INFO *mt_info = (MT_INFO *)job->mt_info;

		mt_info->interval = interval;
		return;
	}
	l_timer_change(job, t_id, timer_type, interval, funcptr, 
		check_retry_status, mseconds_till_next_call);
}
int
l_timer_mt_delete(job, t_id)
LM_HANDLE *job;		/* Current license job */
LM_TIMER t_id;
{
	return l_timer_delete(job, t_id);
}
/*
 *	l_mt_heartbeat:  create the heartbeat thread.  
 */

static
void
l_mt_heartbeat(LM_HANDLE *job,  int interval,  FP_PTRARG funcptr)
{
  pthread_attr_t attr;
  pthread_t 	tid[100];
  MT_INFO *mt_info;

	if (job->mt_info) return; /* already done */
	job->mt_info = (char *)
		l_malloc(job, sizeof(MT_INFO));
	mt_info = (MT_INFO *) job->mt_info;
	mt_info->funcptr = funcptr;
	mt_info->interval = interval;

			
	if (pthread_attr_init(&mt_info->attr))
	{
		return;
	}
#define L_UNIXMT_PMI 0
#define L_UNIXMT_PCI 0
	pthread_mutex_init(&mt_info->mutex, L_UNIXMT_PMI);
	pthread_mutex_init(&mt_info->mutex2, L_UNIXMT_PMI);
	pthread_cond_init(&mt_info->cond, L_UNIXMT_PCI);
	pthread_create(&mt_info->id, & mt_info->attr, 
			l_mt_child, job);
}


static
void *
l_mt_child(void *handle)
{
  LM_HANDLE *job = (LM_HANDLE *)handle;
  MT_INFO *mt_info = (MT_INFO *)job->mt_info;
  pthread_t id;
  struct timespec t;

	if (!mt_info) return 0; /* huh?? */
	/*puts("l_mt_child");*/
	/*pthread_cond_signal(&mt_info->cond);*/
	while (1)
	{
		if (!mt_info->interval) return (0);
		pthread_mutex_lock( &mt_info->mutex2 ); /* used for wait */
		if (!mt_info->interval) return (0);
		t.tv_sec = time(0);
		t.tv_sec += mt_info->interval/1000;  /* interval is millisecs */
		t.tv_nsec = (mt_info->interval % 1000) * 1000;
		/* check for interval being zero -- this means job is 
			being freed */

/* 
 * 		Interval is user's LM_A_CHECK_INTERVAL 
 *		let parent signal that job is being freed 
 */
		/*printf("calling timedwait for %d ms\n", mt_info->interval);*/
		if (pthread_cond_timedwait(&mt_info->cond, &mt_info->mutex2, &t)
							!= ETIMEDOUT)
		{
			pthread_mutex_unlock( &mt_info->mutex2 ); /* free it */
			return(0);
		}

		pthread_mutex_unlock( &mt_info->mutex2 ); /* done with it */
			
/*
 *		If this job was deleted in the meantime, then return
 */
		if (mt_info != (MT_INFO *)job->mt_info) 
		{
			return 0; /* huh?? */
		}
		if (!mt_info->interval) return (0);
		/*puts("locking in child");*/
/*		Use mutex for exclusive access to internals 
 *		all lc_xxx calls are locked out until we're done
 */
		l_mt_lock( job, __FILE__, __LINE__);
		mt_info->funcptr(job);
		/*puts("unlocking in child");*/
		l_mt_unlock( job, __FILE__, __LINE__);
	}

}
void
l_mt_lock(LM_HANDLE *job, char *file, int l)
{
  MT_INFO *mt_info = (MT_INFO *)job->mt_info;

	if (!mt_info) return; 
	/*printf("locking  %s %d\n", file, l);*/
	pthread_mutex_lock( &mt_info->mutex );
}
void
l_mt_unlock(LM_HANDLE *job, char *file, int l)
{
  MT_INFO *mt_info = (MT_INFO *)job->mt_info;

	if (!mt_info) return; 
	/*printf("unlocking  %s %d\n", file, l); */
	pthread_mutex_unlock( &mt_info->mutex );
}

void
l_mt_free(LM_HANDLE *job)
{
	if (job->mt_info)
	{
	  MT_INFO *mt_info = (MT_INFO *)job->mt_info;
	  int rc;

		mt_info->interval = 0;
		l_timer_mt_delete(job, 0);
/*		puts("parent signalling");*/
		pthread_cond_signal(&mt_info->cond);
/*		puts("parent joining");*/

		pthread_join(mt_info->id, 0);
		pthread_cond_destroy(&mt_info->cond);
		pthread_mutex_destroy(&mt_info->mutex);
		pthread_mutex_destroy(&mt_info->mutex2);
#ifndef DEC_UNIX_3
		pthread_attr_destroy(&mt_info->attr);
#endif /* DEC_UNIX_3 */
		free(mt_info);
		job->mt_info = 0;
	}
}
#else
void l_mt_unlock(LM_HANDLE *job, char *file, int l) { }
void l_mt_lock(LM_HANDLE *job, char *file, int l) { }
void
l_mt_free(LM_HANDLE *job) {}

LM_TIMER
l_timer_mt_add(
LM_HANDLE *job,			/* Current license job */
int timer_type, 		/* REAL or VIRTUAL */
int interval,	 		/* if -1, a no-op -- nothing done */
FP_PTRARG funcptr,		/* function to call */
int check_retry_status, 	/* indicates who's using this signal */
int mseconds_till_next_call)	/* if 0, it's a no-op -- never gets called,
				     and UNSET is set instead */
{
	return l_timer_add(job, timer_type, interval, funcptr, 
		check_retry_status, mseconds_till_next_call);
}

void
l_timer_mt_change(job, t_id, timer_type, interval, funcptr, check_retry_status, 
				mseconds_till_next_call)
LM_HANDLE *job;		/* Current license job */
LM_TIMER t_id;
int 	timer_type;
int 	interval;
FP_PTRARG funcptr;
int	check_retry_status;
int	mseconds_till_next_call;
{
	l_timer_change(job, t_id, timer_type, interval, funcptr, 
		check_retry_status, mseconds_till_next_call);
}

int
l_timer_mt_delete(job, t_id)
LM_HANDLE *job;		/* Current license job */
LM_TIMER t_id;
{
	return l_timer_delete(job, t_id);
}
#endif /* LM_UNIX_MT */
