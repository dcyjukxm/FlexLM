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
 *	Module: $Id: ls_reread.c,v 1.32.2.2 2003/07/01 17:04:21 sluu Exp $
 *
 *	Function:	ls_reread()
 *
 *	Description: 	Re-reads the license file and updates daemon's
 *			internal feature database.
 *
 *	M. Christiano
 *	5/15/90
 *
 *	Last changed:  12/31/98
 *
 */
/*-
 *	Note: In v2.61, we changed this algorighm substantially.  Whereas
 *		before we merged the two feature lists "on-the-fly", we
 *		now create the new feature list in one shot (by calling
 *		ls_init_feat(), then walk through it and fix it up.
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lssignal.h"
#include "ls_glob.h"
#include "lsfeatur.h"
#include "ls_adaem.h"
#include "ls_aprot.h"
#include "l_openf.h"
#include "flex_utils.h"

void ls_i_fhostid lm_args((FEATURE_LIST *));

FEATURE_LIST * findfeat(char *, char *, FEATURE_LIST *last);

#ifndef RELEASE_VERSION
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)l_real_getenv(c);\
	}

#define DEBUG(x) if (debug) printf x
#else
#define DEBUG_INIT
#define DEBUG(x)
#endif
extern FEATURE_LIST *ls_flist;
static LM_SERVER *m_list;
#if BORROW
static void rmborrow();
#endif
static FEATURE_LIST *endlist();
static void post_one_reserve lm_args(( FEATURE_LIST *));
static void post_optreread	lm_args((lm_noargs));
static void pre_optreread	lm_args((lm_noargs));
static void reread_options	lm_args((char **, char **));
static void resetgroups		lm_args(( GROUP **));
extern int ls_got_reread;
extern int ls_user_based_reread_delay;


static
OPTIONS *
sFreeDynamicReserve(OPTIONS * pOptions)
{
	OPTIONS *	pHead = NULL;
	OPTIONS *	pNext = NULL;
	OPTIONS *	pCurr = NULL;

	pCurr = pOptions;
	while(pCurr)
	{
		pNext = pCurr->next;

		if(pCurr->type == DYNAMIC_RESERVE)
		{
			/*
			 *	Free this one
			 */
			free_opt(pCurr);
		}
		else
		{
			/*
			 *	Add it to list
			 */
			if(pHead)
			{
				pCurr->next = pHead;
				pHead = pCurr;
			}
			else
			{
				pHead = pCurr;
				pCurr->next = NULL;
			}
		}

		pCurr = pNext;
	}
	return pHead;
}

static
void
sDropAllUsers(FEATURE_LIST * pParent)
{
	FEATURE_LIST *	pFeat = NULL;
	USERLIST *		pUser = NULL;
	USERLIST *		pNext = NULL;
	CLIENT_DATA *	pClient = NULL;
	int				status = 0;

	if(!pParent)
		return;

	while(pFeat = findfeat(pParent->feature, NULL, pFeat))
	{
		pUser = pFeat->u;
		while(pUser)
		{
			pNext = pUser->next;
			pClient = ls_client_by_handle(pUser->handle);
			if(pClient)
			{
				pUser->o = sFreeDynamicReserve(pUser->o);
				f_remove_all(pClient, 1, LL_REASON_USER_REMOVED, 0, NULL);
				/*f_remove(pClient, pFeat->feature, LL_REASON_USER_REMOVED, pFeat->code, 1, 0);*/
				f_drop_client(pClient);
			}
			else if(pUser->linger)
			{
				pUser->o = sFreeDynamicReserve(pUser->o);
			}
			pUser = pNext;
		}
	}
}

void
ls_reread()
{
	FEATURE_LIST * f = NULL, * oldf = NULL, * oldflist = NULL, * fsave = NULL;
	FEATURE_LIST * next = NULL, * newf = NULL, * next_o = NULL, * comp = NULL;
	LM_SERVER * master_list = NULL;
	LS_POOLED * p = NULL, * n = NULL;
	OLD_CODE_LIST * ocl = NULL, * ocl_next = NULL;
	char * newreplog = NULL;
	char * newdeblog = NULL;
	CONFIG *conf;

	DEBUG_INIT
	ls_got_reread = 0;

	/*
	 *	Re-init event log
	 */
	l_flexEventLogCleanup();
	l_flexEventLogInit(NULL, lm_job->vendor);
	LOG(("Rereading license file...\n"));
	if (!ls_s_imaster() ||
		(l_getattr(lm_job, REREAD) != REREAD_VAL) || !ls_ck_feats())
	{
		return;
	}
	for (f = ls_flist; f; f = f->next)
		f->conf = 0; /* P5259 */
	l_init_file(lm_job);	/* re-read the file into memory */


/*
 *	First, make sure we are still on the right host.  If not,
 *	just exit because the end-user is trying something funny.
 *	Bug #332
 */
	master_list = l_master_list_from_job(lm_job);
	if (!ls_host())
	{
		LOG((lmtext("No valid hostids, exiting\n")));
		ls_go_down(EXIT_WRONGHOST);
	}
	oldflist = ls_flist;
/*
 *	Get all features from file (2.61)
 */
	ls_init_feat(master_list);
/*-
 *	Now, walk the old feature list.  Move over any users for
 *	corresponding features.  Move over feature headers for
 *	features that have gone away.
 */
	for (oldf = oldflist; oldf && oldf->feature; oldf = next_o)
	{
		next_o = oldf->next;
		if (oldf->flags & (LM_FL_FLAG_LS_HOST_REMOVED))
			continue;
		ls_i_fhostid(oldf);
#if BORROW
		rmborrow(oldf);
#endif /* BORROW */
		for (newf = ls_flist; newf && newf->feature; newf = newf->next)
		{
			if (newf->flags & (LM_FL_FLAG_LS_HOST_REMOVED |
						LM_FL_FLAG_REREAD_REMOVED))
				continue;

			ls_i_fhostid(newf);

            if ((l_keyword_eq(lm_job, newf->code, oldf->code) &&
				l_keyword_eq(lm_job, newf->feature, oldf->feature)) ||
				ls_pool(oldf, newf->conf))/* P4211 fix */
			{
				int save = 0;
				if (!l_keyword_eq(lm_job, newf->code, oldf->code))
					LOG(( "Pooling %s and %s\n", oldf->code, newf->code));
/*-
 *				Already in the table - update it
 *				Move over exactly 2 things:
 *				-	Dynamic info, like current users
 *				-	options info, since we don't
 *					reread the options file.
 */
/*-
 *				v7 Note -- reread options added, so don't
 *				move any options info over
 */
				LOG((lmtext("Updating feature %s\n"), newf->feature));
				LOG_INFO((INFORM, "The daemon has \
				updated the information for \
				the specified feature.\n"));

#if 1 /* remove this if rereading options */
				if (oldf->res < 0)
					save = - oldf->res;
				else
					save = oldf->res;
				newf->nlic -= save;
				if (newf->nlic < 0)
					newf->nlic = 0;
				newf->res = save;

				newf->timeout = oldf->timeout;


				newf->opt = oldf->opt;
				newf->res_list = oldf->res_list;
				newf->include = oldf->include;
				newf->exclude = oldf->exclude;
				newf->b_include = oldf->b_include;
				newf->b_exclude = oldf->b_exclude;
#endif
				newf->dup_select = oldf->dup_select;
				newf->u = oldf->u;
				newf->queue = oldf->queue;
				newf->lowwater = oldf->lowwater;
				newf->linger = oldf->linger;
				newf->flags = (oldf->flags & ~LM_FL_FLAG_REREAD_REMOVED);

/*-
 *				set up old codes:
 *				This is done for lmreread.  The problem
 *				is that the 20-char encryption code may
 *				change during the reread.  We want to
 *				make sure that an lm_checkin message
 *				will succeed when codes have changed.
 *				So there's a linked list of old codes
 *				from all previous rereads.  check these
 *				on an lm_checkin.  This is not 100%
 *				secure, but 99.99% secure and checkin is
 *				not so crucial anway.
 */
				if (!l_keyword_eq(lm_job, oldf->code, newf->code))
				{
					ocl = (OLD_CODE_LIST *)LS_MALLOC(sizeof (OLD_CODE_LIST));
					strcpy(ocl->code, oldf->code);
					ocl->next = newf->old_codes;
					newf->old_codes = ocl;
				}
/*
 *				The old feature list entry will be freed
 *				later.  These 3 fields are a check.
 */
				oldf->u = (USERLIST *) NULL;
				oldf->queue = (USERLIST *) NULL;
				break;
			}
		}
		if (!newf || !newf->feature)
		{
			if (oldf->nlic > 0)
			{
				LOG((lmtext("Support removed for feature %s(%s)\n"),
						oldf->feature, oldf->code));
				LOG_INFO((INFORM, "The server's support for \
				the specified feature has been removed when \
				the server re-read the license file."));
			}
/*
 *			Not in the new table, install it with 0 licenses.
 */
			fsave = endlist(ls_flist);
			f = oldf;
			/* move from old list to new list */
			if (oldflist == oldf)
				oldflist = oldflist->next;
			if (oldf->last)
				oldf->last->next = oldf->next;
			if (oldf->next)
				oldf->next->last = oldf->last;
			f->flags |= LM_FL_FLAG_REREAD_REMOVED;
			f->conf = 0; /* P5259 */
			f->last = fsave->last;
			f->last->next = f;
			f->next = fsave;
			fsave->last = f;
			for (comp = ls_flist; comp && comp->feature; comp = comp->next)
			{
				if (comp->package == oldf)
				{
					comp->package = 0; /* potential purify bug */
				}
			}
		}
	}
/*
 *	Free the old feature list
 */
	for (f = oldflist; f; f = next)
	{
#if 0
		if (f->u || f->queue || f->opt)
		{
			DLOG(("ls_reread: freeing old feature list: f->u: %x\n", f->u));
			DLOG(("            f->queue: %x, f->opt: %x\n", f->queue, f->opt));
		}
#endif /* debug */

		for (comp = ls_flist; comp && comp->feature; comp = comp->next)
        {
            if (comp->package == f)
            {
                comp->package = 0; /* potential purify bug */
            }
        }
		next = f->next;
		if (f->feature)
			free(f->feature);
		if (f->hostid)
			lc_free_hostid(lm_job, f->hostid);
		if (f->id)
			free(f->id);
		if (f->vendor_def)
			free(f->vendor_def);
		if (f->platforms)
		{
			free(f->platforms[0]); free(f->platforms);
		}

		for (ocl = f->old_codes; ocl; ocl = ocl_next)
		{
			ocl_next = ocl->next;
			free(ocl);
		}
		if (f->pooled)
		{
			for(p = f->pooled; p; p = n)
			{
				n = p->next;
				if (p->lc_vendor_def)
					free(p->lc_vendor_def);
				if (p->lc_dist_info)
					free(p->lc_dist_info);
				if (p->lc_sign)
					free(p->lc_sign);
				if (p->lc_user_info)
					free(p->lc_user_info);
				if (p->lc_asset_info)
					free(p->lc_asset_info);
				if (p->lc_issuer)
					free(p->lc_issuer);
				if (p->lc_notice)
					free(p->lc_notice);
				if (p->parent_featname)
					free(p->parent_featname);
				if (p->parent_featkey)
					free(p->parent_featkey);
				free(p);
			}
		}
		free(f);
	}
/*
 *	We have to reset conf->conf_state & LM_CONF_REMOVED and
 *			conf->server->sflags & L_SFLAG_WRONG_HOST
 *	Otherwise ls_host() ignores them.
 */
 	for (conf = lm_job->line; conf; conf = conf->next)
	{
		conf->conf_state &= ~LM_CONF_REMOVED;
		if (conf->server)
			conf->server->sflags &= ~L_SFLAG_WRONG_HOST;
	}
	ls_host(); /* this is the one that rejects individual feats */
	{
		extern char *sav_master_name;
		extern char *sav_options;

#if 1 /*- Per matt's request */
		reread_options(&newreplog, &newdeblog);
		DEBUG(("Calling reopen report\n"));
		if (newreplog)
		{
			LOG(("Reread REPORTLOG switched to %s\n", newreplog));
		}
		if (newdeblog)
		{
			LOG(("Reread DEBUGLOG switched to %s\n", newdeblog));
		}
		ls_user_based(sav_master_name, sav_options);
		ls_log_reopen_report(newreplog, (CLIENT_DATA *)0,
			newreplog ?  REPLOG_SWITCH : 0);
		if (newdeblog)
		{
			ls_log_reopen_ascii(newdeblog);
			ls_print_feats();
		}
		if (newreplog)
			free(newreplog);
		if (newdeblog)
			free(newdeblog);
#else
		ls_user_based(sav_master_name, sav_options);
		ls_log_reopen_report((char *)0, (CLIENT_DATA *)0, 0);
#endif
	}
	LOG(("...Finished rereading\n"));
}

void
ls_store_master_list(LM_SERVER * master_list)
{
	m_list = master_list;	/* Save old master list in case of re-read */
}

#ifdef BORROW
static
void
rmborrow(FEATURE_LIST * fl)
{
	OPTIONS * o = NULL, * next = NULL, * last = NULL;

	for (last = (OPTIONS *) NULL, o = fl->opt; o; o = next)
	{
		next = o->next;
		if (o->type == OPT_BORROW)
		{
			if (last)
				last->next = o->next;
			else
				fl->opt = o->next;
			free(o);
		}
		else
			last = o;
	}
}
#endif /* BORROW */

static
FEATURE_LIST *
endlist(FEATURE_LIST * f)
{
	FEATURE_LIST * t = NULL;

	for (t = f; t && t->next; t = t->next) ;
	return(t);
}

#if !defined(SIGNAL_NOT_AVAILABLE)
#ifdef VOID_SIGNAL
void
#else
int
#endif
ls_reread_sig(sig)
{
	ls_got_reread = 1;
	signal(REREAD_SIGNAL, ls_reread_sig);
}
#endif /* signal not availiable*/



static
void
reread_options(char ** reportlog, char ** debuglog)
{
  	LOG(("Rereading options file...\n"));
	pre_optreread();
	ls_get_opts(0, 0, reportlog, debuglog);
	post_optreread();
}


static
void
pre_optreread()
{
	FEATURE_LIST * f = NULL;
	int ubase_warning = 0;
	OPTIONS * o = NULL, * next = NULL, * last = NULL, * head = NULL, * curr = NULL;
	USERLIST * pUser = NULL;
	int iCountInUse = 0;
	int iCountNotInUse = 0;
	extern int ls_log_in;
	extern int ls_log_out;
	extern int ls_log_queued;
	extern int ls_log_denied;
	extern GROUP *groups;
	extern GROUP *hostgroups;


	for (f = ls_flist; f; f = f->next)
	{
		iCountInUse = iCountNotInUse = 0;

		if (f->flags &
			(LM_FL_FLAG_LS_HOST_REMOVED | LM_FL_FLAG_REREAD_REMOVED))
		{
			continue;
		}

		/*
		 *	If BUNDLE parent, drop all users currently connected, heartbeats will allow them to reconnect after
		 *	server figures out what it has.
		 */
		if(f->flags & LM_FL_FLAG_BUNDLE_PARENT)
			sDropAllUsers(f);

		/*
		 *		RESERVE:
		 *    	1) change f->res and f->nlic based on new reserved list.
		 *		2) remove f->opt
		 */

		head = curr = f->opt;
		next = last = NULL;
		while(curr)
		{
			next = curr->next;
			free_opt(curr);
			curr = next;
		}
		f->opt = NULL;
		f->nlic += f->res;
		f->res = 0;

/*
 *		INCLUDE_BORROW -- remove old lists, and replace
 *					with new one.
 */
		for (o = f->b_include; o ; o = next)
		{
			next = o->next;
			free_opt(o);
		}
		f->b_include = 0;
/*
 *		EXCLUDE_BORROW -- remove old lists, and replace
 *					with new one.
 */
		for (o = f->b_exclude; o ; o = next)
		{
			next = o->next;
			free_opt(o);
		}
		f->b_exclude = 0;
/*
 *		EXCLUDE -- remove old exclude lists, and replace with new one.
 */
		for (o = f->exclude; o ; o = next)
		{
			next = o->next;
			free_opt(o);
		}
		f->exclude = 0;
/*
 *		INCLUDE --      ignore if USER or HOST_BASED, otherwise
 *		remove old include lists, and replace with new one.
 */
		if (!(f->type_mask & LM_TYPE_HOST_BASED) &&
			!(f->type_mask & LM_TYPE_USER_BASED))
		{
			for (o = f->include; o ; o = next)
			{
				next = o->next;
				free_opt(o);
			}
			f->include = 0;
		}
		else if (!ubase_warning)
		{
			if (ls_user_based_reread_delay != -1)
			{
				LOG(("INCLUDE for USER_ or HOST_BASED delayed by %d minutes\n", ls_user_based_reread_delay/60));
			}
			ubase_warning = 1;
		}
/*
 *		TIMEOUT --      reset f->timeout
 */
 		f->timeout = 0;
/*
 *		BORROW_LOWWATER --      reset f->lowwater
 */
 		f->lowwater = 0;
	}
/*
 *		NOLOG --      reset to on
 */
	ls_log_in = 1;
	ls_log_out = 1;
	ls_log_queued = 1;
	ls_log_denied = 1;
/*
 *		GROUPS --      reset if unused by USER_BASED or HOST_BASED
 */
	resetgroups(&groups);
	resetgroups(&hostgroups);

}


void
free_opt(OPTIONS * o)
{
	if (o->name && o->type2 != OPT_INTERNET)
		free(o->name);
	if(o->pszUser)
	{
		free(o->pszUser);
		o->pszUser = NULL;
	}
	if(o->pszHost)
	{
		free(o->pszHost);
		o->pszHost = NULL;
	}
	if(o->pszDisplay)
	{
		free(o->pszDisplay);
		o->pszDisplay = NULL;
	}
	if(o->pszVendor)
	{
		free(o->pszVendor);
		o->pszVendor = NULL;
	}
	free(o);
}

static
void
post_optreread()
{
	USERLIST * u = NULL, * b = NULL;
	OPTIONS * o = NULL;
	FEATURE_LIST * f = NULL;
/*
 *	Reserve:
 *	3) for each res-in-use (f->u->o), put a new reserve
 *	   in f->reread_reserves list, until we're out of reread_reserves.
 *	4) put remaining new reserves as f->opt.
 *	5) when an existing f->u is checked-in, which has an
 *	   f->u->o reserve, don't move it back to f->opt, but
 *	   remove/free it, and put one from f->reread_reserves instead.
 */

	for (f = ls_flist; f; f = f->next) /* all features */
	{
		if (f->flags &
			(LM_FL_FLAG_LS_HOST_REMOVED | LM_FL_FLAG_REREAD_REMOVED))
			continue;
		for (u = f->u; u; u = u->next)	/* all current users of this */
		{
			for (b = u; b; b = b->next) /* including brothers */
			{
				for (o = b->o; o; o = o->next) /* all resv's */
				{
					if (o->type == RESERVE) /* got one */
					{
						post_one_reserve(f);
					}
				}
			}
		}
	}
}

/*
 *	Move one RESERVE OPTIONS record from f->opt to f->reread_reserves.
 *	Make sure to keep f->opt updated as the entries in f->opt might not
 *	be entirely of type RESERVE but possible of type DYNAMIC_RESERVE, in which
 *	case, we don't move DYNAMIC_RESERVE options to f->reread_reserves.
 */
static
void
post_one_reserve(FEATURE_LIST * f)
{
	OPTIONS * curr = NULL, * prev = NULL, * first = NULL;

	for (prev = 0, first = curr = f->opt; curr;
				curr = curr->next)
	{
		if (curr->type == RESERVE)
		{
/*
 *			move from f->opt to
 *			f->reread_reserves
 */
			if(prev)
			{
				prev->next = curr->next;
			}
			if(first == curr)
				f->opt = curr->next;


			curr->next = f->reread_reserves;
			f->reread_reserves = curr;

			break;

		}
		prev = curr;
	}
}

static
void
resetgroups(GROUP ** group)
{
	GROUP * g = NULL, * last = NULL, * next = NULL;

	for (g = *group; g; g = next)
	{
		next = g->next;
		if (!ls_user_or_hostbased(g->name, 0))
		{
			/* free it */
			if (last)
				last->next = g->next;
			else
				*group = g->next;
			if (g->name)
				free(g->name);
			if (g->list)
				free(g->list);
			free(g);
		}
	}
}

/*
 * user_or_hostbased(group)
 *	returns:  1 if this group is used by USER/HOST_BASED, else 0.
 */
int
ls_user_or_hostbased(char * gname, int logit)
{

	extern FEATURE_LIST * ls_flist;
	FEATURE_LIST * f = NULL;
	OPTIONS * o = NULL;

	for (f = ls_flist; f; f = f->next)
	{
		if (f->flags &
			(LM_FL_FLAG_LS_HOST_REMOVED | LM_FL_FLAG_REREAD_REMOVED))
		{
			continue;
		}
		if ((f->type_mask & LM_TYPE_USER_BASED) ||
		    (f->type_mask & LM_TYPE_HOST_BASED))
		{
			for (o = f->include; o; o = o->next)
			{
				if ((o->type2 == OPT_GROUP) &&
					l_keyword_eq(lm_job, o->name, gname))
				{
					LOG((
	"Cannot reread GROUP \"%s\"; used by \"%s\" for USER/HOST_BASED\n",
						gname, f->feature));
					return 1;
				}
			}
		}
	}
	return 0;
}

void
ls_post_delayed_user_based(void)
{
	FEATURE_LIST * f = NULL;
	OPTIONS * o = NULL, * next = NULL;
	extern long ls_reread_time;

	ls_reread_time = 0;
	for (f = ls_flist; f; f = f->next)
	{
		if (f->flags &
			(LM_FL_FLAG_LS_HOST_REMOVED | LM_FL_FLAG_REREAD_REMOVED)
			|| !f->reread_include)
		{
			continue;
		}


		for (o = f->include; o ; o = next)
		{
			next = o->next;
			free_opt(o);
		}
		f->include = f->reread_include;
		f->reread_include = 0;
	}

}
