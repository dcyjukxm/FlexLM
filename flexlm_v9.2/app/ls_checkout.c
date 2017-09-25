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
 *	Module: $Id: ls_checkout.c,v 1.41 2003/06/13 17:17:36 sluu Exp $
 *
 *	Function: ls_checkout(feature, ndesired, wait, who, version,  
 *				dup_sel, linger, code)
 *
 *	Description: Performs the "checkout" procedure for a feature, and 
 *		it's package, if needed
 *
 *	Parameters:	(char *) feature - the feature desired
 *			(char *) ndesired - The number of licenses
 *			(char *) wait - "Wait until available" flag
 *			(CLIENT_DATA *) who - the user
 *			(char *) version - The feature's version number
 *			(char *) dup_sel - Duplicate license selection criteria.
 *			(char *) linger - How long license is to linger
 *			(char *) code - Encryption code from feature line
 *			(int) sn - Serial number for new USERLIST, if non-0
 *
 *	Return:		0 ->  done
 *			< 0 -> checkout failed
 *			> 0 -> request queued
 *
 *	M. Christiano
 *	2/22/88
 *
 *	Last changed:  12/10/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_log.h"
#include "lsfeatur.h"
#include "ls_glob.h"
#include "ls_adaem.h"
#include "ls_aprot.h" 
#include <stdio.h>
#define LOG_DENIED 1
#define NOLOG_DENIED 0
extern int (*ls_outfilter)();
#ifdef PC
typedef int (LM_CALLBACK_TYPE * LS_CB_OUTFILTER) (void);
#define LS_CB_OUTFILTER_TYPE (LS_CB_OUTFILTER)
#else
#define LS_CB_OUTFILTER_TYPE
#endif

void ls_i_fhostid lm_args((FEATURE_LIST *));
extern int ls_log_denied;
static int in_outfilter;
static int need_more_hostids lm_args(( CLIENT_DATA *, FEATURE_LIST *));
static int get_hostid_from_client lm_args(( int, CLIENT_DATA *));

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

/*
 *	ls_checkout, calls ls_feat_checkout where actual checkout is done.
 *	ls_checkout is largely a wrapper to support packages and
 *	+port@host.
 */

int
ls_checkout(char *			feature,	/* The feature desired */
			char *			ndesired,	/* Number of licenses desired */
			char *			waitflag,	/* "Wait until available" */
			CLIENT_DATA *	who,		/* Who is asking for it */
			char *			version,	/* The feature's version number */
			char *			dup_sel,	/* Duplicate selection criteria */
			char *			linger,		/* How long license lingers after checkin */
			char *			code,		/* Encryption code from license file line */
			int				sn,
			int				freeit)		/* Free resulting USERLIST structs -- should be 1, unless
										   it's file-based */
{
	FEATURE_LIST *fl, *pos = (FEATURE_LIST *)0;
	int ret, parentret = 0, found = 0;
	int grab = atoi(ndesired);
	int log_denied = LOG_DENIED;
	int removed;

	DEBUG_INIT
/*-
 *	If license-key/code == PORT_HOST_PLUS_CODE, loop until success 
 */
	if (!*feature)
	{
		LM_SET_ERRNO(lm_job, LM_BADPARAM, 540, 0);
		ls_log_usage(LL_LOGTO_BOTH, (long)0, (long)0,
				LL_USAGE_UNSUPPORTED, LM_BADPARAM,
				who->name, who->node, who->display, 
				who->vendor_def, "ERROR: BLANK FEATURE", code, 
				(LS_POOLED *)0, version, grab, 0, 
				(int) 0, who, 0, 0, 0);
		return lm_job->lm_errno;
		
	}
		
	while (fl = f_get_feat_next_flag_removed(&pos, feature, code, &removed))
	{
		ret = 0;
/* 
 *			Pre- port@host plus, this test used to be <= 0
 *			Now, 0 is a valid number of licenses
 */

		if ((fl->nlic < 0 && fl->res < 0 ) ||
			(fl->flags & LM_FL_FLAG_NONSUITE_PARENT))
			continue;

		found = 1;

		/* 
		 *	handle suite/bundle components: 
		 */

		/*
		 *	Don't allow them to checkout just the package itself as doing this
		 *	leads to other issues.
		 */
		if(fl->flags & LM_FL_FLAG_SUITEBUNDLE_PARENT)
		{
			LM_SET_ERRNO(lm_job, LM_CANT_CHECKOUT_JUST_PACKAGE, 611, 0);
			return lm_job->lm_errno;

		}

		if ((fl->flags & LM_FL_FLAG_SUITE_COMPONENT) && fl->package)
		{
			FEATURE_LIST *fparent;

			fparent = f_get_feat(fl->package->feature, 
						fl->package->code, 1);
			if (fparent && (fparent->nlic >= 0 || 
						fparent->res >= 0))
			{
				parentret = ret = ls_feat_checkout(fparent, 
					fl->package->feature, ndesired, 
					waitflag, who, fl->package->version,
					dup_sel, linger, 
					fl->package->code, sn, 
					freeit, log_denied);
				if ((ret < 0) && 
					(*waitflag != MSG_CO_WAIT_LOCALTEST))
					ls_log_usage(
					(ls_log_denied && log_denied) ?
					LL_LOGTO_BOTH : LL_LOGTO_REPORT, 
					(long)0, (long)0,
					LL_USAGE_DENIED, (int)ret,
					who->name, who->node, who->display,
					who->vendor_def, fl->feature,
					fl->code, fl->pooled, version, 
					grab, 0, 0, who, 0, 0, 0);
			}
			else
				found = 0;
		} 
		if((fl->flags & LM_FL_FLAG_BUNDLE_COMPONENT) && fl->package)
		{
			/*
			 *	Log that we're checking out the PACKAGE even though we're not.
			 *	Done for SAM products.
			 */
			ls_log_usage(LL_LOGTO_REPORT, (long)sn,
				0L,
				LL_USAGE_OUT, LL_REASON_CLIENT_REQUEST, who->name,
				who->node, who->display, who->vendor_def,
				fl->package->feature, fl->package->code, fl->package->pooled,
				fl->package->version, grab, grab, (int) 0,
				who, 0, 0, 0);
		}
/* 
 *		handle non-suite/bundle components: 
 */
		if (ret >=0 && found )
		{
			ret = ls_feat_checkout(fl,
				feature, ndesired, waitflag, who, 
				version, dup_sel, linger, 
				code, sn, freeit, LOG_DENIED);
			if (ret < 0 && 
				(fl->flags & LM_FL_FLAG_COMPONENT)
				&& fl->package 
				&& (*waitflag != MSG_CO_WAIT_LOCALTEST))
			{
				(void) f_remove(who, 
					fl->package->feature, 
					LL_REASON_PACKAGE, 
					fl->package->code, 
					0, 0);
			}
		}
/* 
 *		If checkout has succeeded or it's not +post@host, break.
 *		ONLY port@host loops on failure.
 */
		if (ret >= 0 || 
			strcmp(code, CONFIG_PORT_HOST_PLUS_CODE) ||
					ret == LM_HOSTID_REQUESTED)
		{
			break;
		}
		DEBUG(("ls_checkout IS looping\n"));
	}
	if (!found)
	{
		if ((who->flexlm_ver >= 6)  && f_pooled(feature, code))
		{
			LM_SET_ERRNO(lm_job, LM_POOL, 363, 0);
		}
		if ((who->flexlm_ver >= 6)  && removed)
		{
			LM_SET_ERRNO(lm_job, LM_SERVER_REMOVED, 395, 0)
		}
		else
		{
			LM_SET_ERRNO(lm_job, NOSERVSUPP, 327, 0);
			if (*waitflag != MSG_CO_WAIT_LOCALTEST) 
			{
				ls_log_usage(LL_LOGTO_BOTH, (long)0, (long)0,
				LL_USAGE_UNSUPPORTED, NOSERVSUPP,
				who->name, who->node, who->display, 
				who->vendor_def, feature, code, 
				(LS_POOLED *)0, version, grab, 0, 
				(int) 0, who, 0, 0, 0);
			}
		}
		ret = lm_job->err_info.maj_errno;
		
	}

	if (ret >= 0 && fl)
		strcpy(code, fl->code);
	return parentret > 0 ? parentret : ret; /* P5923 */
}


/*
 *	ls_feat_checkout -- actual checkout done here
 */
int
ls_feat_checkout(
	FEATURE_LIST *	f,			/* associated FEATURE_LIST, or null if component */
	char *			feature,	/* The feature desired */
	char *			ndesired,	/* Number of licenses desired */
	char *			waitflag,	/* "Wait until available" */
	CLIENT_DATA *	who,		/* Who is asking for it */
	char *			version,	/* The feature's version number */
	char *			dup_sel,	/* Duplicate selection criteria */
	char *			linger,		/* How long license lingers after checkin */
	char *			code,		/* Encryption code from license file line */
	int				sn,			/* Serial # for new USERLIST struct, if non-zero */
	int				freeit,		/* Free resulting USERLIST structs -- should be 1, unless
									file-based */
	int				log_denied)	/* if true, log denied message */
{
	int nlic, nres;
	int grab = atoi(ndesired);
	int status, warning = 0;	/* Did he get them */
	char v[MAX_VER_LEN+1];
	int duplicate;
	int linger_interval;
	int hostid_type;

	memset(v, 0, sizeof(v)); /* P6335 */
	DEBUG(("ls_feat_checkout flist->code is %s code is %s\n", 
					f ? f->code : "component", code));
	if (ls_outfilter && !in_outfilter) 
	{
		int ret;
		in_outfilter = 1;
		ls_attr_setup(feature, ndesired, waitflag,
					 who, version,dup_sel, linger, f->code);
		ret = (* LS_CB_OUTFILTER_TYPE  ls_outfilter)();
		in_outfilter = 0;
		if (ret == 0)
			return lm_job->lm_errno;
		else if (ret == -1)
			return 1; /* queued */
	}
	/* Linger-based borrow must be enabled in the CONFIG */
	if (who->borrow_seconds && !(f->conf && (f->conf->lc_type_mask & LM_TYPE_BORROW)))
	{
		LM_SET_ERRNO(lm_job, LM_NOBORROWSUPP, 564, 0);
		status = LM_NOBORROWSUPP;
		goto exit_ls_feat_checkout;
	}
	/* Linger-based borrow can't be for longer than allowed in FEATURE */
	if(who->borrow_seconds && f->conf)
	{
		/* 
		 *	Check to see if borrow is within allowed period
		 */
		int		max_borrow_seconds = 0;
		
		max_borrow_seconds = f->maxborrow ? (f->maxborrow * (60 * 60)) :
							(f->conf->lc_max_borrow_hours * (60 * 60));
								
		if(who->borrow_seconds > max_borrow_seconds)
		{
			LM_SET_ERRNO(lm_job, LM_BORROW_TOOLONG, 563, 0);
			LOG(("Borrow error: %s %s, %d seconds > %d seconds\n", 
				f->feature, lc_errstring(lm_job), who->borrow_seconds, 
				max_borrow_seconds ));
			status = LM_BORROW_TOOLONG;
			goto exit_ls_feat_checkout;
		}
	}

	if (!ls_s_imaster())
	{
		LM_SET_ERRNO(lm_job, NOSERVSUPP, 328, 0);
		LOG(("Not master, %s\n", lc_errstring(lm_job)));
		return(LM_NOSERVSUPP);
	}
	if (grab <= 0)
		grab = 1;
	if (f->type_mask & LM_TYPE_CAPACITY)
		grab *= who->capacity;

/* Terminal Server Support */
	if (f->type_mask & LM_TYPE_TS_OK)
/*		allow it code here
		not sure if this is needed????
		iReturn = isTSOK()
*/


	l_zcp(v, version, MAX_VER_LEN);
	nlic = f->nlic;
	nres = f->res;

	l_decode_int(dup_sel, &duplicate);
	l_decode_int(linger, &linger_interval);

	if (*waitflag == MSG_CO_WAIT_LOCALTEST)
		grab = 0;
	
	status = f_add(who, feature, grab, version, duplicate, 
		linger_interval, f->code, sn, freeit, f);

exit_ls_feat_checkout:
	if (status > 0)  
	{
		warning = -(status);
		status = 0;
	}

	if (status == LM_NOTTHISHOST || warning == LM_NOTTHISHOST)
	{
		if (hostid_type = need_more_hostids(who, f))
		{
			(void) get_hostid_from_client(hostid_type, who);
			status = LM_HOSTID_REQUESTED;
		}
	}
	if (!status)
	{
	}
	else if (((status == MAXUSERS) || (status == USERSQUEUED) || 
		(status == FEATQUEUE)) &&
		*waitflag == MSG_CO_WAIT_QUEUE && grab - nres <= nlic)
	{
		status = f_queue(who, feature, grab, version,
						linger_interval, code, f);
	}
	else if ((status != LM_HOSTID_REQUESTED) && 
					(*waitflag != MSG_CO_WAIT_LOCALTEST))
	{
		ls_log_usage(
			(ls_log_denied && log_denied) ?
			LL_LOGTO_BOTH : LL_LOGTO_REPORT, 
			(long)0, (long)0,
			LL_USAGE_DENIED, (int)status,
			who->name,  who->node, who->display,
			who->vendor_def, f->feature,
			f->code, f->pooled, v, grab, 0, 0,
			who, 0, 0, 0);
		LOG_INFO((INFORM, "\"user\" was denied access to N \
			licenses of feature.  This is the message \
			that will alert end-users to purchase more \
			licenses of your software."));
	}
	return(status);
}

/*
 *	need_more_hostids -- returns a HOSTID type if true, else 0
 */
static
int
need_more_hostids(CLIENT_DATA *		who,
				  FEATURE_LIST *	f)
{
	int type, gotit;
	HOSTID *idp;

	ls_i_fhostid(f);
	for (idp = f->hostid; idp; idp = idp->next)
	{
		HOSTID *h;

		type = idp->type;
		gotit = 0;

		if (type == HOSTID_USER || type == HOSTID_DISPLAY ||
			type == HOSTID_HOSTNAME || type == HOSTID_INTERNET )
		{
			continue; /* we already have all these types */
		}

		for (h = who->hostids; h; h = h->next)
		{
/*
 *			P2576 -- have to test for all byte orderings
 */

			if (type == h->type && 
				(!strncmp(h->id.string, "NOMORE", 6) ||
				!strcmp(h->id.string, "NOMO" ) ||
				!strcmp(h->id.string, "OMON" ) ||
				!strcmp(h->id.string, "RE" ) ||
				!strcmp(h->id.string, "ER" ) ||
					(!*h->id.string && 
				 (!strncmp(h->id.string + 2, "EROMON", 6 ) ||
				 !strncmp(h->id.string + 2, "NOMORE", 6 ) ||
				!strcmp(h->id.string + 4, "RE") ||
				!strcmp(h->id.string + 4, "ER") ||
				!strcmp(h->id.string + 2, "RE") ||
				!strcmp(h->id.string + 2, "ER") ||
				!strcmp(h->id.string + 4, "NOMO") ||
				!strcmp(h->id.string + 4, "OMON") 
				))))
			{
				gotit++;
			}
		}
		if (!gotit)
			return type;
	}
	return 0;
		
}

/*
 *	get_hostid_from_client -- returns 0 if success, else lm_errno.
 *	If successful, hostid is added to who->hostids.
 */
static
int
get_hostid_from_client(int				hostid_type,
					   CLIENT_DATA *	who)
{
	char msg[LM_MSG_LEN+1];	/* For ls_client_send */
	int error = lm_job->lm_errno;
	int offset = 0;
	HOSTID *idptr;

	for (idptr = who->hostids; idptr; idptr = idptr->next)
	{
		if (idptr->type == hostid_type)
			offset++;
	}

	(void)memset(msg, 0, sizeof(msg));
	sprintf(&msg[MSG_DATA], "%d %d", hostid_type, offset);
	ls_client_send(who, LM_NEED_HOSTID, msg);
	if (error != lm_job->lm_errno)
		return lm_job->lm_errno;
	else
		return 0;

}



