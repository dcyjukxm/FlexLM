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
 *	Module: $Id: lm_dump.c,v 1.3 2003/03/12 21:02:56 sluu Exp $
 *
 *	Function:	lm_license_dump(job, vendor)
 *
 *	Description: 	Dumps the license file data for a given vendor to stdout
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(LM_VENDOR *) vendor - The vendor pointer, if NULL,
 *				then job->daemon is used.
 *
 *	Return:		None - Data is output to stdout.
 *
 *	M. Christiano
 *	6/21/90
 *
 *	Last changed:  5/5/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <stdio.h>

void
lc_license_dump(job, vendor)
LM_HANDLE *job;		/* Current license job */
LM_DAEMON_INFO *vendor;
{
  CONFIG *cf;
  LM_SERVER *ls;

	l_clear_error(job);
	if (vendor == (LM_DAEMON_INFO *) NULL) vendor = job->daemon;
	for (cf = job->line; cf; cf = cf->next)
	{
		(void) printf("%s v%s %s \"%s\" SERVERS (%x): ",
					cf->feature,
					cf->version,
					cf->date,
					cf->lc_vendor_def ?
							cf->lc_vendor_def : "",
					cf->server);
		for (ls = cf->server; ls; ls = ls->next)
			(void) printf("%s/%d ", ls->name, ls->port);
		if (cf->server == (LM_SERVER *) NULL) (void) printf("NONE");
		(void) printf ("\n");
	}
	if (job->line == (CONFIG *) NULL)
	{
		(void) printf("NO license data present\n");
	}
}
