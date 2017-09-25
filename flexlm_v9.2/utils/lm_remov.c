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
 *	Module: $Id: lm_remov.c,v 1.4 2003/02/21 00:00:05 jwong Exp $
 *
 *	Function:	lmremove functions
 *
 *	D. Birns
 *	7/17/97
 *
 *	Last changed:  8/28/97
 *
 */
#include "lmutil.h"
extern int remove_handle;
#define REMOVE_FEATURE 1
#define REMOVE_USER 2
#define REMOVE_HOST 3
#define REMOVE_DISP 4

#define REMOVEH_FEATURE 1
#define REMOVEH_SERVER 2
#define REMOVEH_PORT 3
#define REMOVEH_HANDLE 4

lmutil_remove(newargc, newargv)
int newargc;
char **newargv;
{
  int i;

	if (newargc < 5) 
	{
		usage();
		return -1;
	}
	if (remove_handle)
	{
	    i = lc_removeh(lm_job, newargv[REMOVEH_FEATURE],
			  newargv[REMOVEH_SERVER],
			  atoi(newargv[REMOVEH_PORT]),
			  newargv[REMOVEH_HANDLE]);
	    if (i)
	    {
		if (lm_job->lm_errno == BADPARAM)
			fprintf(ofp, 
			    lmtext("lmremove: license handle %s not found on this license server (%s)\n"),
					newargv[REMOVEH_HANDLE],
					newargv[REMOVEH_SERVER]);
		else
		{
			fprintf(ofp, "lmremove: %s\n", 
				lmtext(lc_errstring(lm_job)));
			if (lm_job->lm_errno == BADCOMM)
				fprintf(ofp, 
lmtext("\t(This license server may not support removing users by handle.)\n"));
		}
		return lm_job->lm_errno;
	    }
	}
	else
	{
	    i = lm_remove(newargv[REMOVE_FEATURE], newargv[REMOVE_USER],
			newargv[REMOVE_HOST], newargv[REMOVE_DISP]);
	    if (i)
	    {
		if (lm_job->lm_errno == BADPARAM)
		{
			fprintf(ofp, 
			    lmtext("lmremove: feature/user/host/display not found\n"));
		}
		else
		{
			fprintf(ofp, "lmremove: %s\n", 
				lmtext(lc_errstring(lm_job)));
		}
		return lm_job->lm_errno;
	    }
	}
	return 0;
}
