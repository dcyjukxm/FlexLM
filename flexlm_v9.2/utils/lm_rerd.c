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
 *	Module: $Id: lm_rerd.c,v 1.8 2003/01/13 21:58:49 kmaclean Exp $
 *
 *	Function:	lmutil_lmreread
 *
 *	D. Birns
 *	7/17/97
 *
 *	Last changed:  11/26/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lmutil.h"

lmutil_reread(newargc, newargv)
int newargc;
char **newargv;
{
  LMGRD_STAT *lmgrds = 0, **lpp = &lmgrds, *this_lmgrd = 0;
  int rc = 0;
  char *vendor = 0;
  int doall = 0;

	exit_code = 0;
	
	if ( *l_vendor) vendor = l_vendor;
	while (rc == 0)
	{
#ifdef PC
                if (!this_lmgrd && !l_getenv(lm_job, "LM_TSUITE"))
                {
                        fprintf(ofp, "[Detecting lmgrd processes...]\n");
                        fflush(ofp);
                }
#endif
		if ((rc = l_rerd(
			lm_job, vendor, this_lmgrd, 
				this_lmgrd ? 0 : lpp)) > 0)
		{
			fprintf(ofp, "lmreread successful\n");
			if (!doall || !this_lmgrd->next) break;
		}
		else if (rc < 0)
		{
			fprintf(ofp, "lmreread failed: ");
			if ((lm_job->lm_errno == LM_BADPARAM) && 
						(newargc >= 2))
				fprintf(ofp, "daemon \"%s\" not found: ",
						newargv[1]);
			fprintf(ofp, "%s\n", lc_errstring(lm_job));
			exit_code = lm_job->lm_errno;
			if (!doall || !this_lmgrd->next) break;
		}
		else if (lmgrds && !this_lmgrd)
		{
			lpp = 0;
			if (!(this_lmgrd = l_select_lmgrd(&lmgrds)))
			{
				fprintf(ofp, 
				"No server selected, exiting\n");
				break;
			}
		}
		else
		{
			fprintf(ofp, 
				"lmreread failed, exiting\n");
			exit_code = lm_job->lm_errno;
			if (!doall || !this_lmgrd->next) break;
		}
                if (this_lmgrd == ALL_LMGRDS)
                {
                        this_lmgrd = lmgrds;
                        doall = 1;
                }
                else if (doall)
                {
                        if ( this_lmgrd->next)
                        {
                                rc = 0; /* so we loop */
                                this_lmgrd = this_lmgrd->next;
                        }
                        else break;
                }
	}
	if (lmgrds) lc_free_lmgrd_stat(lm_job, lmgrds);

}
