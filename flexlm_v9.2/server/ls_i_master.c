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
 *	Module: $Id: ls_i_master.c,v 1.8 2003/04/18 23:47:59 sluu Exp $
 *
 *	Function: ls_i_master(ls, hostname)
 *
 *	Description: "Informs" another server that I am the master, returns
 *			his idea of who the master is, if it is not us.
 *
 *	Parameters:	(LM_SERVER *) ls - The server to notify
 *			(char *) hostname - Our host name
 *
 *	Return:		(char *) - NULL - OK, we are the master
 *				  <> NULL - Name of the master
 *
 *	M. Christiano
 *	5/19/88
 *
 *	Last changed:  7/14/98
 *
 */

#include <errno.h>
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
extern int ls_read_wait;	/* How long to wait for a solicited read */

char *
ls_i_master(ls, hostname)
LM_SERVER *ls;
char *hostname;
{
  char _msg[LM_MSG_LEN+1];	/* For ls_client_send */
  char *whoisit = NULL;		/* Assume all will be OK */
  char msgtype, *msg;
  int i;
  CLIENT_ADDR ca;

	(void)memset(&ca, '\0', sizeof(ca));
	(void) memset(_msg, '\0', sizeof(_msg));
	ca.is_fd = 1;
	ca.transport = LM_TCP;

	strncpy(&_msg[MSG_DATA], hostname, MAX_SERVER_NAME);/* LONGNAMES */
	ca.addr.fd = ls->fd1;
	ls_server_send(&ca, LM_I_MASTER, _msg);
	if (ls_serv_receive(&ca, &msgtype, &msg) <= 0)
	{
		return("");
	}
	if (msgtype == LM_TRY_ANOTHER)
	{
		char *whodidit = ls->name;
		/*
		*	    Either the majority was already up and elected
		*	    a master, or this guy knows about someone we 
		*	    don't know about yet.
		*/
		DLOG(("Finding master -- it's supposed to be %s or %s\n", 
		whodidit, msg));
		for (ls = ls_s_first(), i = 0; ls; ls = ls->next, i++)
		{
			if (!strcmp(msg, ls->name))
			{
/*
 *				If TIMEDOUT is set, forget this guy - he has 
 *				already timed out.  The other server will 
 *				see the light eventually.
 */
				if (!(ls->state & C_TIMEDOUT))
				{
					int update = 1;

					if ( (ls->fd1==LM_BAD_SOCKET) && 
					(!(ls->sflags & L_SFLAG_US)))
					{
						ca.addr.fd = ls_sconnect(msg, 0, &ls->exptime);
						if (ca.addr.fd != LM_BAD_SOCKET)
						{
							ls->fd1 = ca.addr.fd;
							ls->state |= 
							  (C_SOCK | C_SENT);
							ls_c_init(&ca, 
								COMM_NUMREV);
							DLOG((
				"I thought I was master, but %s said %s is\n",
								whodidit, msg));
						}
						else
						{ 		/* This guy is a joker */
							update = 0;
							LOG((
			"Master should be %s, but I can't connect\n", msg));
						}
					}
					if (update)
					{
						whoisit = ls->name;
						ls_s_setmaster(i);
					}
				}
				else 
				{ 
					DLOG(("But that server timed out!!\n"));
				}
				break;		
			} /* if */
		} /* for */
	}
	else if (msgtype != LM_OK)
	{
		return("");
	}
	return(whoisit);
}
