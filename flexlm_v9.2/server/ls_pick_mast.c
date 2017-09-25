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
 *	Module: $Id: ls_pick_mast.c,v 1.6 2003/01/13 22:31:38 kmaclean Exp $
 *
 *	Function: ls_pick_mast()
 *
 *	Description: Picks a master from the connected server nodes.
 *
 *	Parameters:	(extern) (LM_QUORUM) quorum - The quorum structure
 *
 *	Return:		(int) - The index of the node to wait for, or
 *				< 0 if we have picked a master.
 *
 *	Side effects:	quorum - The quorum structure, filled in.
 *
 *	M. Christiano
 *	3/6/88
 *
 *	Last changed:  8/7/97
 *
 */


#include "lmachdep.h"
#ifndef PC	
#include <sys/time.h>
#endif
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
extern char *ls_i_master();
extern char ls_our_hostname[];

extern LM_QUORUM quorum;
#define q quorum



ls_pick_mast()
{
  LM_SERVER *ls;
  int i;
  int waitmaster = -1;	/* Assume success */
  int num_us = 0;

	ls_s_setmaster(-1);

/*
 *	Find out who we think is the master
 */
	if (ls_s_qnum() == 1)
/*
 *	If only one is required, we are it
 */
	{
		for (ls = ls_s_first(), i=0; ls; ls = ls->next, i++)
		{
			if (ls_on_host(ls->name))
			{
				ls_s_setmaster(i);
				break;
			}
		}
		if (q.master >= 0) q.list[q.master].state |= C_MASTER_READY;
	}
	else 
	{
	  char *thismaster = (char *) NULL;
/*
 *	    See if we can all agree on who is the master
 */
	    	for (ls = ls_s_first(), i = 0; ls; ls = ls->next, i++)
		{
			if (ls->state & C_MASTER_READY)	 /* Already got it */
			{
				ls_s_setmaster(i);
				break;
			}
			else if (ls->state & C_CONNECTED || 
				(ls->sflags & L_SFLAG_US))
			{
				if (ls->sflags & L_SFLAG_US) num_us++;
				if (q.master == -1 ||
				    (q.alpha_order && 
				     (strcmp(ls_s_master(), ls->name) > 0) ) )
						ls_s_setmaster(i);
			}
		}
		if (q.master >= 0) ls = &(q.list[q.master]);
		else		    ls = (LM_SERVER *) NULL;

		if (ls && (ls->sflags & L_SFLAG_US))	/* WE are the master */
		{
		  int hungcount = 0;
		  char *hung_servers[MAX_SERVERS];

			if (num_us > 1)
			{
/*
 *				Some joker has specified more than one
 *				node name as the same node.  Inform him and
 *				exit.
 */
				LOG((lmtext("Multiple SERVER lines for host \"%s\", exiting\n"), ls->name));
				LOG_INFO((CONFIG, "The user has put the same \
					server node name on multiple lines.  \
					This must be corrected before lmgrd \
					will run."));
				ls_go_down(EXIT_BADCONFIG);
			}
			for (i = 0; i < MAX_SERVERS; i++) 
				hung_servers[i] = (char *) NULL;

			for (i = 0, ls = ls_s_first(); ls; ls = ls->next, i++)
			{
			    if ((!(ls->sflags & L_SFLAG_US)) && 
						(ls->state & C_CONNECTED))
			    {
				thismaster = ls_i_master(ls, ls_our_hostname);
				if (thismaster)
				{
					if (*thismaster) break;
					else 
					{
					    hung_servers[hungcount] = ls->name;
					    hungcount++;
					}
				}
			    }
			}
			if (thismaster && *thismaster)
			{
			    for (ls = ls_s_first(), i=0; ls; 
							ls = ls->next, i++)
			    {
				if (!strcmp(thismaster, ls->name))
				{
				    ls_s_setmaster(i);
				    if (ls->state & C_CONNECTED)
				    {
				        LOG((lmtext( "selected (EXISTING) master %s\n")
								, ls->name));
				    }
				    else
				        waitmaster = i;  /* Wait for him */
				    break;
				}
			    }
			}
			if (q.count - hungcount < q.quorum)
			{
			    LOG((lmtext("There are %d servers we can't read from! (quorum: %d):"), 
							hungcount, q.quorum));
			    for (i=0; i < MAX_SERVERS && hung_servers[i]; i++)
			    {
				_LOG((" %s", hung_servers[i]));
			    }
			    _LOG(("\n"));
			}
			else if (ls_s_imaster())
			{
			  int agree = 1;
/*
 *			    Everyone agrees.  We are the master
 */
			    for (ls = ls_s_first(); ls; ls = ls->next)
			    {
				if (!(ls->sflags & L_SFLAG_US) && 
					ls->state & C_CONNECTED)
				{
				    if (ls_mast_rdy(ls, ls_our_hostname))
				    {
					DLOG(("THEY AGREED at first, but changed their minds! (%s)\n", ls->name));
					agree = 0;
				    }
				}
			    }
			    if (agree)
				q.list[q.master].state |= C_MASTER_READY;
			}
		} 
	} 
	return(waitmaster);
}
