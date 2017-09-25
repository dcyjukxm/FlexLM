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
 *	Module: $Id: l_lookup.c,v 1.2 2003/01/13 22:41:53 kmaclean Exp $
 *
 *	Function:	l_lookup(job, feature)
 *
 *	Description: 	Looks up "feature" in in-memory database.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) feature
 *
 *	Return:		(CONFIG *) - The config line for feature.
 *
 *	M. Christiano
 *	6/21/90
 *
 *	Last changed:  7/15/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <stdio.h>

static l_servcheck();

CONFIG *
l_lookup_serv(job, feature, server, port)
LM_HANDLE *job;		/* Current license job */
char *feature;
char *server;
int port;
{
  CONFIG *c;

	for (c = job->line; c; c = c->next)
	{
		if (l_keyword_eq(job, feature, c->feature) &&
		    (!port || l_servcheck(c, server, port))) break;
	}
	if (c == (CONFIG *) NULL) 
	{
		LM_SET_ERRNO(job, LM_NOFEATURE, 64, 0);
	}
	return(c);
}

CONFIG * API_ENTRY
l_lookup(job, feature)
LM_HANDLE *job;		/* Current license job */
char *feature;
{
	return(l_lookup_serv(job, feature, "", 0));
}

static 
l_servcheck(c, server, port)
CONFIG *c;
char *server;
int port;
{
  LM_SERVER *s = c->server;
  int match = 1;

	while (s)
	{
		if (s->port == port && !strcmp(s->name, server)) break;
		s = s->next;
	}
	if (s == (LM_SERVER *) NULL) match = 0;
	return(match);
}
