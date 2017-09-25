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
 *	Module: $Id: lm_baddate.c,v 1.4 2003/01/13 22:41:45 kmaclean Exp $
 *
 *	Function: lc_baddate()
 *
 *	Description: This is a stub function, since lc_baddate has now
 *		     been superseded by automatic bad date detection.
 *
 *	Parameters:  lc_baddate() - job
 *
 *	Return:		(int) - 0 - OK, date is OK
 *				1 - Found at least one file whose
 *				creation date is in the future more than
 *                              24 hours.
 *
 *	Last changed:  9/9/98
 *
 */
#include "lmclient.h"
#include "l_prot.h"
#ifdef PC
API_ENTRY l_baddate(LM_HANDLE *);
#endif

int API_ENTRY
lc_baddate(job)
LM_HANDLE_PTR job;
{
  int ret;
  int val;
	if (LM_API_ERR_CATCH) return job->lm_errno;

	val = job->options->flags & LM_OPTFLAG_CHECK_BADDATE;
	if (!val) job->options->flags |= LM_OPTFLAG_CHECK_BADDATE;
	ret =  l_baddate(job);
	if (!val) job->options->flags &= ~LM_OPTFLAG_CHECK_BADDATE;
	LM_API_RETURN(int, ret)
}
