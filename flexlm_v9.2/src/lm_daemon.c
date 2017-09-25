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
 *	Module: $Id: lm_daemon.c,v 1.7 2003/01/13 22:41:45 kmaclean Exp $
 *
 *	Function: lc_daemon(job, daemon, options, port)
 *
 *	Description: Gets the path name for the DAEMON specified.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) daemon - the DAEMON desired.
 *
 *	Return:		(char *) - The pathname to the DAEMON, or
 *				  NULL, if no DAEMON line found for this daemon.
 *			(char *) - options - Path to the options file.
 *			(int *) port - The port number (VMS only)
 *	IMPORTANT: 	This function has a static, and is therefore
 *			not reentrant.  This should not be called
 *			by regular apps, so it's probably okay.
 *
 *	M. Christiano
 *	3/6/88
 *
 *	Last changed:  11/2/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "lsmaster.h"
#include "l_prot.h"
#include <stdio.h>

static char *program = (char *)0;

/*
 *	Daemon lines are as follows:
 *
 *	DAEMON name path [port] [options-file]
 *
 *		The port field is only present (but requried) on VMS
 *		The options-file is optional on all system.
 */
#ifdef VMS
#define REQUIRED 4
#else
#define REQUIRED 3
#endif

char * API_ENTRY
lc_daemon(job, daemon, options, port)
LM_HANDLE *job;		/* Current license job */
char *daemon;
char *options;
int *port;
{
  DAEMON *dlist, *dp, *next;
  int sav = job->lfptr;

	/* doesn't set errors besides CANTMALLOC */
	*options = 0;
	*port = -1;

	if (program) /* we've already used it -- free it */
	{
		free(program);
		program = (char *)0;
	}
	for (job->lfptr = LFPTR_FILE1; job->lic_files[job->lfptr]; 
		job->lfptr++)
	{
		if (!(dlist = l_cur_dlist(job))) continue;
		for (dp = dlist; dp; dp = dp->next)
		{
			if (!strcmp(dp->name, daemon))
			{
				if (dp->options_path && !*options)
					strcpy(options, dp->options_path);
				if (dp->path && !program)
				{
					program = (char *)l_malloc(job, 
						strlen(dp->path) + 1);
					strcpy(program, dp->path);
				}
				*port = dp->tcp_port;
				break;
			}
		}
		/* free DAEMON */
		for (dp = dlist; dp; dp = next)
		{
			next = dp->next;
			if (dp->path) free(dp->path);
			if (dp->options_path) free(dp->options_path);
			free(dp);
		}
		if (program && *options) break;
	}
	job->lfptr = sav;
	return program;

}
