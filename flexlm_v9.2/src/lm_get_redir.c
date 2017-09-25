/******************************************************************************

	    COPYRIGHT (c) 1993, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_get_redir.c,v 1.4 2003/04/14 22:31:12 brolland Exp $
 *
 *	Function:	lc_get_redir(job)
 *			l_do_redir(job)
 *
 *	Description: 	Gets the HOSTID REDIRECTION data from a license file
 *			l_do_redir() - update structs, etc.
 *
 *	Parameters:	None
 *
 *	Return:		(LM_HOSTID_REDIRECT *) The hostid redirection data.
 *
 *	M. Christiano
 *	1/28/93		(From lm_get_feats.c)
 *
 *	Last changed:  7/2/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_openf.h"
#include <stdio.h>
#include <errno.h>

#if defined(PC) 
typedef int (LM_CALLBACK_TYPE *     LM_CALLBACK_REDIRECT)
					( HOSTID *from,
				          HOSTID *to, 
				          VENDORCODE *code,
					  char *signature );
#define LM_CALLBACK_REDIRECT_TYPE (LM_CALLBACK_REDIRECT)
#else 
#define LM_CALLBACK_REDIRECT_TYPE
#endif

LM_HOSTID_REDIRECT * API_ENTRY
lc_get_redir(job)
LM_HANDLE *job;		/* Current license job */
{
  LICENSE_FILE *lf;
  char line[MAX_CONFIG_LINE+1];
  char *f1, *f2, *f3, *f4;
  int nflds;
  LM_HOSTID_REDIRECT *tmp, *out = (LM_HOSTID_REDIRECT *) NULL;
  LM_HOSTID_REDIRECT *last = (LM_HOSTID_REDIRECT *) NULL;

	if ( !(f1 = malloc((MAX_CONFIG_LINE+1)*4)) )
	{
		LM_SET_ERRNO(job, LM_CANTMALLOC, 172, 0);
		/*
		job->lm_errno = CANTMALLOC;
		job->u_errno = 0;
		*/
		return 0;
	}
	else
	{
		f2 = f1+MAX_CONFIG_LINE+1;
		f3 = f2+MAX_CONFIG_LINE+1;
		f4 = f3+MAX_CONFIG_LINE+1;
	}

	if (job->options->redirect_verify)
	{
	    if ((lf = l_open_file(job, LFPTR_FIRST)) != (LICENSE_FILE *)NULL)
	    {
		while (l_lfgets(job, line, MAX_CONFIG_LINE, lf, 0))
		{
			nflds = sscanf(line, "%s %s %s %s", f1, f2, f3, f4);	/* overrun threat */
	        	if (nflds >= 4 && 
				l_keyword_eq(job, f1, LM_RESERVED_REDIRECT))
			{
				tmp = (LM_HOSTID_REDIRECT *)
					malloc(sizeof(LM_HOSTID_REDIRECT));
				if (tmp == (LM_HOSTID_REDIRECT *) NULL)
				{
					LM_SET_ERRNO(job, LM_CANTMALLOC, 173, 0);
					free( f1 ); 
					return(tmp);
				}
				if (l_get_id(job, &tmp->from, f2) ||
					l_get_id(job, &tmp->to, f3))
				{
					free( f1 ); 
					return(tmp);
				}
				if ((*LM_CALLBACK_REDIRECT_TYPE
				      job->options->redirect_verify)
				       (tmp->from, tmp->to, &job->code,
									f4))
				{
/*
 *					Good signature.  Link it into the list
 */
					if (last == (LM_HOSTID_REDIRECT *) NULL)
					{
						out = tmp;
					}
					else
					{
						last->next = tmp;
					}
					last = tmp;
				}
				else
					(void) free(tmp);
			}
		}
		if (last) last->next = (LM_HOSTID_REDIRECT *) NULL;
		l_lfclose(lf);
	    }
	}
	free( f1 );
	return(out);
}

void
l_do_redir(job)
LM_HANDLE *job;		/* Current license job */
{
	if (job->redirect)
	{
	  LM_HOSTID_REDIRECT *x, *y;
/*
 *              Free the old stuff
 */
		for (x = job->redirect; x; x = y)
		{
			y = x->next;
			(void) free(x);
		}
	}
	job->redirect = lc_get_redir(job);  /* re-read redirection */
}
