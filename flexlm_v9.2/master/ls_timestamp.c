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
 *	Module: $Id: ls_timestamp.c,v 1.5 2003/01/13 22:26:50 kmaclean Exp $
 *
 *	Function:	ls_timestamp() 
 *
 *	Description:	Timestamp the log file, when required.
 *
 *	Parameters:	None
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	10/26/88
 *
 *	Last changed:  10/23/96
 *
 */


#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsmaster.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "../machind/lsfeatur.h"
#include "../app/ls_aprot.h" 
#include "l_m_prot.h"

static int first = 1;		/* Initialization flag */
static long last_log_time;	/* Last time we timestamped the log file */
int _lm_timestamp_interval;	/* Interval to do timestamps on */

void
ls_timestamp()
{
  long curtime;
  int day, mon, yr;	
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif

#ifdef THREAD_SAFE_TIME
	l_get_date(&day, &mon, &yr, &curtime, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&day, &mon, &yr, &curtime);
#endif
	if (first)
	{
		first = 0;
		last_log_time = curtime;    /* Don't need to log @ startup */
	}
/*
 *	If it is time to timestamp the log, do it.
 */
	if (curtime - last_log_time >= _lm_timestamp_interval)
	{
		LOG((lmtext("TIMESTAMP %d/%d/%d\n"), mon+1, day, yr + 1900));
		LOG_INFO((INFORM, "lmgrd timestamps the log file at periodic \
			intervals."));
		last_log_time = curtime;    /* Update */
	}
}
