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
 *	Module: $Id: ls_lost.c,v 1.5 2003/01/13 22:31:38 kmaclean Exp $
 *
 *	Function: ls_lost()
 *
 *	Description:	Processes the "lost quorum" condition.
 *
 *	Parameters:	None.
 *			(CLIENT_DATA *) user - The user structure (common)
 *
 *	Return:		Client sockets are dropped.
 *
 *	M. Christiano
 *	3/25/88
 *
 *	Last changed:  10/23/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lmselect.h"
#include "ls_glob.h"
#include "lssignal.h"
#include <sys/param.h>
#include <errno.h>

static int close_it lm_args((CLIENT_DATA *, void *));


static int
close_it(cd, data)
CLIENT_DATA *cd;
void *data;
{
	if (cd->addr.is_fd) 
	{
	  LM_SOCKET fd = cd->addr.addr.fd;

		DLOG(("Closing fd %d\n", cd->addr.addr.fd));
		ls_down(&fd, "in ls_lost");
	}
	return 1;
}

void 
ls_lost(sig)
int sig;
{
	ls_all_clients(close_it, 0);
	f_nousers();	/* Clear it all out */
}
