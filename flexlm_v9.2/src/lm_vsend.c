/******************************************************************************

	    COPYRIGHT (c) 1992, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_vsend.c,v 1.8 2003/06/18 22:31:43 jwong Exp $
 *
 *	Function: lc_vsend()
 *
 *	Description: 	Send vendor-specified message to daemon
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) message - message to be send
 *
 *	Return:		(char *) retmsg - Returned message  (NULL if error)
 *
 *	M. Christiano
 *	8/14/92
 *
 *	Last changed:  10/11/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"

char * API_ENTRY
lc_vsend(job, message)
LM_HANDLE *job;		/* Current license job */
char *message;
{
  char msg[LM_MSG_LEN+1];
  char *retmsg = (char *) NULL;

/*
 *	Make a copy of the message so l_sndmsg() doesn't run off the end
 *	of the input string, which could cause a segment violation.
 */
	if (LM_API_ERR_CATCH) return 0;

	if (l_connect_by_conf_for_vsend(job, (CONFIG *)0) == 0)
	{
		(void) bzero(msg, LM_MSG_LEN);
		(void) strncpy(msg, message, LM_MSG_LEN);
		(void) l_sndmsg(job, LM_VENDOR_MSG, msg);
		l_rcvmsg_type(job, LM_VENDOR_RESP, &retmsg);
		LM_API_RETURN (char *,retmsg)
	}
	else
		return 0;
}
