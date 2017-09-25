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
 *	Module: $Id: ls_pause.c,v 1.5 2003/01/13 22:31:38 kmaclean Exp $
 *
 *	Function: ls_pause(msec)
 *
 *	Description: 	"Sleeps" for the specified number of millisecs.
 *
 *	Parameters:	(int) msec - The number of milliseconds to sleep
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	4/8/88
 *
 *	Last changed:  12/26/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <stdio.h>
#ifdef PC		
#include <pcsock.h>
#else
#include <sys/time.h>
#endif


void
ls_pause(msec)
int msec;
{
  struct timeval pause_time;
  int usec, sec;

	sec = msec/1000;
	usec = (msec % 1000) * 1000;
	pause_time.tv_sec = sec;
	pause_time.tv_usec = usec;
	(void) l_select(0, (int *) 0, (int *) 0, (int *) 0, &pause_time);

#if 0
	lm_nap(msecs); /*optional way of doing it*/
#endif
}
