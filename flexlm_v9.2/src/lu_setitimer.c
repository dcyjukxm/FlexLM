/******************************************************************************

	    COPYRIGHT (c) 1991, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lu_setitimer.c,v 1.5 2003/01/14 21:46:33 kmaclean Exp $
 *
 *	Function: lu_setitimer() lu_getitimer()
 *
 *	Description: 4.2bsd setitimer() function 
 *
 *	Parameters/Returns:	See appropriate man pages (sunOS3)
 *				NOTE: This setitimer() tosses the
 *				      microseconds value in the time struct.
 *				NOTE: This setitimer always uses SIGALRM
 *
 *	M. Christiano
 *	1/17/89
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmachdep.h"
#ifdef PC
#include <sys/types.h>
#endif
#ifdef USE_SYS_TIMES_H
#include <sys/times.h>
#else
#include <sys/time.h>
#endif 

/*
 *	Simulate setitimer using alarm
 */

#ifdef NO_setitimer

lu_setitimer(type, val, oval)
int type;	/* Ignored */
#ifdef CRAY_NV1
const struct itimerval *val;
struct itimerval *oval;
#else	/* CRAY_NV1 */
struct itimerval *val, *oval;
#endif	/* CRAY_NV1 */
{
  int i;

/* 
 *	We have to distinguish three cases:
 *		1) times are exactly zero -- means turn off alarm
 *		2) times are small -- set alarm to 1 second
 *		3) times are near or > 1 -- call alarm for rounded seconds
 */
			
	if ((val->it_value.tv_sec ==0)&& (val->it_value.tv_usec ==0))
	{
		/* This is a special case, and means turn off alarm */
		alarm(0);
	} 
	else 
	{
		i = val->it_value.tv_sec;
		if (val->it_value.tv_usec > 500000) i++;
		if (i==0) i = 1; /* always wait at least 1 second */
		i = alarm((unsigned) i);
	}
	if (oval)
	{
		oval->it_value.tv_sec = (long) i;
		oval->it_value.tv_usec = (long) 0;
	}
	return(0);
}
lu_getitimer(type, val)
int type;	/* Ignored */
struct itimerval *val;
{
	int i;
	(void) memset(val, 0, sizeof(struct itimerval));
	if (!val) return -1;
	val->it_value.tv_sec = alarm(0);
	(void)alarm(val->it_value.tv_sec);
	return 0;
}

#endif
