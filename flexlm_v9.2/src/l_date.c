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
 *	Module: $Id: l_date.c,v 1.17 2003/04/15 23:48:17 brolland Exp $
 *
 *	Function: l_date(job, expdate)
 *		  l_get_date(d, m, y, tvsec)
 *		  l_bin_date(expdate)
 *		  l_extract_date(job, code)
 *		  l_asc_date(bin_date)
 *
 *	Description: l_get_date gets the date, 
 *		     l_date Verifies that the date is before "expdate".
 *		     l_bin_date encodes the date from "expdate", or the current
 *				date, if expdate is not specified.
 *		     l_extract_date extracts the encoded date from "code"
 *		     l_asc_date turns the return of l_bin_date into the
 *				ascii format used by the rest of the system.
 *
 *	Parameters:	expdate (char *) - The expiration date ("dd/mmm/yy")
 *			d, m, y (int *) - The current day, month, year.
 *			tvsec (long *) - The raw seconds field (timeval.tv_sec)
 *			(char *) code - License key
 *			(char *) bin_date - return from l_bin_date
 *
 *	Return:		(int) - 0 - OK, date is OK
 *				LONGGONE - Date has passed
 *
 *	M. Christiano
 *	2/25/88
 *
 *	Last changed:  11/20/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <sys/types.h>
#include <time.h>
#ifdef PC
#include <windows.h> 
#else
#ifndef VMS
#ifdef USE_SYS_TIMES_H
#include <sys/times.h>
#else
#include <sys/time.h>
#endif
#endif	/* VMS */
#endif /* PC */

#define FAR_FUTURE_DATE 0x7fffffff

static const char * const months[] = { 
			"jan", "feb", "mar", "apr", "may", "jun", "jul", 
			"aug", "sep", "oct", "nov", "dec" , ""};
static int leap_year 	lm_args((int));
static int l_mktime 	lm_args(( char *));
static int expired 	lm_args((int, int, int, int, int, int));

const
char *
API_ENTRY
l_month_int(int m)
{
	if ((m < 12) && (m >= 0)) return months[m];
	else return "Invalid";
}



/*
 *	l_date -- returns 0 == success, <>0 == lm_errno;
 */

API_ENTRY
l_date(job, date, what)
LM_HANDLE *job;		/* Current license job */
char *date;	/* The expiration date */
int what;
{
  int day, year, mon;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif

  char month[10];
  int d, m, y;	/* Current date */
  long tvsec;

	(void) sscanf(date, "%d-%[^-]-%d", &day, month, &year);	/* overrun threat */
	if (year == 0) return(0);	/* year = 0 -> NO Expiration */
	if (year >= 1900) year -= 1900;	/* Account for 1991 vs 91 in file */
#ifdef THREAD_SAFE_TIME
	l_get_date(&d, &m, &y, &tvsec, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&d, &m, &y, &tvsec);
#endif
	mon = l_int_month(month);
	if (mon == -1) 
	{
		LM_SET_ERROR(job, LM_BADDATE, 31, 0, date, LM_ERRMASK_ALL);
		return LM_BADDATE;
	}
	if ((what == L_DATE_EXPIRED) &&  expired(y, m, d, year, mon, day))
	{
		LM_SET_ERROR(job, LM_LONGGONE, 32, 0, date, LM_ERRMASK_ALL);
		return LM_LONGGONE;
	}
	else if (what == L_DATE_START_OK && expired(year, mon, day, y, m, d)) 
	{
		LM_SET_ERROR(job, LM_TOOEARLY, 392, 0, date, LM_ERRMASK_ALL);
		return LM_TOOEARLY;
	}
	return 0;
}
/*
 *	expired -- actually can test either way -- reverse the
 *	order if needed
 */
static
int
expired(y, m, d, year, mon, day)
int y;
int m;
int d;
int year;
int mon;
int day;
{
  int cur, exp;
/*
 *	This algorithm is only good for determining
 *	if the date has passed or not
 *	372 and 31 or numbers that happen to work because they're
 *	>= the largest number of days in a year/month
 */
	cur = (y * 372) + (m * 31) + d;
	exp = (year * 372) + (mon * 31) + day;
	if (cur <= exp) return 0;
	return LM_LONGGONE;
}

/*
 *	lc_expire_days -- takes conf as arg.
 *	returns:	> 0 -- number of days till expiration
 *			0   -- it will expire today
 *			< 0 -- _lm_errno (LONGGONE, BADPARAM)
 */			
long
API_ENTRY
lc_expire_days(job, conf)
LM_HANDLE *job;
CONFIG *conf;
{
   time_t now, then;
#ifdef PC16
	long days;
#else
   int days;
#endif
	if (LM_API_ERR_CATCH) return job->lm_errno;

	if (!conf)
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 252, 0);
		LM_API_RETURN (long, LM_BADPARAM)
	}
	now = time(0);
	then = l_date_to_time(job, conf->date);
	if (then == FAR_FUTURE_DATE) LM_API_RETURN( long, LM_FOREVER)
	days = (then - now )/* /86400 P??? */;
	if (days < 0)
	{
		LM_SET_ERRNO(job, LM_LONGGONE, 253, 0);
		LM_API_RETURN( long, LM_LONGGONE)
	}
	else
		LM_API_RETURN( long, days/86400)
		
}

/*
 *	l_date_to_time
 *	use mktime where possible.  When not, use l_mktime, which
 *	is not as good, because it doesn't understand timezones.
 */
long API_ENTRY 
l_date_to_time(job, datestr)
LM_HANDLE *job;
char *datestr;
{
#ifdef NO_MKTIME
	return l_mktime(datestr);
#else
   int cd, cy = 1, cm;
   struct tm conftm;
   char month[15];

	(void) sscanf(datestr, "%d-%[^-]-%d", &cd, month, &cy);	/* overrun threat */
	cm = l_int_month(month);
	if (!cy || cy > 2038) return FAR_FUTURE_DATE; /* date way in the future */
	memset(&conftm, 0, sizeof(conftm));
	conftm.tm_sec = 59; /* just before midnight */
	conftm.tm_min = 59;
	conftm.tm_hour = 23;
	conftm.tm_mday = cd;
	conftm.tm_mon = cm;
	conftm.tm_isdst = -1; /* P5614 */
	if (cy >= 1900) cy -= 1900;	/* Account for 1991 vs 91 in file */
	conftm.tm_year = cy;
#if (defined(SUNOS4) || defined(TRUE_BSD)) && !defined(MIPS)
	return (long) timelocal(&conftm);
#else
	return (long) mktime(&conftm);
#endif
#endif  /* NO_MKTIME */
}

/*
 *	l_get_date:  WARNING:  After 2000, the year is 100 + n!
 *			Often, we should add 1900 to the result.
 *			A problem occurs when comparing results to
 *			Other C calls with the same year convention
 *			(Like stat(), I think).
 */
void API_ENTRY
#ifdef THREAD_SAFE_TIME
l_get_date(day, month, year, tvsec, ptst)
#else /* !THREAD_SAFE_TIME */
l_get_date(day, month, year, tvsec)
#endif
int *day;
int *month;
int *year;
long *tvsec;
#ifdef THREAD_SAFE_TIME
struct tm * ptst;
#endif 
{

	struct tm *t;
	time_t x;

	x = *tvsec = (long) time(0);
#ifdef THREAD_SAFE_TIME
	localtime_r(&x, ptst);
	t = ptst;
#else /* !THREAD_SAFE_TIME */
	t = localtime(&x);
#endif
	*day = t->tm_mday;
	*month = t->tm_mon;
	*year = t->tm_year;
}

static char bin_date[5];
static char *hex = "0123456789ABCDEF";


char * API_ENTRY
l_bin_date(date)
char *date;
{
  int d, m, y, i, j;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  char month[10];
  long junk;
  
	if (date)
	{
		(void) sscanf(date, "%d-%[^-]-%d", &d, month, &y);	/* overrun threat */
		m = l_int_month(month);
	}
	else
	{
#ifdef THREAD_SAFE_TIME
		l_get_date(&d, &m, &y, &junk, &tst);
#else /* !THREAD_SAFE_TIME */
		l_get_date(&d, &m, &y, &junk);
#endif
	}
	if (y >= 1900) y -= 1900;	/* Account for 1991 vs 91 in file */
	i = d + (m << 5) + (y << 9);
	for (j = 3; j >= 0; j--)
	{
		bin_date[j] = hex[i & 0x0f];
		i >>= 4;
	}
	bin_date[4] = '\0';
	return(bin_date);
}

char * API_ENTRY
l_extract_date(job, code)
LM_HANDLE *job;
char *code;
{
	if (strlen(code) == MAX_CRYPT_LEN && !strchr(code, ' ')) 
	{
		bin_date[0] = code[1];
		bin_date[1] = code[3];
		bin_date[2] = code[5];
		bin_date[3] = code[7];
		return(bin_date);
	}
	else if (job->options->flags & LM_OPTFLAG_LKEY_START_DATE)
		return l_bin_date(0); /* punt */
	else return 0;

}


char * API_ENTRY
l_asc_date(bdate)
char *bdate;
{
  static char adate[12];
  int idate;
  int year;
  int month;

	if (!bdate) return "1-jan-1990";
	if (!l_good_bin_date(bdate))
	{
		return "1-jan-2025";
	}


	(void) sscanf(bdate, "%x", &idate);	/* overrun threat */
	year = (idate >> 9) & 0x7f;
	if (year > 99) year += 1900;

	month = (idate >> 5) & 0xf;
	if (month > 11) return 0;
	(void) sprintf(adate, "%d-%s-%d", idate & 0x1f, 
					months[month], year);
	return(adate);
}

API_ENTRY
l_start_ok(job, bdate)
LM_HANDLE *job;		/* Current license job */
char *bdate;
{
  int idate, m, d, y;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  long cur, start;
  long tvsec;


	if (!bdate) return 0; /* ok */
	if (!l_good_bin_date(bdate))
	{
		LM_SET_ERRNO(job, LM_BADDATE, 244, 0);
		return(job->lm_errno);
	}
	(void) sscanf(bdate, "%x", &idate);	/* overrun threat */
#ifdef THREAD_SAFE_TIME
	l_get_date(&d, &m, &y, &tvsec, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&d, &m, &y, &tvsec);
#endif
	if (m == -1) 
	{
		LM_SET_ERRNO(job, LM_BADDATE, 33, 0);
	}
	else
	{
		cur = ((long)y * 372) + ((long)m * 31) + (long)d;
		start = ((((long)idate >> 9) & 0x7f) * 372) + 
			((((long)idate >> 5) & 0xf) * 31) + 
			((long)idate & 0x1f);
		if (cur >= start) return(0);
		else 
		{
			LM_SET_ERRNO(job, LM_TOOEARLY, 34, 0);
		}
	}
	return(job->lm_errno);
}

/* 
 *	l_int_month - Return the integer month number from the month string
 */
API_ENTRY
l_int_month(month)
char *month;
{
  char *p = month;
  int i;

	while (*p) 
	{
		if (isupper((int) *p)) *p = tolower((int) *p);
		p++;
	}
	for (i = 0; *months[i]; i++)
	{
		if (!strcmp(month, months[i])) break;
	}
	if (i<12) return(i);
	else	return(-1);
}

/* 
 *	l_good_bin_date -- returns 1 if ok, else 0
 */
int API_ENTRY
l_good_bin_date(hexdatestr)
char *hexdatestr;

{
  char *cp;	
	if (!hexdatestr) return 1;
	if (strlen(hexdatestr) != 4)
		return 0;
	for (cp = hexdatestr; *cp; cp++)
		if (!isxdigit(*cp))
			return 0;
	return 1;
}
	


/*
 *	l_midnight -- guaranteed to return no more than once a day
 *			around midnight
 */
int API_ENTRY 
l_midnight(lnow)
long lnow;
{
  static time_t lastmidnight = 0;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  struct tm *t;
  time_t now = (time_t) lnow;
#define ONE_HOUR (60*60)

#ifdef THREAD_SAFE_TIME
	localtime_r(&now, &tst);
	t = &tst;
#else /* THREAD_SAFE_TIME */
	t = localtime(&now);
#endif
	if ((t->tm_hour == 0) && ((now - lastmidnight) > ONE_HOUR))
	{
		lastmidnight = now;
		return 1;
	}
	return 0;
}

long
API_ENTRY
l_date_compare(job, date1, date2)
LM_HANDLE *job;
char *date1;
char *date2;
{
	if (!date1 || !*date1 || !date2 || !*date2)
	{
		LM_SET_ERRNO(job, LM_BADDATE, 270, 0);
		return LM_BADDATE;
	}
	return ((long)(l_date_to_time(job, date1) - 
					l_date_to_time(job, date2)));
}
#ifdef NO_MKTIME
/*
 *	like unix mktime -- should be portable 
 */
static 
int
l_mktime(str)
char *str;
{
  int m = 0, d, y, elapsed, i, d1, d2;
  char mon[100];

	if (!str || !*str) return 0;

	sscanf(str, "%d-%[^-]-%d", &d, mon, &y);	/* overrun threat */
	if (!y || y > 2038) return FAR_FUTURE_DATE; /* date way in the future */

	if (y > 1900) y -= 1900;
	l_uppercase(mon);
	if (!strncmp(mon, "JAN", 3))  m=0;
	else if (d+=31, !strncmp(mon, "FEB", 3)) m=1;
	else if (d+=28, !strncmp(mon, "MAR", 3)) m=2;
	else if (d+=31, !strncmp(mon, "APR", 3)) m=3;
	else if (d+=31, !strncmp(mon, "MAY", 3)) m=4;
	else if (d+=30, !strncmp(mon, "JUN", 3)) m=5;
	else if (d+=30, !strncmp(mon, "JUL", 3)) m=6;
	else if (d+=31, !strncmp(mon, "AUG", 3)) m=7;
	else if (d+=31, !strncmp(mon, "SEP", 3)) m=8;
	else if (d+=30, !strncmp(mon, "OCT", 3)) m=9;
	else if (d+=31, !strncmp(mon, "NOV", 3)) m=10;
	else if (d+=30, !strncmp(mon, "DEC", 3)) m=11;
	else return 0; /* no good */
	elapsed = ((y-70) * 365) + d;

	if ((d>(31+28)) && leap_year(y)) d++;

	for (i=70;i<y;i++)
	{
		if (leap_year(i)) elapsed++;
	}
	return (elapsed * 24 * 60 * 60); /* 24 hours + 60 min * 60 sec */
}
/*
 *	leap_year -- returns true if leap year
 */
static 
int
leap_year(year)
int year;
{
	return (!(year % 4) && ((year % 100) || !(year%400)) );
}

#endif /* NO_MKTIME */
