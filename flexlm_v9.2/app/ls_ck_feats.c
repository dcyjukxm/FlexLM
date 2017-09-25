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
 *	Module: $Id: ls_ck_feats.c,v 1.4 2003/01/13 22:22:34 kmaclean Exp $
 *
 *	Function:	ls_ck_feats()
 *
 *	Description: 	Checks the FEATURESET line, if required
 *
 *	Parameters:	None.
 *
 *	Return:		0 - FEATURESET line invalid
 *			<> 0 - FEATURESET line not required or OK
 *
 *	M. Christiano
 *	4/17/90
 *
 *	Last changed:  7/22/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lsfeatur.h"
#include "ls_adaem.h"
#include "lgetattr.h"
#include "ls_log.h"
#include "ls_aprot.h"
static int ck_feats_currfile lm_args((lm_noargs));

ls_ck_feats()
{
  int ret = 1;

	if (ls_use_featset)
	{
		for (lm_job->lfptr = 0; lm_job->lfptr < lm_job->lm_numlf;
			lm_job->lfptr++)
			if (!ck_feats_currfile())
				ret = 0;
	}
	return ret;
}
static
int
ck_feats_currfile()
{
  int ok = 1;
		
	if (ok = lm_ck_feats(lm_job->vendor))
		return ok;
	switch (_lm_errno)
	{
	case NOFEATSET:
	ls_log_error(
		lmtext("FEATURESET line missing in license file"));
		LOG_INFO((FATAL, "The FEATURESET line was \
			missing from the license file.  The \
			daemon will not continue."));
		break;


	case CANTCOMPUTEFEATSET:
		return 1;

	case BADFEATSET:
		ls_log_error(
		lmtext("FEATURESET incorrect in license file"));
		LOG_INFO((FATAL, "The FEATURESET data in \
			the license file is incorrect. \
			The daemon will not continue."));
		break;

	case FUNCNOTAVAIL:
		ls_log_error(
		lmtext("FEATURESET not enabled in this version"));
		LOG_INFO((FATAL, "The FEATURESET capability \
			is not available in this version of \
			FLEXlm."));
		break;

	default:
		ls_log_error(
			lmtext("Unknown error on FEATURESET line"));
		LOG_INFO((FATAL, "The FEATURESET data in \
			the license file is incorrect. \
			The daemon will not continue."));
		break;
	}
	LOG((lmtext("license file is %s\n"), lm_job->lic_files[lm_job->lfptr]));
	return(ok);
}
