/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation Software Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation Software Inc and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation Software Inc.

/*****************************************************************************/
/*	
 *	Module:	lsv4_logtime.c v1.3.0.0
 *
 *	Function: ls_logtime(), ls_time_line()
 *
 *	v4 report.log compatibility package
 *	Description: Logs the time of day.
 *			ls_logtime() - Log time followed by \n
 *			ls_time_line() - Log time only
 *
 *	Parameters:	(struct tm *) t - the local time of day
 *
 *	Return:		None - time is logged
 *
 *	D. Birns
 *	4/5/96
 *
 *	Last changed:  4/5/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#if defined( sco) || defined(sinix)
#include <time.h>	
#endif

void
ls_logtime(t)
struct tm *t;
{
#ifndef LOG_TIME_AT_START

	_LOG((" (%d/%d/%d %d:%s%d:%s%d)\n",
		t->tm_mon+1, t->tm_mday, t->tm_year,
		t->tm_hour, t->tm_min < 10 ? "0" : "", t->tm_min,
		t->tm_sec < 10 ? "0" : "", t->tm_sec));

#else

	(void) printf("\n");
#endif

}

void
ls_time_line(t)
struct tm *t;
{
	_LOG((" (%d/%d/%d %d:%s%d:%s%d)",
		t->tm_mon+1, t->tm_mday, t->tm_year,
		t->tm_hour, t->tm_min < 10 ? "0" : "", t->tm_min,
		t->tm_sec < 10 ? "0" : "", t->tm_sec));
}

