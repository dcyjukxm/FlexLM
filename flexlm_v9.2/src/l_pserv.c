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
 *	Module: $Id: l_pserv.c,v 1.2 2003/01/13 22:41:54 kmaclean Exp $
 *
 *	Function:	l_print_server
 *
 *	Description: 	Prints a server line to a string.
 *
 *	M. Christiano
 *	4/19/90
 *
 *	Last changed:  10/23/96
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"


void
l_print_server(job, server, buf) /* generates printable string (no newline) */
LM_HANDLE *job;		/* Current license job */
LM_SERVER *server;	/* prints only one server line */
char *buf;	/* must be at least MAX_CONFIG_LINE+1 long */
{
	char *ptr=buf;

	(void) sprintf(ptr, "%s %s %s", LM_RESERVED_SERVER,
			server->name, l_asc_hostid(job, server->idptr));
	ptr += strlen(ptr);
	if (server->port) {
		(void)sprintf(ptr," %d",server->port);
	}
	/* TBD - check for overflow? */
}

/* end */
