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
 *	Module: $Id: ls_daemon.c,v 1.26.12.1 2003/06/25 00:28:02 sluu Exp $
 *
 *	Function: ls_daemon() - main program for application daemons
 *
 *	Description:	The main program for application daemons.
 *
 *	Parameters:	None.
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	2/15/88
 *
 *	Last changed:  08/07/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lmselect.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lsfeatur.h"
#include "ls_adaem.h"
#include "ls_aprot.h"
#include "ls_log.h"
#include "flexevent.h"


int ls_i_am_lmgrd = 0;		/* We are not lmgrd */
char *prog;			/* Our program path */
int *server_tab = 0;		/* Table of server PIDs */
extern char ls_our_hostname[];
int ls_allow_lmremove = 1;	/* toggled by flag in LM_OK response to 
				   LM_VHELLO msg */
int ls_imaster;


void
ls_daemon(argc, argv)
int argc;
char *argv[];
{
  LM_SOCKET tcp_s;	/* The tcp socket file descriptor */
  LM_SOCKET spx_s;	/* The spx socket file descriptor */
  char *master_name;
  LM_SERVER *master_list;
  CONFIG *ctmp;

	ls_app_init(argc, argv, &master_list, &master_name, &tcp_s, &spx_s);  

	if (!strcmp(master_name, ls_our_hostname))
		ls_imaster = 1;
	else
		ls_imaster = 0;


	for (ctmp = lm_job->line; ctmp; ctmp = ctmp->next)
		if (ctmp->users) break;

	if (!ctmp) ctmp = lm_job->line; /* P5331 */
	
	if (ctmp && ls_quorum(ctmp->server, master_name, 0))
	{
		ls_main(tcp_s, spx_s, master_name);
		/*
		 *	Cleanup event logging resources
		 */
		l_flexEventLogCleanup();
	}
	else
	{
		LOG((lmtext("No master, exiting\n")));
		LOG_INFO((INFORM, "The vendor daemon was started without a \
					master node specified."));
	}
	ls_go_down(EXIT_NOMASTER);	/* Just as good as no quorum */
}
