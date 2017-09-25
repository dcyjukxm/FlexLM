/******************************************************************************

	    COPYRIGHT (c) 1993, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_timers.h,v 1.4 2003/01/13 22:13:13 kmaclean Exp $
 *
 *	Description: Headers for using l_timer_* functions
 *
 *	D. Birns
 *	6/93
 *
 *	Last changed:  11/6/94
 *
 */

#ifndef _L_TIMERS_H
#define _L_TIMERS_H
#define LM_TIMER_CHECK		1 /* used to check the connection*/
#define LM_TIMER_RETRY		2 /* used to retry a failed connection*/
#define LM_TIMER_USER_FUNC 	3 /* set by the user */
#define LM_TIMER_CONNECT 	4 /* used by l_basic_conn */
#define LM_TIMER_LMGRD 		5 /* used by lmgrd */
#define LM_TIMER_VDAEMON	6 /* used by vendor daemon */
#define LM_TIMER_USER_CAUGHT	7 /* caught a user_routine not from lm_alarm*/
#define LM_TIMER_SLEEP		8 /* lm_sleep*/
#define LM_TIMER_UNSET 		-1234 /* if time to next call is 0, it
				       * is UNSET -- this is so that 
				       * lm_alarm can work like setitimer
				       */

#define LM_VIRTUAL_TIMER 5678 	/* not officially supported */

typedef void (*FP_INTARG) lm_args((int));
typedef void (*FP_PTRARG) lm_args((LM_HANDLE *));

LM_TIMER API_ENTRY l_timer_add(LM_HANDLE *, int, int, FP_PTRARG, int, int);
void 		l_timer_change 	(LM_HANDLE *, LM_TIMER, int, int, FP_PTRARG, 
						int, int);
int		l_timer_delete (LM_HANDLE *, LM_TIMER timer);
LM_TIMER API_ENTRY l_timer_mt_add(LM_HANDLE *, int,int,FP_PTRARG,int, int);
void 		l_timer_mt_change 	(LM_HANDLE *, LM_TIMER, int, int, FP_PTRARG, 
						int, int);
int		l_timer_mt_delete (LM_HANDLE *, LM_TIMER timer);
void		l_timer_job_done lm_args((LM_HANDLE *));
void 		l_mt_free(LM_HANDLE *);
typedef void (* SIGFUNCP) lm_args((int));
#ifdef SGI4
SIGFUNCP	l_timer_signal ();
#else
SIGFUNCP	l_timer_signal 		lm_args((LM_HANDLE *, int,  SIGFUNCP));
#endif
#endif /* _L_TIMERS_H */
