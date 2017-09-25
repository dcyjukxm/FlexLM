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
 *	Module: $Id: ls_mast_rdy.c,v 1.7 2003/04/18 23:47:59 sluu Exp $
 *
 *	Function: ls_mast_rdy(ls, hostname)
 *
 *	Description: "Informs" another server that I am the master and
 *				I am ready.
 *
 *	Parameters:	(LM_SERVER *) ls - The server to notify
 *			(char *) hostname - Our host name
 *
 *	Return:		(int) - 0 - OK, we are the master
 *				<> 0 - Bad.
 *
 *	M. Christiano
 *	5/19/88
 *
 *	Last changed:  10/23/96
 *
 */

#include <errno.h>
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
extern int ls_read_wait;	/* How long to wait for a solicited read */

ls_mast_rdy(ls, hostname)
LM_SERVER *ls;
char *hostname;
{
  char _msg[LM_MSG_LEN+1];	/* For ls_server_send */
  char msgtype, *msg;
  CLIENT_ADDR ca;

	(void) memset((char *) &ca, '\0', sizeof(ca));
	(void) memset((char *)_msg, '\0', sizeof(_msg));
	ca.is_fd = 1;
	ca.transport = LM_TCP;
	ca.addr.fd = ls->fd1;
	
	(void) strncpy(&_msg[MSG_DATA], hostname, MAX_SERVER_NAME);/* LONGNAMES */
	msgtype = LM_NO_MESSAGE;
	ls_server_send(&ca, LM_MASTER_READY, _msg);
	if (ls_serv_receive(&ca, &msgtype, &msg) <= 0)
		return(1);
	if (msgtype == LM_OK)
		return(0);
	else
		return(1);
}
