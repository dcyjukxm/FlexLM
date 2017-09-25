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
 *	Module: $Id: ls_logtime.c,v 1.7 2003/01/13 22:31:38 kmaclean Exp $
 *
 *	Function: ls_logtime(), ls_time_line()
 *
 *	Description: Logs the time of day.
 *			ls_logtime() - Log time followed by \n
 *			ls_time_line() - Log time only
 *
 *	Parameters:	(struct tm *) t - the local time of day
 *
 *	Return:		None - time is logged
 *
 *	M. Christiano
 *	9/2/88
 *
 *	Last changed:  10/23/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include <time.h>	

void
ls_logtime(t)
struct tm *t;
{
#ifndef LOG_TIME_AT_START

	_LOG((" (%d/%d/%d %d:%s%d:%s%d)\n",
		t->tm_mon+1, t->tm_mday, t->tm_year + 1900,
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
		t->tm_mon+1, t->tm_mday, t->tm_year + 1900,
		t->tm_hour, t->tm_min < 10 ? "0" : "", t->tm_min,
		t->tm_sec < 10 ? "0" : "", t->tm_sec));
}

