/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_since.c,v 1.4 2003/01/13 22:22:37 kmaclean Exp $
 *
 *	Function:	ls_since(u, d)
 *
 *	Description: 	Tells if a client has responded since a specified time
 *
 *	Parameters:	(USERLIST) u - The user we want to know about.
 *			(long) d - cutoff time.
 *
 *	Return:		(int) - 1 - this user has talked to us since the cutoff
 *					time
 *				0 - We haven't heard from this client since the
 *					cutoff time.
 *
 *	M. Christiano
 *
 *	4/3/90
 *
 *	Last changed:  10/23/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "ls_comm.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsfeatur.h"
#include "ls_glob.h"
#include "ls_aprot.h"

ls_since(u, d)
USERLIST *u;	/* User we are concerned about */
long d;		/* Time cutoff */
{
  CLIENT_DATA *client = ls_client_by_handle(u->handle);

	if (!client)
		return 0;

	{
		if ((unsigned)d <= client->lastcomm)
			return(1);
		else
			return(0);
	}
}
