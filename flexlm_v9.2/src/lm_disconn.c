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
 *	Module: $Id: lm_disconn.c,v 1.11 2003/01/13 22:41:45 kmaclean Exp $
 *
 *	Function: lc_disconn(job, force)
 *
 *	Description: Drops the connection to a FLEXlm server, if not in use.
 *
 *	Parameters:	(int) force - Force the disconnection, if non-zero.
 *
 *	Return:		(int) - < 0 - Error (see job->lm_errno)
 *				> 0 - Number of users of socket.
 *				== 0 - OK, socket shut down.
 *
 *	M. Christiano
 *	6/26/90
 *
 *	Last changed:  5/27/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#ifdef USE_WINSOCK
#include <pcsock.h>
#endif

API_ENTRY
lc_disconn(job, force)
LM_HANDLE *job;		/* Current license job */
int force;
{
  int rc = NOSOCKET;

	/* doesn't set errors */
	if (force || (job->daemon->usecount <= 1))
	{
		job->daemon->usecount = 0;
		if (job->daemon->socket != LM_BAD_SOCKET)
		{
		  int oflag;
/*
 *			Ignore any errors from this shutdown/close
 */

#ifdef PC
			/* P6213 -- NO_HARVEST was set incorrectly, causing
			   heartbeat failures */

			oflag = job->flags;
			job->flags &= ~LM_FLAG_CONNECT_NO_HARVEST;
			l_conn_harvest(job);
			job->flags |= oflag;
#endif /* PC */
#ifndef PC
			shutdown(job->daemon->socket, 2);
#endif
			network_close(job->daemon->socket);
		}

		job->daemon->socket = LM_BAD_SOCKET;
/* 
 *		Forget the old encryption 
 */
		job->daemon->encryption = 0;	
	}
	else
	{
/*
 *		If this socket is still in use, return the count of users.
 */
		job->daemon->usecount--;
	}
	rc = job->daemon->usecount;
	return(rc);
}
