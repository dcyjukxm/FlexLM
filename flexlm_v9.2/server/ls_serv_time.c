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
 *	Module: $Id: ls_serv_time.c,v 1.6 2003/01/13 22:31:39 kmaclean Exp $
 *
 *	Function: ls_serv_time(select_mask)
 *
 *	Description: Times out connections to servers.
 *
 *	Parameters:	(SELECT_MASK) select_mask - Mask of file descriptors 
 *						    to select on.
 *
 *	Return:		0 - Not waiting any longer
 *			<> 0 - Still waiting.
 *			select_mask updated.
 *
 *	M. Christiano
 *	6/7/88
 *
 *	Last changed:  10/23/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lmselect.h"

ls_serv_time(select_mask)
SELECT_MASK select_mask;	/* File descriptors to select on */
{
  LM_SERVER *ls;
  int x;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  long curtime;
  int still_waiting;		/* Still waiting for the connections
				   we started to finish or time out */

/*
 *	Time out any connections that haven't made it.
 */
#ifdef THREAD_SAFE_TIME
	l_get_date(&x, &x, &x, &curtime, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&x, &x, &x, &curtime);
#endif
	curtime -= ls_conn_timeout;			/* Adjust */
	still_waiting = 0;
	for (ls = ls_s_first(); ls; ls = ls->next)
	{
		if ((ls->state & C_SOCK) && !(ls->state & C_CONNECTED) &&
				!(ls->state & C_TIMEDOUT))
		{
		    if (curtime > ls->exptime)
		    {
			LOG((lmtext("Connection to %s TIMED OUT\n"), ls->name));
			LOG_INFO((INFORM, " The daemon could not complete \
					the connection to node \
					%s in the time allocated."));
			ls_down(&ls->fd2, "in ls_serv_time");
			ls->state |= C_TIMEDOUT; 
		    }
		    else
			still_waiting = 1;
		}
	}
	return(still_waiting);
}
