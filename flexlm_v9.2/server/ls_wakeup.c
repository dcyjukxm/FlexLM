/******************************************************************************

	    COPYRIGHT (c) 1988, 2003  by Macrovision Corporation.
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
 *	Module: $Id: ls_wakeup.c,v 1.4 2003/01/13 22:31:39 kmaclean Exp $
 *
 *	Function: ls_wakeup(sec, wake)
 *
 *	Description: 	"Wakes" the process in "sec" seconds
 *
 *	Parameters:	(int) sec - The number of seconds before wakeup
 *			(int *) wake - Variable to be set when waked up
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	6/7/88
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmachdep.h"
#ifndef PC
#include <sys/time.h> 	/*for ITIMER_REAL*/
#endif
#include "lmclient.h"
#include "l_prot.h"
#include "l_timers.h"
static int *wakeup;	/* Flag when we got waked up */
extern LM_HANDLE *lm_job;

static
#ifdef VOID_SIGNAL
void 
#endif
_timer(sig)
{
	*wakeup = 1;
	return;
}

ls_wakeup(sec, wake)
int sec;
int *wake;
{
	wakeup = wake;			/* Save the address */
	*wakeup = 0;

	(void)l_timer_add(lm_job, LM_REAL_TIMER, 0, (FP_PTRARG)_timer, 
			LM_TIMER_VDAEMON, sec * 1000);
	return 0 ;
}
