/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_supersede.c,v 1.4 2003/02/13 22:03:08 sluu Exp $
 *
 *	Function: 	l_supersede()
 *
 *	Description: 	remove superseded feature lines
 *
 *	Parameters:	job
 *
 *	Return:		void
 *
 *	D. Birns
 *	9/5/95
 *
 *	Last changed:  11/4/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

static void remove_conf	lm_args((LM_HANDLE *, CONFIG *));
static long supersedes	lm_args(( LM_HANDLE *, CONFIG *, CONFIG *));

void 
l_supersede(LM_HANDLE *job, 
	int packages /* flag -- process only packages */
	)
{
  CONFIG *conf, *cp, *next;

	for (conf = job->line; conf; conf = conf->next)
	{
		if (!(conf->lc_options_mask & LM_OPT_SUPERSEDE)
			|| (packages && (conf->type != CONFIG_PACKAGE))
			|| (!packages && (conf->type == CONFIG_PACKAGE)))
			continue;
		for (cp = job->line; cp; cp = next)
		{
			next = cp->next;
			if ((packages && (cp->type != CONFIG_PACKAGE))
			 || (!packages && (cp->type == CONFIG_PACKAGE)))
			{
				continue;
			}
			if ((cp != conf) && supersedes(job, conf, cp))
			{
				remove_conf(job, cp);
			}
		}
	}
}
/*
 *	supersedes returns >0 if conf1 supersedes conf2
 *	else 0
 */
static 
long
supersedes(
	LM_HANDLE *job,
	CONFIG *conf1, 
	CONFIG *conf2
	)

{
  char date1[DATE_LEN+1], date2[DATE_LEN+1];
  long rc = 0;
  int err = job->lm_errno;
  char **cpp;

	if(conf1 && conf2 && strcmp(conf1->daemon, conf2->daemon))
		return 0;

	if (!conf1->lc_supersede_list)
	{
		if (!l_keyword_eq(job, conf1->feature, conf2->feature)) 
			return 0;
	}
	else
	{
		for (cpp = conf1->lc_supersede_list; *cpp; cpp++)
		{
			if (!strcmp(*cpp, "*ALL*") ||
				l_keyword_eq(job, *cpp, conf2->feature)) 
			{
				rc = 1;
				break;
			}
		}
		if (!rc) return 0;
	}

	if (!(conf1->lc_options_mask & LM_OPT_SUPERSEDE ) &&
				!(conf2->lc_options_mask & LM_OPT_SUPERSEDE))
		return 0;

	if (conf1->lc_issued) strcpy(date1, conf1->lc_issued);
	else strcpy(date1, l_asc_date(l_extract_date(job, conf1->code)));

	if (conf2->lc_issued) strcpy(date2, conf2->lc_issued);
	else strcpy(date2, l_asc_date(l_extract_date(job, conf2->code)));

	rc = l_date_compare(job, date1, date2);
	if (err != job->lm_errno) return 0;  /* error found */

	if (rc > 0 && (conf1->lc_options_mask & LM_OPT_SUPERSEDE))
		return rc;
	return 0;
}

static
void
remove_conf(job, this)
LM_HANDLE *job;
CONFIG *this;
{
  CONFIG *sav = (CONFIG *)0, *conf;

	for (conf = job->line; conf; conf = conf->next)
	{
		if (conf == this) /* remove it */
		{
			if (sav)
				sav->next = conf->next;
			else
				job->line = conf->next;
			l_free_conf(job, this);
			return;
		}
		sav = conf;
	}
}
