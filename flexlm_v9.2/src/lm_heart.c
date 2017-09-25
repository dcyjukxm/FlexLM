/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_heart.c,v 1.6 2003/01/13 22:41:47 kmaclean Exp $
 *
 *	Function: 	lc_heartbeat
 *
 *	Description: 	heartbeat functions
 *
 *	Parameters:	(LM_HANDLE *)job
 *			(int *) num_reconnects
 *			(int)	minutes -- ret num reconnects in this many min.
 *
 *	Return:		> 0:current number of failed reconnects.
 *			0: success
 *			<0: lm_errno
 *
 *	D. Birns
 *	1/16/96
 *
 *	Last changed:  9/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#ifndef UNIX
#include <time.h>
#endif

int
API_ENTRY
lc_heartbeat(job, ret_num_reconnects, minutes)
LM_HANDLE *job;
int *ret_num_reconnects;
int minutes;
{
  int ret, i, cnt;
  long now = (long)time(0);
  int min;

/* 	
 *	alloc recent_reconnects array once per job
 */
	if (LM_API_ERR_CATCH) return job->lm_errno;

	if (!job->num_minutes && minutes)
	{
		job->num_minutes = minutes;
		min = job->num_minutes < 10 ? 10 : job->num_minutes;

		job->recent_reconnects = (long *)(l_malloc(job, 
			min * sizeof(long)));
	} 
	else min = job->num_minutes < 10 ? 10 : job->num_minutes;
	if (ret_num_reconnects) *ret_num_reconnects = 0; /* assume 0 */
	ret =  l_timer_heart(job);
	if (ret_num_reconnects && (minutes || job->num_minutes)  && 
					job->last_failed_reconnect && !ret)
	{
/* 
 *		server just came back 
 *		num_minutes serves more than one function:
 *			We count the number of reconnects since now - minutes
 *			num_minutes is ALSO the size of the recent_reconnects
 *			array
 */
		now -= job->num_minutes * 60;
		for (cnt = 0, i = 0; i < min; i++)
		{
			if (job->recent_reconnects[i] >= now)
				cnt++;
		}
		*ret_num_reconnects = cnt;
		job->last_failed_reconnect = 0;
	}
	LM_API_RETURN(int, ret)
}
