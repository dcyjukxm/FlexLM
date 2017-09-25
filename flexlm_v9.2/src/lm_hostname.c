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
 *	Module: $Id: lm_hostname.c,v 1.4 2003/04/18 23:48:07 sluu Exp $
 *
 *	Function: lc_hostname(job, flag)
 *
 *	Description: 	Returns the current host name
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(int) flag - 0 for host system's idea of name
 *				   - <> 0 for FLEXlm's idea of name
 *
 *	Return:		(char *) host name
 *
 *	M. Christiano
 *	3/16/90
 *
 *	Last changed:  5/27/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#ifdef USE_WINSOCK
#include <pcsock.h>
#endif

static char hostname[MAX_HOSTNAME+1];

char * API_ENTRY
lc_hostname(job, flag)
LM_HANDLE *job;		/* Current license job */
int flag;
{
	if (flag && job->options->host_override[0])
		return(job->options->host_override);
	else
	{
		(void) gethostname(hostname, MAX_HOSTNAME);
		if (!job->options->host_override[0])
		{
			strncpy(job->options->host_override, hostname, 
							MAX_SERVER_NAME);/* LONGNAMES */
			job->options->host_override[MAX_SERVER_NAME] = '\0';/* LONGNAMES */
		}
		return(hostname);
	}
}
