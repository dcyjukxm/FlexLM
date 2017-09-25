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
 *	Module: $Id: lm_lic_where.c,v 1.3 2003/01/13 22:41:47 kmaclean Exp $
 *
 *	Function: lc_lic_where(job)
 *
 *	Description: 	Returns the path to the license file
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *
 *	Return:		(char *) license file path
 *
 *	M. Christiano
 *	3/23/90
 *
 *	Last changed:  11/13/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <stdio.h>

char * API_ENTRY
lc_lic_where(job)
LM_HANDLE *job;		/* Current license job */
{

	if (job->lic_files == (char **) NULL) l_init_file(job);

	if (job->lic_files[0] && (job->lfptr == LFPTR_INIT))
		job->lfptr = 0;
	if (job == (LM_HANDLE *) NULL || job->lfptr == LFPTR_INIT ||
	    			job->lic_files == (char **) NULL)
	{
		return((char *) NULL);
	}
	else
	{
		return(job->lic_files[(int)job->lfptr]);
	}
}
