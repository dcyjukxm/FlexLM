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
 *	Module: $Id: lm_down.c,v 1.12 2003/01/13 21:58:48 kmaclean Exp $
 *
 *	Function:	lmdown functions
 *
 *	D. Birns
 *	7/18/96
 *
 *	Last changed:  12/8/98
 *
 */
#include "lmutil.h"

extern int lmdown_prompt;
extern int lmdown_force;

lmutil_down()
{
  char tmp[MAX_VENDOR_NAME + 1];
  int shutdown = 0;
  LMGRD_STAT *this_lmgrd = 0, *lmgrd = 0;
  int doall = 0;
  int flags = 0;
        
        *tmp = 0;
	if (lmdown_force) flags |= LM_DOWN_FORCE;
        (void) l_flush_config(lm_job); /* read the lic file */
        
        while (shutdown == 0)
        {
                
#ifdef PC
                if (!this_lmgrd && !l_getenv(lm_job, "LM_TSUITE"))
                {
                        fprintf(ofp, "[Detecting lmgrd processes...]\n");
                        fflush(ofp);
                }
#endif /* PC */
                shutdown = l_shutdown(lm_job, 0, flags, 0, l_vendor, 
                                this_lmgrd, this_lmgrd ? 0 : &lmgrd);
                if (shutdown > 0)
                {
                        fprintf(ofp, lmtext(
                                "    %d FLEXlm License Server%s shut down\n"), 
                                        shutdown, shutdown>1 ? "s" : "");
                        if (shutdown > 1)
                                fprintf(ofp, lmtext(
                "    Please wait 1 minute for lmgrd processes to shutdown\n"));

                }
                else if (shutdown < 0)
                {
                        if (*l_vendor)
                        {
                                fprintf(ofp, lmtext(
                        "    Can't find lmgrd for vendor \"%s\"\n"), 
                                        l_vendor);
                                if (doall && this_lmgrd->next) continue;
                                return -1;
                        }
                        fprintf(ofp, "Shutdown failed: %s\n", 
                        lc_errstring(lm_job));
			if (lm_job->lm_errno == LM_BORROW_DOWN)
			fprintf(ofp, " If you're on the same host as the server,\n you need to use -force argument to lmdown.\n Shutdown can't be done from any other host.\n");
                }
                else if (shutdown == 0 && (!this_lmgrd || lmdown_prompt))
                {
                        if (!(this_lmgrd = l_select_lmgrd(&lmgrd)))
                        {
                                fprintf(ofp,  "No server selected, exiting\n");
                                if (lmgrd) lc_free_lmgrd_stat(lm_job, lmgrd);
                                return 0;
                        }
                                
                }
                if (this_lmgrd == ALL_LMGRDS)
                {
                        this_lmgrd = lmgrd;
                        doall = 1;
                }
                else if (doall)
                {
                        if ( this_lmgrd->next)
                        {
                                shutdown = 0; /* so we loop */
                                this_lmgrd = this_lmgrd->next;
                        }
                        else break;
                }
                        
        }
        if (lmgrd) lc_free_lmgrd_stat(lm_job, lmgrd);
        
        if (*tmp) 
        {
                strcpy(lm_job->vendor, tmp);
        }
        return lm_job->lm_errno;
}

