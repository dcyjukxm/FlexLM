/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_get_feats.c,v 1.7 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function:	lc_get_feats(job, daemon)
 *
 *	Description: 	Gets the FEATURESET data for a given daemon.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) daemon - The daemon name
 *
 *	Return:		(char *) The featureset encryption data.
 *
 *	M. Christiano
 *	6/11/90		(From ls_feat_set.c 4/7/92)
 *
 *	Last changed:  9/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_openf.h"
#include <stdio.h>
#include <errno.h>

static char * get_featset_from_file lm_args(( LM_HANDLE *, char *));

char * API_ENTRY
lc_get_feats(job, daemon)
LM_HANDLE *job;		/* Current license job */
char *daemon;
{

  char *ret = 0;
  int sav;

	if (LM_API_ERR_CATCH) return 0;

	if (job->featureset == NULL)
	{
		job->featureset = (char *)l_malloc(job, MAX_CONFIG_LINE + 1);
	}
	
	sav = job->lfptr;
	for (job->lfptr = 0; job->lfptr < job->lm_numlf; job->lfptr++)
	{
		if (ret = get_featset_from_file(job, daemon)) break;
	}
	if (!ret)
	{
		LM_SET_ERRNO(job, LM_NOFEATURE, 171, 0);
	}
	job->lfptr = sav;
	LM_API_RETURN(char *,ret)
}

static
char *
get_featset_from_file(job , daemon)
LM_HANDLE *job;
char *daemon;
{
  LICENSE_FILE *lf;
  char line[MAX_CONFIG_LINE+1];
  char f1[MAX_CONFIG_LINE+1], f2[MAX_CONFIG_LINE+1];
  int nflds;
  char *out = 0;

	
	if (lf = l_open_file(job, LFPTR_CURRENT))
	{
		while (l_lfgets(job, line, MAX_CONFIG_LINE, lf, 0))
		{
			nflds = sscanf(line, "%s %s %s", f1, f2, 	/* overrun checked */
							job->featureset);
	        	if (nflds >= 3 && 
				l_keyword_eq(job, f1, LM_RESERVED_FEATURESET) &&
						l_keyword_eq(job, f2, daemon))
			{
				out = job->featureset;
				break;
			}
		}
		l_lfclose(lf);
	}
	return out;
}
