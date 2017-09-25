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
 *	Module: $Id: lm_perror.c,v 1.26 2003/04/18 23:48:07 sluu Exp $
 *
 *	Function: lc_perror(job, string)
 *		  lc_errstring(job, err)
 *
 *	Description: Prints the latest license manager error code, ala perror.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) string - The user string to print. (lc_perror)
 *			(int) err - The error string to retrieve. (lc_errstring)
 *
 *	Return:		None.
 *			lc_errstring() returns (char *).
 *
 *	M. Christiano
 *	2/13/88
 *
 *	Last changed:  12/8/98
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <stdio.h>
#ifdef PC  
#include <windows.h>
#include <stdlib.h>     
#ifdef WINNT
#include <process.h>
#endif /* WINNT */
#endif /* PC */
#ifdef UNIX 
#if !(defined(MAC10) || defined(GLIBC) || defined(ALPHA_LINUX) || defined(VMLINUX64)  \
			|| defined(RHLINUX64)) 
extern char *sys_errlist[];
#endif
#include <errno.h>
#endif /* UNIX */
/*
 *	Long Error message format can be 8 or 9 lines
 */
#define MAX_ERRSIZE 1024

/*
 *	ERROR codes
 */

#ifdef  NO_INIT_SHARE
extern char **_lm_errlist;
extern int _lm_nerr;
#else
static char const * const _lm_errlist[] = { 
#include "lmerrors.h"
			};
int _lm_nerr = (sizeof(_lm_errlist)/sizeof(_lm_errlist[0]));
#endif
static const char * const long_errs[] = {
#include "lm_lerr.h"
};
static const char * const context[] = {
#include "lcontext.h"
};


static long l_malloc_errstring	lm_args((LM_HANDLE *));
static void eadd_str		lm_args(( LM_HANDLE *, const char *));


LM_CHAR_PTR
API_ENTRY
lc_errstring(LM_HANDLE * job)
{
	int error = job->err_info.maj_errno;
	int err = -1 * error;
	int longerr = 0;
	int idx = job->lm_errno * -1;
	char buf[MAX_ERRSIZE + 1] = {'\0'};

	if (setjmp(job->jobcatch))
		return 0;

	if (job->lm_errno >= LM_LAST_ERRNO && job->lm_errno <= 0)
	{
		job->err_info.short_err_descr = _lm_errlist[idx];
		job->err_info.long_err_descr = long_errs[idx];
	}
	if (!l_malloc_errstring(job))
		return 0;
	if  (job->options->flags & LM_OPTFLAG_LONG_ERRMSG)
		longerr = 1;
	else
		longerr = 0;

#ifndef RELEASE_VERSION
	if (getenv("L_SHORT_ERRS"))
		longerr = 0;	/* overrun checked */
#endif /* RELEASE_VERSION */

	if (!error && job->err_info.warn)
		error = job->err_info.warn;

	if (error > 0)
		eadd_str(job, (char *)job->err_info.sys_err_descr);
	else if (err > 0 && err < _lm_nerr)
	{
		eadd_str(job, job->err_info.short_err_descr);
		if (longerr && job->err_info.long_err_descr)
		{
			strcat(job->err_info.errstring, "\n");
			eadd_str(job, job->err_info.long_err_descr);
		}
	}
	else if (err >= _lm_nerr)
		strcpy(job->err_info.errstring, "INVALID FLEXlm error code");
	else /* err == 0 */
		strcpy(job->err_info.errstring, "");
  	if (longerr)
	{
		char **	cpp = NULL;

		if (!job->err_info.lic_files)
			job->err_info.lic_files = job->lic_files;

		if (*job->err_info.feature && 
				(job->err_info.mask & LM_ERRMASK_FEAT))
		{
			sprintf(buf, "\n%-15s%s", "Feature:", 
						job->err_info.feature);
			eadd_str(job, buf);
		}
			
		if (job->err_info.context )
		{
			char const *cp = context[idx] ? context[idx] : "Context";

			sprintf(buf, "\n%s:", cp);
			sprintf(buf, "%-15s %s", buf, job->err_info.context);
			eadd_str(job, buf);
		}
		if (job->err_info.lic_files &&
			(job->err_info.mask & LM_ERRMASK_PATH))
		{
#define MAX_LINE_LEN 68
#define MAXER_LINE_LEN 78
			int l = 0;
			int slen = strlen(job->err_info.errstring);
			char *str = job->err_info.errstring + slen;
			int flen = 0;
			char *cp = NULL;

			sprintf(str, "\n%-15s", "License path:");
			slen += (l = strlen(str));
			str += l;
			for (cpp = job->err_info.lic_files; *cpp; cpp++)
			{
				cp = *cpp;
				flen = strlen(cp);
				if ((slen + flen + 75) > MAX_ERRSIZE) 
				{
					sprintf(str, "[...]");
					break;
				}
				while (*cp)
				{
#ifndef PC
					if (*cp == '\\')
					{
						cp++;
						continue;
					}
#endif /* PC */
					if ((*cp == '\n'))
					{
						*str++ = *cp++; l = 0; slen++;
						continue;
					}
					else if ((l >= MAX_LINE_LEN && 
						!isalnum(*cp) && *cp != '.' &&
						(*cp != '_')) || 
						(l > MAXER_LINE_LEN)
						)
					{
						strcat(str, " -\n   ");
						slen += strlen(" -\n   ");
						l = 3;
						str += strlen(str);
					}
					*str++ = *cp++; l++; slen++;
						
				}
				if (cpp[1])
				{
					*str++ = PATHSEPARATOR;
					l++; slen++;
					if (l >= MAX_LINE_LEN)
					{
						strcat(str, 
							"\n   ");
						slen += strlen(" -\n   ");
						l = 3;
						str += strlen(str);
					}
				}
			}
		}
				
	}

	if (job->err_info.sys_errno &&
			(job->err_info.mask & LM_ERRMASK_SYS))
	{
		if (longerr)
		{
			sprintf(buf,
			"\n%-15s%d,%d.  System Error: %d \"%s\"",
					"FLEXlm error:",
					-1*err, 
					job->err_info.min_errno, 
					job->err_info.sys_errno, 
					job->err_info.sys_err_descr);
			eadd_str(job, buf);
				
		}
		else
		{
			sprintf(buf, 
				" (%d,%d:%d \"%s\")", 
					-1*err, 
					job->err_info.min_errno, 
					job->err_info.sys_errno, 
					job->err_info.sys_err_descr);
			eadd_str(job, buf);
		}
	}
	else if (err)
	{
		if (longerr)
		{
			sprintf(buf, "\n%-15s%d,%d",
				"FLEXlm error:",
				-1*err, 
				job->err_info.min_errno) ;
			eadd_str(job, buf);
		}
		else
		{
			sprintf(buf, " (%d,%d)", -1*err, 
						job->err_info.min_errno) ;
			eadd_str(job, buf);
		}
	}
#if defined (SUPPORT_METER_BORROW) && !defined(DEBUG_TIMERS )
	if (job->err_info.borrow_errno)
	{
		if (longerr)
			eadd_str(job, "\n");
		else
			eadd_str(job, " "); 
		l_borrow_errstring(job);
		if (longerr)
			eadd_str(job, "\n");
		job->err_info.borrow_errno = 0; /* clear it */
	}
#endif /* SUPPORT_BORROW_METER */

	if (longerr)
		eadd_str(job, 
"\nFor further information, refer to the FLEXlm End User Manual,\n\
available at \"www.macrovision.com\".");

	return(job->err_info.errstring);
}


void
API_ENTRY
lc_perror(
	LM_HANDLE *	job,		/* Current license job */
	char *		string)		/* The user's string */
{
	char *errstr = lc_errstring(job);
	char msg_buf[MAX_ERRSIZE+1] = {'\0'};

	if (errstr && *errstr)
		sprintf(msg_buf, "%s: %s", lmtext(string), errstr);
	else
		sprintf(msg_buf, "%s", lmtext(string));
#ifdef WINNT
    {
		static char *diag = NULL;
        if (diag == NULL)
			diag = getenv ("FLEXLM_DIAGNOSTICS");	/* overrun checked */
        if(diag)
			printf("%s\n",msg_buf);
    }
#endif

#if defined( PC16) || defined(WINNT)


	if (job->options->flags & LM_OPTFLAG_PERROR_MSGBOX)
	{
		MessageBox( GetActiveWindow(), msg_buf, "Flexible License Manager",
                        MB_OK | MB_TASKMODAL | MB_SETFOREGROUND | MB_APPLMODAL);
	}
	else
	{
		fprintf(stderr, "%s\n", msg_buf);
	}
#endif /*  PC16  || WINNT*/
#if !defined (PC)
	fprintf(stderr, "%s\n", msg_buf);
#endif /* !PC */
#ifdef OS2
	fprintf(stderr, "%s\n", msg_buf);
#endif /* OS2 */
}

LM_CHAR_PTR
API_ENTRY
lc_errtext(
	LM_HANDLE_PTR	job,
	int				errnum)
{
	int		err = -1 * errnum;
	char ** errptr = NULL;
  
	if(job->err_info.errstring)
	{
		/*
		 *	Clear previous message if one already set.  This
		 *	fixes bug P6276 where we were clearing the contents
		 *	of job->err_info when we didn't need to.
		 */
		free(job->err_info.errstring);
		job->err_info.errstring = NULL;
	}

	if (job->options->flags & LM_OPTFLAG_LONG_ERRMSG)
		errptr = (char **)long_errs ;
	else
		errptr = (char **)_lm_errlist;

	if (!l_malloc_errstring(job))
		return 0;

	if ((errnum < 0) && (err >= _lm_nerr)) 
	{
		strcpy(job->err_info.errstring, 
				lmtext("INVALID FLEXlm error code"));
	}
	else if (errnum < 0) 
	{
		l_zcp(job->err_info.errstring, 
					errptr[errnum*-1], MAX_ERRSIZE);
	}
	else if (errnum > 0) 
	{
		l_zcp(job->err_info.errstring, SYS_ERRLIST(errnum), 
							MAX_ERRSIZE);
	}
	else /* err == 0 */
	{
		strcpy(job->err_info.errstring, lmtext("no error"));
	}
	LM_API_RETURN(char *, job->err_info.errstring)
}



/*
 * 	l_malloc_errstring -- 
 */
static
long
l_malloc_errstring(LM_HANDLE * job)
{
	if (!job->err_info.errstring) 
	{
		job->err_info.errstring = l_malloc(job, MAX_ERRSIZE + 1);
	}
	else
		memset(job->err_info.errstring, 0, MAX_ERRSIZE + 1);
	return (long)job->err_info.errstring;
}

static
void
eadd_str(
	LM_HANDLE *		job,
	const char *	str)
{
	if (!str)
		return;
	if ((strlen(job->err_info.errstring) + strlen(str)) < MAX_ERRSIZE)
		strcat(job->err_info.errstring, str);
}
