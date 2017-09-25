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
 *	Module: $Id: ls_quorum.c,v 1.7 2003/01/13 22:31:39 kmaclean Exp $
 *
 *	Function: ls_quorum(master_list, master_name)
 *
 *	Description:	Connect sockets to a quorum of the servers
 *
 *	Parameters:	(LM_SERVER *) master_list - the list of master nodes.
 *			(char *) maste_name - The name of the master node (if
 *						known, otherwise "")
 *
 *	Return:		(int) - 1 if a master was selected, 0 if not
 *			master_list - filled in with descriptors.
 *
 *	M. Christiano
 *	3/6/88
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
extern int ls_i_am_lmgrd;	/* Are we lmgrd ?? */
LM_QUORUM quorum;		/* The one and only LM_QUORUM in this server */
#define q quorum
static LM_SERVER *ml = (LM_SERVER *) NULL;
static int first = 1;
static void do_reread(LM_SERVER *master_list);
#define SLOG(x) if (q.debug) { DLOG(x); }

#ifndef NO_REDUNDANT_SERVER
ls_quorum(LM_SERVER *master_list, char *master_name, int reread)
{
  LM_SERVER *ls;
  CLIENT_ADDR server_ca;
  int pass = 0;
  int i;
#if 0
  int reportpass = 10;	/* How many passes to do before reporting "Trying..." */
#endif
  int already_connected = 0;	/* How many were already connected */
  int already_got_us = 0;	/* Only assume connection to ourselves once*/ 

	if (reread)
	{
		do_reread(master_list);
		return 0; /* return value unused */

	}
	memset(&server_ca, '\0', sizeof(server_ca));
	server_ca.is_fd = 1;
	server_ca.transport = LM_TCP;
	q.count = 0;
	q.quorum = 0;
	if (first)
	{
/*-
 *		If the order was wrong before, it is still wrong ...
 *		WRONG: the other server might have a new license file!
 *		q.alpha_order re-initialized to 0 outside here in v2.38
 */
		first = 0;
	}
	q.alpha_order = 0;	/* Use order of servers in file */
	q.master = -1;		/* NO master yet */
/*
 *	Store the master list and find out how many make a quorum
 */
	for (ls = master_list; ls; ls = ls->next) q.quorum++;
/*- 
 *	Don't update q.quorum yet -- it is used in the malloc/bcopy below 
 */
	if (ml == (LM_SERVER *) NULL)
	{
		ml = (LM_SERVER *) LS_MALLOC(q.quorum * sizeof(LM_SERVER));
		bcopy((char *) master_list, (char *) ml, 
						q.quorum * sizeof(LM_SERVER));	
		for (ls = ml; ls->next; ls++)		/* Fix up the links */
		{
			ls->next = ls + 1;
		}
	}

	q.quorum = q.quorum/2 + 1;	/* Must be a majority */
	q.list = master_list = ml;
	SLOG(("ls_quorum: quorum set to %d\n", q.quorum));

/*
 *	Before we start, we must receive "quorum" order messages from
 *	"quorum - 1" servers (all but ourselves).
 */
	q.n_order = q.quorum * (q.quorum - 1);
/*
 *	Now, try to connect to all the other servers
 */
	for (ls = master_list; ls; ls = ls->next)
	{	
		SLOG(("connecting to %s\n", ls->name));
		if (ls_on_host(ls->name) && !already_got_us)	
		{
			SLOG(("it's us %s\n", ls->name));
			/* it's us */
			already_got_us = 1; /*don't repeat this*/
			q.count++;
			ls->fd1 = ls->fd2 = LM_BAD_SOCKET;
			ls->state = (C_SENT | C_CONNECT1 | C_CONNECT2 
						| C_CONNECTED); 
						/* Completely connected */
			ls->sflags |= L_SFLAG_US;
		}
		else if (ls->fd1 != LM_BAD_SOCKET) 
		{ 		/* Already connected, count it */
			SLOG(("already connected to %s\n", ls->name));
			SLOG(("ls_quorum: %s connection already started\n", 
								ls->name));
			if (ls->state & C_CONNECTED)
			{
				SLOG(("ls_quorum: %s already connected\n", 
								ls->name));
				already_connected++;
				q.count++; 
			}
		}
		else if (ls->fd1 == LM_BAD_SOCKET)  /* Don't try ones 
							already connected */
		{
			SLOG(("calling sconnect to %s\n", ls->name));
			server_ca.addr.fd = 
				ls_sconnect(ls->name, ls->port, &ls->exptime);
			ls->fd1 = server_ca.addr.fd; /* Flag whether we got it */
			ls->state &= ~(C_SOCK | C_SENT | C_CONNECT1 | 
				    C_CONNECTED | C_MASTER_READY | C_TIMEDOUT |
				    C_NOHOST);
			if ( server_ca.addr.fd != LM_BAD_SOCKET) 
			{
				ls_c_init(&server_ca,COMM_NUMREV);
				ls->state |= (C_SOCK | C_SENT);
			}
			else if (_lm_errno == BADHOST)
			{
				SLOG(("setting NOHOST for %s\n", ls->name));
				ls->state = C_NOHOST;
			}
		}
	}
	ls = master_list;
/*
 *	If we don't have a quorum, keep trying to connect
 */
	while (ls)
	{

/*
 *		Keep trying connections until we have a quorum
 *		Don't try hosts that are either already connected or
 *		ones that can't be found in the network database.
 */
		if (!(ls->sflags & L_SFLAG_US) && ls->fd1 == LM_BAD_SOCKET
						&& !(ls->state & C_NOHOST))
		{
			if (pass == 0)
			{
				LOG((lmtext("Trying connection to %s\n"), 
								ls->name));
				LOG_INFO((INFORM, " The daemon is attempting \
					a connection to node %s."));
			}
			SLOG(("still trying sconnect to %s\n", ls->name));
		
			server_ca.addr.fd = 
				ls_sconnect(ls->name, ls->port, &ls->exptime);

			    /* Flag whether we got it */
			ls->fd1 = server_ca.addr.fd; 

			ls->state &= ~(C_SOCK | C_SENT | C_CONNECT1 | 
				    C_CONNECTED | C_MASTER_READY | C_TIMEDOUT);
			if (server_ca.addr.fd != LM_BAD_SOCKET) 
			{
				ls_c_init(&server_ca, COMM_NUMREV);
				ls->state |= (C_SOCK | C_SENT);
			}
		}
		ls = ls->next;
	}
	if (q.count > q.quorum)
		q.n_order = q.quorum * (q.count	- 1);

/*
 *	If we had already connected to some of the servers, we
 *	had received their order messages
 */
	if (already_connected > 0)
	{
		q.n_order -= (q.quorum * already_connected);
	}
/*
 *	If a master is already selected, fill it in now (this is only
 *	true in vendor daemons).
 */
	if (master_name && *master_name)
	{
	    for (i = 0, ls = q.list; ls; ls = ls->next, i++)
	    {
		if (
#ifdef FIFO
			lm_job->options->commtype == LM_LOCAL || 
#endif /* FIFO */
			!strcmp(master_name, ls->name))
		{
			q.master = i;
			break;
		}
	    }
	}
	return(q.master >= 0);
}
#endif /* no redundant server */

static
void
do_reread(LM_SERVER *master_list)
{
 int i;
	for (i = 0; i < 3; i++)
		if (*master_list[i].name)
			l_zcp(quorum.list[i].name, master_list[i].name, 
				MAX_HOSTNAME);
}
