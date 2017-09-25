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
 *	Module: $Id: ls_ck_client.c,v 1.8 2003/01/13 22:22:34 kmaclean Exp $
 *
 *	Function: ls_ck_client(select_mask, s)
 *
 *	Description:	Writes to all clients periodically to detect dead nodes.
 *
 *	Parameters:	(SELECT_MASK) select_mask - Mask of file descriptors 
 *						    to select on.
 *			(int) s - The socket we accept connections on.
 *			(global CLIENT_DATA) _client - Global client structure,
 *					to detect who has finished connecting.
 *
 *	Return:		None.
 *			All file descriptors corresponding to clients have
 *			a LM_HEARTBEAT message written to them.
 *			
 *
 *	M. Christiano
 *	3/13/89
 *
 *	Last changed:  8/10/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lmselect.h"
#include "lsfeatur.h"
#include "ls_adaem.h"
#include "ls_aprot.h"
#include "ls_log.h"

#ifndef NO_UIO_H
#include <sys/uio.h>		/* To keep lint happy */
#endif

#ifdef PC 
#include <winsock.h>
#else
#include <sys/socket.h>
#endif
static int client_queued lm_args(( CLIENT_DATA *));


/*ls_ck_udp_clients:
 *
 * This function has two modes -- first it is called from ls_all_clients()
 * with *currtime set to a real time.  This time is used to expire clients.
 * from ls_all_clients, we can't actually delete clients, since we'd
 * be deleting from a malloc'd tree which we're currently walking -- so
 * I make a static list in this function.
 * Then, after ls_all_clients is done, it is called once again, with 
 * 0,0 args, and the list is walked, and clients are deleted from the
 * list 
 */
int
#ifdef ANSI
ls_ck_udp_clients( CLIENT_DATA *cd, void *currtime)
#else
ls_ck_udp_clients(cd, currtime)
CLIENT_DATA *cd;
void *currtime; /*if Non-ZERO -- mark for delete -- if zero -- do delete*/
#endif
{
  int timeout = -1;
  int udp = 0;
  typedef struct dlist {
	    CLIENT_DATA *cd;
	    struct dlist *next;
	} DLIST;
  static DLIST *delete_list = (DLIST *)NULL;
  static DLIST *d = (DLIST *)NULL;
  extern int ls_allow_borrow;

	if (cd)
	{
		if (cd->addr.transport == LM_UDP)
		{
			udp = 1;
			timeout = cd->udp_timeout;
		}
/*
 *		ls_allow_borrow was not used.  It's used now
 *		to make licenses never timeout based on LM_A_TCP_TIMEOUT
 */
		else if ((cd->addr.transport == LM_TCP) && !ls_allow_borrow)
		{
		  LM_SERVER *ls;
			for (ls = ls_s_first(); ls; ls = ls->next)	
			{
				if ((ls->fd1 == cd->addr.addr.fd) ||
					(ls->fd2 == cd->addr.addr.fd))
					return 1; /* it's a redundant server */
			}
			timeout = cd->tcp_timeout;
		}
		else return 1;
	}
	
	if (currtime) /*harvest for delete_list*/
	{
	  int elapsed_time = *(unsigned int *)currtime - cd->lastcomm;
	    if (elapsed_time < 0)
		elapsed_time = 0;
	    if ((cd->lastcomm != LS_DELETE_THIS_CLIENT) && 
		(timeout > 0) && (elapsed_time > (unsigned) timeout))
	    {
		if (client_queued(cd) ) /* nothing */;
		else
#ifdef SUPPORT_UDP
		if (udp) ls_closeout_this_udp_client(cd);
		else 
#endif /* SUPPORT_UDP */
		{
			ls_closeout_this_tcp_client(cd, 0, 0);
			cd->lastcomm = LS_DELETE_THIS_CLIENT;
		}

/* 
 *		NOTE:  the TCP closeout sets len to 0.  That indicates
 *		we did NOT get a message from this client, and the 
 *		the reportlog will log as necessary.
 */
			    
	    }
/*
 *		Add to delete_list
 */
	    if (cd->lastcomm == LS_DELETE_THIS_CLIENT)
	    {
		if (!delete_list)
		{
		    delete_list = (DLIST *) LS_MALLOC(sizeof (DLIST));
		    d = delete_list;
		}
		else 
		{
		    d->next = (DLIST *) LS_MALLOC(sizeof (DLIST));
		    d = d->next;
		}
		d->cd = cd;
		d->next = NULL;
	    }
	}
	else /*do deletes*/
	{
	    for (d=delete_list; d; )
	    {
		ls_delete_client(&(d->cd->addr));
		delete_list = d;
		d = d->next;
		free(delete_list);
	    }
	    delete_list = (DLIST *)NULL;
	}
	return 1; /*ls_all_clients needs 1 to keep working*/
}
static
int
client_queued(cd)
CLIENT_DATA *cd;
{
  extern FEATURE_LIST *ls_flist;
  FEATURE_LIST *f;
  USERLIST * q, *b;

	for (f = ls_flist; f; f = f->next)
	{
		for (q = f->queue; q; q = q->next)
		{
			for (b = q; b ; b = b->brother)
			{
				if (b->handle == cd->handle)
				{
					return 1;
				}
			}
		}
			
	}
	return 0;
}
