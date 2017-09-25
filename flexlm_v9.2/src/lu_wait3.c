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
 *	Module: $Id: lu_wait3.c,v 1.5 2003/01/13 22:41:49 kmaclean Exp $
 *
 *	Function: lu_wait3()
 *
 *	Description: 4.2bsd functions that are "missing" on various
 *		     FLEXlm platforms.
 *
 *	Parameters/Returns:	See appropriate man pages (sunOS3)
 *
 *	M. Christiano
 *	1/17/89
 *
 *	Last changed:  10/23/96
 *
 */

#include "lmachdep.h"

/*
 *	Simulate wait3 using a wait and an interrupting signal.
 */

#ifdef NO_wait3
#undef wait3
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>


#if !defined(NO_sigvec) && defined(VMS)
static struct sigvec newvec;
#endif
static struct itimerval t;

/* ARGSUSED */
static 
#ifdef VOID_SIGNAL
void 
#endif
_t(i) 	
int i;
{ 
	(void) signal(SIGALRM, SIG_IGN); 
	t.it_value.tv_sec = 0;
	setitimer(ITIMER_PROF, &t, 0);
#ifndef VOID_SIGNAL
	return(0);
#endif
}

#if defined (VMS)
#define SV_INTERRUPT 0		/*- Sun says this is in system 5 !! */
#endif

#ifdef VMS
static
ls_wu(sec)
int sec;
{
#ifdef NO_sigvec
	(void) signal(SIGALRM, _t);
#else
	newvec.sv_handler = _t;
	newvec.sv_flags = SV_INTERRUPT;	/* Need to interrupt wait() on sys5 */
	newvec.sv_mask = 0;
	sigvec(SIGALRM, &newvec, 0);
#endif
	t.it_interval.tv_sec = t.it_interval.tv_usec = t.it_value.tv_usec = 0;
	t.it_value.tv_sec = sec;
	setitimer(ITIMER_PROF, &t, 0);
}
#endif /* VMS */
lu_wait3(status, x, y)
#ifdef VMS
union wait *status;
#else
int *status;
#endif
int x, y;
{
#ifdef VMS

	ls_wu(1);
	return(wait(status));

#else	/* VMS */

	return (waitpid(-1, status, WNOHANG));

#endif	/* !VMS */
}

#endif
