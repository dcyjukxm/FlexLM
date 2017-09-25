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
 *	Module: $Id: lm_log.c,v 1.5 2003/01/13 22:41:47 kmaclean Exp $
 *
 *	Function: lc_log(job, message)
 *
 *	Description: 	Log a user message in the daemon logfile
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) message - message to be logged
 *
 *	Return:		None
 *
 *	M. Christiano
 *	3/22/90
 *
 *	Last changed:  9/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"

void API_ENTRY
lc_log(job, message)
LM_HANDLE *job;		/* Current license job */
char *message;
{
  char msg[LM_MSG_LEN+1];

/*
 *	Make a copy of the message so l_sndmsg() doesn't run off the end
 *	of the input string, which could cause a segment violation.
 */
	if (LM_API_ERR_CATCH) return;

	(void) memset(msg, '\0', LM_MSG_LEN);
	(void) strncpy(msg, message, LM_MSG_LEN);
	if (job->daemon && job->daemon->commtype == LM_FILE_COMM)
	{
		(void) l_file_log(job, msg);
	}
	else
	{
		(void) l_sndmsg(job, LM_LOG, msg);
	}
	LM_API_RETURN_VOID()
}
