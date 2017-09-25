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
 *	Module: $Id: ls_args.c,v 1.5 2003/04/18 23:47:51 sluu Exp $
 *
 *	Function: ls_args(argcp, argvp)
 *
 *	Description:	"Fixes" the calling arguments so that they look
 *			like the ones used by lmgrd.
 *
 *	Parameters:	(int *) argcp - pointer to argc
 *			(char ***) argvp - pointer to argv
 *
 *	Return:		Command line arguments fixed up for "topdog" server.
 *
 *	M. Christiano
 *	8/2/89	5/9/91 - Adapted from ls_vms_call.c
 *
 *	Last changed:  6/18/97
 *
 */
#include <stdio.h>
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "lsfeatur.h"
#include "ls_aprot.h"
#include "flex_utils.h"

#ifdef USE_WINSOCK
#include <pcsock.h>
#endif

#if defined (UNIX) && defined(ANSI)
#include <unistd.h>
#endif

extern LM_HANDLE *lm_job;

#define NUMARGV 5
/*
 *	Fixed argv positions
 */
#define DASH_T	1
#define HOSTNAME 2
#define VER 3
#define PORT 4		/* Position for port */
static char *newargv[NUMARGV];
static char hostname[MAX_HOSTNAME + 1];
static char version[10];
static char cport[8];

void
ls_args(argcp, argvp)
int *argcp;
char **argvp[];
{
  char **argv = *argvp;
  LM_SERVER *l;
  int port;

/*
 *	GENERATE:
 *	    prog -T master_name ver port
 */
	newargv[0] = argv[0];
	newargv[DASH_T] = "-T";
#ifdef FIFO
	if (lm_job->options->commtype == LM_LOCAL)
		strcpy(hostname, "local");
	else
#endif
		(void) gethostname(hostname, MAX_HOSTNAME);	/* LONGNAMES */
	newargv[HOSTNAME] = hostname;
	(void) sprintf(version, "%d.%d", lm_job->code.flexlm_version, 
				lm_job->code.flexlm_revision);
	newargv[VER] = version;

/*
 *	Get our daemon's assigned port number from the license file
 */
	l = lc_master_list(lm_job);
	if (l == (LM_SERVER *) NULL) port = 0;
	else port = l->port;
	(void) sprintf(cport, "%d", port);
	newargv[PORT] = cport;	/* Port */

	*argcp = NUMARGV;
	*argvp = newargv;
}
