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
 *	Module: $Id: ls_serv_conn.c,v 1.5 2003/01/13 22:31:39 kmaclean Exp $
 *
 *	Function: ls_serv_conn(ca, msg, client)
 *
 *	Description: Processes connections from other servers
 *
 *	Parameters:	(CLIENT_ADDR *) ca - The new connection's address 
 *			(char *) msg - The message data
 *			(CLIENT_DATA *) client - The client data
 *
 *	Return:		(char *) - Name of DAEMON to be connected to.
 *				   Daemon status updated.
 *
 *	M. Christiano
 *	2/18/88
 *
 *	Last changed:  1/8/99
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "ls_comm.h"    /* Server-server comm */
#include <stdio.h>
static char _sp_[MAX_DAEMON_NAME+1];

extern LM_QUORUM quorum;	/* The LM_QUORUM in this server */
#define SLOG(x) if (quorum.debug) { DLOG(x); }
#define _SLOG(x) if (quorum.debug) { _DLOG(x); }

char *
ls_serv_conn(ca, msg, client)
CLIENT_ADDR *ca;
char *msg;
CLIENT_DATA *client;
{
  char _msg[LM_MSG_LEN+1];      /* For ls_server_send */
  LM_SERVER *ls = 0;
  char *name = &msg[MSG_HEL_HOST];
  char *daemon = &msg[MSG_HEL_DAEMON];
  char *sp = (char *) 0;

	memset(_msg, 0, sizeof(_msg));
	if (*name == 0)
	{
		LOG((lmtext("BAD SCONNECT message: (%c v%c.%c %s %s)\n"), 
				msg[MSG_CMD], msg[MSG_HEL_VER], 
				msg[MSG_HEL_VER+1], &msg[MSG_HEL_HOST], 
				&msg[MSG_HEL_DAEMON]));
		LOG_INFO((ERROR, "An invalid "SERVER CONNECT" message was \
				received."));
		DLOG(("responsding with WHAT\n"));
		ls_server_send(ca, LM_WHAT, _msg);
	}
	else
	{
/*
 *	    This is a good SHELLO message, now see if it is for
 *	    us.  If not, we are a master and it is for
 *	    one of our vendor servers.
 */
	    if (!strcmp(lm_job->vendor, daemon))
	    {
	    	for (ls = ls_s_first(); ls; ls = ls->next)
	    	{
			if (!strcmp(ls->name, name))
			{
				if (ls->fd2 != LM_BAD_SOCKET)
				{
					DLOG(("OOPS: got incoming connection on fd2 while we were already connected! (name %s fd: %d)\n", name, ls->fd2));
					DLOG(("\t%s state %d ca->addr.fd %d\n", name, ls->state, ca->addr.fd));
					ls_s_fd2_down(ls);
				}
				ls_server_send(ca, LM_SHELLO_OK, _msg); 
				ls_s_sconn(ls, ca);
				break;	/* Got it */
			}
		}
			
		if (!ls) 
		{
			SLOG(("Can't find server for message for daemon"));
			_SLOG(("%s or %s from %d\n", daemon, name, 
							ca->addr.fd));
			ls_server_send(ca, LM_WHAT, _msg); 
		}
	    }
	    else
	    {
		strncpy(_sp_, daemon, MAX_DAEMON_NAME);
		_sp_[MAX_DAEMON_NAME] = '\0';
		sp = _sp_;
	    }
	}
	return(sp);
}
