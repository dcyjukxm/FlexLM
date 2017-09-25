/******************************************************************************

	    COPYRIGHT (c) 1997, 2003  by Macrovision Corporation.
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
 *	Module: $Id: l_ping.c,v 1.2 2003/01/13 22:13:13 kmaclean Exp $
 *
 *	Function:	l_ping
 *
 *	Description: 	return 0 if can connect to port@host.
 *
 *	Parameters:	port, hostname
 *
 *	Return:		FLEXlm errno if error, else 0 if success
 *
 *	D. Birns
 *	7/7/97
 *
 *	Last changed:  6/7/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>


l_ping(job, port, hostname, transport)
LM_HANDLE *job;
int port;	/* in natural, host order */
char *hostname;
int transport;
{
  COMM_ENDPOINT endpoint;
  int s;


	if (!hostname || !*hostname)
		return LM_BADPARAM;
	endpoint->transport_addr.port = htons(port);
	endpoint->transport = transport;
	if (l_connect_endpoint(job, &endpoint, hostname))
		lc_disconn(job, 1);
	else
		return job->lm_errno;
}
