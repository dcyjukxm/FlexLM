/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_master.c,v 1.7 2003/01/13 22:41:53 kmaclean Exp $
 *
 *	Function: 	ls_master_list -- like lc_master_list, but
 *			initializes from the job's configs...
 *
 *	D. Birns
 *	3/31/97
 *
 *	Last changed:  9/2/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
LM_SERVER * API_ENTRY
l_master_list_from_job(job)
LM_HANDLE *job;
{
  CONFIG *ctmp;
  LM_SERVER *s, *master_list = 0;
  int i, cnt;

	l_reset_job_servers(job);

	master_list = 0;
	for (ctmp = job->line; ctmp; ctmp = ctmp->next)
		if (ctmp->users) break;

	if (ctmp || (ctmp = job->line)) /* P5331 we didn't use job->line if necess.*/
	{
		master_list = ctmp->server;
		while ( !master_list && ctmp)
		{
			ctmp = ctmp->next; 
			if (ctmp) master_list = ctmp->server;
		}
	}

	if (master_list)
	{
		for (cnt = 0, s = master_list; s; s = s->next, cnt++)
			;
		s = master_list;
		master_list = job->servers;
		for (i = 0; i < cnt; i++)
		{
			memcpy(master_list, s, sizeof(LM_SERVER));
			if (s->idptr)
				master_list->idptr = 
					lc_copy_hostid(job, s->idptr);
			master_list->next = master_list + 1;
			master_list++;
			s = s->next;
		}
		master_list--;
		master_list->next = 0;
		master_list = job->servers;
	}
	return master_list;
}

