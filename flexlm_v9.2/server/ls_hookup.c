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
 *	Module: $Id: ls_hookup.c,v 1.6 2003/04/18 23:47:59 sluu Exp $
 *
 *	Function: ls_hookup(cmd, input_ca, master, user, select_mask)
 *
 *	Description: Performs server "hookup" functions for redundant operation
 *
 *	Parameters:	(char *) cmd - The incoming command
 *			CLIENT_ADDR *input_ca - The address the input came from.
 *			(char *) master - The master node name.
 *			(CLIENT_DATA *) user - A particular user
 *
 *	Return:		(SELECT_MASK *) select_mask - Updated for any file 
 *								open/closes
 *
 *	M. Christiano
 *	5/19/88 - (from ls_m_process.c)
 *
 *	Last changed:  1/8/99
 *
 */
#ifndef lint
static char *sccsid = "ls_hookup.c:3.16";
#endif

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lmselect.h"

extern int ls_i_am_lmgrd;


char *
ls_hookup(cmd, input_ca, master, user, select_mask)
char *cmd;
CLIENT_ADDR *input_ca;
char *master;
CLIENT_DATA *user; /*A particular user*/
SELECT_MASK select_mask;
{
  LM_SERVER *ls;
  char msg[LM_MSG_LEN];	/* For ls_client_send */
  char *retval = 0;		/* 0 for success */

	(void) memset(msg, '\0', sizeof(msg));
	switch(cmd[MSG_CMD])
	{

		case LM_NEWSERVER:
/*
 *			Our connection ended up at a non-topdog server
 *			on the other node.  We need to shut this one
 *			down and try to connect again.
 */
			ls = ls_s_find(input_ca->addr.fd);
			if (ls)
			{
				int tfd = input_ca->addr.fd;
				ls->state = 0;
				ls_down((LM_SOCKET *)&tfd, "in ls_hookup");
				ls->fd1 = ls_sconnect(ls->name, ls->port, 
								&ls->exptime);
				if (ls->fd1 != LM_BAD_SOCKET) 
				{
					ls->state |= (C_SOCK | C_SENT);
				}
				break;
			}

		case LM_ORDER:
/*
 *			This is a server ordering message from another
 *			server.  See if we still need it.
 */
			ls_s_order(cmd);
			break;

		case LM_SHELLO:
/*
 *			Find this host in the table, and see if we have
 *			connected to him yet.
 */
			*(user->name) = '\0';	/* NOT a user */
			retval = ls_serv_conn(input_ca, cmd, user);
			break;

		case LM_SHELLO_OK:

			ls_s_pconn(input_ca);
			break;

		case LM_I_MASTER:

			if (!strcmp(master, &cmd[MSG_DATA]))
			{
				msg[MSG_DATA] = 0;
				ls_client_send(user, LM_OK, msg);
			}
			else if (*master == 0)
			{
/*
 *				Tell this server it's OK, even though we
 *				don't really know yet.
 */
				msg[MSG_DATA] = 0;
				ls_client_send(user, LM_OK, msg);
			}
			else
			{
				(void) strncpy(&msg[MSG_DATA], master, 
							MAX_SERVER_NAME);/* LONGNAMES */
				ls_client_send(user, LM_TRY_ANOTHER, msg);
/*
 *				If we are an application server, this
 *				is fatal.  Exit and let the master server
 *				restart us.
 */
				if (!ls_i_am_lmgrd)
					ls_go_down(EXIT_BICKER);
			}
			break;

		case LM_MASTER_READY:

/*
 *			The master node is "ready".  Now we can start
 *			the vendor daemons.
 */
			ls_s_mready(cmd, user);
			break;

		case LM_TRY_ANOTHER:
/*
 *			This should only happen in vendor servers when
 *			they connect to the master on the other node.
 *			HOWEVER, it could also happen on the connect to
 *			another vendor server, if it doesn't have the
 *			right idea of who is the master.
 */
			if (!ls_i_am_lmgrd)
			{
			  int port;

				port = atoi(&cmd[MSG_TRY_TCP_PORT]);
				for (ls = ls_s_first(); ls; ls = ls->next)
					if (ls->fd1 == input_ca->addr.fd) break;
				if (ls)
				{
				  CLIENT_ADDR ca;

					if (port == 0) port = ls->port;
					memset(&ca, 0, sizeof(ca));
					ca.is_fd = 1;
					ca.addr.fd = ls->fd1;
					ca.transport = LM_TCP;
					ls_delete_client(&ca);
					ls_mask_clear(ls->fd1);
					ls_down(&ls->fd1, "on TRY_ANOTHER");
					ls->fd1 = ls_sconnect(ls->name, port, 
								&ls->exptime);
					if (ls->fd1 == LM_BAD_SOCKET)
/*-
 *					V2.26: bug #276:  if this fails, it is
 *					probably because the other lmgrd got
 *					our request while an "old" copy of the
 *					vendor daemon was still running.  Try
 *					it again, in case a newer copy is
 *					running now.  DO NOT update the
 *					timeout time, so that we don't end
 *					up looping here forever.
 */
					{
					  long _junk;
					    ls->state &= ~(C_SOCK | C_SENT);
					    ls->fd1 = ls_sconnect(ls->name, 
							ls->port, &_junk);
					}
					else
					{
						ls->state |= (C_SOCK | C_SENT);
						ca.addr.fd = ls->fd1;
						ls_c_init(&ca, COMM_NUMREV);
						MASK_SET(select_mask, ls->fd1);
					}
				}
			}
			break;

		case LM_NO_SUCH_FEATURE:
		case LM_NO_CONFIG_FILE:
/*
 *		The license daemon wasn't able to find our DAEMON name
 */
			if (!ls_i_am_lmgrd)
			{
			  char *name = "???";

			    for (ls = ls_s_first(); ls; ls = ls->next)
				if (ls->fd1 == input_ca->addr.fd) break;
			    if (ls) name = ls->name;

			    if (cmd[MSG_CMD] == LM_NO_CONFIG_FILE)
			    {
				LOG((lmtext("license daemon on %s unable to locate DAEMON line for %s, exiting\n"), 
							name, lm_job->vendor));
				LOG_INFO((CONFIG, "The license daemon on \
					node %s was unable to locate the \
					the DAEMON line for our daemon's \
					name (%s); thus all the vendor \
					daemons will be shut down. \
					Locate the configuration file and be \
					sure that the DAEMON line is present, \
					then re-start all the license  \
					daemons."));
			        ls_go_down(EXIT_BADDAEMON);
			    }
			    else
			    {
				LOG((lmtext("WARNING - license daemon on %s has not started %s\n"),
							name, lm_job->vendor));
				LOG_INFO((INFORM, "The license daemon on \
					node %s has not started our vendor \
					daemon (%s) yet.  This will usually \
					only happen on server nodes that \
					are running under a very heavy load."));
			    }
			}
			else
			{
				msg[MSG_DATA] = 0;
				DLOG(("WHAT response to NO_SUCH_FEATURE msg"));
				ls_client_send(user, LM_WHAT, msg);
			}
			break;

		default:
			DLOG(("WHAT response to unknown msg 0x%x\n", 
				cmd[MSG_CMD]));
			msg[MSG_DATA] = 0;
			ls_client_send(user, LM_WHAT, msg);
			break;
	}
	return(retval);
}
