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
 *	Module: $Id: l_validdate.c,v 1.4 2003/04/16 17:26:37 brolland Exp $
 *
 *	Function: l_validdate(job, date)
 *
 *	Description: Verifies that the date specified is valid.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) date - a trial date
 *
 *	Return:		(int) - 0 - OK, date if valid
 *				BADDATE - day month or year is invalid
 *				LONGGONE - date has expired
 *				DATE_TOOBIG - Year > 2027 (MAX_BINDATE_YEAR)
 *
 *	M. Christiano
 *	2/18/88	(from isvaliddate.c)
 *
 *	Last changed:  9/2/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

API_ENTRY
l_validdate(job, date)
LM_HANDLE *job;		/* Current license job */
char *date;
{
  int day, year, mon;
  char month[10];
  int leapyear;

	if (job->options && l_keyword_eq(job, date, LM_RESERVED_UNEXPIRING))
	{
		if (!job->options || !job->options->license_fmt_ver ||
		(strcmp(job->options->license_fmt_ver, LM_BEHAVIOR_V6) >= 0))
		return 0;
	}
	(void) sscanf(date, "%d-%[^-]-%d", &day, month, &year);	/* overrun threat */
	leapyear = (!(year % 4) && ((year % 100) || !(year%400)));
	mon = l_int_month(month);

	if ((day < 1 || day > 31) ||
	    (mon < 0 || mon > 11) ||
	    ((mon == 3 || mon == 5 || mon == 8 || mon == 10) && day > 30) || 
	    (mon == 1 && day > 29) ||
	    (mon == 1 && day > 28 && !leapyear) ||
	    (year < 0 || ((year > 99) && year < 1900)))
	{
		return(BADDATE);
	}
	if (year > MAX_BINDATE_YEAR)
	{
		return(DATE_TOOBIG);
	}
	if (l_date(job, date, L_DATE_EXPIRED)) 
	{
		return(LONGGONE);
	}
	return(0);
}
