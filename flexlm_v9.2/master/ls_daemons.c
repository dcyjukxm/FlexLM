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
 *	Module: $Id: ls_daemons.c,v 1.4 2003/01/13 22:26:49 kmaclean Exp $
 *
 *	Function: ls_daemons(fd, daemons)
 *
 *	Description: Send the DAEMON port numbers to another server.
 *
 *	Parameters:	(int) fd - The socket to send the daemon info to.
 *			(DAEMON *) daemons - List of DAEMONs we started
 *
 *	Return:		None 
 *
 *	M. Christiano
 *	5/31/88
 *
 *	Last changed:  10/23/96
 *
 */

#ifndef lint
static char *sccsid = "ls_daemons.c:3.10";
#endif

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsmaster.h"
#include "../machind/lsfeatur.h" 
#include "../app/ls_aprot.h" 
#include "l_m_prot.h" 

void 
ls_daemons(fd, daemons)
int fd;			/* Socket for transmission */
DAEMON *daemons;	/* The DAEMONs we started */
{
  DAEMON *start;
  char msg[LM_MSG_LEN+1];
  CLIENT_ADDR ca;

	memset((char *) &ca, '\0', sizeof(ca));
	ca.is_fd = 1;
	ca.transport = LM_TCP;
	ca.addr.fd = fd;

/*
 *	Send the list of DAEMON port numbers to another server
 */
	for (start = daemons; start; start = start->next)
	{
		memset(msg, 0, sizeof(msg));
		strncpy(&msg[MSG_SPP_NAME], start->name, MAX_DAEMON_NAME);
		l_encode_int(&msg[MSG_SPP_PORT], start->tcp_port);
		l_encode_int(&msg[MSG_SPP_UDP_PORT], start->udp_port);
		ls_server_send(&ca, LM_DAEMON, msg); 
	}
}
