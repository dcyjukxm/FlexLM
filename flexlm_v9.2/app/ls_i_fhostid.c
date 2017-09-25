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
 *	Module: $Id: ls_i_fhostid.c,v 1.2 2003/01/13 22:22:36 kmaclean Exp $
 *
 *	Function: ls_i_fhostid(master_list)
 *
 *	Description: Initialize the feature table for the server
 *
 *	Parameters:	ls_flist - The feature table.
 *			(LM_SERVER *) master_list - The servers
 *
 *	Return:		ls_flist filled in with the number of licenses available
 *
 *
 *	M. Christiano
 *	8/15/95
 *
 *	Last changed:  7/2/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "lsfeatur.h"

void
ls_i_fhostid(f)
FEATURE_LIST *f;
{
	if (f->id && *f->id && !f->hostid)
	{
		if (l_get_id(lm_job, &f->hostid, f->id))
			LOG(("Error converting hostid for %s: %s\n", f->id,
				lc_errstring(lm_job)));
	}
}
