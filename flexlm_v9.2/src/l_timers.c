/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_timers.c,v 1.21 2003/04/16 18:25:16 brolland Exp $
 *
 *	Function: lc_alarm(job, funcptr, repeat_interval,
 *				mseconds_till_next_call)
 *		  lc_disalarm(job, timer)
 *		  lm_sleep(seconds)
 *		  lc_nap(job, milliseconds)
 *		  l_timer_add(job, timer_type, interval, funcptr,
 *				check_retry_status, mseconds_till_next_call)
 *		  l_timer_delete(job, timer)
 *		  l_timer_change(job, timer, timer_type, interval, funcptr,
 *				check_retry_status, mseconds_till_next_call)
 *
 *	All times in MILLISECONDS -- 1/1000 of a second, except lm_sleep()
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *      		(int) lm_timer_type:	LM_REAL_TIMER or
 *						LM_VIRTUAL_TIMER
 *			(void )(*funcptr)(int):	function to be called
 *			(int)repeat_interval:	a la setitimer
 *			(int)mseconds_till_next_call:	a la setitimer:
 *							if 0, it is UNSET
 *			(int) timer_type:	LM_REAL_TIMER or
 *						LM_VIRTUAL_TIMER
 *			(int)check_retry_status: LM_TIMER_CHECK or
 *						LM_TIMER_RETRY or
 *						LM_TIMER_USER_FUNC
 *						LM_TIMER_CONNECT
 *			(int)sig:		will be SIGALRM or SIGVTALRM
 *						depending on timer_type
 *			(LM_TIMER) timer	timer handle returned from
 *						l_timer_add or lc_alarm
 *
 *		NOTE for l_timer_change:	parameters which are
 *						NOT changed are LM_TIMER_UNSET
 *
 *	Return:		lc_alarm and l_timer_add return (LM_TIMER *)handle.
 *			lc_disalarm and l_timer_delete return remaining
 *			milliseconds on success or job->lm_errno.
 *			l_timer_router is a void.
 *
 *	Description: 	Client application can schedule timers without
 *			conflicting with FLEXlm timers by calling lc_alarm().
 *			Unlimited number of timers can be scheduled.
 *			If repeat_interval is 0, the timer will only
 *			go off once.  The user is returned a handle,
 *			which can only be used as an argument to
 *			lc_disalarm() to remove scheduled event.
 *
 *			NOTE:  Virtual timers are handled separately
 *			from real timers.  The reason is as follows:
 *			I set the clock time that the timer should go off.
 *			For real timers, this works as expected.  But for
 *			virtual timers, we don't really know the clock
 *			time.  next_time is set to number of mseconds.
 *			When a virtual timer goes off, we decrement all
 *			the remaining virtual timers by the number of
 *			mseconds in the one that went off.
 *
	Examples of timer behavior:

	Ex 1.
		lm_alarm(func, 2000, 2000);

		This will be called in 2 seconds and 2 seconds thereafter

	Ex. 2.
		func()
		{
			#busy stuff that takes 3 seconds
		}
		...
		lm_alarm(func, 2000, 2000);

		func will be called in 2 seconds, and every 4 seconds
		thereafter.  This is done to prevent infinite loop.

	Ex. 3
		lm_alarm(func1, 2000, 2000);
		lm_alarm(func2, 1000, 2000);

		in 2 seconds, both func1 and func2 will get called,
		func1 first.  Then every second thereafter func2
		gets called, and func1 every 2 seconds.

	Ex. 4
		func1()
		{
			lm_alarm(func2, 0, 1000);
		}
		...
		lm_alarm(func1, 2000, 2000);

		in 2 seconds, func1 will get called,  returning
		immediately.  Thereafter, every odd second,
		func2 will get called and every even second func1
		will get called.

		This is exactly equivalent to:

		lm_alarm(func1, 2000, 2000);
		lm_alarm(func2, 2000, 3000);

	Ex. 5
		lm_alarm(func1, 2000, 2000);
		lm_sleep(5);
		func2();

		In 2 seconds, func1, will get called, and every
		2 seconds thereafter.  In 5 seconds, func2 will
		get called.

	Examples 6 and 7 show that infinite looping is internally
	automatically avoided.

	Ex. 6
		func1()
		{
			lm_sleep(3);
		}
		....
		lm_alarm(func1, 2000, 2000);

		In 2 seconds func1 will get called, and every 4
		seconds thereafter.

	Ex. 7
		func()
		{
			#busy stuff, takes 3 seconds to complete
		}
		...
		lm_alarm(func, 2000, 2000);

		This will get called in 2 seconds, and once every 4 seconds
		thereafter.

	Ex. 8
		func1()
		{
			#busy stuff that takes 3 seconds, or lm_sleep(3)
		}
		lm_alarm(func1, 2000, 2000);
		lm_alarm(func2, 2000, 1000);

		func1 gets called in 2 seconds, and every 4 seconds
		thereafter.  func2 gets called in 1 second, and every
		2 seconds thereafter.

	In example 9, lm_sleep and select(0,0,0,0,<time>) have almost
	identical behavior.

	Ex. 9

            A.

                for (start = time(0);
		     time(0) - start < finish;
                     finish -= (time(0) - start)
                {
                        select(0,0,0,0,<finish seconds>);
                }

            B.

                lm_sleep(finish);

        The difference is that in B, lm_sleep() will be interrupted by
        non-SIGALRM signals, while in A, the select loop will last at
        least <finish> seconds, regardless of the signals that occur
        (as long as any interrupts eventually return).  If B were put
        in the same for(;;) loop, it would then have identical behavior.

 *
 *	D. Birns
 *	6/93
 *	Last changed:  12/8/98
 *
 */
#include "lmachdep.h"
#include <sys/types.h>

#ifdef PC
#include <winsock.h>
#else
#include <stdio.h>
#ifdef ANSI
#include <unistd.h>
#endif /* ANSI */
#endif /* PC */

#include <signal.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lm_attr.h"
#include "l_timers.h"
#include "lgetattr.h"
#include "l_time.h"
typedef struct _lm_time_type {
	int 	seconds;
	int	milliseconds;
} LM_TIME_TYPE;

#ifdef DEBUG_TIMERS
#ifndef lm_sleep
#define lm_sleep(x) lc_sleep(lm_job, x)
#endif
#define API_ENTRY
#endif
#ifdef getenv
#undef getenv
#endif /* getenv */

#ifdef PC
static unsigned int active_timer = 0;
typedef void (LM_CALLBACK_TYPE * LM_CALLBACK_FUNCPTR) (int job);
#define LM_CALLBACK_FUNCPTR_TYPE (LM_CALLBACK_FUNCPTR)
typedef void (LM_CALLBACK_TYPE * LM_CALLBACK_FUNCPTR1) (LM_HANDLE * job);
#define LM_CALLBACK_FUNCPTR1_TYPE (LM_CALLBACK_FUNCPTR1)
#ifdef WINNT
#undef SetTimer
#undef KillTimer
#define SetTimer SetTimer_NT
#define KillTimer KillTimer_NT
extern int lm_timers_in_use;
#endif /* WINNT */
#else
#define LM_CALLBACK_FUNCPTR_TYPE
#define LM_CALLBACK_FUNCPTR1_TYPE
#endif



/*
 *	TIMER_TAB
 *	Used for internal and application timers returned by lc_alarm()
 */
typedef struct _lm_timer{
	struct _lm_timer *next;	/* linked list of timers */
	LM_TIMER id; 		/* id number -- given out serially from 1 */
	int timer_type;		/* LM_REAL_TIMER or LM_VIRTUAL_TIMER */
	int interval;		/* Value to set to next time timer goes off */
	FP_PTRARG funcptr; 	/* function to call -- arg will be SIGALRM or
				 * SIGVTALRM or job handle */
	int check_retry_status;	/* flag for checking connection, or retrying
				 *  connection -- interval is different for
				 *  each.
				 */
	LM_TIME_TYPE next_time;	/* next time function called */
	LM_HANDLE *job;
	char block;		/* set while funcptr is called to prevent */
				/* infinite loop */
}	TIMER_TAB;


#ifndef NO_TIMERS
#define INIT_NUM_TIMERS 5
static TIMER_TAB free_list_start[INIT_NUM_TIMERS];
static TIMER_TAB *free_list = (TIMER_TAB *)-1;
static TIMER_TAB *last_virtual_timer = NULL;
static TIMER_TAB *first_virtual_timer = NULL;
static TIMER_TAB *next_virtual_timer = NULL;

static TIMER_TAB *last_real_timer = NULL;
static TIMER_TAB *first_real_timer = NULL;
static TIMER_TAB *next_real_timer = NULL;
static LM_TIMER  next_id_num = (LM_TIMER) 1;
/*
 *	These two statics are used by lm_sleep
 */
static int got_sigalarm;
static int got_sleep_cb;

/****************hacks for sunos5**********************************/
#if (defined(SUNOS5) && !defined(_SIGACTION_) && defined(GCC))
		/*for some reason, sun doesn't define this if __STDC__*/
	typedef struct {                /* signal set type */
		unsigned long   __sigbits[4];
	} sigset_t;
	struct sigaction {
		int sa_flags;
		void (*sa_handler)();
		sigset_t sa_mask;
		int sa_resv[2];
	};
#endif
/****************end hacks for sunos5**********************************/
/*********************local prototypes here************************/

static int 		check_for_user_timers 	lm_args((LM_HANDLE *));
static void 		l_block_sigalarm	lm_args((int));
#ifndef PC
static void 		l_pause 		lm_args((LM_HANDLE *));
#endif
static void 		l_timer_reset 		lm_args((LM_HANDLE *, int,
						LM_TIME_TYPE *));
static void 		l_timer_increment 	lm_args((TIMER_TAB *, int));
static LM_TIME_TYPE *	l_timer_now 		lm_args((LM_TIME_TYPE *));
static TIMER_TAB * 	l_timer_to_tab 		lm_args((LM_TIMER));
static LM_TIME_TYPE * 	l_timer_msecs2time 	lm_args((int));
static unsigned long 	l_timer_time2msecs 	lm_args((LM_TIME_TYPE *));
static void 		l_timer_route_or_set 	lm_args((LM_HANDLE *, int, int));
#ifdef VOID_SIGNAL
static void		l_timer_router 		lm_args((int sig));
#else
static int		l_timer_router 		lm_args((int sig));
#endif
static void 		l_timer_set 		lm_args((LM_HANDLE *, int, int,
						struct itimerval *));

#ifndef RELEASE_VERSION
static double 		timet2d 	lm_args(( LM_TIME_TYPE *));
#endif
static int 		timet_compare 	lm_args((LM_TIME_TYPE *,
						LM_TIME_TYPE *));
static int 		timet_diff 	lm_args((LM_TIME_TYPE *,
						LM_TIME_TYPE *));
static void 		timet_plus_msecs lm_args(( LM_TIME_TYPE *, int));
/**********************end local prototypes**************************/

#ifdef PC
void CALLBACK timer_proc( HWND hwnd, UINT msg, UINT idTimer, DWORD dwTime )
{
	l_timer_router(SIGALRM);
}
#endif /* PC */

#ifndef NO_TIMERS

#ifndef RELEASE_VERSION
static void dumpit();
static int debug_timers = 0;
#define DEBUG(x) if (debug_timers) x;
#define NOTRELEASE(x) x;
#else
#define DEBUG(x)
#define NOTRELEASE(x)
#endif
static SIGFUNCP _user_timer_func;
#endif
#ifdef getenv
#undef getenv
#endif

/*
 *	l_timer_to_tab -- return TIMER_TAB * when given LM_TIMER
 */
static
TIMER_TAB *
l_timer_to_tab(t_id)
LM_TIMER t_id;
{
  TIMER_TAB *t;

	DEBUG(printf("timer_to_tab: t_id  %d\n", t_id));
	for (t = first_real_timer;t; t=t->next)
		if (t->id == t_id)
			return t;
	for (t = first_virtual_timer;t; t=t->next)
		if (t->id == t_id)
			return t;
	return (TIMER_TAB *)0;
}

/*
 *	l_timer_add -- add a timer
 */
LM_TIMER API_ENTRY
l_timer_add(job, timer_type, interval, funcptr, check_retry_status,
						mseconds_till_next_call)
LM_HANDLE *job;			/* Current license job */
int timer_type; 		/* REAL or VIRTUAL */
int interval;	 		/* if -1, a no-op -- nothing done */
FP_PTRARG funcptr;		/* function to call */
int check_retry_status; 	/* indicates who's using this signal */
int mseconds_till_next_call;	/* if 0, it's a no-op -- never gets called,
				     and UNSET is set instead */
{
  TIMER_TAB **first, **timers;
  int i;
  LM_TIMER ret;

/*
 *		set up free_list on first time through
 */
	if (free_list == (TIMER_TAB *)-1)
	{
	  TIMER_TAB *t;

		t = free_list = free_list_start;

		for (i=0;i<INIT_NUM_TIMERS-1;i++)
		{
			t->next = t + 1;
			t = t->next;
		}
	}
	if (interval  == -1 || !mseconds_till_next_call)
	{
#ifndef DEBUG_TIMERS
		if (!job->lm_errno)
		{
			LM_SET_ERRNO(job, LM_BADPARAM, 99, 0);
		}
#endif
		return 0;
	}

/*
 *	set pointers based on type
 */
	if (timer_type == LM_REAL_TIMER)
	{
		first = &first_real_timer;
		timers = &last_real_timer;
	}
	else
	{
		first = &first_virtual_timer;
		timers = &last_virtual_timer;
	}

/*
 *	initialize if necessary
 */
	if (!*timers)
	{
		NOTRELEASE (if (getenv("DEBUG_TIMERS"))	/* overrun checked */
						debug_timers = 1);
		*first = *timers = (TIMER_TAB *)l_mem_malloc(job,
			sizeof(TIMER_TAB), (void **)&free_list);
		if (!*timers)
		{
			LM_SET_ERRNO(job, LM_CANTMALLOC, 101, 0);
			return 0;
		}
	}
	else
	{
		(*timers)->next = (TIMER_TAB *)l_mem_malloc(job,
			sizeof(TIMER_TAB), (void **)&free_list);
		if (!(*timers)->next)
		{
			LM_SET_ERRNO(job, LM_CANTMALLOC, 101, 0);
			return 0;
		}

		memset((char *) (*timers)->next, 0,sizeof(TIMER_TAB));
		*timers = (*timers)->next;
	}
/*
 * 	initialize TIMER_TAB struct
 */
	ret = (*timers)->id = next_id_num++;
	(*timers)->next = NULL;
	(*timers)->timer_type = timer_type;
	(*timers)->interval = interval;
	(*timers)->funcptr = funcptr;
	(*timers)->check_retry_status = check_retry_status;
	(*timers)->job = job;
	(*timers)->block = 0;
	if (timer_type == LM_REAL_TIMER)
	{
		l_timer_now(&(*timers)->next_time);
		timet_plus_msecs(&(*timers)->next_time,
				   mseconds_till_next_call);
	}
	else
	{
		memcpy((char *)&(*timers)->next_time,
			(char *)l_timer_msecs2time(mseconds_till_next_call),
			sizeof(LM_TIME_TYPE));
	}
	DEBUG(
		printf(
		"add: now %g id %d intrv %d stat %s next=%g next %d func 0x%x\n",
			timet2d(l_timer_now((LM_TIME_TYPE *)0)),
			(*timers)->id, interval,
			check_retry_status == LM_TIMER_CHECK ? "CHECK" :
			check_retry_status == LM_TIMER_RETRY ? "RETRY" :
			check_retry_status == LM_TIMER_USER_FUNC ? "USER_FUNC" :
			check_retry_status == LM_TIMER_CONNECT ? "CONNECT" :
			check_retry_status == LM_TIMER_LMGRD ? "LMGRD" :
			check_retry_status == LM_TIMER_VDAEMON ? "VDAEMON" :
			check_retry_status == LM_TIMER_USER_CAUGHT ? "USER_CAUGHT" :
			check_retry_status == LM_TIMER_SLEEP ? "SLEEP" :
			check_retry_status == LM_TIMER_UNSET ? "UNSET" :
			"UNKNOWN",
			timet2d(&(*timers)->next_time),
			mseconds_till_next_call,
			funcptr));
	if (!check_for_user_timers(job))
		l_timer_reset(job, timer_type, (LM_TIME_TYPE *)NULL);
#ifdef WINNT
        /* When setting a time, increment the global use value */
        lm_timers_in_use++;
#endif

	return ret;
}

/*
 *	l_timer_router -- call a timer's function -- reset next time, and
 *			  call l_timer_reset
 */
static
#ifdef VOID_SIGNAL
void
#endif
l_timer_router(sig) /* called when SIGALRM goes off */
int sig;
{

  TIMER_TAB *t; /*real or virtual timer*/
  int timer_type;

#ifdef VMS
/*
 *	Let the select() emulation routine know that the timer fired,
 *	so it can fake EINTR
 */
	sys$setef(TIMER_EF);
#endif /* VMS */

/*
 *	set pointers based on type
 */

	if (sig == SIGALRM)
	{
		got_sigalarm =1; /*used by lm_sleep*/
		t = next_real_timer;
		timer_type = LM_REAL_TIMER;
	}
	else
	{
		t = next_virtual_timer;
		timer_type = LM_VIRTUAL_TIMER;
	}

	if (!t) {
		NOTRELEASE(
			printf("SIGALRM went off without handler %s %d\n",
			__FILE__, __LINE__));
		return;
	}
	DEBUG( printf("router: id %d func 0x%x, now %g job %x\n",
				t->id, t->funcptr,
			timet2d(l_timer_now((LM_TIME_TYPE *)0)), t->job));
/*
 *	We're going to call the func now.  1) block this function
 *	from being called while we're in it, 2) reset
 *	timers, so that we can set the next timer to go off while we're
 *	in this function 3) set the job before the function's called.
 */
	l_timer_signal(t->job, sig, l_timer_router);
	t->block = 1;
	l_timer_reset(t->job, timer_type, (LM_TIME_TYPE *)NULL);

/*
 *	Unix timers normally get the signal as the first argument
 *	If the routine to be called is lm_timer(), then I call it
 *	with the job handle, otherwise, I use the signal number
 */
	switch (t->check_retry_status )
	{
		case LM_TIMER_CHECK:
		case LM_TIMER_RETRY:
			(* LM_CALLBACK_FUNCPTR1_TYPE(t->funcptr))(t->job);
			break;
		default:
#ifdef PC
			(* LM_CALLBACK_FUNCPTR_TYPE (t->funcptr))(sig);
#else
			(*(FP_INTARG)(t->funcptr))(sig);
#endif /* PC */
	}

	if (t->interval > 0)
	{
		l_timer_increment(t, sig); /*to prevent infinite loop*/
		t->block = 0; /*reset time, but don't call*/
		l_timer_reset(t->job, timer_type, (LM_TIME_TYPE *)NULL);
	}
	else /* This timer won't be used again */
	{
		l_timer_delete(t->job, t->id); /*delete does the reset*/
	}
}

static
void
l_timer_increment(t, sig)
TIMER_TAB *t;
int sig;
{
	if (t->interval > 0)
	{
		if (sig == SIGALRM)
		{
		  LM_TIME_TYPE now;
			l_timer_now(&now);

			while (timet_compare(&t->next_time,  &now) <=0)
				timet_plus_msecs(&t->next_time,
								t->interval);
			if (t->block &&
				(timet_diff(&t->next_time, &now) < 500))
				timet_plus_msecs(&t->next_time,
								t->interval);

		}
		else
		{
			LM_TIME_TYPE sav_decr;
			memcpy((char *)&sav_decr, (char *)&t->next_time,
					sizeof(LM_TIME_TYPE));
			timet_plus_msecs(&t->next_time,  t->interval);
							/*will get decremented
							in reset_timers*/
		}
	}
	else
	{

		t->next_time.seconds = LM_TIMER_UNSET;
	}
	DEBUG( printf("end %g\n", timet2d(&t->next_time)));
}

/*
 *	l_timer_delete -- delete a timer
 */

int
l_timer_delete(job, t_id)
LM_HANDLE *job;		/* Current license job */
LM_TIMER t_id;
{
	TIMER_TAB *timer, *t, *tsav, **first, **next;
	LM_TIME_TYPE sav_decr;
	int remaining_time;

	if (!t_id) /*already deleted or error*/
		return 0;

	if (!(timer = l_timer_to_tab(t_id)))
		return(0);
	DEBUG( printf("delete: id %d func %x\n", timer->id,
						timer->funcptr));

/*
 *	Set the funcptr to null, so if we are asked to delete it again,
 * 	we know to ignore the request
 */
	timer->funcptr = 0;

/*
 *	Setup pointers based on type
 */
	if (timer->timer_type == LM_REAL_TIMER)
	{
		first = &first_real_timer;
		next = &next_real_timer;
	}
	else
	{
		memcpy((char *)&sav_decr, (char *)&timer->next_time,
						sizeof(LM_TIME_TYPE));
		first = &first_virtual_timer;
		next = &next_virtual_timer;
	}

/*
 *	Find the location in the linked list of timers
 */
	for (t = *first, tsav = NULL; t; t=t->next)
	{
		if (t == timer)
			break;
		tsav = t;
	}
	if (!t) /*sanity check*/
	{
		/*not a serious condition, so don't set job->lm_errno*/
		return BADPARAM;
	}
	remaining_time = timet_diff(&t->next_time, l_timer_now((LM_TIME_TYPE *)0));

/*
 *	Fix the linked list up to remove this timer
 */
	if (tsav) /*if this was not the first in the list*/
	{
		tsav->next = t->next;
	}
	else if (t == *first)
	{
		*first= (*first)->next;
		if (!*first) last_real_timer = NULL;
	}
	else
	{
		NOTRELEASE(printf( "EEKS! %s line %s\n",
					__FILE__, __LINE__));
	}
	if (t == last_real_timer)
	{
		last_real_timer = tsav; /*may be NULL*/
	}
	if (*next == t)

	{
		l_timer_reset(job, t->timer_type, &sav_decr);
	}
	if (remaining_time < 0) remaining_time = 0; /*<0 would mean error*/
	l_mem_free((char *)t, (void **)&free_list);

#ifdef WINNT
        /* When killing a timer decrement the global use value */
        lm_timers_in_use--;
#endif
	return remaining_time;
}

/*
 *	l_timer_job_done -- turn off all timers for the job
 *			    needed by lm_free_job()
 */

void
l_timer_job_done(job)
LM_HANDLE *job;
{
    TIMER_TAB *t, *next ;
	for (next = t = first_real_timer; t; t=next)
	{
		next = t->next;
		if (t->job == job)
			l_timer_delete(job, t->id);
	}
	for (next = t = first_virtual_timer; t; t=next)
	{
		next = t->next;
		if (t->job == job)
			l_timer_delete(job, t->id);
	}
}

/*
 *	l_timer_reset -- goes through timers list, identifying next
 *			  timer to go off
 */

static
void
l_timer_reset(job, timer_type, virtual_decrement)
LM_HANDLE *job;		/* Current license job */
int timer_type;
LM_TIME_TYPE *virtual_decrement; /*Only used for LM_VIRTUAL_TIMER*/
{
	TIMER_TAB *t = NULL, **first_timer, **next_timer;
	LM_TIME_TYPE *next_timep = NULL;
	unsigned long msecs;

	l_block_sigalarm(1);

/*
 *	set pointers based on type
 */
	if (timer_type == LM_REAL_TIMER)
	{
		first_timer = &first_real_timer;
		next_timer = &next_real_timer;
	}
	else
	{
		first_timer = &first_virtual_timer;
		next_timer = &next_virtual_timer;
	}


/*
 *	find the next timer that should go off
 */

	for (*next_timer = NULL, t = *first_timer; t; t=t->next)
	{
		if (timer_type == LM_VIRTUAL_TIMER)
		{
			int diff;
			diff = timet_diff(&t->next_time,
					virtual_decrement);
			t->next_time.seconds = diff/1000;
			t->next_time.milliseconds = diff%1000;
		}
		if (!t->block &&
			(t->next_time.seconds != LM_TIMER_UNSET) &&
			(!next_timep ||
				(timet_compare(&t->next_time,
								next_timep)<0)))
		{
			next_timep = &t->next_time;
			*next_timer = t;
		}
	}

	DEBUG(
		if (!*next_timer)
		{
			puts("reset: no next_timer");
			printf("reset: ");
			dumpit();
		}
	);
	if (!*next_timer)
	{
		/* disable all timers */
		l_timer_set(job, timer_type, 0, (struct itimerval *)0);
		l_block_sigalarm(0);
		return;
	}

	if (timer_type == LM_REAL_TIMER)
		msecs = timet_diff(&(*next_timer)->next_time,
				l_timer_now((LM_TIME_TYPE *)0));
	else
		msecs = l_timer_time2msecs(&(*next_timer)->next_time);

	l_block_sigalarm(0);
	l_timer_route_or_set(job, msecs, timer_type);
}

/*
 *	l_timer_route_or_set -- If it's time for the timer to go off,
 *				call setitimer, else call
 *				kill(getpid(),SIGALRM);
 */
static
void
l_timer_route_or_set(job, msecs, timer_type)
LM_HANDLE *job;		/* Current license job */
int msecs;
int timer_type;
{
	if (msecs <= 0)
	{
		DEBUG(puts("reset calling router"));
		l_timer_router(timer_type ==
					LM_REAL_TIMER ? SIGALRM : SIGVTALRM);
	}
	else
	{

/*
 *	reset the alarm to go off
 */
		if (msecs > 0) {
			l_timer_set(job, timer_type, msecs,
						(struct itimerval *)0);
		}
	}
}

/*
 *	lc_alarm() -- set an alarm to go off
 */
LM_TIMER API_ENTRY
lc_alarm(job, funcptr, repeat_interval, mseconds_till_next_call)
LM_HANDLE *job;		/* Current license job */
FP_INTARG funcptr;
int repeat_interval;
int mseconds_till_next_call;
{

/*
 *	check if application alarms are supported
 */
#ifndef DEBUG_TIMERS
	if (l_getattr(job, FULL_FLEXLM) != FULL_FLEXLM_VAL)
	{
		LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 103, 0);
		return 0;
	}
#endif


/*
 *	Try to warn the user if they think the alarm is seconds
 *	< 10 milliseconds is probably smaller than any systems
 *	granularity, so don't accept it.
 */
	if (mseconds_till_next_call < 10 ||!funcptr)
	{
#ifndef DEBUG_TIMERS
		if (!job->lm_errno)
		{
			LM_SET_ERRNO(job, LM_BADPARAM, 104, 0);
		}
#endif
		return 0;
	}
	return (l_timer_add(job, LM_REAL_TIMER, repeat_interval,
				(FP_PTRARG)funcptr, LM_TIMER_USER_FUNC,
				mseconds_till_next_call));
}
/*
 * 	lc_disalarm -- turn off a timer
 */
int API_ENTRY
lc_disalarm(job, timer)
LM_HANDLE *job;		/* Current license job */
LM_TIMER timer;
{
	return l_timer_delete(job, timer);
}

/*
 *	check_for_user_timers -- user timers MUST be set before any
 *				 call to lm_*() function  -- in that
 *				 case, we save it, and add it to the
 *				 timer list
 *		RETURN:		0 -- no user timer found
 *				1 -- user timer found
 */

static
int
check_for_user_timers(job)
LM_HANDLE *job;		/* Current license job */
{

#ifdef VMS /* No need for checking user_timers */
	return 0;
#else

    struct itimerval _lm_user_interval;

	memset(&_lm_user_interval, 0, sizeof(struct itimerval));
	_user_timer_func = (SIGFUNCP)l_timer_signal(job, SIGALRM, (SIGFUNCP)SIG_IGN);
	if (_user_timer_func == (SIGFUNCP)l_timer_router ||
		(int)_user_timer_func < 100)
	{
		/*No user timer*/
		l_timer_signal(job, SIGALRM, l_timer_router);
		return 0;
	}

#ifdef UNIX
	getitimer(ITIMER_REAL, &_lm_user_interval);

/*
 *	Some platforms (solaris) don't get timer set by alarm()
 *	via getitimer(), so we have to use alarm also
 */
	if (!_lm_user_interval.it_interval.tv_sec &&
				    !_lm_user_interval.it_value.tv_sec)
	{
		_lm_user_interval.it_value.tv_sec = alarm(0);
	}
#endif /* UNIX */
	/* turn off to avoid a window */
	l_timer_set(job, LM_REAL_TIMER,0, (struct itimerval *)0);
	l_timer_signal(job, SIGALRM, l_timer_router);
	if ((int)_lm_user_interval.it_value.tv_sec > 0)
	{
		l_timer_add(job, LM_REAL_TIMER,
			    (int)(_lm_user_interval.it_interval.tv_sec * 1000 +
			    _lm_user_interval.it_interval.tv_usec / 1000),
			    (FP_PTRARG)_user_timer_func,
			    LM_TIMER_USER_CAUGHT,
			    (int)(_lm_user_interval.it_value.tv_sec * 1000 +
			    _lm_user_interval.it_value.tv_usec / 1000));
		return 1;
	}
	return 0;
#endif
}

/*
 *	l_timer_change -- change a timer's value and call l_timer_reset
 */

void
l_timer_change(job, t_id, timer_type, interval, funcptr, check_retry_status,
				mseconds_till_next_call)
LM_HANDLE *job;		/* Current license job */
LM_TIMER t_id;
int 	timer_type;
int 	interval;
FP_PTRARG funcptr;
int	check_retry_status;
int	mseconds_till_next_call;
{
	TIMER_TAB *timer;

	if (!(timer = l_timer_to_tab(t_id))) return;

	if ((long)funcptr != LM_TIMER_UNSET)
		timer->funcptr = funcptr;
	if (check_retry_status != LM_TIMER_UNSET)
		timer->check_retry_status = check_retry_status;
	if (interval != LM_TIMER_UNSET)
		timer->interval = interval;
	if (timer_type != LM_TIMER_UNSET)
		timer->timer_type = timer_type;

	if (timer->timer_type == LM_REAL_TIMER)
		l_timer_now(&timer->next_time);

	timet_plus_msecs(&timer->next_time, mseconds_till_next_call);
	if (!check_for_user_timers(job))
		l_timer_reset(job, timer->timer_type, NULL);
}

/*
 *	l_sleep_cb -- the callback used by lm_sleep()
 */
static
void
l_sleep_cb(i)
int i;
{
#ifdef DEBUG_TIMERS
	void debug_funcptr();
#endif
	DEBUG(
		printf("sleep_cb: now %g\n",
				timet2d(l_timer_now((LM_TIME_TYPE *)0)))
	);
	got_sleep_cb = 1;
	return;
}

#if !defined(WINNT) && !defined(NLM) && !defined(OS2)
/*
 *	lc_sleep -- like sleep -- for seconds
 *	lc_nap -- lm_sleep for milliseconds
 */

int API_ENTRY
lc_nap(job, milliseconds)
LM_HANDLE *job;		/* Current license job */
int milliseconds;
{
	LM_TIME_TYPE starttime;

/*
 *	check if application alarms are supported
 */
#ifndef DEBUG_TIMERS
	if (l_getattr(job, FULL_FLEXLM) != FULL_FLEXLM_VAL)
	{
		LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 105, 0);
		return 0;
	}
#endif

	l_timer_now(&starttime);

	DEBUG(printf("sleep:  now %g\n", timet2d(&starttime)));
/*
 *	set timer using lc_alarm with dummy callback l_sleep_cb
 */
	l_timer_add(job, LM_REAL_TIMER, 0, (FP_PTRARG)l_sleep_cb,
			LM_TIMER_SLEEP, milliseconds);
/*
 *	pause until we get the right signal
 */
	got_sleep_cb = 0;
	got_sigalarm = 1;
	while (got_sleep_cb == 0 && got_sigalarm == 1)
	{
		l_pause(job);
	}


	return (milliseconds -  (timet_diff(l_timer_now((LM_TIME_TYPE *)0),
						&starttime)));
}

int API_ENTRY
lc_sleep(job, seconds)
LM_HANDLE *job;		/* Current license job */
int seconds;
{
	return (lc_nap(job, seconds*1000)/1000);
}

/*
 *	l_pause -- calls pause()
 */
static
void
l_pause(job)
LM_HANDLE *job;
{
	DEBUG(printf("l_pause: now %g\n",
		timet2d(l_timer_now((LM_TIME_TYPE *)0))));
#ifndef VMS
	pause();
#else	/* VMS */
	sys$waitfr(L_TIMER_EF);
#endif	/* VMS */
}
#endif	 /* PC */

/*
 *	l_timer_set() - Reset the timer to the specified interval
 *			calls setitimer
 */



static
void
l_timer_set(job, timer_type, msecs, save_interval)
LM_HANDLE *job;		/* Current license job */
int timer_type;
int msecs;
struct itimerval *save_interval;
{
#ifndef PC
  struct itimerval timeout;
  int t;
  int (*func ) () = (int(*)())0;
  extern setitimer();
#endif

	DEBUG(
		printf("setitimer %d msecs, now %g\n", msecs,
			timet2d(l_timer_now((LM_TIME_TYPE *)0)));

		if ( save_interval )
		{
		    printf( "l_timer_set() called with save_interval!=0.\n");
		    printf( "Such calling sequence is not supported on NT.\n");
		}
	);

#ifdef PC
	/*-
 	 *	Instead of using SetTimer(), we could optionally use
	 *	timeSetEvent() which is a real async timer on NT.
	 *	However, using timeSetEvent() will make our software
	 *	runs only on NT but not on WIN32s!
	 */
	active_timer = SetTimer( 0, 0, msecs, (TIMERPROC)timer_proc );
#else
	timeout.it_interval.tv_sec = 0;
	timeout.it_interval.tv_usec = 0;
	timeout.it_value.tv_sec = msecs/1000;
	timeout.it_value.tv_usec = (msecs%1000) * 1000;

	t = (timer_type == LM_REAL_TIMER ? ITIMER_REAL : ITIMER_VIRTUAL);
	func = (int(*)())job->options->setitimer;
	if (!func) func = (int(*)())setitimer;

#ifdef VMS
	(*func)(job, t, &timeout, save_interval);
#else
	(*func)(t, &timeout, save_interval);
#endif
#endif /* PC */

}

/*
 *	l_timer_now -- if tp is non-zero, fill it in with current time.
 *			return pointer to local copy of time.  This
 *			pointer is VOLATILE.
 *
 */
static
LM_TIME_TYPE *
l_timer_now(tp)
LM_TIME_TYPE *tp;
{
	static LM_TIME_TYPE ret;
	struct timeval tv;
	static time_t basetime = 0;

    l_gettimeofday(&tv, 0);
	if (!basetime) basetime = tv.tv_sec;
	tv.tv_sec -= basetime;
	ret.seconds = tv.tv_sec;
	ret.milliseconds = tv.tv_usec/1000;
	if (tp)
	{
		memcpy((char *)tp, (char *)&ret, sizeof(LM_TIME_TYPE));
	}
	return &ret;
}

/*
 *	l_block_sigalarm
 */
static
void
l_block_sigalarm(i)
int i;
{
#if defined(VMS)
	/* We need a sigblock equivalent for VMS, but this
	 * is a low priority for now
	 */
#else
#ifdef USE_SIGBLOCK
  static unsigned int mask = 0;
  unsigned int blockalarm = 1<<(SIGALRM-1);

	if (i)
		mask = sigblock(blockalarm);
	else
	{
		mask = mask & ~blockalarm;
		sigsetmask(mask);
	}
#else /* USE_SIGBLOCK */
#ifdef PC
	if ( active_timer )
	{
		KillTimer( NULL, active_timer );
		active_timer = 0;
	}
#else
  static sigset_t mask;
	int set = 0;
	if (!set)
	{
		set = 1;
		sigemptyset(&mask);
		sigaddset(&mask, SIGALRM);
	}
	if (i)
		sigprocmask(SIG_BLOCK, &mask, (sigset_t *)NULL);
	else
		sigprocmask(SIG_UNBLOCK, &mask, (sigset_t *)NULL);
#endif /* PC */
#endif /* USE_SIGBLOCK */
#endif /* VMS */
}

/*
 *	l_timer_signal -- wrapper for sigaction(SIGALRM, new)
 *			returns old signal
 */

SIGFUNCP
l_timer_signal(job, sig, new)
LM_HANDLE *job;
int sig;
SIGFUNCP new;
{
#ifndef UNIX
	return (SIGFUNCP)0;
#else

  SIGFUNCP ret;
  void (*(*func)())() = (void(*(*)())())0;
#ifndef NO_SIGACTION
  struct sigaction new_sa;
  struct sigaction old_sa;

	DEBUG(printf ("l_timer_signal: sig %d func 0x%x\n", sig, new));
	memset(&new_sa, 0, sizeof(new_sa));
	memset(&old_sa, 0, sizeof(old_sa));
	new_sa.sa_handler = new;
#ifdef SA_RESTART
	if (sig == SIGALRM &&
		(!(job->flags & (LM_FLAG_IS_VD | LM_FLAG_LMGRD |
						LM_FLAG_IN_CONNECT))))
			new_sa.sa_flags = SA_RESTART;
#endif
#endif /* NO_SIGACTION */

	func = job->options->sighandler;

#ifdef NO_SIGACTION
	if (!func) func = signal;
#endif /* NO_SIGACTION */

	if (func)
	{
		ret = (*func)(sig, new);
	}
#ifndef NO_SIGACTION
	else
	{
		sigaction(sig, &new_sa, &old_sa);
		ret = old_sa.sa_handler;
	}
#endif /* NO_SIGACTION */
        return ret;
#endif /* UNIX */
}

/*
 *	timet_plus_msecs -- add msecs to tp
 */
static
void
timet_plus_msecs(tp, msecs)
LM_TIME_TYPE *tp;
int msecs;
{
	tp->milliseconds += msecs%1000;
	tp->seconds += tp->milliseconds/1000 + msecs/1000;
	tp->milliseconds %= 1000;
}

/*
 *	timet_diff -- return the difference, in milliseconds
 *				between two times -- subtract the second
 *				from the first.
 */
static
int
timet_diff(tp1, tp2)
LM_TIME_TYPE *tp1, *tp2;
{
	int ret;
	LM_TIME_TYPE t;
	memcpy((char *)&t, (char *)tp1, sizeof(t));
	ret = t.milliseconds - tp2->milliseconds;
	if (ret <0)  /*borrow*/
	{
		t.seconds --;
		ret += 1000;
	}
	ret += (t.seconds - tp2->seconds) * 1000;
	return ret;
}

/*
 *	l_timer_msecs2time - return a volatile pointer to a LM_TIME_TYPE
 *			     struct
 */
static
LM_TIME_TYPE *
l_timer_msecs2time( msecs)
int msecs;
{
	static LM_TIME_TYPE t;
	t.seconds = msecs/1000;
	t.milliseconds = msecs%1000;
	return &t;
}

/*
 *	l_timer_time2msecs - convert time to msecs -- dangerous -- may
 *				not fit!!
 */
static
unsigned long
l_timer_time2msecs( tp)
LM_TIME_TYPE *tp;
{
	unsigned long msecs;
	msecs = tp->milliseconds + (tp->seconds * 1000);
	return msecs;
}


/*
 *	timet_compare -- like strcmp()
 */
static
int
timet_compare(tp1, tp2)
LM_TIME_TYPE *tp1, *tp2;
{
	if (tp1->seconds != tp2->seconds) return (tp1->seconds - tp2->seconds);
	return (tp1->milliseconds - tp2->milliseconds);
}

#ifndef RELEASE_VERSION
/*
 *	timet2d -- convert LM_TIME_TYPE * to a double
 */
static
double
timet2d(tp)
LM_TIME_TYPE *tp;
{
	double ret = 0;
	if (!tp) return ret; /*safety precaution*/
	ret = tp->seconds + (double)((double)tp->milliseconds/1000.0);
	return ret;
}
#endif
#ifndef RELEASE_VERSION
static void dumpit()
{
	TIMER_TAB *t;
	puts("dump---------------------------");
	for (t = first_real_timer; t;t=t->next)
	{
		printf("interval %d ", t->interval);
		printf("id %d ", t->id);
		printf("next_time %g ", timet2d(&t->next_time));
		printf("funcptr 0x%x ", t->funcptr);
		printf("%d(%s) ", t->check_retry_status,
			t->check_retry_status == LM_TIMER_CHECK ? "CHECK" :
			t->check_retry_status == LM_TIMER_RETRY ? "RETRY" :
			t->check_retry_status == LM_TIMER_USER_FUNC ? "USER_FUNC" :
			t->check_retry_status == LM_TIMER_CONNECT ? "CONNECT" :
			t->check_retry_status == LM_TIMER_LMGRD ? "LMGRD" :
			t->check_retry_status == LM_TIMER_VDAEMON ? "VDAEMON" :
			t->check_retry_status == LM_TIMER_USER_CAUGHT ? "USER_CAUGHT" :
			t->check_retry_status == LM_TIMER_SLEEP ? "SLEEP" :
			t->check_retry_status == LM_TIMER_UNSET ? "UNSET" :
			"?");
		if (t->block) printf(" block ");
		if (t == next_real_timer) printf(" next");
		puts("");
	}
	puts("enddump---------------------------");

}
#endif /* RELEASE_VERSION */
#else /* NO_TIMERS */

/****************************************************************************
 *
 *	dummy l_timer_* and lc_alarm routines
 *
 ****************************************************************************/

LM_TIMER API_ENTRY
lc_alarm(job, funcptr, repeat_interval, mseconds_till_next_call)
LM_HANDLE *job;		/* Current license job */
void (*funcptr) lm_args((int));
int repeat_interval;
int mseconds_till_next_call;
{
	return (l_timer_add(job, LM_REAL_TIMER, repeat_interval,(FP_PTRARG)
			funcptr, LM_TIMER_USER_FUNC, mseconds_till_next_call));

}

LM_TIMER
API_ENTRY
l_timer_add(job, timer_type, interval, funcptr, check_retry_status,
						mseconds_till_next_call)
LM_HANDLE *job;		/* Current license job */
int timer_type; /*REAL or VIRTUAL*/
int interval; /*if -1, a no-op -- nothing done*/
FP_PTRARG funcptr;
int check_retry_status; /*indicates who's using this signal*/
int mseconds_till_next_call;	/*  if 0, it's a no-op -- never gets called,
				 *	and UNSET is set instead */
{
	return((LM_TIMER) 1);	/* pretend it's a real handle */
}

API_ENTRY
lc_disalarm(job, timer)
LM_HANDLE *job;		/* Current license job */
LM_TIMER timer;
{
	return l_timer_delete(job, timer);
}

l_timer_delete(job, timer)
LM_HANDLE *job;
LM_TIMER timer;
{
	return 0;
}

API_ENTRY
lc_nap(job, seconds)
LM_HANDLE *job;
int seconds;
{
        return 0;
}

API_ENTRY
lc_sleep(job, seconds)
LM_HANDLE *job;
int seconds;
{
	return 0;
}

void
l_timer_change(job, t_id, timer_type, interval, funcptr, check_retry_status,
				mseconds_till_next_call)
LM_HANDLE *job;		/* Current license job */
LM_TIMER t_id;
int 	timer_type;
int 	interval;
FP_PTRARG funcptr;
int	check_retry_status;
int	mseconds_till_next_call;
{
	/*return 0;*/
}

/*
 *	l_timer_job_done -- turn off all timers for the job
 *			    needed by lm_free_job()
 */

void l_timer_job_done(job)
LM_HANDLE *job;
{
}

#endif /* NO_TIMERS */

#ifdef VMS
int
setitimer(job, which, new, old)
LM_HANDLE *job;
int which;
struct itimerval *new;
struct itimerval *old;
{
  int interval[2];
  int status, ef;

	ef = L_TIMER_EF; /* ef_5 reserved for l_timers.c */

	if (old) memset(old, 0, sizeof(struct itimerval)); /* not used */

/*
 *	Only use the it_value member of struct for l_timers.c
 */
	interval[0] = -10000000 * new->it_value.tv_sec; /* -ten million */
	interval[0] += -10 * new->it_value.tv_usec;
	interval[1] = -1;
	if (!interval[0]) return 0;

	if (!(sys$cantim(SIGALRM, 0) & 1))
	{
		LM_SET_ERRNO(job, LM_VMS_SETIMR_FAILED, 107, 0);
		return -1;
	}
	if (!(sys$clref(ef) & 1)) /* Clear the event flag */
	{
		LM_SET_ERRNO(job, LM_VMS_SETIMR_FAILED, 106, 0);
		return -1;
	}
/*
 *	Assume that the function to be called is l_timer_router
 */
	status = sys$setimr(ef, interval, l_timer_router, SIGALRM);
	if ((status & 0x1) == 0)
	{
		LM_SET_ERRNO(job, LM_VMS_SETIMR_FAILED, 108, 0);
		return -1;
	}
	return 0;
}
#endif /* VMS */




/*******************END OF CODE -- FOLLOWING IS FOR DEBUGGING ONLY**********/

#ifdef DEBUG_TIMERS
#include "lm_code.h"
#include <stdio.h>

FILE *ofp;
LM_HANDLE *lm_job;
LM_DATA;

LM_HANDLE *job;
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, VENDOR_KEY2,
	VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
#if defined (SGI) || defined (VMS)
#define CLOSE 1050
#else
#define CLOSE 250
#endif /* SGI */

LM_TIME_TYPE start_time;
int print_errors = 1;
int error_cnt = 0;

time_equal(t1, t2)
{
	int i;
	i  = t1 - t2;
	if (i < CLOSE && i > (-1 * CLOSE)) return 1;
	return 0;
}

void
debug_funcptr(which, i, t_first_time, t_each_time, line)
{
#define MAX_HANDLERS 40
	static int save_line[MAX_HANDLERS];
	static int last_time[MAX_HANDLERS];
	static int first_time[MAX_HANDLERS];
	static int each_time[MAX_HANDLERS];
	static int first[MAX_HANDLERS];
	fprintf(ofp, ".");
	fflush(stdout);

	if (i==-1)  {
		save_line[which] = line;
		first_time[which] = t_first_time;
		each_time[which] = t_each_time;
		first[which] = 1;
		last_time[which] = now();
		return;
	}
	if (first[which])
	{
		if (!time_equal(now() - last_time[which], first_time[which]))
		{
			if (print_errors)
			{
			   fprintf(ofp, "\nERROR: first time h%d should be %d, is %d line %d\n\n",
				which,
				first_time[which], now() - last_time[which],
				save_line[which]);
			}
			else
			{
				error_cnt++;
			}
		}
		first[which] = 0;
	}
	else
	{
		if (!time_equal((now() - last_time[which]) , each_time[which]))
			if (print_errors)
			{

			   fprintf(ofp, "\nERROR: interval h%d should be %d, is %d, line %d\n\n",
				which,
				each_time[which], now() - last_time[which],
				save_line[which]);
			}
			else
			{
				error_cnt++;
			}

	}
	last_time[which] = now();
}
void h1_funcptr(i) { debug_funcptr(1, i); }
void h2_funcptr(i) { debug_funcptr(2, i); }
void h3_funcptr(i) { debug_funcptr(3, i); }
void h4_funcptr(i) { debug_funcptr(4, i); }
void h5_funcptr(i) { debug_funcptr(5, i); }

/*
 * 	h8_funcptr does a simulated sleep with select()
 */
void h8_funcptr(i)
{
	struct timeval t;
	int x;
	LM_TIME_TYPE tt;

	debug_funcptr(8, i);
	memset((char *) &t, sizeof(t), 0);
	l_timer_now(&tt);
	x = tt.seconds;
	while (1)
	{
		l_timer_now(&tt);
		t.tv_sec = (x - tt.seconds) + 4;
		if (t.tv_sec <= 0) break;
		l_select(0,(int *)0,(int *)0,
				(int *)0,&t);
	}

}
void h9_funcptr(i) { debug_funcptr(9, i); }
/*
 *	h10_funcptr calls lm_sleep()
 */
void
h10_funcptr(i)
{
	int x, sleep_ret;


	debug_funcptr(10, i);
	x = now();
	sleep_ret = 4;
	lm_sleep(4);
	if (!time_equal((now() - x) , 4000) )
	{
		if (print_errors)
			fprintf(ofp, "\nERROR sleep didn't last 4, only %d\n\n",
							now() - x);
		else
			error_cnt ++;
	}
}
void
h11_funcptr(i)
{
	debug_funcptr(11, i);
	debug_funcptr(7,-1,3,0,__LINE__);
	lm_sleep(3);
}


void userfunc(i)
{
	debug_funcptr(6, i);
}
now()
{
	return(timet_diff(l_timer_now((LM_TIME_TYPE *)0), &start_time));
}

#if _0

main(argc, argv)
int argc;
char **argv;
{


  int i, rc;
  LM_HANDLE *h1, *h2, *h3;
  LM_TIMER t1, t2, t3, t4, t5, t8, t9, t10, t11;
#ifndef VMS
  extern int setitimer();
#endif


	ofp = stdout;
	argv++;
	argc--;
	for (; argc > 0; argc--, argv++)
	{
		if (!strcmp(*argv, "-o"))
		{
			argc--;
			argv++;
			if (!(ofp = fopen(*argv, "w")))
			{
				fprintf(stderr, "%s: ", *argv);
				perror("Can't open for writing");
				exit(1);
			}

		}
	}
	lm_job = (LM_HANDLE *)malloc(sizeof (LM_HANDLE));
	lm_job->options = (LM_OPTIONS *)malloc(sizeof (LM_OPTIONS));
#ifdef VMS
	L_TIMER_EF = 50;
#else
	lm_job->options->sighandler = signal;
#endif
	lm_job->options->setitimer = (LM_PFV)setitimer;

	NOTRELEASE (if ((int)getenv("DEBUG_TIMERS")) debug_timers = 1);
	fprintf(ofp, "--------------------------------------------------------\n");
	fprintf(ofp, "                      TIMERS TEST\n");


/*
 *	Here's how this works:
 *
 *	call debug_funcptr(function number, -1, first call, interval, __LINE__)
 *	the system is then prepared to print errors when it doesn't
 *	occur at the appointed times.
 */


	l_timer_now(&start_time); /*start time is used by now() function*/

/*
 *	userfunc is function number 6
 *	This is called with signal/alarm -- this is ok for users ONLY
 *	if it happens before the first l_timer_*() function.
 */
	fprintf(ofp, "[group 1]");
#ifndef VMS
	debug_funcptr(6, -1, 5000, 0, __LINE__);
	signal(SIGALRM, userfunc);
	alarm(5);
#endif	/* VMS */

/*
 *	l_sleep_cb is function number 7
 */
	debug_funcptr(7, -1, now() + 1000,0,__LINE__);
	lm_sleep(1);

	debug_funcptr(7, -1, 10000,0,__LINE__);
	lm_sleep(10);

/*
 *	h1_funcptr is function number 1, and h<n> is function #n.
 */
	debug_funcptr(1, -1, 10000, 10000, __LINE__);
	if (!(t1 = l_timer_add(lm_job, LM_REAL_TIMER, 10000,
			(FP_PTRARG)h1_funcptr, LM_TIMER_VDAEMON, 10000)))
		fprintf(ofp, "t1 failed");

	debug_funcptr(2, -1, 6000 , 3000, __LINE__);
	if (!(t2 = l_timer_add(lm_job, LM_REAL_TIMER, 3000,
			(FP_PTRARG)h2_funcptr, LM_TIMER_LMGRD, 6000)))
		fprintf(ofp, "t2 failed");

/*
 *	NOTE:  h3_funcptr never gets called, because the 0 seconds
 * 	till next call effectively disables it.  That's because
 * 	l_timer_add is supposed to work similar to setitimer
 */
	debug_funcptr(3, -1, 0, 0, __LINE__);
	if ((t3 = l_timer_add(lm_job, LM_REAL_TIMER, 12000, (FP_PTRARG)h3_funcptr, 0, 0)))
		fputs("t3 should have failed", ofp);

	debug_funcptr(4, -1, 2000, 8000, __LINE__);
	if (!(t4 = lc_alarm(lm_job, h4_funcptr, 8000, 2000)))
		fputs("t4 failed", ofp);

	debug_funcptr(5, -1, 1000, 8000, __LINE__);
	if (!(t5 = lc_alarm(lm_job, h5_funcptr, 8000, 1000)))
		fputs("t5 failed", ofp);

	for(i=0;i<10;i++) {l_pause(lm_job); }/*let 5 alarms go off*/

	debug_funcptr(7, -1, 1000,0,__LINE__);
	lm_sleep(1);

	debug_funcptr(1, -1, 0, 0, __LINE__);
	if (l_timer_delete(lm_job, t1)<0) fprintf(ofp, "delete_timer failed");

	debug_funcptr(7, -1, 1000,0,__LINE__);
	lm_sleep(1);

	debug_funcptr(4, -1, 0, 0, __LINE__);
	if (lc_disalarm(lm_job, t4)<0) fprintf(ofp, "delete_timer failed");

	debug_funcptr(7, -1, 1000,0,__LINE__);
	lm_sleep(1);

	for(i=0;i<5;i++) l_pause(lm_job); /*let five alarms go off*/

	debug_funcptr(7, -1, 1000,0,__LINE__);
	lm_sleep(1);

	debug_funcptr(2, -1, 0, 0, __LINE__);
	if (l_timer_delete(lm_job, t2)<0) fprintf(ofp, "delete_timer failed");

	debug_funcptr(7, -1, 1000,0,__LINE__);
	lm_sleep(1);

	debug_funcptr(7, -1, 1000,0,__LINE__);
	lm_sleep(1);

	debug_funcptr(5, -1, 0, 0, __LINE__);
	if (lc_disalarm(lm_job, t5) < 0) fprintf(ofp, "delete_timer failed");

	debug_funcptr(2, -1,6000 , 3000, __LINE__);
	if (!(t2 = l_timer_add(lm_job, LM_REAL_TIMER, 3000, (FP_PTRARG)h2_funcptr,
						LM_TIMER_CONNECT, 6000)))

	for(i=0;i<4;i++) {l_pause(lm_job);} /*4 alarms go off*/

	fputs("", ofp);
	fprintf(ofp, "done.\n");
	fprintf(ofp, "--------------------------------------------------------\n");
	exit(1); /* for now */
#ifdef VMS
	exit(1);
#else
	lc_disalarm(lm_job, t2);

	fprintf(ofp, "[group 2]");

	print_errors = 1;
	debug_funcptr(2, -1, 2000,2000,__LINE__);
	t2 = lc_alarm(lm_job, h2_funcptr, 2000,2000);

	/* h8_funcptr does a select until 4 seconds pass*/
	debug_funcptr(8, -1, 1000,8000,__LINE__);
	t8 = lc_alarm(lm_job, h8_funcptr, 8000, 1000);

	debug_funcptr(9, -1, 3000,0,__LINE__);
	t9 = lc_alarm(lm_job, h9_funcptr, 0, 3000);

	for(i=0;i<4;i++) {l_pause(lm_job);} /*4 alarms go off*/

	lc_disalarm(lm_job, t8);
	lc_disalarm(lm_job, t2);
	lc_disalarm(lm_job, t9);

	fprintf(ofp, "\n[group 3]");
	print_errors = 1;

/*	same with sleep instead of select */
/*
 *	This time there's no errors.  That's because lm_sleep ends up
 *	calling l_timer_reset, and this then sets the next timer which
 *	is not necessarily the sleep_cb.
 */
	debug_funcptr(2, -1, 2000,2000,__LINE__);
	t2 = lc_alarm(lm_job, h2_funcptr, 2000,2000);

	/* h10_funcptr lm_sleeps until 4 seconds pass*/
	debug_funcptr(10, -1, 1000,8000,__LINE__);
	t10 = lc_alarm(lm_job, h10_funcptr, 8000, 1000);

	debug_funcptr(9, -1, 3000,0,__LINE__);
	t9 = lc_alarm(lm_job, h9_funcptr, 0, 3000);

	for(i=0;i<5;i++) {l_pause(lm_job); } /*5 alarms go off*/

/*
 *	Make sure we a timer won't call itself indefinitely
 *	if timer takes x seconds to run, but has an interval
 *	<x seconds, we don't want to get in an infinite loop.
 */
	lc_disalarm(lm_job, t2);
	lc_disalarm(lm_job, t10);
	lc_disalarm(lm_job, t9);
	fprintf(ofp, "\n[group 4]");

	print_errors = 0; /*expect 4 errors*/
	error_cnt = 0;
	debug_funcptr(11, -1, 1000,1000,__LINE__);
	/*h11 takes 3 seconds t run*/
	t11 = lc_alarm(lm_job, h11_funcptr, 1000, 1000);

	for(i=0;i<5;i++) {l_pause(lm_job); } /*5 alarms go off*/
	if (error_cnt != 4)
	{
		fprintf(ofp, "\nExpected 4 errors at end, got %d\n\n", error_cnt);
	}
	lc_disalarm(lm_job, t11);


	fprintf(ofp, "\n[group 5]"); /*test user timers*/
	print_errors = 0;
	error_cnt = 0;


	debug_funcptr(1, -1, 4000, 4000, __LINE__);
	if (!(t1 = l_timer_add(lm_job, LM_REAL_TIMER, 4000, (FP_PTRARG)h1_funcptr,
						LM_TIMER_USER_FUNC, 4000)))
		fprintf(ofp, "t1 failed");

	debug_funcptr(2, -1, 6000 , 3000, __LINE__);
	if (!(t2 = l_timer_add(lm_job, LM_REAL_TIMER, 3000, (FP_PTRARG)h2_funcptr,
							LM_TIMER_CHECK, 6000)))
		fprintf(ofp, "t2 failed");

	for(i=0;i<10;i++) {dbg_user_timer(); l_pause(lm_job); }
	if (error_cnt != 19)
	{
		fprintf(ofp, "\nExpected 19 errors at end, got %d\n\n", error_cnt);
	}


	fprintf(ofp, "done.\n");
	fprintf(ofp, "--------------------------------------------------------\n");

	exit(0);
#endif /* VMS */
}

#endif /* _0 define */


#ifndef VMS
#ifdef VOID_SIGNAL
void
#else
int
#endif /* VOID_SIGNAL*/
(*savfunc) lm_args((int));
struct itimerval remaining_time;
LM_TIME_TYPE savtime;
void
userfunc2(i)
{
	int d, q;
	signal(SIGALRM, savfunc);
	d = timet_diff(l_timer_now((LM_TIME_TYPE *)0), &savtime);
	remaining_time.it_value.tv_usec -= (d%1000) * 1000;
	if (remaining_time.it_value.tv_usec <0) {
		remaining_time.it_value.tv_sec--;
		remaining_time.it_value.tv_usec += 1000000;
	}
	remaining_time.it_value.tv_sec -= (d/1000);
	q = remaining_time.it_value.tv_sec;


	if ((int)remaining_time.it_value.tv_sec > 0 &&
				(int)remaining_time.it_value.tv_usec > 0)
		setitimer(ITIMER_REAL, &remaining_time, 0);
	else
		kill(getpid(), SIGALRM);
}
dbg_user_timer()
{
	struct itimerval i;
	memset((char *) &i, 0, sizeof(i));
	i.it_value.tv_sec = 5;
	l_timer_now(&savtime);
	setitimer(ITIMER_REAL, &i, &remaining_time);
	savfunc = signal(SIGALRM, userfunc2);
}
#endif /* VMS */
#ifdef getenv
#undef getenv
#endif
char *l_real_getenv(x) char *x;{ return getenv(x);}
char *l_getenv(x, y) LM_HANDLE *x; char *y;{ return getenv((char *)y);}

#endif /* DEBUG_TIMERS */
