/******************************************************************************

	    COPYRIGHT (c) 1989, 2003  by Macrovision Corporation.
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
 *	Module: $Id: ts_dump.c,v 1.3 2003/01/13 22:55:18 kmaclean Exp $
 *
 *	Function: ts_dump(feature)
 *
 *	Description: Verifies that the program is running on the specified host.
 *
 *	Parameters:	id (int) - The host id
 *
 *	Return:		(int) - 0 - OK, ie. we are running on the specified
 *					host.
 *				NOTTHISHOST - We are not running on the
 *						specified host.
 *
 *	M. Christiano
 *	8/30/89
 *
 *	Last changed:  7/16/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "l_prot.h"
#include "code.h"
#include <stdio.h>

#ifdef USE_WINSOCK	  
#include <pcsock.h>	  
#endif			  
		 
#ifndef ANSI
char *strcpy();
#endif
LM_DATA_REF;

void
ts_dump(feature)
char *feature;
{
  char msg[LM_MSG_LEN+1];
  int d;
  CONFIG *c;

	(void) memset(msg,0,sizeof(msg));
	(void) lm_init("demo", &code, (LM_HANDLE **) NULL);
	c = l_lookup(lm_job, feature);
	if (!c)
	{
		lm_perror("Feature lookup failed");
	}
	else
	{
		d = l_connect(lm_job, c->server, c->daemon, 
				lm_job->daemon->commtype);
		if (d < 0)
			lm_perror("server_connect failed");
		else
		{
			msg[MSG_CMD] = LM_DUMP; 
			strcpy(&msg[MSG_DATA], ""); 
			l_msg_cksum(msg, COMM_NUMREV, LM_TCP);
			network_write(d, msg, LM_MSG_LEN);
		}
	}
}
