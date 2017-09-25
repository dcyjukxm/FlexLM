/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_error.c,v 1.16 2003/01/13 22:41:51 kmaclean Exp $
 *
 *	Function: 	l_set_error
 *
 *	Parameters: 	LM_HANDLE_PTR job;
 *			int this_lmerrno; -- major error number
 *			int this_minor;
 *			int this_u_errno; -- if 0, don't erase old one
 *			LM_CHAR_PTR context; -- optional
 *
 *	Return: 	void
 *
 *	D. Birns
 *	7/15/97
 *
 *	Last changed:  11/7/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#ifdef UNIX
#if!defined (LINUX ) && !defined(GLIBC) && !defined(ALPHA_LINUX) && !defined(MAC10)
extern char *sys_errlist[];
#endif /* ANSI */
#include <errno.h>
#endif /* UNIX */
#ifdef USE_WINSOCK
/*#define PC_MT_CONNECT*/
static char const * const l_winsock_error1[] = { 
#include "winerr1.h"
			};
static char const * const l_winsock_error2[] = { 
#include "winerr2.h"
			};


#endif /*USE_WINSOCK */

void
API_ENTRY
l_set_error(job, this_lmerrno, this_minor, this_u_errno, context, mask, thread)
LM_HANDLE_PTR job;
int this_lmerrno;
int this_minor;
int this_u_errno;
LM_CHAR_PTR context;
int mask;
char *thread;
{
  LM_ERR_INFO *err_info = &job->err_info;
#if !defined(PC) && !defined(VMS)
  extern int sys_nerr;
#endif /* !defined(PC) && !defined(VMS) */
#ifdef PC_MT_CONNECT
  LM_CONNINFO *conninfo = (LM_CONNINFO *)thread;
        if (conninfo)
        {
                err_info = &conninfo->err_info;
#if 0          
                fprintf(stdout, "thread port %d: %d %d %d %s\n", 
                        htons ((unsigned short)(
                        conninfo->endpoint.transport_addr.port & 0xffff)),
                        this_lmerrno, this_minor,
                        this_u_errno, context ? context : "");
#endif /* 0 */
        }
#endif /* PC_MT_CONNECT */
                
                
#ifndef PC
	if (this_u_errno == EINTR) this_u_errno = 0;
#endif /* PC */
	if( job->err_info.maj_errno && job->err_info.min_errno)
	{
		switch(this_lmerrno)
		{
		case LM_CANTCONNECT:
		case LM_POOL:
		case LM_NOSERVER:  
		case LM_VENDOR_DOWN:
			return;
		}
	}

	if (this_lmerrno == LM_CANTMALLOC) 
	{
	        job->lm_errno = err_info->maj_errno = LM_CANTMALLOC;
		return; /* If this happens, all bets are off -- save it */
	}
		
	/* Note -- maj_errno is set in LM_SET_ERRNO mask, for sec reasons */
	err_info->maj_errno = this_lmerrno;
	err_info->min_errno = (this_minor); 
	if ( err_info->context) free(err_info->context);
	err_info->context = 0;
	if (context)
	{
		if (err_info->context = (char *)
					calloc(1, strlen(context) + 1))
		{
			strcpy(err_info->context, context);
		}
	}
	if ((this_u_errno) > 0)
		err_info->sys_errno = this_u_errno; 
#ifndef USE_WINSOCK
	if (this_u_errno >0 && (this_u_errno <  SYS_NERR)) 
		err_info->sys_err_descr = SYS_ERRLIST(this_u_errno);
#else
	if (this_u_errno >0 && (this_u_errno <  SYS_NERR)) 
		err_info->sys_err_descr = SYS_ERRLIST(this_u_errno);
	else if (this_u_errno >0 && (this_u_errno > 9999) && (this_u_errno <  11000))
		err_info->sys_err_descr = l_winsock_error1[this_u_errno-10004];
	else if (this_u_errno >0 && (this_u_errno >  11000))	
		err_info->sys_err_descr = l_winsock_error2[this_u_errno-11001];
	else    err_info->sys_err_descr="";


#endif
#if defined( SUPPORT_METER_BORROW) && !defined (DEBUG_TIMERS)
	if ((this_lmerrno ==  LM_BORROW_ERROR) ||
			(this_lmerrno ==  LM_NONETOBORROW) ||	
			(this_lmerrno ==  LM_BORROWLOCKED) ||
			(this_lmerrno ==  LM_BORROW_NOCOUNTER) ||
			(this_lmerrno ==  LM_BORROWED_ALREADY) ||
			(this_lmerrno ==  LM_UNBORROWED_ALREADY) ||	
			(this_lmerrno ==  LM_BORROW_METEREMPTY) ||
			(this_lmerrno ==  LM_NOBORROWMETER))
		l_borrow_err(job);
#endif /* SUPPORT_METER_BORROW */
		
	err_info->mask = mask;
}
void API_ENTRY
l_clear_error(job)
LM_HANDLE *job;
{
	char feat[MAX_FEATURE_LEN + 1];
	if (job) 
	{
		l_free_err_info(&job->err_info);
		l_zcp(feat, job->err_info.feature, MAX_FEATURE_LEN);
		memset(&job->err_info, 0, sizeof(job->err_info));
		l_zcp(job->err_info.feature, feat, MAX_FEATURE_LEN);
	}
}
void API_ENTRY
l_free_err_info(e)
LM_ERR_INFO *e;
{
	if (e->errstring)
		free(e->errstring);
	if (e->context)
		free(e->context);
	e->errstring = e->context = 0;
}
void API_ENTRY
l_set_error_path(job)
LM_HANDLE *job;
{
	job->err_info.lic_files = job->lic_files;
}

int API_ENTRY
l_err_info_cp(job, toe, frome)
LM_HANDLE_PTR job; 
LM_ERR_INFO *toe;
LM_ERR_INFO *frome;
{
	if (toe->errstring)
		free (toe->errstring);
	if (toe->context)
		free (toe->context);
	memcpy(toe, frome, sizeof(LM_ERR_INFO));
	if (frome->errstring)
	{
		if (!(toe->errstring = 
			(char *)l_malloc(job, strlen(frome->errstring) + 1)))
			return job->lm_errno;
		strcpy(toe->errstring, frome->errstring);
	}
	if (frome->context)
	{
		if (!(toe->context = 
			(char *)l_malloc(job, strlen(frome->context) + 1)))
			return job->lm_errno;
		strcpy(toe->context, frome->context);
	}
/* 
 *	lic_files would have to be malloc'd, and it's
 *	not worth it at this time 
 */
	toe->lic_files = 0; 
	return 0;
}

LM_ERR_INFO_PTR
API_ENTRY 
lc_err_info (job)
LM_HANDLE *job;
{
	if (!job)
		return 0;
	return(&job->err_info);
}
int 
API_ENTRY
lc_get_errno(job)
LM_HANDLE *job;
{
	if (!job)
                return LM_BADPARAM;
	return(job->lm_errno);
}
