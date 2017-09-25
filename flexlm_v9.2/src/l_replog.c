/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_replog.c,v 1.2 2003/01/13 22:41:55 kmaclean Exp $
 *
 *	Function: l_replog
 *
 *	Description: Sends LM_REPFILE message to server
 *
 *	Parameters: job
 *
 *	Return:		0 (success), or errno
 *
 *	D. Birns
 *	8/21/95
 *
 *	Last changed:  10/23/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lgetattr.h"
#include "lm_comm.h"
#include "l_prot.h"

int API_ENTRY
l_replog(job)
LM_HANDLE *job;
{

	if (l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL)
	{
		LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 261, 0);
		return LM_FUNCNOTAVAIL;
	}
	return (!l_sndmsg(job, LM_SWITCH_REPORT, LM_SWITCH_REPORT_CLIENT));
}
