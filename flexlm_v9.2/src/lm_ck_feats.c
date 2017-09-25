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
 *	Module: $Id: lm_ck_feats.c,v 1.2 2003/01/13 22:41:45 kmaclean Exp $
 *
 *	Function:	lc_ck_feats(job, vendor)
 *
 *	Description: 	Checks the FEATURESET line, if required
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) vendor - vendor name
 *
 *	Return:		0 - FEATURESET line invalid
 *			<> 0 - FEATURESET line not required or OK
 *
 *	M. Christiano
 *	4/17/90		(4/7/92: adapted from ls_ck_feats.c)
 *
 *	Last changed:  5/5/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"

API_ENTRY
lc_ck_feats(job, vendor)
LM_HANDLE *job;		/* Current license job */
char *vendor;
{
  int ok = 0;

	l_clear_error(job);
	if (l_getattr(job, FEATURESET) == FEATURESET_VAL)
	{
	  char *fs = lc_feat_set(job, vendor, &(job->code), 0);
	  char *fsline = lc_get_feats(job, vendor);

		if ((fsline == (char *) NULL))
		{
			LM_SET_ERRNO(job, LM_NOFEATSET, 150, 0);
		}
		else if (fs == (char *) NULL)
		{
			LM_SET_ERRNO(job, LM_CANTCOMPUTEFEATSET, 151, 0);
		}
		else if (strcmp(fs, fsline))
		{
			LM_SET_ERRNO(job, LM_BADFEATSET, 152, 0);
		}
		else
			ok = 1;
	}
	else
	{
		LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 153, 0);
	}

	return(ok);
}
