/******************************************************************************

	    COPYRIGHT (c) 1999, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_host.c,v 1.18.2.1 2003/06/25 20:53:36 sluu Exp $
 *
 *	Function:	ls_host()
 *
 *	Description: 	Checks server hostid for all features in job
 *
 *	Parameters:	job
 *
 *	Return:		number of good hostids.
 *
 *	D. Birns
 *	20 March 1997
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "lsfeatur.h"
#include "ls_aprot.h"
#include "ls_log.h"
#include "flexevent.h"

extern FEATURE_LIST *ls_flist;
extern char ls_our_hostname[];
static void deleteit lm_args(( FEATURE_LIST *, CONFIG *));
static void ls_badhostid lm_args((CONFIG *, int, int));
static void check_for_delete(CONFIG *c, LM_SERVER *s);

/*
 *	ls_host: returns 1, ok
 *			 0, all features for wrong host
 *			 -1, no features to serve
 */
int
ls_host()
{
  LM_SERVER *s;
  CONFIG *c;
  int ret = 0;
  int found = 0;
  int quorum = 0;
  int this_quorum;
/* 
 *	P4258
 */
	for (c = lm_job->line; c; c = c->next)
	{
		if (!c->users  && !(c->lc_type_mask & LM_TYPE_FLOAT_OK)) continue;
		for (s = c->server; s; s = s->next) quorum ++;
		break;
	}

	for (c = lm_job->line; c; c = c->next)
	{

		if (l_keyword_eq(lm_job, c->daemon, lm_job->vendor) ) found = 1;

		if (c->conf_state == LM_CONF_REMOVED || 
			(c->server && (c->server->sflags & L_SFLAG_WRONG_HOST)) ||
			(!l_keyword_eq(lm_job, c->daemon, lm_job->vendor) ))
			continue;
		if (c->users <= 0 && !(c->lc_type_mask & LM_TYPE_FLOAT_OK)) 
		{
			/* uncounted */
			ret++;
			continue;
		}
		for (this_quorum = 0, s = c->server; s; s = s->next) 
			this_quorum ++;
		if (c->server && (this_quorum != quorum))
		{
			ls_badhostid(c, this_quorum - quorum, 0);
		}

		for (s = c->server; s; s = s->next)
		{
			if (!l_host(lm_job, s->idptr)) break;
		}
		if (c->lc_type_mask & LM_TYPE_FLOAT_OK)
		{
			if (l_host(lm_job, c->idptr)) ls_badhostid(c, 0, 0);
			if (c->floatid && l_host(lm_job, c->floatid)) ls_badhostid(c, 0, 1);
		}
		else if (c->server && !s) 	ls_badhostid(c, 0, 0);
		else 		
		{
			if (!*ls_our_hostname && c->server) 
				l_zcp(ls_our_hostname, c->server->name, 
							MAX_HOSTNAME);
			ret++;
		}
	}
	if (!ret && !found) return -1; /* no features for this vendor */
	return ret;
}
static
void
ls_badhostid(c, mix_quorums, floatid)
CONFIG *c;
int mix_quorums;
int floatid;
{
  LM_SERVER *s;
  char buf[MAX_CONFIG_LINE + 1];
  char *	ppszInStr[20] = {NULL};
	if (mix_quorums)
	{
		if (mix_quorums == 1 || mix_quorums == -1)
		{
			LOG(("SERVER line missing from one of the licenses:\n"));
		}
		else
		{
			LOG((
			"Cannot run both 3-server and 1-server licenses:\n" ));
		}
		LOG(("\t%s:%s\n", 
				lm_job->lic_files[lm_job->line->lf],
				lm_job->lic_files[c->lf]));
		LOG(("Exiting\n"));
		ls_go_down(EXIT_BADCONFIG);

	}
	else if (floatid)
	{
		LOG(("Wrong FLOAT_OK=hostid on line for %s in:\n", c->feature));
		LOG(("\t%s\n", lm_job->lic_files[c->lf]));
	}
	else 
	{
		LOG(("Wrong hostid on SERVER line for license file:\n"));
		LOG(("\t%s\n", lm_job->lic_files[c->lf]));
	}

	c->conf_state = LM_CONF_REMOVED;

	if (!(c->lc_type_mask & LM_TYPE_FLOAT_OK))
		c->server->sflags |= L_SFLAG_WRONG_HOST;

	if (c->lc_type_mask & LM_TYPE_FLOAT_OK)
	{
		if (floatid)
		{
			*buf = 0;
			if (lc_hostid(lm_job, c->floatid->type, buf) || !*buf)
			{
				sprintf(buf, 
					"(Can't get hostid of type %d [%s])", 
					c->floatid->type, lc_errstring(lm_job));
				
				
			}
			LOG(("FLOAT_OK=%s, hostid is %s\n",
				l_asc_hostid(lm_job, c->floatid), buf));

		}
		else
		{
			*buf = 0;
			if (lc_hostid(lm_job, c->idptr->type, buf) || 
								!*buf)
			{
				sprintf(buf, 
					"(Can't get hostid of type %d [%s])", 
					c->idptr->type, lc_errstring(lm_job));
			}
			LOG(("SERVER line says %s, hostid is %s\n",
				l_asc_hostid(lm_job, c->idptr), buf));
			if(l_flexEventLogIsEnabled())
			{
				ppszInStr[0] = lm_job->lic_files[c->lf];
				ppszInStr[1] = l_asc_hostid(lm_job, c->idptr);
				ppszInStr[2] = buf;


				l_flexEventLogWrite(lm_job,
									FLEXEVENT_ERROR,
									CAT_FLEXLM_LMGRD,
									MSG_FLEXLM_VENDOR_WRONG_HOSTID,
									3,
									ppszInStr,
									0,
									NULL);
			}
		}
	}
	else if (!mix_quorums )
	{
		for (s = c->server; s ; s = s->next)
		{
			*buf = 0;
			if (s->idptr)
			{
				if (lc_hostid(lm_job, s->idptr->type, buf) || !*buf)
					sprintf(buf, "(Can't get hostid of type %d [%s])", 
						s->idptr->type, lc_errstring(lm_job));
				LOG(("SERVER line says %s, hostid is %s\n",
					l_asc_hostid(lm_job, s->idptr), buf));

				if(l_flexEventLogIsEnabled())
				{
					ppszInStr[0] = lm_job->lic_files[c->lf];
					ppszInStr[1] = l_asc_hostid(lm_job, s->idptr);
					ppszInStr[2] = buf;


					l_flexEventLogWrite(lm_job,
										FLEXEVENT_ERROR,
										CAT_FLEXLM_LMGRD,
										MSG_FLEXLM_VENDOR_WRONG_HOSTID,
										3,
										ppszInStr,
										0,
										NULL);
				}
			}
			LOG(("Invalid hostid on SERVER line\n"));



		}
	}
	if (c->lc_type_mask & LM_TYPE_FLOAT_OK)
		check_for_delete(c, (LM_SERVER *)-1);
	else
	{
		s = c->server;
		for (c = lm_job->line; c; c = c->next) 
			check_for_delete(c, s);
	}
}
static
void
check_for_delete(CONFIG *c, LM_SERVER *s)
{
  FEATURE_LIST *f;
  LS_POOLED *p;

	if (((s == (LM_SERVER *)-1) && (c->lc_type_mask & LM_TYPE_FLOAT_OK) )
		|| (c->server == s))
	{
		for (f = ls_flist; f; f = f->next)
		{
			if (f->flags & LM_FL_FLAG_LS_HOST_REMOVED)
				continue;
			if (l_keyword_eq(lm_job, f->code, c->code))
			{
				deleteit(f, c);
				break;
			}
			else for (p= f->pooled; p; p = p->next)
			{
				if (l_keyword_eq(lm_job, f->code, c->code))
				{
					deleteit(f, c);
					break;
				}
			}
			if (f->flags & LM_FL_FLAG_LS_HOST_REMOVED)
				break;
		}
	}
}
static
void
deleteit(f, c)
FEATURE_LIST *f;
CONFIG *c;
{
	f->flags |= LM_FL_FLAG_LS_HOST_REMOVED;
	LOG((
	"Disabling %d license%sfrom feature %s(%s)\n", 
		(c->lc_type_mask & LM_TYPE_FLOAT_OK) ? 1 : c->users, 
			(c->users <= 1) ? " " : "s " ,
			c->feature, c->code));
}
