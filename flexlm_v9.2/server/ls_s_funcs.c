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
 *	Module: $Id: ls_s_funcs.c,v 1.8 2003/01/13 22:31:39 kmaclean Exp $
 *
 *	Function: ls_s_init(), ls_s_master(), ls_s_quorum(), ls_s_first(),
 *		  ls_s_havequorum(), ls_s_setmaster(), ls_s_imaster(),
 *		  ls_s_havemaster(), ls_s_masterup(),
 *
 *		  ls_s_find(), ls_s_dump(), ls_s_reset(), ls_s_down(),
 *		  ls_s_shut_if_client()
 *
 *		  ls_s_order(), ls_s_sconn(), ls_s_pconn()
 *
 *	Description:	Misc. functions dealing with servers and quorums.
 *
 *	Parameters:	(LM_SERVER *) master_list - the list of master nodes.
 *			(CLIENT_DATA *) client - The client data structures
 *			(int) fd - File descriptor
 *
 *	Return:		various.
 *
 *	M. Christiano
 *	1/8/94
 *
 *	Last changed:  1/8/99
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"

extern LM_QUORUM quorum;	/* The LM_QUORUM in this server */
#define q quorum

#define SLOG(x) if (q.debug) { DLOG(x); }

extern int ls_i_am_lmgrd;

/*
 *	ls_s_init() - Initialize this module
 */

extern int ls_dump_send_data;

#if !defined(NO_REDUNDANT_SERVER)
void
ls_s_init()
{
	if (getenv("FLEXLM_SERVER_DEBUG") != (char *) NULL)
	{
		q.debug = 1;
		ls_dump_send_data = 1;
	}
	else
		q.debug = 0;
}
#endif /* noredundant server */
/*----------------------------------------------------------------------------
 *
 *	Queries on the server database
 */

/*
 *	ls_s_master() - Return the master's name
 */

char *
ls_s_master()
{
  int i = q.master;

	if (q.list && (i >= 0 && i < MAX_SERVERS))
	{
		return(q.list[i].name);
	}
	else
		return("");
}

/*
 *	ls_s_qnum() - Return the number required for a quorum.
 */

ls_s_qnum()
{
	return(q.quorum);
}

/*
 *	ls_s_havequorum() - Do we have a quorum?
 */
#if !defined(NO_REDUNDANT_SERVER)

ls_s_havequorum()
{
	return((q.count > 0) && (q.count >= q.quorum));
}
/*
 *	ls_s_imaster() - Is the master us? (1 if so, 0 if not)
 */

ls_s_imaster()
{
	return((q.master >= 0) && (q.list[q.master].sflags & L_SFLAG_US));
}

/*
 *	ls_s_havemaster() - Have we picked a master? (1 if so, 0 if not)
 */
#endif /* NO_REDUNDANT_SERVER */
ls_s_havemaster()
{
	return((q.master >= 0));
}

/*
 *	ls_s_masterup() - Is the master connected? (1 if so, 0 if not)
 */

ls_s_masterup()
{
	return((q.list[q.master].state & C_CONNECTED));
}

/*
 *	ls_s_first() - Return the first server in the list
 */

LM_SERVER *
ls_s_first()
{
	return(q.list);
}

/*
 *	ls_s_find() - Find a server by fd1
 */

LM_SERVER *
ls_s_find(fd1)
LM_SOCKET fd1;
{
  LM_SERVER *ls;

	for (ls = q.list; ls; ls = ls->next)
		if (fd1 == ls->fd1) break;

	return(ls);
}

/*----------------------------------------------------------------------------
 *
 *	Operations on the server database
 */


/*
 *	ls_s_setmaster() - Set the master offset
 */

void
ls_s_setmaster(i)
int i;
{
	q.master = i;
}

/*
 *	ls_s_reset() - Reset the state of all servers and shut them down.
 */

void
ls_s_reset()
{
  LM_SERVER *ls;

	SLOG(("Resetting all servers\n"));
	for (ls = q.list; ls; ls = ls->next)
	{

		ls_s_down(ls);
	}
	q.count = q.quorum = 0;
	q.master = -1;
}
/*
 *	ls_s_shut_if_client() - Shuts down the server if this 
 *				CLIENT_DATA struct refers to it.
 */
#if !defined(NO_REDUNDANT_SERVER)

ls_s_shut_if_client(client)
CLIENT_DATA *client;
{
  int didit = 0;
  LM_SERVER *ls;

	for (ls = q.list; ls; ls = ls->next)
	{
	    if (client->addr.addr.fd == ls->fd1 || 
						client->addr.addr.fd == ls->fd2)
	    {
#ifdef UNIX
		extern int errno;
#endif
		didit = 1;
		SLOG(("ls_shut_if_client calling ls_s_down net_errno is %d\n",
                        net_errno));
		ls_s_down(ls);
		DLOG((lmtext("[fd%d] %s:%d\n"),
				ls->fd1 == client->addr.addr.fd ? 1 : 2,
				__FILE__, __LINE__));
		break;
	    }
	}
	return(didit);
}
#endif /* (NO_REDUNDANT_SERVER) */

/*
 *	ls_s_down() - Shut down connection to sever and clean up state
 */
void
ls_s_down(ls)
LM_SERVER *ls;
{
  CLIENT_ADDR ca;

	memset(&ca, 0, sizeof(ca));
	ca.is_fd = 1; 
	ca.transport = LM_TCP;
	SLOG(("shutting down server %s (ls_s_down() (fd:%d,%d))\n", ls->name,
		ls->fd1, ls->fd2));
	/* only fd1 has a client to be deleted [???] */
	if (ls->fd1 != LM_BAD_SOCKET)
	{
		ca.addr.fd = ls->fd1; 
		if (ls_lookup_client(&ca))
		{
			SLOG((" Calling delete client on fd1 %d\n", ls->fd1));
			ls_delete_client(&ca);
		}
	}
	if (ls->fd2 != LM_BAD_SOCKET)
	{
		ca.addr.fd = ls->fd2; 
		if (ls_lookup_client(&ca))
		{
			SLOG((" Calling delete client on fd2 %d\n", ls->fd2));
			ls_delete_client(&ca);
		}
	}
	if (ls->state & C_CONNECTED) 
	{
		q.count--;
		LOG((lmtext("Lost connection to %s\n"), ls->name));
		LOG_INFO((INFORM, "A daemon can no longer communicate  \
				with its peer on node %s, which can cause the  \
				clients to have to reconnect, or cause the  \
				number of daemons to go below the minimum  \
				number, in which case clients may \
				start exiting.  If the license daemons lose  \
				the connection to the master, they will kill  \
				all the vendor daemons; vendor daemons  \
				will shut themselves down."));
	}
	ls_down(&ls->fd1, "ls_s_down on fd1");
	ls_down(&ls->fd2, "ls_s_down on fd2");
	ls->state = 0;

}
void
ls_s_fd2_down(ls)
LM_SERVER *ls;
{
  CLIENT_ADDR ca;
	memset(&ca, 0, sizeof(ca));
	ca.addr.fd = ls->fd2;
	ca.is_fd = 1;
	ca.transport = LM_TCP;
	ls_delete_client(&ca);
	SLOG(("shutting down fd2 for server %s ((fd:%d,%d))\n", ls->name,
		ls->fd1, ls->fd2));
	ls_mask_clear(ls->fd2);
	if (ls->state & C_CONNECTED) 
	{
		q.count--;
	}
	ls_down(&ls->fd2, "ls_s_down on fd2");
	ls->state &= ~C_CONNECTED;
}

#if !defined(NO_REDUNDANT_SERVER)

/*
 *	ls_s_dump() - Dump the server and quorum data
 */

void
ls_s_dump()
{
  int i;
  LM_SERVER *ls;

	DLOG((lmtext("SERVER QUORUM DATA, connected to")));
	_DLOG((lmtext(" %d, quorum: %d, master: %d\n\n"), q.count, q.quorum, 
								q.master));
	for (i = 0, ls = q.list; ls; ls = ls->next, i++)
	{
		if (i == (q.master)) 
		{
			_DLOG((lmtext(" MASTER ")));
		}
		else		  
		{
			_DLOG(("        "));
		}

		_DLOG((lmtext("Host: %8s, port: %d,"), ls->name, ls->port));
		_DLOG((lmtext("fd1: %d, fd2: %d"), ls->fd1, ls->fd2));
		_DLOG((lmtext(" state (%x:"), ls->state));
		if (ls->state & C_CONNECTED) 
		{ 
			_DLOG((lmtext(" CONNECTED"))); 
		}
		else
		{
			if (ls->state & C_SOCK)     _DLOG((lmtext(" SOCK")));
			if (ls->state & C_SENT)     _DLOG((lmtext(" SENT")));
			if (ls->state & C_CONNECT1) _DLOG((lmtext(" CONNECT1")));
			if (ls->state & C_CONNECT2) _DLOG((lmtext(" CONNECT2")));
		}
		if (ls->state & C_MASTER_READY) _DLOG((lmtext(" MASTER_READY")));
		_DLOG((")\n"));
	}
}
#endif /* (NO_REDUNDANT_SERVER) */
/*
 *	ls_s_order() - process the LM_ORDER message from another server
 */
void
ls_s_order(cmd)
char *cmd;
{
  int i;
  LM_SERVER *ls;

	SLOG(("%s at %s, q.n_order: %d\n", &cmd[MSG_ORDER_HOST], 
					&cmd[MSG_ORDER_N], q.n_order));
	if (q.alpha_order == 0 && q.n_order > 0)
	{
	  int n = atoi(&cmd[MSG_ORDER_N]);
/*
 *		Make sure his order is the same as ours.
 *		If not, revert to alpha order.
 */
		for (ls = q.list, i=1; ls; ls = ls->next, i++)
			if (!strcmp(ls->name, &cmd[MSG_ORDER_HOST])) break;
		if (i == n) q.n_order--;
		else
		{
			DLOG(("we matched at %d, msg said %d\n", i, n));
			q.alpha_order++;
			q.n_order = 0;	/* Don't need more */
			LOG((lmtext("License File SERVER line order mismatch.\n")));
			LOG((lmtext("Using alphabetical order\n")));
			LOG_INFO((INFORM, "The SERVER host \
					names in the license files on the \
					several server hosts are in different \
					order.  Rather than using the order \
					of the hosts as the preferred master \
					server, the alphabetical order of the \
					host names will be used."));
		}
	}
}

#if !defined(NO_REDUNDANT_SERVER)

/*
 *	ls_s_sconn() - Process the incomming connection from the other server
 *			on the secondary (fd2) connection.
 */

void
ls_s_sconn(ls, ca)
LM_SERVER *ls;
CLIENT_ADDR *ca;
{
  CLIENT_ADDR tmp_ca;

	SLOG(("SHELLO on fd2 (%d) from %s\n", ca->addr.fd, ls->name));
	ls->fd2 = ca->addr.fd;
	ls->state |= C_CONNECT2;
	if (ls->state & C_CONNECT1)
	{
	  LM_SERVER *lms;

		ls->state |= C_CONNECTED;
		q.count++;
		if (q.master >= 0)
		{
		    lms = &q.list[q.master];
		    if (ls_i_am_lmgrd && (lms->sflags & L_SFLAG_US) && 
			lms->state & C_MASTER_READY)
				(void) ls_mast_rdy(ls, lms->name);
		}
		LOG((lmtext("Connected to %s\n"), ls->name));
		LOG_INFO((INFORM, "This daemon has completed the connection to \
						its peer on node %s."));
	}
	else if ((ls->state & (C_SOCK | C_SENT)) == 0)
	{
#if 1
/*
 *		This is a new server
 */
		SLOG(("We had not initiated connection to %s, doing it now\n", 
								ls->name));
		ls->fd1 = ls_sconnect(ls->name, ls->port, &ls->exptime);
		if (ls->fd1 != LM_BAD_SOCKET)
		{
			(void) memset(&tmp_ca, '\0', sizeof(tmp_ca));
			tmp_ca.is_fd = 1;
			tmp_ca.transport = LM_TCP;
			tmp_ca.addr.fd = ls->fd1;
			(void)ls_c_init(&tmp_ca, COMM_NUMREV);
			ls->state |= (C_SOCK | C_SENT);
		}
		else
		{
			ls->state = 0;
			ls_down(&ls->fd2, "on second socket");
			LOG((lmtext(
			  "Cannot connect to %s after incoming connection\n"),
								ls->name));
			LOG_INFO((INFORM, "This daemon was unable to connect \
					to the named daemon after the incoming \
					connection from the other daemon was \
					processed."));
		}
#endif
	}
	else
	{
	  int x;	/* dummy for l_get_date() */
#ifdef THREAD_SAFE_TIME
	  struct tm tst;
#endif
/*
 *		Update expiration time, in case 
 *		fd1 sat there for a long time.
 */
#ifdef THREAD_SAFE_TIME
		l_get_date(&x, &x, &x, &ls->exptime, &tst);
#else /* !THREAD_SAFE_TIME */
		l_get_date(&x, &x, &x, &ls->exptime);
#endif
	}
}
#endif /* (NO_REDUNDANT_SERVER) */


/*
 *	ls_s_pconn() - Process the connection on the primary (fd1) connection
 */

void
ls_s_pconn(ca)
CLIENT_ADDR *ca;
{
  LM_SERVER *ls;

        for (ls = q.list; ls; ls = ls->next)
	{
		if (ca->addr.fd == ls->fd1)
		{
			SLOG(("Connection complete on fd1 to %s (fd1:%d fd2:%d)\n", ls->name, ls->fd1, ls->fd2));
			ls->state |= C_CONNECT1;
			if (ls->state & C_CONNECT2)
			{
			  LM_SERVER *lms;

				ls->state |= C_CONNECTED;
				q.count++;
				if (q.master >= 0)
				{
					lms = &q.list[q.master];
					if (ls_i_am_lmgrd && 
						(lms->sflags & L_SFLAG_US) && 
						(lms->state & C_MASTER_READY))
						(void) ls_mast_rdy(ls, lms->name);
				}
				LOG((lmtext("Connected to %s\n"), ls->name));
				LOG_INFO((INFORM, "This daemon is connected \
							to its peer on \"node\"."));
			}
			break;
		}
	}
}

/*
 *	ls_s_mready() - Master node is ready
 */

void
ls_s_mready(cmd, user)
char *cmd;
CLIENT_DATA *user;
{
  char msg[LM_MSG_LEN]; /* For ls_client_send */
  LM_SERVER *ls;
  int i;

	SLOG(("Got MASTER_READY from %s\n", &cmd[MSG_DATA]));
	(void) memset((char *)msg, 0, sizeof(msg));
	if ((q.master >= 0) && !strcmp(q.list[q.master].name, &cmd[MSG_DATA]))
	{
		SLOG(("Sent OK to (known) master (%s)\n", q.list[q.master].name));
		q.list[q.master].state |= C_MASTER_READY;
		msg[MSG_DATA] = 0;
		ls_client_send(user, LM_OK, msg);
	}
	else if (q.master < 0 || (((q.list[q.master].state)
						     & C_MASTER_READY) == 0))
	{
		for (ls = q.list, i=0; ls; ls = ls->next, i++)
		{
			if (!strcmp(ls->name, &cmd[MSG_DATA]))
			{
				SLOG(("Set %s to master (C_MASTER_READY)\n", 
								ls->name));
				q.master = i;
				ls->state |= C_MASTER_READY;
				break;
			}
		}
		if (ls) 
		{
			SLOG(("Sent OK to master (%s)\n", ls->name));
			msg[MSG_DATA] = 0;
			ls_client_send(user, LM_OK, msg);
		}
		else
		{
			SLOG(("Didn't find %s in server database\n", 
							&cmd[MSG_DATA]));
		}
	}
	else	
/* 
 *		WE already had a master in C_MASTER_READY state - eeks
 *		break the connections to the old master and this
 *		new guy - everyone will come up again.
 *		v0.9:	If any of the vendor daemon's sockets still
 *			exist on the master node, this is probably 
 *			someone trying to cheat!  Give them time to 
 *			timeout, and if they are still there, log the 
 *			message, but DO NOT RESTART THE DAEMONS!
 */
	{
		DLOG(("2 MASTERS: %s (master: \"%s\") responding with WHAT\n", 
					&cmd[MSG_DATA], q.list[q.master].name));
		msg[MSG_DATA] = 0;
		ls_client_send(user, LM_WHAT, msg);
		for (ls = ls_s_first(); ls; ls = ls->next)
		{
			if (!(ls->sflags & L_SFLAG_US))
			{
				ls_down(&ls->fd1, "on first socket");
				ls_down(&ls->fd2, "on second socket");
				ls->state = 0;
			}
		}
		q.master = -1;
		q.count = 1;
	}
	SLOG(("Done ls_s_mready()\n"));
}

/*
 *	DEBUG: Check the quroum structure for inconsistencies
 */

#ifndef RELEASE_VERSION
void
ls_q_check()
{
  LM_SERVER *ls;
  int i = 0;
  int inconsistent = 0;

	for (ls = ls_s_first(); ls; ls = ls->next) 
	{
		if (ls->state & C_CONNECTED) i++;

		if (!(ls->sflags & L_SFLAG_US) &&
		    (((ls->state & (C_SOCK | C_CONNECTED)) && (ls->fd1 == LM_BAD_SOCKET)) ||
		    (((ls->state & (C_SOCK | C_CONNECTED)) == 0) && 
						(ls->fd2 != LM_BAD_SOCKET)) ||
		    ((ls->state & (C_CONNECT2 | C_CONNECTED)) && (ls->fd1 == LM_BAD_SOCKET)) ||
		    (((ls->state & (C_CONNECT2 | C_CONNECTED)) == 0) && 
						(ls->fd2 != LM_BAD_SOCKET))))
							inconsistent++;
	}
	if (inconsistent || (i != q.count))
	{
		DLOG(("ERROR: Inconsistent server quorum structure\n"));
		DLOG(("inconsistent: %d, i: %d, q.count: %d\n",
				inconsistent, i, q.count));
		ls_s_dump();
	}
}
#endif	/* !RELEASE_VERSION */
