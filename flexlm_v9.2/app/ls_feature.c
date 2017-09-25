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
 *	Module: $Id: ls_feature.c,v 1.102 2003/06/17 20:07:27 jwong Exp $
 *//*-
 *	Functions:
 *		f_add(user, feature, nlic, s, ver, dup_sel, linger, code, sn,
 *						free, pFeature) 	- Add user of feature
 *		f_count() - Count # of licenses a user uses (!count_duplicates)
 *		f_featcount(fptr, n, nlic) - Count # of users & # of lic. used
 *		f_dump(fd) - Debug dump (to fd) of feature database.
 *		f_list(feature, n, nlic, tot, queue, opts, noforce, pFeature) - List
 *							users of "feature".
 *		f_nousers() - Remove all users from all features
 *		f_queue(user, feature, nlic, s, ver, pFeature) - queue user.
 *		f_remove(user, feature, s, flag, free, iIgnoreExp) - Remove "user" from feat
 *		f_user_remove(feature, user, free) - Remove "user" from feature.
 *		f_user_remove_handle(feature, handle) - Remove from feature.
 *		f_remove_all(user, s) - Remove "user" from all features.
 *		f_remove_children(s) - Remove all children's (of s) users.
 *		f_remove_feat(feature) - Remove this feature from the list of
 *					 available features for checkout.
 *		f_replace_feat(feature, n) - Replace this feature in the
 *						list of features for checkout.
 *		ls_feat_dump() - Dump out all dynamic feature info to log file
 *		f_dequeue() - De-queue a queued user, if possible
 *		f_checklinger(free) - Check all "lingering" licenses
 *		f_get_feat() - Get a feature descriptor
 *
 *	Local functions:
 *		f_dup_remove() - remove a (duplicate) user
 *		f_get_license_handle() - Lookup a license_handle for a feature
 *		return_reserved() - Return the reservations to the list
 *		f_remove_old(f, freeit) - Remove "timed-out" users of a feature
 *		f_res_avail(fptr, user, n, options) - Returns # of available
 *						      reservations for user
 *		ls_is_duplicate() - Compare a USERLIST to a CLIENT_DATA for dup.
 *		count_res() - Counts the # of reservations in an options list
 *		f_user_dump() - Dumps out a user's data
 *		f_client_dump() - Dumps out a client's data
 *		makeclient() - Creates a CLIENT_DATA from a USERLIST
 *		f_filter() - Check that the user is OK for this feature.
 *		f_brother_head() - get me to the head of this brother list.
 *
 *	Description: These routines manipulate the server's internal
 *			database of users vs features.
 *
 *	Parameters:	(char *) feature - the feature name
 *			(CLIENT_DATA *) user - client data structure
 *			(int *) n - the number of users
 *			(int) N - The number of licenses allowed
 *			(int *) nlic - The number of licenses used.
 *			userlist (char **) - The list of user strings.
 *			(int) nlic - The number of licenses to checkout.
 *			(FEATURE_LIST *) fptr - The feature pointer (if name not
 *						specified).
 *			(USERLIST **) queue - The queue of users (from f_list)
 *			(OPTIONS **) options - The options for f (from f_list)
 *			(char *) ver - The requested version of "feature".
 *			(int) flag - Flag to f_remove that user was removed
 *					due to inactivity
 *			(int) tot - Total number of licenses available
 *			(int) linger - License linger interval
 *			(char *) code - License file encryption code
 *			(int) sn - Serial number for new user (if non-0)
 *			(FEATURE_LIST *) pFeature - The feature that you want to examine
 *
 *
 *	Return:		f_list returns (USERLIST *)
 *			ls_feat_dump returns (void)
 *			all others return (int)
 *
 *	M. Christiano
 *	Last changed:  1/12/99
 *
 */
/*-
 *	Define DEBUG_CHECKS to include the code that checks the
 *	users structure.
 */
#ifndef RELEASE_VERSION
#define DEBUG_CHECKS
#endif

#define LOOKUP_EXACT 		1
#define LOOKUP_IGNORE_LINGERS 	1
#define LOOKUP_ALL 		0

int ls_log_in = 1;
int ls_log_out = 1;
int ls_log_denied = 1;
int ls_log_queued = 1;
long ls_curr_time;

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lmselect.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_log.h"
#include "ls_glob.h"
#include "ls_comm.h"
#include "lsfeatur.h"		/*- Dynamic feature data */
#include "ls_adaem.h"
#include "ls_aprot.h"
#include "lm_attr.h"
#include "flexevent.h"
#include <stdio.h>
#if !defined(PC) && !defined(apollo)
#include <netinet/in.h>
#endif

#define NUSERS_INC 20		/*- Number of USERLIST to allocate each time */
static int cur_serialno = 1;
static int group_handle = 1;
#define MAX_SERIALNO 10000000

static USERLIST *u_free = (USERLIST *) NULL;	/*- Free list of users */
void ls_i_fhostid lm_args((FEATURE_LIST *));

extern long ls_currtime;

extern int ls_min_lmremove;
static int ls_in_f_remove = 0;
extern int ls_in_vd_info;
extern int ls_borrow_return_early;
extern int ls_hud_hostid_case_sensitive;

extern void (*ls_borrow_out)(char * szBorrowData, int iSize);
extern void (*ls_borrow_in)(char * szBorrowData, int iSize);

/*-
 *		static prototypes here
 */
void f_drop_client	lm_args((CLIENT_DATA *));
static int f_is_res_for_client	lm_args(( OPTIONS *, CLIENT_DATA *));
static int f_match_hostids 	lm_args(( HOSTID *, CLIENT_DATA *));
static CLIENT_DATA * 	ls_client_by_ulist lm_args((USERLIST *));
static void f_delete_udp_client_if_done lm_args((CLIENT_DATA *client));
static void return_reserved	lm_args((OPTIONS *,FEATURE_LIST *));
static USERLIST * f_dup_remove 	lm_args(( CLIENT_DATA *, USERLIST *,
					FEATURE_LIST *, USERLIST *, int *,
					int *nback, int));
static int f_res_avail		lm_args(( FEATURE_LIST *, CLIENT_DATA *, int,
					OPTIONS **));
static void more_u 		lm_args((lm_noargs));
static void f_remove_old	lm_args(( FEATURE_LIST *, int));
static count_res		lm_args(( OPTIONS *));
static void logres		lm_args(( FEATURE_LIST *, int, long));
static CLIENT_DATA * makeclient lm_args((USERLIST *));
static USERLIST * f_get_license_handle lm_args(( char *, int, USERLIST **));
static int f_filter		(CLIENT_DATA *, FEATURE_LIST *, int);
static void recycle_user	lm_args(( USERLIST *));
static int f_remove_client_from_q lm_args(( FEATURE_LIST *, CLIENT_DATA *,
					USERLIST **, int));
static int f_remove_linger	lm_args(( USERLIST *, CLIENT_DATA *,
							FEATURE_LIST **));
static int f_remove_one_u	lm_args(( USERLIST *, CLIENT_DATA *, char *,
				      	FEATURE_LIST **, USERLIST **,
					int, int));

static USERLIST *f_brother_head lm_args(( FEATURE_LIST *, USERLIST *));
/*-
 *		end static prototypes
 */
int
fCanDynamicUnreserve(
	FEATURE_LIST *	pFeature,
	CLIENT_DATA *	pClient);
#ifdef PC
typedef int (LM_CALLBACK_TYPE * LS_CB_INCALLBACK) (void);
#define LS_CB_INCALLBACK_TYPE (LS_CB_INCALLBACK)

typedef int (LM_CALLBACK_TYPE * LS_CB_CHECK_VENDOR_ID) (HOSTID * h1,
					HOSTID * h2);
#define LS_CB_CHECK_VENDOR_ID_TYPE (LS_CB_CHECK_VENDOR_ID)

#else

#define LS_CB_INCALLBACK_TYPE
#define LS_CB_CHECK_VENDOR_ID_TYPE

#endif /* PC */

int
f_dynamic_reserve(
	FEATURE_LIST *	pFeatEntry,		/*- The feature desired */
	char *			szUser,
	int				iNumLic,		/*- How many we want to reserve) */
	int				bIgnore);		/*	If set, ignore the package itself */


/*-
 *	Free this USERLIST structure
 */
#define UFREE(x)	{ x->next = u_free; u_free = x; }



#define DIRECTION_OUT		0
#define DIRECTION_IN		1

static
void
sOutputBorrow(
	USERLIST *	u,
	char *		feature,
	int			direction)
{
	char	buffer[4096] = {'\0'};
	int		size = sizeof(buffer);
	char *	pCurr = buffer;
	int		len = 0;
	long	flags = 0;

	/*
	 *	length_of_data(including this field):major_ver:minor_ver:\
	 *	borrow_revision:dup_select:feature:name:node:diplay:project:\
	 *	vendor_def:version:code:count:linger:endtime:normal_count:flags
	 */


	memset(buffer, ':', sizeof(buffer));

	pCurr += sizeof(int) + 1;	/* skip first field */
	memset(pCurr++, FLEXLM_VERSION, 1);	/* VERSION */
	pCurr++;
	memset(pCurr++, FLEXLM_REVISION, 1);/* REVISION */
	pCurr++;
	memset(pCurr++, FLEXLM_BORROW_REVISION, 1);	/* BORROW REVISION */
	pCurr++;
	memcpy(pCurr, &u->dup_select, sizeof(u->dup_select));
	pCurr += sizeof(u->dup_select) + 1;
	memcpy(pCurr, feature, strlen(feature));
	pCurr += strlen(feature) + 1;
	memcpy(pCurr, u->name, strlen(u->name));
	pCurr += strlen(u->name) + 1;
	memcpy(pCurr, u->node, strlen(u->node));
	pCurr += strlen(u->node) + 1;
	memcpy(pCurr, u->display, strlen(u->display));
	pCurr += strlen(u->display) + 1;
	memcpy(pCurr, u->project, strlen(u->project));
	pCurr += strlen(u->project) + 1;
	memcpy(pCurr, u->vendor_def, strlen(u->vendor_def));
	pCurr += strlen(u->vendor_def) + 1;
	memcpy(pCurr, u->version, strlen(u->version));
	pCurr += strlen(u->version) + 1;
	memcpy(pCurr, u->code, strlen(u->code));
	pCurr += strlen(u->code) + 1;
	memcpy(pCurr, &u->count, sizeof(u->count));
	pCurr += sizeof(u->count) + 1;
	memcpy(pCurr, &u->linger, sizeof(u->linger));
	pCurr += sizeof(u->linger) + 1;
	memcpy(pCurr, &u->endtime, sizeof(u->endtime));
	pCurr += sizeof(u->endtime) + 1;
	memcpy(pCurr, &u->normal_count, sizeof(u->normal_count));
	pCurr += sizeof(u->normal_count) + 1;
	if(u->flags & LM_ULF_BORROW_INIT)
	{
		flags = u->flags & (LM_ULF_BORROW_INIT ^ 0xffffffff);
		memcpy(pCurr, &flags, sizeof(flags));
	}
	else
	{
		memcpy(pCurr, &u->flags, sizeof(u->flags));
	}
	pCurr += sizeof(u->flags);
	len = pCurr - buffer;
	memcpy(buffer, &len, sizeof(len));

	/*
	 *	memset rest of buffer to 0
	 */
	memset(pCurr, 0, size - len);
	if(direction == DIRECTION_OUT)
	{
		if(ls_borrow_out)
			ls_borrow_out(buffer, len);
	}
	else
	{
		if(ls_borrow_in)
			ls_borrow_in(buffer, len);
	}

}

static
void
sUnreserveLicense(
	FEATURE_LIST *	pFeature,
	CLIENT_DATA *	pClient)
{
	int		iNumToFree = 0;
	char	buffer[MAX_LONGNAME_SIZE] = {'\0'};	/* LONGNAMES */

	if(!pFeature || !pClient)
		return;



	if(pFeature->flags & LM_FL_FLAG_BUNDLE_COMPONENT)
	{
		iNumToFree = fCanDynamicUnreserve(pFeature, pClient);

		if(iNumToFree)
		{

			if(pFeature->dup_select && pFeature->suite_dup_group)
			{
				if(pFeature->suite_dup_group & LM_DUP_USER)
				{
					sprintf(buffer, pClient->name);
				}
				strcat(buffer, ":");
				if(pFeature->suite_dup_group & LM_DUP_HOST)
				{
					strcat(buffer, pClient->node);
				}
				strcat(buffer, ":");
				if(pFeature->suite_dup_group & LM_DUP_DISP)
				{
					strcat(buffer, pClient->display);
				}
				strcat(buffer, ":");
				if(pFeature->suite_dup_group & LM_DUP_VENDOR)
				{
					strcat(buffer, pClient->vendor_def);
				}
			}
			else
			{
				strcpy(buffer, "*:*:*:*");
			}

			f_dynamic_reserve_remove(pFeature->package, buffer, iNumToFree);
		}
	}
}

/****************************************************************************/
/**	@brief	Determines total number of licenses for a feature, including
 *			the number that are currently borrowed and reserved.
 *
 *	@param	pFeat				Feature in question
 *	@param	pTotalCheckedOut	Pointer to int that receives the total number
 *								of licenses, including reserved
 *	@param	pTotalBorrowed		Number of licenses currently borrowed
 *
 *	@return	NONE
 ****************************************************************************/
static
void
sQueryLicense(
	FEATURE_LIST *	pFeat,
	int *			pTotalCheckedOut,
	int *			pTotalBorrowed)
{
	USERLIST *	pUser = NULL;
	OPTIONS *	pOpt = NULL;
	int			rv = 0;

	if( (pFeat == NULL) || (pTotalCheckedOut == NULL) || (pTotalBorrowed == NULL) )
		goto done;

	*pTotalCheckedOut = *pTotalBorrowed = 0;

	/*
	 *	Count regular users
	 */
	for(pUser = pFeat->u; pUser; pUser = pUser->next)
	{
		/*
		 *	Don't worry about brothers
		 */
		if(pUser->flags & LM_ULF_BORROWED)
		{
			(*pTotalBorrowed) += pUser->count;
		}
		(*pTotalCheckedOut) += pUser->count;
	}

	/*
	 *	Count any reservations
	 */
	for(pOpt = pFeat->opt; pOpt; pOpt = pOpt->next)
	{
		if(pOpt->type == RESERVE || pOpt->type == DYNAMIC_RESERVE)
		{
			(*pTotalCheckedOut)++;
		}
	}
done:
	return;
}

/*-
 *	f_add - add this user to the feature list
 */
int
f_add(
	CLIENT_DATA *	client,		/*- The user */
	char *			feature,	/*- The feature desired */
	int				nlic,		/*- The number of licenses to add */
	char *			ver,		/*- The version of this feature */
	int				dup_sel,	/*- Duplicate selection criteria */
	int				linger,		/*- How long license is to linger after checkin */
	char *			code,		/*- Encryption code from feature line */
	int				sn,			/*- Serial number for new feature - if non-zero */
	int				freeit,		/*- Free resulting USERLIST struct should be 1, unless file based */
	FEATURE_LIST *	pFeature)
{
	FEATURE_LIST *f;
	USERLIST *u, *saveu, *l;
	int x;
	long xl;
	int warning = 0;
	int avail;
	int haveremoved = 0;	/*- Flag that we have removed timed-out users */
	int numres;		/*- Number of reservations at start */
	int numLicNeeded = 0;
	int numLicResForUser = 0;

	if (!u_free)
		more_u();		/*- Allocate USER space */
	u = f_lookup(client, feature, &l, &f, LOOKUP_ALL, code, LOOKUP_ALL,
								pFeature ? &pFeature : 0, dup_sel);
	saveu = u;
	if (!f)
	{
		if(l_flexEventLogIsEnabled())
		{
			l_flexEventLogWrite(lm_job,
								FLEXEVENT_ERROR,
								CAT_FLEXLM_LMGRD,
								MSG_FLEXLM_VENDOR_FEATUREDB_ERROR,
								0,
								NULL,
								0,
								NULL);
		}
		LOG( (lmtext("FEATURE database corrupted")));
		LOG_INFO((FATAL, "This daemon has a corrupted feature database.\n"));
		LM_SET_ERRNO(lm_job, FEATCORRUPT, 331, 0)
		return(lm_job->lm_errno);
	}

	if (f->flags & (LM_FL_FLAG_LS_HOST_REMOVED | LM_FL_FLAG_REREAD_REMOVED))
	{
		if (client->flexlm_ver  >= 6)
		{
			LM_SET_ERRNO(lm_job, LM_SERVER_REMOVED, 395, 0)
		}
		else
		{
			LM_SET_ERRNO(lm_job, LM_NOSERVSUPP, 401, 0)
		}
		return(lm_job->lm_errno);
	}
	if (client->L_CLIENT_KEY_REQ )
	{
		LM_KEYLIST *kl;
		for (kl = f->conf->lc_keylist; kl; kl = kl->next)
			if (kl->sign_level == client->L_CLIENT_KEY_REQ)
				break;
		if (!kl)
		{
			LM_SET_ERRNO(lm_job, LM_SIGN_REQ, 525, 0)
			return(lm_job->lm_errno);
		}
	}

	if (f->flags & LM_FL_FLAG_ALL_RESERVED)
	{
		OPTIONS *o;
/*
 *		they're all reserved, find out if any apply to us
 */
		for (o = f->res_list; o; o = o->next)
		{
			if (f_is_res_for_client(o, client))
				break;
		}
		if (!o)
		{
			if (client->flexlm_ver >= 6)
				LM_SET_ERRNO(lm_job, LM_RESVFOROTHERS, 396, 0)
			else
				LM_SET_ERRNO(lm_job, LM_MAXUSERS, 400, 0)
			return lm_job->lm_errno;
		}
	}
/*-
 *	Handle MINIMUM
 */
	if (nlic < f->minimum)
		nlic = f->minimum;
/*-
 *	Check that the user belongs here.
 */
	if (f_filter(client, f, dup_sel))
	{
		return(lm_job->lm_errno);
	}


/*-
 *	If anyone is queued for these licenses, don't give any out
 *	(UNLESS we can use only reserved licenses - v1.06).
 */
	x = nlic;		/*- How many licenses desired */
	if (u)
		x -= u->count;	/*- Account for the ones we have */
	numLicNeeded = x;


/*-
 *	If anyone is queued, kick out any users that have timed out or
 *	lingered too long.
 */
	if (f->queue && f->timeout)
	{
		f_checklinger(freeit);
		f_remove_old(f, freeit);
		haveremoved = 1;
	}
/*
 *	Check options MAX
 */
	if (ls_max_exceeded(f, client, x))
	{
		return (lm_job->lm_errno);

	}


	numres = count_res(f->opt);	/*- Reservations available @ start */

	if (nlic && f->queue && f_res_avail(f, client, 0, (OPTIONS **) 0) < x)
	{
		LM_SET_ERRNO(lm_job, LM_USERSQUEUED, 332, 0);
		return (lm_job->lm_errno);
	}
/*-
 *	Check hostids
 */
	if (client->flexlm_ver >= 5 &&
		!(f->conf->lc_type_mask & LM_TYPE_FLOAT_OK))
	{
		ls_i_fhostid(f);
		if (!f_match_hostids(f->hostid, client))
		{
			LM_SET_ERRNO(lm_job, LM_NOTTHISHOST, 333, 0);
			return (lm_job->lm_errno);
		}
	}
/*-
 *	make sure the version (s)he is asking for is <= the
 *	version we are supporting.
 */
	if (l_compare_version(lm_job, ver, f->version) > 0)
	{
		LM_SET_ERRNO(lm_job, LM_SERVLONGGONE, 334, 0);
		return (lm_job->lm_errno);
	}
/*
 *	Check the expiration date
 */
	if (l_date(lm_job, f->expdate, L_DATE_EXPIRED))
	{
		return (lm_job->lm_errno);
	}
	{
		int day, year;
		char month[10];
		extern int ls_baddate_detected;

		sscanf(f->expdate, "%d-%[^-]-%d", &day, month, &year);	/* overrun checked*/
		if (year)
		{
			if (ls_baddate_detected)
			{
				LM_SET_ERRNO(lm_job, LM_BADSYSDATE, 335, 0);
				return (lm_job->lm_errno);
			}
			if (client->flags & CLIENT_FLAG_BADDATE_DETECTED)
			{
				LM_SET_ERRNO(lm_job, LM_CLOCKBAD, 336, 0);
				return (lm_job->lm_errno);
			}
		}
	}
/*
 *	Verify platform
 */
	if (f->platforms)
	{
		char plat[MAX_PLATFORM_NAME + 1];
		int bad = 1;
		char **cpp;
		char *cp;

/*
 *		strip osver from platform name
 */
		l_zcp(plat, client->platform, MAX_PLATFORM_NAME);
		for (cp = plat; *cp; cp++)
		{
			if (*cp == '_')
			{
				for (cp++; *cp && isalpha(*cp); cp++)
					;
				*cp = 0;
				cp[2] = 0;
				break;
			}
		}
/*
 *		P2862
 *		Note that we use sgi_u, and not sgi32_u -- this way
 *		we can rewrite the buffer without an ABW
 */
		l_lowercase(plat);
		if (L_STREQ(plat, "sgir8_u"))
			strcpy(plat, "sgi64_u");
		else if (L_STREQ(plat, "sgi32_u"))
			strcpy(plat, "sgi_u");
/*
 *		end of P2862 fix
 */
		for (cpp = f->platforms; *cpp; cpp++)
		{
			l_lowercase(*cpp);
			/* P2862 */
			if (L_STREQ(*cpp, "sgi32_u"))
					strcpy(*cpp, "sgi_u");
			else if (L_STREQ(*cpp, "sgir8_u"))
					strcpy(*cpp, "sgi64_u");
			if (l_keyword_eq(lm_job, *cpp, plat))
			{
				bad = 0;
				break;
			}

		}
		if (bad)
		{
			LM_SET_ERRNO(lm_job, LM_PLATNOTLIC, 337, 0);
			return (lm_job->lm_errno);
		}
	}

	if (f->flags & LM_FL_FLAG_USER_BASED_ERR)
	{
		LM_SET_ERRNO(lm_job, LM_USER_BASED, 338, 0);
		return lm_job->lm_errno;
	}
	if (!nlic)
		return 0;/* LOCALTEST */
/*-
 *	Next, find out how many total licenses are available for
 *	this client (= reserved slots available + unreserved slots)
 */
retry:
	f_list(feature, code, &x, &avail, &x, (USERLIST **) &xl,
							(OPTIONS **) &xl, 1, f);
	avail = f->nlic - avail;
	avail += f_res_avail(f, client, 0, (OPTIONS **) 0);


/*
 *	Enforce BORROW_LOWWATER
 */
	if(client->borrow_seconds && f->lowwater)
	{
		int		iTotalCheckedOut = 0;
		int		iTotalBorrowed = 0;
		int		iTotalBorrowAllowed = 0;
		/* Don't count reservations for this user do a total count on the options in sQueryLicense() */
		int		iTotalAvailable = avail - f_res_avail(f, client, 0, (OPTIONS **)0 );

		/*
		 *	Determine number of licenses currently used (checked out, borrowed, reserved)
		 */
		sQueryLicense(f, &iTotalCheckedOut, &iTotalBorrowed);

		/*
		 *	Determine max number of licenses that can be borrowed.  The calculation is:
		 *	Total_num_licenses - borrow_lowwater = max_number_of_licenses_that_can_be_borrowed
		 */
		iTotalAvailable += iTotalCheckedOut;
		iTotalBorrowAllowed = iTotalAvailable - f->lowwater;
		if( (iTotalBorrowAllowed > 0) && ((iTotalBorrowed + nlic) > iTotalBorrowAllowed) )
		{
			LM_SET_ERRNO(lm_job, LM_NONETOBORROW, 456, 0);
			return lm_job->lm_errno;
		}
	}

	if (!(f->dup_select & LM_DUP_NONE) && u)
	{
		/*- No "u" -> same as count_duplictes */
		int nhave = 0;


		f_count(u, &x, &nhave);
/*-
 *		    	If we don't have enough, see if we can kick some
 *		    	people out.
 */
		if (!(f->flags & LM_FL_FLAG_UNCOUNTED) && (nlic > nhave + avail)
				&& f->timeout && !haveremoved)
		{
			f_checklinger(freeit);
			f_remove_old(f, freeit);
			haveremoved = 1;
			goto retry;	/*- Try it again */
		}
		if ((f->flags & LM_FL_FLAG_UNCOUNTED) ||(nlic <= nhave + avail))
		{
			USERLIST *uu, *new;
			int gothim = 0;
/*-
 *				Tack this user onto the end of the list.
 */

			for (l = u; l; l = l->brother)
			{
				if ((l->handle == client->handle) &&
					!strcmp(l->name, client->name) )

				{
					u = l;	/*- Get exact user */
					gothim++;
					break;
				}
			}
			if (gothim)
			{
/*-
 *				If we already have this user, see if this
 *				is an upgrade.  (P311, 6/22/92)
 */
				nlic -= u->count;
				if (nlic > 0)
				{
/*-
 *				    We have asked for more licenses than we
 *				    already had.  Upgrade this license.
 */
					x = f_res_avail(f, client, 0,
						(OPTIONS **) 0);
					if (x > 0)
					{
/*-
 *						More reservations available.
 *						Get them.
 */
						struct options **_l = &(u->o),
											 *_o = u->o;

						for (; _o;  _l = &(_o->next),   _o = _o->next) ;
						x = f_res_avail(f, client, nlic, _l);
					}
/*-
 *				    x is the delta number of reservations we got
 *				    nlic is the total delta number of licenses
 *				    we want.  Thus,	We have (delta licenses
 *				    - delta reservations) more "normal" licenses
 *				    now.  Total is just incremented by delta.
 */
					u->normal_count += (nlic - x);
					u->count += nlic;
					ls_log_usage(LL_LOGTO_BOTH,
						(long)u->license_handle,
					saveu?
					(long)saveu->license_handle :
					0L, LL_USAGE_UPGRADE,
					LL_REASON_CLIENT_REQUEST,
					u->name, u->node, u->display,
					u->vendor_def,
					f->feature, f->code, f->pooled,
					ver, u->count, nlic, (int) 0,
					client, u->group_handle, u->flags,
					u->linger);
					logres(f, numres,
					  (long) u->license_handle);
				}
				u = (USERLIST *) NULL;	/*- Already had one */
			}
			else	/*- !gothim */
			{
				for (l = u, uu = u->brother; uu;
					uu = uu->brother)
				l = uu;
				new = u_free;
				recycle_user(new);
				u_free = u_free->next;
				l->brother = new;
				u = new;
/*-
 * 					Assume no RES
 */
				u->o = (OPTIONS *) NULL;
				if (nlic <= nhave)
					u->normal_count = nlic;
				else
				{
					x = f_res_avail(f, client,
						nlic-nhave, &(u->o));
					u->normal_count = nlic - x;
				}
			}
		}
		else
		{
			if (f->nlic == 0 && f->res == 0)
				LM_SET_ERRNO(lm_job, LM_NOFEATURE, 339, 0)
			else
				LM_SET_ERRNO(lm_job, LM_MAXUSERS, 340, 0)
			return lm_job->lm_errno;
		}
	}
	else
	{
/*-
 *		This user hasn't appeared yet OR THIS SERVER
 *		counts duplicates
 */


		if (u)
		{
			nlic -= u->count;	/*- How many more we need */
		}
/*-
 *		If we don't have enough, see if we can kick some
 *		people out.
 */
		if (! (f->flags & LM_FL_FLAG_UNCOUNTED) &&
				(nlic > avail) && f->timeout && !haveremoved)
		{
			f_remove_old(f, freeit);
			haveremoved = 1;
			goto retry;	/*- Try it again */
		}

		/*
		 *	Do a dynamic reserve here for packages=bundle
		 */
		numLicResForUser = f_res_avail(f, client, numLicNeeded, NULL);

		if(f->flags & LM_FL_FLAG_BUNDLE_PARENT && (numLicNeeded > 0 && numLicNeeded > numLicResForUser))
		{

			/*
			 *	Checking out a BUNDLE, dynamically reserve all the components
			 *	then, checkout only the component I'm interested in.
			 */
			char	buffer[1024] = {'\0'};

			if(f->dup_select && f->suite_dup_group)
			{
				if(f->suite_dup_group & LM_DUP_USER)
				{
					sprintf(buffer, client->name);
				}
				strcat(buffer, ":");
				if(f->suite_dup_group & LM_DUP_HOST)
				{
					strcat(buffer, client->node);
				}
				strcat(buffer, ":");
				if(f->suite_dup_group & LM_DUP_DISP)
				{
					strcat(buffer, client->display);
				}
				strcat(buffer, ":");
				if(f->suite_dup_group & LM_DUP_VENDOR)
				{
					strcat(buffer, client->vendor_def);
				}
			}
			else
			{
				/*
				 *	NO SUITE_DUP_GROUP specified, matches anything
				 */
				strcpy(buffer, "*:*:*:*");
			}

			f_dynamic_reserve(f, buffer, numLicNeeded - numLicResForUser, 0);
		}

		else if(f->flags & LM_FL_FLAG_BUNDLE_COMPONENT &&
				f->package &&
				numLicNeeded &&
				(numLicNeeded > numLicResForUser))
		{
			/*
			 *	This should occur when SUITE_DUP_GROUP is set and the user wants more components
			 *	of the BUNDLE.  The question is whether or not to also DYNAMICALLY RESERVE the BUNDLE
			 *	as well......
			 */
			/*
			 *	Check to see if we have enough
			 */
			if(!((numLicNeeded - numLicResForUser) > f->nlic))
			{
				char	buffer[1024] = {'\0'};

				if(f->dup_select && f->suite_dup_group)
				{
					if(f->suite_dup_group & LM_DUP_USER)
					{
						sprintf(buffer, "%s", client->name);
					}
					strcat(buffer, ":");
					if(f->suite_dup_group & LM_DUP_HOST)
					{
						strcat(buffer, client->node);
					}
					strcat(buffer, ":");
					if(f->suite_dup_group & LM_DUP_DISP)
					{
						strcat(buffer, client->display);
					}
					strcat(buffer, ":");
					if(f->suite_dup_group & LM_DUP_VENDOR)
					{
						strcat(buffer, client->vendor_def);
					}

				}
				else
				{
					/*
					 *	NO SUITE_DUP_GROUP specified, matches anything
					 */
					strcpy(buffer, "*:*:*:*");
				}
				f_dynamic_reserve(f->package, buffer, numLicNeeded - numLicResForUser, 1);
			}
		}

		if (!(f->flags & LM_FL_FLAG_UNCOUNTED) && (nlic > avail))
		{
			if ((f->nlic <= 0) && (f->res == 0))
			{
				LM_SET_ERRNO(lm_job, LM_NOFEATURE, 341, 0)
			}
			else
			{
				LM_SET_ERRNO(lm_job, LM_MAXUSERS, 342, 0)
			}
			return lm_job->lm_errno;
		}
		else if (u == (USERLIST *) NULL)
		{
			l = NULL;	/*- Assume no last entry */
			if (f)
			{
				u = f->u;
				while (u)
				{
					l = u;
					u = u->next;
				}
			}
			u = u_free;
			recycle_user(u);
			u_free = u_free->next;
			if (l)
				l->next = u;
			else
				f->u = u;

			/*-Assume none from reserved */
			u->o = (OPTIONS *) NULL;

			x = f_res_avail(f, client, nlic, &(u->o));
			u->normal_count = nlic - x;
		}
		else
		{
/*-
 *			This user already had some licenses, and nlic is
 *			the delta number of licenses we need, at this point.
 */
			if (nlic > 0)
			{
/*-
 *				We have asked for more licenses than we
 *				already had.  Upgrade this license.
 */
				x = f_res_avail(f, client, 0, (OPTIONS **) 0);
				if (x > 0)
				{
/*-
 *				More reservations available.  Get them.
 */
					struct options **_l = &(u->o), *_o = u->o;

					for (; _o; _l = &(_o->next),
						   _o = _o->next) ;
						x = f_res_avail(f,
							client, nlic,
							_l);
				}
/*-
 *				x is the delta number of reservations we got.
 *				nlic is the total delta number of licenses
 *				we want.  Thus,	We have (delta licenses -
 *				delta reservations) more "normal" licenses
 *				now.  Total is just incremented by delta.
 */
				u->normal_count += (nlic - x);
				u->count += nlic;

				ls_log_usage(LL_LOGTO_BOTH,
				(long)u->license_handle,
				saveu ? (long)saveu->license_handle :0L,
				LL_USAGE_UPGRADE,
				LL_REASON_CLIENT_REQUEST, u->name,
				u->node, u->display, u->vendor_def,
				f->feature,
				f->code, f->pooled, ver, u->count,
				nlic, (int) 0,
				client, u->group_handle, u->flags, u->linger);
				logres(f, numres,
				(long) u->license_handle);
			}
			u = (USERLIST *) NULL;	/*- Already had one */
		}
	}
	if (!u)
	{
		return(lm_job->lm_errno);
	}

/*-
 *	Finally, log it and fill in the rest of the "u" struct.
 */
	if (saveu)
		u->group_handle = saveu->group_handle;
	else if (f->dup_select && (f->dup_select != LM_DUP_NONE))
		u->group_handle = group_handle++;
	else
		u->group_handle = 0; /*- doesn't apply */

	u->dup_select = f->sticky_dup ? f->dup_select : dup_sel;
	l_zcp(u->name, client->name, MAX_USER_NAME);	/* LONGNAMES */
	l_zcp(u->node, client->node, MAX_SERVER_NAME);/* LONGNAMES */
	l_zcp(u->display, client->display, MAX_DISPLAY_NAME);/* LONGNAMES */
	l_zcp(u->project, client->project, MAX_PROJECT_LEN);/* LONGNAMES */
	l_zcp(u->vendor_def, client->vendor_def,
					MAX_VENDOR_CHECKOUT_DATA);
	l_zcp(u->version, ver, MAX_VER_LEN);
	l_zcp(&(u->code[0]), code, MAX_CRYPT_LEN);
	if (sn)
		u->license_handle = sn;		/*- For file-based */
	u->handle = client->handle;
	u->count = nlic;
	u->time = ls_currtime;
	if (linger > f->linger)
		u->linger = linger;
	else
		u->linger = f->linger;
	u->endtime = 0;			/*- Not lingering now */
	u->next = (USERLIST *) NULL;
	u->brother = (USERLIST *) NULL;

/*
 *	If linger-borrowed, flag it so that
 *	lmstat and so on can correctly report it
 */
	if (client->borrow_seconds)
		u->flags |= LM_ULF_BORROWED;

	ls_log_usage(ls_log_out ? LL_LOGTO_BOTH :
		LL_LOGTO_REPORT, (long)u->license_handle,
		saveu ? (long)saveu->license_handle : 0L,
		LL_USAGE_OUT, LL_REASON_CLIENT_REQUEST, u->name,
		u->node, u->display, u->vendor_def,
		f->feature, f->code, f->pooled, ver, nlic, nlic, (int) 0,
		client, u->group_handle, u->flags, u->linger);
	logres(f, numres, (long) u->license_handle);
	LOG_INFO((INFORM, "USER has checked out N licenses of FEATURE\
			at mm/dd/yy hh:mm"));
	return -(warning);
}

extern int (*ls_incallback)();

/*-
 *	f_remove - remove this user from the feature list
 *
 *	More than one USERLIST entry can exist for this client in the
 *	FEATURE_LIST for this feature, when using CHECKOUT_DATA
 *	(client->vendor_def).
 */
f_remove(
	CLIENT_DATA *	client,		/*- The user */
	char *			feature,	/*- The feature desired  or LM_CI_ALL_FEATURES if empty */
	int				reason,		/*- reason -- from ls_log.h */
	char *			code,		/*- Encryption code from feature */
	int				freeit,		/*- Free USERLIST structs ? should be 1, unless	file based */
	int				iIgnoreExp)	/*	If set, ignore check if time has expired for linger */
{
	USERLIST *u = (USERLIST *) NULL, *prev;
	FEATURE_LIST *fptr;		/*- Pointer to the feature entry */
	FEATURE_LIST *start_here = *feature ? 0 : ls_flist;
	int removeit = 0;
	int gotone = 0;


/*-
 *	Due to an optimization, lingers are only examined once in the
 *	loop below.  This flag controls this:
 */
	int lingers_flag = LOOKUP_ALL; /*- first time, get the lingers */


/*-
 *	ls_in_f_remove is a flag  for lm_reread fix: when we get down to
 *	f_get_feat, we need to check the old_codes for matches, and
 *	remove those clients that match the old code also.
 *	This should only happen when a client's being removed
 *	Be sure to reset when leaving this function!
 */
	ls_in_f_remove = 1;

	if (!client)
		return(0); /*- prevent a core dump */

/*-
 *	This loop checks each USERLIST entry when vendor->def is
 *	used and multiple checkouts happen from a single client
 *	with different ->vendor_def's.
 */
	while (u = f_lookup(client, feature, &prev, &fptr, LOOKUP_EXACT,
			code, lingers_flag, start_here ? &start_here : 0, 0))
	{
		gotone = 1;
		if (u->endtime == 0 && u->linger > 0)
		{
/*-
 *			This is a client that is supposed to linger.
 *			f_remove_linger starts the lingering time
 *			it will be removed later.
 */
			removeit = f_remove_linger(u, client, &fptr);
		}
		else if (u->endtime != 0)
		{
			if ((ls_currtime > u->endtime + u->linger) || iIgnoreExp)
			{
/*
 *				Got a lingering entry which is expired.
 *				remove it.
 */
				removeit = 1;
			}
		}
		else /*- regular, non-lingering */
		{
			removeit = 1;
		}

		if (removeit)
		{

			if (!f_remove_one_u(u, client, feature,
					&fptr, &prev, reason, freeit))
			{
				ls_in_f_remove = 0;
				return 0;
			}
		}
		if (fptr)
			gotone += f_remove_client_from_q(fptr, client,
						&prev, freeit);
		lingers_flag = LOOKUP_IGNORE_LINGERS;
	}
/*
 *	We have to check the queue even if the lookup above failed to
 *	get a user from the FEATURE_LIST
 */
	if (fptr && !gotone)
		gotone = f_remove_client_from_q(fptr, client, &prev, freeit);
	if ((removeit || !gotone) && client->addr.transport == LM_UDP)
		f_delete_udp_client_if_done(client);
	ls_in_f_remove = 0;
	return(gotone);
}

/*-
 *	Return 1, normal, 0 if f_dup_remove failed
 *	(cut from f_remove, and probably only used by f_remove)
 */
static
int
f_remove_one_u(
	USERLIST *		u,
	CLIENT_DATA *	client,
	char *			feature,
	FEATURE_LIST **	fptrp,
	USERLIST **		prevp,
	int				reason,
	int				freeit)
{
	int nback;
	USERLIST *oldu = u, *thisu;
	USERLIST *prev = *prevp;
	FEATURE_LIST *fptr = *fptrp;
	int nlic;
	int numres;
	USERLIST * brother;

	if (ls_incallback)
	{
		char *waitflag = "0";
		char dup_sel[MAX_LONG_LEN+1];
		char linger[MAX_LONG_LEN+1];
		char n[10];
/*-
 *			Process the checkin callback
 */
		(void) sprintf(n, "%d", u->count);
		(void) sprintf(dup_sel, "%d", fptr->dup_select);
		(void) sprintf(linger, "%d", u->linger);
		ls_attr_setup(feature, n, waitflag, client,
				u->version, dup_sel,
				linger, &(u->code[0]));
		(* LS_CB_INCALLBACK_TYPE ls_incallback)();
	}
/*-
 *	Look up the user again to get the head of the brother list
 */

	u = f_brother_head(fptr, u);

	if (!u)
	{

		u = oldu;
		/* reset prev, since f_lookup failed */
		prev = *prevp;
	}
	numres = count_res(fptr->opt);
	if (fptr->dup_select & LM_DUP_NONE)	/*- count duplicates */
	{
		if (prev != NULL)
			prev->next = u->next;	/*- Update feature users list */
		else
			fptr->u = u->next;	/*- ... wherever it is */
		nlic = u->count;		/*- How many are coming back */
		nback = nlic;
		thisu = u;			/*- All the same for logging */
		if (u->o)
			return_reserved(u->o, fptr);
		if (freeit)
			UFREE(u)	/*- Free this one */
	}
	else
	{
		thisu = f_dup_remove(client, u, fptr, prev, &nlic,
							&nback, freeit);
		if (thisu == (USERLIST *) NULL || nback == -1)
		{
			return(0); /*- all done */
		}
	}
/*-
 *		Log the fact that this license came back
 */

	if (thisu == u)
		brother = u->brother;
	else
		brother = u;

	ls_log_usage(ls_log_in ?
		LL_LOGTO_BOTH : LL_LOGTO_REPORT,
		(long)thisu->license_handle,
		brother ? (long)brother->license_handle
				: 0L ,
		LL_USAGE_IN, reason, thisu->name,
		thisu->node, thisu->display,
		thisu->vendor_def, fptr->feature,
		fptr->code,
		fptr->pooled , u->version,
		nlic, nback, (int)thisu->time,
		client, thisu->group_handle, thisu->flags, thisu->linger);
	logres(fptr, numres, (long) thisu->license_handle);
/*-
 *	Start anyone in the queue
 */
	if (nback > 0)
	{
		while (fptr->queue)
		{			/*- De-queue the next user */
			if (f_dequeue(fptr, freeit) == 0)
			{
				break;
			}

		}
	}
	*prevp = prev;
	*fptrp = fptr;
	return 1;
}

/*-
 *	f_remove_client_from_q
 *
 *	remove this user from the queue for this feature
 *	returns: 1 if a user is removed, else 0
 *	(cut from f_remove, and probably only used by f_remove)
 */

static
int
f_remove_client_from_q(
	FEATURE_LIST *	fptr,
	CLIENT_DATA *	client,
	USERLIST **		prevp,
	int				freeit)
{
	USERLIST *prev = *prevp;
	int gotone = 0;
	USERLIST *q = fptr->queue, *qq;

	prev = (USERLIST *) NULL;
	while (q != (USERLIST *)NULL)
	{
		if ((client->handle == q->handle) &&
			!strcmp(q->name, client->name) &&
			!strcmp(q->node, client->node) &&
			!strcmp(q->display, client->display))
		{
/*-
 *			This user is in this queue element -- remove
 *			and log the fact that he was dequeued.
 */
			ls_log_usage(LL_LOGTO_BOTH, (long)q->license_handle,
				(long)0,
				LL_USAGE_DEQUEUED, LL_REASON_CLIENT_REQUEST,
				q->name,
				q->node, q->display, q->vendor_def,
				fptr->feature, fptr->code, fptr->pooled,
				q->version, q->count, q->count,
				(int) 0, client, q->group_handle, q->flags,
				q->linger);
			LOG_INFO((INFORM, "USER has removed himself \
					from the queue at mm/dd/yy hh:mm"));
			if (prev)
				prev->next = q->next;
			else
				fptr->queue = q->next;
			qq = q->next;
			if (freeit)
				UFREE(q)	/*- Free this one */
			q = qq;
			gotone = 1;
		}
		else
		{
			prev = q;
			q = q->next;
		}
	}

	/*
	 *	See if anything can be dequeud
	 */
	if(gotone && fptr->queue)
		f_dequeue(fptr, 1);

	*prevp = prev;
	return gotone;
}

/*-
 *	f_remove_linger --
 *	returns 1 if removed
 *	(cut from f_remove, and probably only used by f_remove)
 */



static
int
f_remove_linger(
	USERLIST *		u,
	CLIENT_DATA *	client,
	FEATURE_LIST **	fptr)
{
	USERLIST *thisu;
	int removeit = 0;
	int linger = u->linger;

/*-
 *	Optimize:  If 'u' has a brother that is going to linger,
 *	just get rid of 'u' WITH a checkin message.
 *	For more info: see note at bottom of file:
 */

	thisu = u;
	u = f_brother_head(*fptr, u);

	while (u)
	{
		if (u != thisu && u->linger >= linger)
		{
/*-
 *			If the other user had already started to
 *			linger, bump that user up so that he will re-start
 *			to linger now.  If his linger interval was
 *			longer, adjust his end time.
 */
			if (u->handle < 0)
			{
				u->endtime = ls_currtime;
				if (u->linger > linger)
					u->endtime -= (u->linger -
							      linger);
			}
			/*
			 *	Not really needed for persistent borrow but might be needed to fix
			 *	bug where the license server closes a connection on a valid client
			 */
			u->flags |= LM_ULF_REMOVED;
			removeit = 1;
			break;
		}
		u = u->brother;
	}
	u = thisu;
	if (removeit == 0)
	{
		u->endtime = ls_currtime;
		u->orgHandle = u->handle;	/* save original handle */
		u->handle = LM_LINGERED_HANDLE_VALUE;
		/*
		 *	See comments above
		 */
		u->flags |= LM_ULF_REMOVED;

		if(ls_borrow_out)
		{
			sOutputBorrow(u, (*fptr)->feature, DIRECTION_OUT);
		}
	}
	return removeit;
}


/*-
 *	f_nousers() - Remove all users of all features
 */

void
f_nousers()
{
	FEATURE_LIST *f;
	USERLIST *u, *next;
	int x, max;
	CLIENT_DATA *c;

	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		for (u = f->u; u; u = next)
		{
			USERLIST *b;
			c = ls_client_by_ulist(u);
		        f_count(u, &x, &max);
			ls_log_usage(ls_log_in ? LL_LOGTO_BOTH :
					LL_LOGTO_REPORT,
				(long)u->license_handle,
				u->brother ?
				    (long)u->brother->license_handle :
				    0L,
				LL_USAGE_IN,
				LL_REASON_SHUTDOWN, u->name,
				u->node, u->display, u->vendor_def,
				f->feature, f->code, f->pooled,
				u->version,
				max, max,
				(int)u->time, c, u->group_handle, u->flags,
				u->linger);
			LOG_INFO((INFORM, "The application daemon has been shut\
				down and hence N licenses of \
				FEATURE were automatically checked back in\
				at mm/dd/yy hh:mm"));
			next = u->next;
			for (b = u; b; b = b->brother)
			{
				if (b->o) return_reserved(b->o, f);
				UFREE(b)
			}
		}
		f->u = (USERLIST *) NULL;
		for (u = f->queue; u; u = next)
		{
			next = u->next;
			UFREE(u)
		}
		f->queue = (USERLIST *) NULL;
	}
}


/*-
 *	f_checklinger() - Check all "lingering" licenses
 */

static int inhere = 0;	/*- This routine can't be re-entered at signal
				level */

void
f_checklinger(int freeit)	/*- Free resulting USERLIST struct should be 1, unless
								file based */
{
	FEATURE_LIST *f;
	USERLIST *u, *b, *next, *nextb;
	int	iNumToFree = 0;

	if (inhere)
		return; /*- don't let this reentrancy happen */

	inhere++;
	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		for (u = f->u; u; u = next)
		{
			next = u->next;
			if ((u->endtime) &&
				(ls_currtime > u->endtime + u->linger))
			{
			    CLIENT_DATA *junk = makeclient(u);

				if(ls_borrow_in)
				{
					sOutputBorrow(u, f->feature, DIRECTION_IN);
				}

				f_remove(junk, f->feature,
					LL_REASON_CLIENT_REQUEST, &(f->code[0]),
								freeit, 0);
				sUnreserveLicense(f, junk);

				if (junk->handle < 0)
					junk->handle = -junk->handle;
				if(junk->handle == -(LM_LINGERED_HANDLE_VALUE))
				{
					/*
					 *	Restore original handle.  This MIGHT result in clients
					 *	being dropped that shouldn't be....
					 */
					junk->handle = u->orgHandle;
				}

				if(!(u->flags & LM_ULF_BORROW_INIT))
					f_drop_client(junk); /* P5910 */
			}
			for (b = u->brother; b; b = nextb)
			{
				nextb = b->brother;
				if ((b->endtime) &&
					(ls_currtime > b->endtime + b->linger))
				{
					CLIENT_DATA *junk = makeclient(b);

					if(ls_borrow_in)
					{
						sOutputBorrow(b, f->feature, DIRECTION_IN);
					}
					f_remove(junk, f->feature,
						LL_REASON_CLIENT_REQUEST,
						&(f->code[0]), freeit, 0);

					sUnreserveLicense(f, junk);

					if (junk->handle < 0)
						junk->handle = -junk->handle;
					if(junk->handle == -(LM_LINGERED_HANDLE_VALUE))
					{
						/*
						 *	Restore original handle.  This MIGHT result in clients
						 *	being dropped that shouldn't be....
						 */
						junk->handle = u->orgHandle;
					}
					if(!(u->flags & LM_ULF_BORROW_INIT))
						f_drop_client(junk); /* P5910 */
				}
			}
		}
	}
	inhere = 0;
}

/*-
 *	f_queue - Put the user in the queue for a feature.
 */
f_queue(
	CLIENT_DATA *	client,		/*- The user */
	char *			feature,	/*- The feature desired */
	int				nlic,		/*- The number of licenses to add */
	char *			ver,
	int				linger,		/*- How long license is to linger after checkin */
	char *			code,		/*	Encryption code from feature line */
	FEATURE_LIST *	pFeature)	/*	Actual feature itself */
{
	USERLIST *u, *l, *n;
	FEATURE_LIST *f;
	extern int ls_log_denied;

	if(!pFeature)
	{
		if (!(f = f_get_feat(feature, code, 1)))
		{

			if ( (client->flexlm_ver >= 6) && f_pooled(feature, code))
				LM_SET_ERRNO(lm_job, LM_POOL, 362, 0)
			else
				LM_SET_ERRNO(lm_job, LM_NOSERVSUPP, 343, 0)
			return(lm_job->lm_errno);
		}
	}
	else
	{
		f = pFeature;
	}

/*-
 *	Find the end of the queue
 */
	l = (USERLIST *)NULL;
	for (n = f->queue; n != (USERLIST *)NULL; n = n->next)
	{
		l = n;
	}
	if (!u_free)
		more_u();	/*- Allocate USER space */
	u = u_free;
	recycle_user(u);
	u_free = u_free->next;
	l_zcp(u->name, client->name, MAX_USER_NAME);	/* LONGNAMES */
	l_zcp(u->node, client->node, MAX_SERVER_NAME);/* LONGNAMES */
	l_zcp(u->display, client->display, MAX_DISPLAY_NAME);/* LONGNAMES */
	l_zcp(u->project, client->project, MAX_PROJECT_LEN);
	l_zcp(u->vendor_def, client->vendor_def,
					MAX_VENDOR_CHECKOUT_DATA);
	l_zcp(u->version, ver, MAX_VER_LEN+1);
	l_zcp(&(u->code[0]), code, MAX_CRYPT_LEN);
	u->time = ls_currtime;
	u->count = nlic;
	u->handle = client->handle;
	if (linger > f->linger)
		u->linger = linger;
	else
		u->linger = f->linger;
	u->endtime = 0;			/*- Not lingering now */
	u->next = NULL;
	if (l)
		l->next = u;
	else
		f->queue = u;
	ls_log_usage(ls_log_queued ? LL_LOGTO_BOTH :
		LL_LOGTO_REPORT,
		(long)u->license_handle,
		(long)0,
		LL_USAGE_QUEUED, LL_REASON_CLIENT_REQUEST,
		u->name, u->node, u->display, u->vendor_def,
		f->feature, f->code, f->pooled, u->version,
		nlic, nlic,
		(int) 0, client, u->group_handle, u->flags, u->linger);
	LOG_INFO((INFORM, "USER has been put in the queue for N \
		licenses of FEATURE at mm/dd/yy hh:mm"));
	return(1);
}

int
f_pooled(char *	feature, char * code)
{
	int pooled;

	ls_in_vd_info = 1;
	pooled = (int)f_get_feat_next(0, feature, code);
	ls_in_vd_info = 0;
	return pooled;
}

/*
 *	This was taken from f_remove().  This is needed to handle deleting users from a suite
 *	that have checked out multiple components of the suite.  Doing a f_remove in that situation
 *	would result in the first entry that matched the user being deleted.  This is because when
 *	a user borrows a license, they're really doing a checkout(linger), checkin.  The client disconnects
 *	when completed.  Because of this, when doing a lmborrow -return, since we have to construct a generic
 *	client to do the delete, we match the first one for the user, instead of the exact one.  This results
 *	in f_remove deleting the first one it finds as opposed to the right one.  Whew...
 */
static
int
sRemove(
	CLIENT_DATA *	client,		/*- The user */
	char *			feature,	/*- The feature desired  or LM_CI_ALL_FEATURES if empty */
	int				reason,		/*- reason -- from ls_log.h */
	char *			code,		/*- Encryption code from feature */
	int				freeit,		/*- Free USERLIST structs ? should be 1, unless	file based */
	int				iIgnoreExp,	/*	If set, ignore check if time has expired for linger */
	int				iNumLic)	/*	Number of users this entry should have */
{
	USERLIST *u = (USERLIST *) NULL, *prev = NULL;
	FEATURE_LIST *fptr;		/*- Pointer to the feature entry */
	FEATURE_LIST *start_here = *feature ? 0 : ls_flist;
	int removeit = 0;
	int gotone = 0;
	int bFound = 0;


/*-
 *	Due to an optimization, lingers are only examined once in the
 *	loop below.  This flag controls this:
 */
	int lingers_flag = LOOKUP_ALL; /*- first time, get the lingers */



/*-
 *	ls_in_f_remove is a flag  for lm_reread fix: when we get down to
 *	f_get_feat, we need to check the old_codes for matches, and
 *	remove those clients that match the old code also.
 *	This should only happen when a client's being removed
 *	Be sure to reset when leaving this function!
 */
	ls_in_f_remove = 1;

	if (!client)
		return(0); /*- prevent a core dump */

/*-
 *	This loop checks each USERLIST entry when vendor->def is
 *	used and multiple checkouts happen from a single client
 *	with different ->vendor_def's.
 */
	u = f_lookup(client, feature, &prev, &fptr, 0/*LOOKUP_EXACT*/,
			code, lingers_flag, start_here ? &start_here : 0, 0);
	if(u)
	{
		/*
		 *	Check to see if we have the right one
		 */
		int dup_sel = u->dup_select & ~LM_DUP_VENDOR;
		if(u->count != iNumLic)
		{
			/*
			 *	Try to find a better match or else return
			 */
			while(u)
			{
				/*
				 *	Set prev pointer or the remove won't update list correctly.
				 */
				prev = u;
				u = u->next;
				if(u)
				{
					if(ls_is_duplicate(u, client, dup_sel))
					{
						if(u->count == iNumLic)
						{
							bFound = 1;
							break;
						}
					}
					else
						break;
				}
			}
			if(!bFound)
			{
				/*
				 *	Couldn't find a match for this client.
				 */
				ls_in_f_remove = 0;
				return 0;
			}
		}


		gotone = 1;
		if (u->endtime == 0 && u->linger > 0)
		{
/*-
 *			This is a client that is supposed to linger.
 *			f_remove_linger starts the lingering time
 *			it will be removed later.
 */
			removeit = f_remove_linger(u, client, &fptr);
		}
		else if (u->endtime != 0)
		{
			if ((ls_currtime > u->endtime + u->linger) || iIgnoreExp)
			{
/*
 *				Got a lingering entry which is expired.
 *				remove it.
 */
				removeit = 1;
			}
		}
		else /*- regular, non-lingering */
		{
			removeit = 1;
		}

		if (removeit)
		{
			if (!f_remove_one_u(u, client, feature,
					&fptr, &prev, reason, freeit))
			{
				ls_in_f_remove = 0;
				return 0;
			}
		}
		if (fptr)
			gotone += f_remove_client_from_q(fptr, client,
						&prev, freeit);
		lingers_flag = LOOKUP_IGNORE_LINGERS;
	}
/*
 *	We have to check the queue even if the lookup above failed to
 *	get a user from the FEATURE_LIST
 */
	if (fptr && !gotone)
		gotone = f_remove_client_from_q(fptr, client, &prev, freeit);
	if ((removeit || !gotone) && client->addr.transport == LM_UDP)
		f_delete_udp_client_if_done(client);
	ls_in_f_remove = 0;
	return(gotone);
}


/*-
 *	f_user_remove() - Remove this user from this feature. - Used only
 *			  by the lmremove command.
 */


f_user_remove(
	char *			feature,	/*- The feature */
	CLIENT_DATA *	client,		/*- The user data */
	int				freeit,		/*-	should be 1, unless file based */
	int				iForce)		/*	Force the removeal, only used when returning a borrowed
									license */
{
	USERLIST *u, *next;
	USERLIST tempUser;
	FEATURE_LIST *f;
	int status = 0;
	int ret = 0;
	int logged = 0;
	int	bFoundOne = 0;
	int numlic = 0;
	CLIENT_DATA *cd;


	f = f_get_feat(feature, "", 1);	    /*- Find a feature with any code */
	while(f)
	{
/*-
 *	    	We have the feature desired, now go get the user

 */
		bFoundOne = 0;
		LM_SET_ERRNO(lm_job, 0, 344, 0) /*- Flag we found feature */
		for (u = f->u; u; u = next)
		{
			int dup_sel = u->dup_select & ~LM_DUP_VENDOR;

			next = u->next;
			if (ls_is_duplicate(u, client, dup_sel))
			{
				if (ls_currtime - u->time >= ls_min_lmremove || iForce)
				{

					cd = ls_client_by_ulist(u);/* P 2144 */
					if(cd || client->handle == LM_LINGERED_HANDLE_VALUE  || (u->flags & LM_ULF_BORROW_INIT))
					{
						if(!cd)
						{
							/*
							 *	Make client since this USERLIST entry was created by the
							 *	license server at startup to restore borrowed/lingering
							 *	licenses and has no corresponding CLIENT to go with it.
							 */
							cd = makeclient(u);
						}
						if (!logged)
						{
							logged = ret = 1;
							LOG((lmtext(
		"REMOVING %s@%s:%s from %s by administrator request.\n"),
							client->name,
							client->node,
							client->display,
							feature));
						}
						/*
						 *	Found a matching user/feature
						 */
						bFoundOne = 1;
						/*
						 *	Count the number of licenses, in case we need to delete this number
						 *	from the package itself.
						 */
						numlic = u->count;
/*-
 *						P1459 --
 *						used to f_drop all brothers
 */

/*-
 *						P365: MUST do the f_remove_all()
 *						after dropping the connections,
 *						since the userlist stuff will
 * 						change after f_remove_all()
 */
						/*
						 *	Save this info just in case we need it for ls_borrow_in
						 */
						memset(&tempUser, 0, sizeof(USERLIST));
						memcpy(&tempUser, u, sizeof(USERLIST));

						if(iForce && !ls_borrow_return_early)
						{
							ret = -3;
							return ret;
						}

						status = f_remove_all(cd, freeit,
							LL_REASON_USER_REMOVED, iForce, feature);
						if(!iForce)
						{
							/*
							 *	Since with a borrowed license the connection was severed as
							 *	part of the borrow (checkout followed by checkin), don't call f_drop
							 *	because the client entry has ALREADY been deleted.
							 */
							f_drop(u); /* P2144 */
						}
						if(status == LM_BORROW_RETURN_EARLY_ERR)
						{
							/*
							 *	Tried to return a borrowed license early but server doesn't support this.
							 */
							ret = -3;
						}
						if(status == 0 && tempUser.flags & LM_ULF_BORROWED && iForce)
						{
							/*
							 *	Only borrowed license can be returned early
							 */
							if(ls_borrow_in)
								sOutputBorrow(&tempUser, feature, DIRECTION_IN);
							/*
							 *	Log fact that this was returned early
							 */
							ls_log_usage(
								LL_LOGTO_REPORT,
								(long)tempUser.license_handle,
								0L ,
								LL_USAGE_IN,
								LL_REASON_REQUEST_SERVICED,
								tempUser.name,
								tempUser.node,
								tempUser.display,
								tempUser.vendor_def,
								feature,
								tempUser.code,
								0,
								tempUser.version,
								tempUser.count,
								tempUser.count,
								(int)tempUser.time,
								client,
								tempUser.group_handle,
								tempUser.flags,
								tempUser.linger);
						}
					}
					else
					{
						ret = -2;
					}
				}
				else
				{
					ret = -1;
				}
			}
		}
		if(!bFoundOne && f->next && f->next->feature &&
			strcmp(f->next->feature, feature) == 0)
		{
			f = f->next;
		}
		else
		{
			if(status == 0 && ret == 1 &&
				f->flags & LM_FL_FLAG_SUITEBUNDLE_COMPONENT &&
				f->package && f->package->feature && f->package->code)
			{
				FEATURE_LIST *	pParent = NULL;
				int				i = 0;
				int				iIgnoreExpire = 0;

				if(iForce && ls_borrow_return_early)
				{
					iIgnoreExpire = 1;
				}
				else
				{
					iIgnoreExpire = 0;	/* If linger time hasn't expired, don't delete it */
				}

				pParent = f_get_feat(f->package->feature, f->package->code, 1);
				if(pParent)
				{
					sRemove(client, pParent->feature, LL_REASON_USER_REMOVED,
						pParent->code, freeit, iIgnoreExpire, tempUser.count);
					if(status == 0 && tempUser.flags & LM_ULF_BORROWED && iForce)
					{
						/*
						 *	Only borrowed license can be returned early
						 */
						if(ls_borrow_in)
						{
							tempUser.dup_select = pParent->dup_select;	/* DUP setting for components different than parent */
							sOutputBorrow(&tempUser, pParent->feature, DIRECTION_IN);
						}
					}
				}
				if(f->flags & LM_FL_FLAG_BUNDLE_COMPONENT)
					sUnreserveLicense(f, client);
			}
			break;
		}
	}
	return ret;
}

/*-
 *	f_user_remove_handle() - Remove this user from this feature. - Used only
 *			  by the lmremove command.
 */


f_user_remove_handle(
	char *	feature,	/*- The feature */
	char *	handle,		/*- The feature handle */
	int		freeit,		/*- Free resulting USERLIST struct -- should be 1
							unless file-based */
	int		iForce)		/*	Used only when returning early a borrowed license */
{
	USERLIST *u, *head;
	int toosoon = 0;
	int logged = 0;
	int remove_linger = 0;
	CLIENT_DATA *client;
	int hand;
	int status = 0;

	hand = atoi(handle);
	u = f_get_license_handle(feature, hand, &head);	/*- Find this handle */
	if (u)
	{
/*-
 *	    We have the handle desired
 */
	    if (head)
			u = head;
	    LM_SET_ERRNO(lm_job, 0, 345, 0) /*- Flag that we found the feature */

	    if (ls_currtime - u->time >= ls_min_lmremove)
	    {
			head = u;
			client = ls_client_by_ulist(u);	/* P2144 */
	/*-
	 *		P1459 -- used to f_drop all brothers
	 */

	/*-
	 *		P365: MUST do the f_remove_all() after dropping the
	 *		connections, since the userlist stuff will change
	 *		after f_remove_all()
	 */
			if (client)
			{
				logged = 1;
				LOG((lmtext(
					"REMOVING %s@%s:%s from %s\n"),
					u->name, u->node, u->display, feature));
				LOG((lmtext(
				"by administrator request. (license handle %d)\n"),
								hand));
				LOG_INFO((INFORM, "The system administrator has \
						requested that the specified user \
						have all his/her licenses removed.  \
						This was done by running \"lmremove\""));
				status = f_remove_all(client, freeit,
							LL_REASON_USER_REMOVED, iForce, feature);
				if(!iForce)
				{
					/*
					 *	Since with a borrowed license the connection was severed as
					 *	part of the borrow (checkout followed by checkin), don't call f_drop
					 *	because the client entry has ALREADY been deleted.
					 */
					f_drop(u); /* P2144 */
				}
			}
			else
				remove_linger = -2;

	    }
	    else
	    {
			toosoon = -1;
	    }
	}
	if (remove_linger)
		return (remove_linger);
	else if (logged)
		return(logged);
	else
		return(toosoon);
}

/*-
 *	f_remove_all - remove this user from all feature lists
 */

int
f_remove_all(
	CLIENT_DATA *	client,		/*- The user */
	int				freeit,		/*- Free resulting USERLIST struct -- should be 1
									unless file-based */
	int				reason,		/*- from ls_log.h LL_REASONs_* */
	int				iForce,		/*	Set when returning a borrowed license early */
	char *			szFeature)	/*	if set, only remove user from this feature */
{
	FEATURE_LIST *f;
	USERLIST *u, *q, *b;
	int ret = 0;
	int iNumToFree = 0;

	if (!client)
		return 0;

	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		if(szFeature && strcmp(f->feature, szFeature))
			continue;
		for (u = f->u; u ; u = u->next)
		{
			for (b = u; b ; b = b->brother)
			{
				if( (b->handle == client->handle) ||
					(iForce && ls_borrow_return_early &&
						(b->flags & LM_ULF_BORROWED) &&
						((b->handle) == client->handle))
					)
				{
					int		iIgnoreExpire = 0;

					if(iForce && ls_borrow_return_early &&
						(b->flags & LM_ULF_BORROWED) && (b->handle == client->handle))
					{
						iIgnoreExpire = 1;
					}
					else
					{
						iIgnoreExpire = 0;	/* If linger time hasn't expired, don't delete it */
					}
					f_remove(client, f->feature, reason, f->code, freeit, iIgnoreExpire);
					/*
					 *	Handle unreserving stuff here.
					 */
					if((f->flags & LM_FL_FLAG_SUITEBUNDLE_COMPONENT))
					{
						iNumToFree = fCanDynamicUnreserve(f, client);
						if(iNumToFree)
						{
							sUnreserveLicense(f, client);
						}
						iNumToFree = 0;
					}

					goto removed_it;
				}
				else if(iForce && !ls_borrow_return_early)
				{
					/*
					 *	return an error to "lmborrow -return" indicating server doesn't allow
					 *	early return of licenses, that way it doesn't delete the local borrow info.
					 */
					ret = LM_BORROW_RETURN_EARLY_ERR;
				}
			}
		}
		for (q = f->queue; q ; q = q->next)
		{
			for (b = q; b ; b = b->brother)
			{
				if (b->handle == client->handle)
				{
					f_remove(client, f->feature, reason, f->code, freeit, 0);
					goto removed_it;
				}
			}
		}
removed_it:
		; /*- null statement */

	}
	return ret;
}


/*-
 *	f_remove_children(s) - Remove all children of s
 */

void
f_remove_children(void)
{
	FEATURE_LIST *f;
	USERLIST *u, *ub, *nextu, *prev, *thisu;
	int x;
	int numres;
	CLIENT_DATA *c;

	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		u = f->u;
		prev = (USERLIST *) NULL;
		while (u)
		{
		    nextu = u->next;
		    if (f->dup_select & LM_DUP_NONE)	/*- count duplicates */
		    {
				numres = count_res(f->opt);
				if (prev != NULL)/*- Update feature user list */
					prev->next = u->next;
				else
					f->u = u->next;
				if (u->o)
					return_reserved(u->o, f);

				c = ls_client_by_ulist(u);

				ls_log_usage(ls_log_in ?
					LL_LOGTO_BOTH : LL_LOGTO_REPORT,
					(long)u->license_handle,
					(long) 0,
					LL_USAGE_IN,
					LL_REASON_SERVER_CRASH, u->name,
					u->node, u->display,
					u->vendor_def, f->feature,
					f->code,
					f->pooled,
					u->version, u->count, u->count,
					(int)u->time, c, u->group_handle,
					u->flags, u->linger);
				LOG_INFO((INFORM, "The application daemon died\
				for some reason and hence N licenses of \
				FEATURE were automatically checked back in\
				at mm/dd/yy hh:mm"));
				UFREE(u)	/*- Update free list */
		    }
		    else
		    {
				for (ub = u; ub; ub = ub->brother)
				{
					CLIENT_DATA *junk = makeclient(ub);
					USERLIST * brother;
					CLIENT_DATA *ctmp;

					numres = count_res(f->opt);
					junk->use_vendor_def = 0;
					thisu = f_dup_remove(junk, u,
								f, prev, &x, &x, 1);
					ctmp = ls_client_by_ulist(u);

					if (thisu == u)
						brother = u->brother;
					else
						brother = u;

					ls_log_usage(LL_LOGTO_BOTH,
					(long)thisu->license_handle,
					brother ?
					  (long)brother->license_handle
					  : 0L,
					LL_USAGE_IN,
					LL_REASON_SERVER_CRASH,
					thisu->name, thisu->node,
					thisu->display,
					thisu->vendor_def,
					f->feature, f->code,
					f->pooled,
					u->version, thisu->count,
					thisu->count,
					(int) thisu->time, ctmp,
					thisu->group_handle, thisu->flags, thisu->linger);
					logres(f, numres,
					(long) thisu->license_handle);
					LOG_INFO((INFORM, "The application daemon died\
					for some reason and hence N licenses of \
					FEATURE were automatically checked back in\
					at mm/dd/yy hh:mm"));
				}
		    }
		    u = nextu;
		}
		while (f->queue)	/*- De-queue the next user */
		{
			if (f_dequeue(f, 1) == 0)
				break;
		}
	}
}


/*-
 *	f_list - return the list of users of this feature
 */
USERLIST *
f_list(
	char *			feature,		/*- The feature desired */
	char *			code,			/*- Encryption code for this feature */
	int *			n,				/*- The number of users (returned) */
	int *			nlic,			/*- The number of licenses in use (returned) */
	int *			tot_count,		/*- Total number of licenses available */
	USERLIST **		queue,			/*- Returned queue of users */
	OPTIONS **		options,		/*- Returned options list */
	int				noforce,		/*- Don't force exact feature/code match if daemon doesn't use all feature lines */
	FEATURE_LIST *	pFeature)		/*	If set, the actual feature to examine */
{
	FEATURE_LIST *f;

/*-
 *	See if there are any users for this feature.
 */
	if(pFeature)
	{
		f = pFeature;
	}
	else
	{
		f = f_get_feat(feature, code, noforce);
	}
/*-
 *	If we found the feature, return the users and queued users
 */
	if (f && f->conf)	/* Only return valid features */
	{
/*-
 *		First, return the options
 */
		*options = f->opt;

		/*
		 *  Check to see if we're serving this feature or not because if the number of
		 *  user_based entries is greater than that allowed, we don't serve this feature.
		 *  P6405.
		 */
		if(f->flags & LM_FL_FLAG_USER_BASED_ERR)
		{
			*tot_count = 0;
		}
		else
		{
			*tot_count = f->nlic + f->res;	/*- Total, including res. */
		}
		f_featcount(f, n, nlic);
		*queue = f->queue;
		return(f->u);
	}
	else
	{
/*-
 *		If the feature isn't there, no one has ever checked it out -
 *		return no users.
 */
		*n = *nlic = *tot_count = 0;
		*queue = (USERLIST *) NULL;
		*options = (OPTIONS *) NULL;
		return(NULL);
	}
}
/*-
 *	f_featcount() - Count the users and licenses of a feature in use.
 */

void
f_featcount(
	FEATURE_LIST *	f,
	int *			n,
	int *			nlic)
{
	USERLIST *u;
	int x;

	*n = *nlic = 0;
	for (u = f->u; u; u = u->next)
	{
		(*n)++;
		if (f->dup_select & LM_DUP_NONE) /*- count duplicates */
			(*nlic) += u->normal_count;
		else
			(*nlic) += f_count(u, &x, &x);
	}
}

/*-
 *	f_res_avail() - Return the number of available reservation
 *			slots for the specified user of this feature.
 */
static
int
f_res_avail(
	FEATURE_LIST *	fl,		/*- The feature desired */
	CLIENT_DATA *	client,	/*- User making request */
	int				want,	/*- How many we want (if we are grabbing them) */
	OPTIONS **		list)	/*- The listhead (if specfied, grab them and link them here) */
{
	int num = 0;
	int grabbed;
	OPTIONS *lasto;
	OPTIONS **linkto = list;

/*-
 *	See how many are available
 */
	if (fl->feature && *(fl->feature) && fl->opt)
	{
		OPTIONS *o;

		lasto = (OPTIONS *) NULL;

		o = fl->opt;
		while (o)
		{
			grabbed = 0;
			if (f_is_res_for_client(o, client))
			{

				num++;
/*-
*					If "list" was specified, then link the
*					structs to it and mark them as used.
*/
				if (want && list)
				{
					grabbed = 1;
					if (lasto)
						lasto->next = o->next;
					else
						fl->opt = o->next;
					*linkto = o;
					linkto = &(o->next);
					*linkto = (OPTIONS *) NULL;
					want--;
					if (!want)
						break;
				}
			}
			if (grabbed)
			{
				if (lasto)
					o = lasto->next;
				else
					o = fl->opt;
			}
			else
			{
				lasto = o;
				o = o->next;
			}
		}
	}
	return(num);
}

int
f_reserve_avail(
	FEATURE_LIST *	fl,		/*- The feature desired */
	CLIENT_DATA *	client,	/*- User making request */
	int				want,	/*- How many we want (if we are grabbing them) */
	OPTIONS **		list)	/*- The listhead (if specfied, grab them and link them here) */
{
	return f_res_avail(fl, client, want, list);
}

int
f_dynamic_res_avail(
	FEATURE_LIST *	fl,		/*- The feature desired */
	CLIENT_DATA *	client,	/*- User making request */
	int				want,	/*- How many we want (if we are grabbing them) */
	OPTIONS **		list)	/*- The listhead (if specfied, grab them and link them here) */
{
	int num = 0;
	int grabbed;
	OPTIONS *lasto;
	OPTIONS **linkto = list;

/*-
 *	See how many are available
 */
	if (fl->feature && *(fl->feature) && fl->opt)
	{
		OPTIONS *o;

		lasto = (OPTIONS *) NULL;

		o = fl->opt;
		while (o)
		{
			grabbed = 0;
			if(f_is_res_for_client(o, client) && o->type == DYNAMIC_RESERVE)
			{

				num++;
/*-
*					If "list" was specified, then link the
*					structs to it and mark them as used.
*/
				if (want && list)
				{
					grabbed = 1;
					if (lasto)
						lasto->next = o->next;
					else
						fl->opt = o->next;
					*linkto = o;
					linkto = &(o->next);
					*linkto = (OPTIONS *) NULL;
					want--;
					if (!want)
						break;
				}
			}
			if (grabbed)
			{
				if (lasto)
					o = lasto->next;
				else
					o = fl->opt;
			}
			else
			{
				lasto = o;
				o = o->next;
			}
		}
	}
	return(num);
}

FEATURE_LIST * findfeat(char *, char *, FEATURE_LIST *last);

static
int
sEnoughLicenses(
	FEATURE_LIST *	pFeature,
	int				iNumLic,
	int				ignore)
{
	FEATURE_LIST *	pParent = NULL;
	FEATURE_LIST *	pComponent = NULL;
	int				rv = 0;

	if(!pFeature || !iNumLic)
		return 0;

	if(pFeature->flags & LM_FL_FLAG_BUNDLE_COMPONENT && pFeature->package && pFeature->package->code)
	{
		pParent = f_get_feat(pFeature->package->feature, pFeature->package->code, 1);
		if(!pParent)
			goto done;
	}
	else if(pFeature->flags & LM_FL_FLAG_BUNDLE_PARENT)
	{
		pParent = pFeature;
	}

	if(pParent)
	{
		while(pComponent = findfeat(pParent->feature, NULL, pComponent))
		{
			if(ignore && (pComponent->flags & LM_FL_FLAG_BUNDLE_PARENT))
				continue;
			if(pComponent->nlic < iNumLic)
				goto done;
		}
		rv = 1;
	}

done:

	return rv;
}


int
f_dynamic_reserve(
	FEATURE_LIST *	pFeatEntry,		/*- The feature desired */
	char *			szUser,
	int				iNumLic,		/*- How many we want to reserve) */
	int				ignore)			/*	If set, ignore package itself */

{
	int				iRV = 0;
	OPTIONS *		pNewOptions = NULL;
	char			num[256] = {'\0'};

	if(pFeatEntry && szUser && iNumLic)
	{
		if(!sEnoughLicenses(pFeatEntry, iNumLic, ignore))
		{
			/*
			 *	Not enough license available, error out.
			 */

			iRV = -1;
			goto done;
		}

		/*
		 *	Build OPTION struct with number of features to be reserved
		 */
		sprintf(num, "%d", iNumLic);

		ls_build_reserve_opt(ignore ? DYNAMIC_RESERVE_EXCLUDE_PACKAGE : DYNAMIC_RESERVE, num, pFeatEntry->feature,
						"BUNDLE", szUser, pFeatEntry, 0);
	}
done:
	return iRV;
}

int
f_dynamic_reserve_remove(
	FEATURE_LIST *	pFeatEntry,		/*- The feature desired */
	char *			szUser,
	int				iNumLic)		/*- How many we want to free up */

{
	char		num[256]= {'\0'};

	if(pFeatEntry && szUser && iNumLic)
	{
		sprintf(num, "%d", iNumLic);
		ls_remove_opt(DYNAMIC_RESERVE, num, pFeatEntry->feature, "BUNDLE", szUser, pFeatEntry, 0);
	}
	return 0;
}


/*-
 *	f_dump - dump the entire feature/user database
 */

#ifdef INCLUDE_DUMP_CMD

void
f_dump(FILE * fd)	/*- File descriptor to do dump on */
{
	USERLIST *u, *b;
	FEATURE_LIST *f;
	int i, l, x;
	CLIENT_DATA *c;

	if (fd == (FILE *)0)
		fd = stdout;
	(void) fprintf(fd, "DUMP for %s\n", lm_job->vendor);
						/*- Flag start of list */
	i = 0;
	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		(void) fprintf(fd,
   "%s;#:%d;O:%d;V:%s;E:%s;Vdef:%s;C:%s;I:%s:T:%d;L:%d;D:%x%s;R:%d;Comp:%d\n",
			f->feature, f->nlic, f->overdraft, f->version,
			f->expdate, f->vendor_def ? f->vendor_def : "",
			f->code, f->id, f->timeout,
			f->linger, f->dup_select, f->sticky_dup ? "(S)" : "",
			f->res, f->flags & LM_FL_FLAG_SUITEBUNDLE_COMPONENT ? 1 : 0);
	}
	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		(void) fprintf(fd, "Users of \"%s\":\n", f->feature);
		i = 0;
		for (u = f->u; u; u = u->next)
		{
			if (((f->dup_select & LM_DUP_NONE) == 0) && u->brother)
			{
						/*- NOT count duplicates */
				(void) f_count(u, &x, &l);
			}
			else
				l = u->count;
			i++;
			(void)fprintf(fd, "((%d) %d:%d:%s@%s@%s(%s) %d v%s (L: %d)",
						u->license_handle,
						0,
						u->handle,
						u->name,
						u->node,
						u->display,
						u->vendor_def,
						l,
						u->version,
						u->linger);
			if ((c = ls_client_by_ulist(u)))
			{
				char string[MAX_INET+1];

				*string = 0;
				if (c)
					l_addr_to_inet( c->inet_addr, 4, string);
				(void) fprintf(fd, " (I: %s))", string);
			}
			else
				(void)fprintf(fd, ")");

			for (b = u->brother; b; b = b->brother)
			{
				(void) fprintf(fd,
					"\n  ->((%d) %d:%d:%s@%s@%s(%s) v%s (L: %d)",
					b->license_handle,
					0, b->handle, b->name, b->node,
					b->display, b->vendor_def,
					b->version, b->linger);
				(void) fprintf(fd, ")");
			}
			if (u->next)
				(void) fprintf(fd, "\t\t|\n\t\tV\n");
		}
		(void) fprintf(fd, "(%d USERS)\n", i);
		if (f->queue)
		{
			(void) fprintf(fd, "Users queued for \"%s\":\n",
							f->feature);
			for (u = f->queue; u; u = u->next)
			{
				(void) fprintf(fd, "((%d) %d:%d:%s@%s@%s(%s) %d v%s (L: %d)",
					u->license_handle,
					0, u->handle, u->name, u->node,
					u->display, u->vendor_def, u->count,
					u->version, u->linger);
				{
					char string[MAX_INET+1];
					CLIENT_DATA *d = ls_client_by_ulist(u);
					l_addr_to_inet( d->inet_addr, 4,
								string);
					(void) fprintf(fd, " (I: %s))", string);
				}
			}
			(void) fprintf(fd, "\n");
		}
	}
}
#endif



/*-
 *	f_count() - Return the number of unreserved licenses a user is using
 */

int
f_count(
	USERLIST *	u,
	int	*		tot_res,
	int *		max)	/*- Max # any one user is using (RESERVED + unreserved) */
{
	int n = 0;
	int totr = 0;

	while (u)
	{
		if (u->count > n)
			n = u->count;
		totr += (u->count - u->normal_count);
		u = u->brother;
	}
	*max = n;
	*tot_res = totr;
	return(n - totr);
}

/*-
 *	ls_feat_dump() - Dump all feature usage info to report log file
 */

void
ls_feat_dump()
{
	USERLIST *u, *b;
	FEATURE_LIST *f;
	int num;
	int max;
	int x;

	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		for (u = f->u; u; u = u->next)
		{
			CLIENT_DATA *c = ls_client_by_ulist(u);
			if(!c)
				c = makeclient(u);
			if(((f->dup_select & LM_DUP_NONE) == 0) && u->brother)
						/*- NOT count duplicates */
			{
				(void) f_count(u, &x, &max);
				num = max;
				(void) f_count(u->brother, &x, &max);
				num -= max;
			}
			else
				num = u->count;
/*
 *			The first user in the brother chain
 *			doesn't specify a brother
 */
			ls_log_usage(LL_LOGTO_REPORT,
					(long)u->license_handle, (long) 0,
					LL_USAGE_INUSE,
					LL_REASON_CLIENT_REQUEST, u->name,
					u->node, u->display, u->vendor_def,
					f->feature,
					f->code, f->pooled, u->version,
					u->count, num, (int) 0, c,
					u->group_handle, u->flags, u->linger);
			num = count_res(u->o);
			if (num > 0)
			{
				ls_log_res_usage(LL_LOGTO_REPORT,
						(long) u->license_handle,
						 LL_USAGE_INUSE, NULL,
						(int) NULL, num);
			}
			for (b = u->brother; b; b = b->brother)
			{
			    CLIENT_DATA *_c = ls_client_by_ulist(b);

				if(!_c)
					_c = makeclient(b);

				(void) f_count(b, &x, &max);
				num = max;
				if (b->brother)
				{
					(void) f_count(b->brother, &x, &max);
					num -= max;
				}
				ls_log_usage(LL_LOGTO_REPORT,
					(long)b->license_handle,
					(long)u->license_handle,
					LL_USAGE_INUSE,
					LL_REASON_CLIENT_REQUEST, b->name,
					b->node, b->display, b->vendor_def,
					f->feature,
					f->code, f->pooled, b->version,
					b->count, num, (int) 0,
					_c, b->group_handle, b->flags, b->linger);
				num = count_res(b->o);
				if (num > 0)
					ls_log_res_usage(LL_LOGTO_REPORT,
							(long) b->license_handle
						 	, LL_USAGE_INUSE,
							NULL, (int)NULL, num);
			}
		}
		if (f->queue)
		{
			for (u = f->queue; u; u = u->next)
			{
				CLIENT_DATA *c = ls_client_by_ulist(u);

				if(!c)
					c = makeclient(u);

				ls_log_usage(LL_LOGTO_REPORT,
					(long)u->license_handle,
					(long)0,
					LL_USAGE_INQUEUE,
					LL_REASON_CLIENT_REQUEST, u->name,
					u->node, u->display,
					u->vendor_def, f->feature,
					f->code, f->pooled,
					u->version, u->count, u->count,
					(int) 0, c, u->group_handle, u->flags, u->linger);
			}
		}
	}
}

/*-
 *	INTERNAL support routines
 */


/*-
 *	f_get_feat -- front end to f_get_feat_next
 */
FEATURE_LIST *
f_get_feat(
	char *	feature,	/*- The feature desired */
	char *	code,		/*- The feature's encryption code */
	int		noforce)	/*- Don't force exact match (for daemons that
							don't use all feature lines) */
{
	return f_get_feat_next((FEATURE_LIST **)0, feature, code);
}

/*-
 *	f_get_feat_next -- walks ls_flist, starting at start_here, looking
 *			for matching feature and (if not null) code.
 */
FEATURE_LIST *
f_get_feat_next(
	FEATURE_LIST **	start_here, /*- to walk the list, if no code is provided */
	char *			feature,	/*- The feature desired */
	char *			code)		/*- The feature's encryption code */
{
	return f_get_feat_next_flag_removed( start_here, feature, code, 0);
}

/*-
 *	f_get_feat_next_flag_removed -- like feat_next, but sets
 *	flag if the reason there are none is returned is that they
 *	were removed
 */
FEATURE_LIST *
f_get_feat_next_flag_removed(
	FEATURE_LIST **	start_here,		/*- to walk the list, if no code is provided */
	char *			feature,		/*- The feature desired */
	char *			code,			/*- The feature's encryption code */
	int *			removed_flag)	/* flag if the feature was removed in reread */
{
	FEATURE_LIST *f;
	OLD_CODE_LIST *op;
	FEATURE_LIST *sh;
	int flag = 0;

	if(!start_here || !*start_here)
		sh = ls_flist;
	else
		sh = *start_here;
	if(removed_flag)
		*removed_flag = 0;

	for (f = sh; f && f->feature && *(f->feature); f = f->next)
	{
		if (*feature && !l_keyword_eq(lm_job, f->feature, feature))
			continue;
		if ( (f->flags & LM_FL_FLAG_LS_HOST_REMOVED))
		{
			flag = 1;
			continue;
		}
/*-
 *		If the code is NULL (pre-v3 client), or the code compares,
 *		THEN DECLARE A MATCH
 */
		if ((*code == 0) ||
				!strcmp(code, CONFIG_PORT_HOST_PLUS_CODE) ||
				l_keyword_eq(lm_job, f->code, code) ||

				(f->conf && f->conf->lc_sign && /* P5259 */
				l_keyword_eq_n(lm_job, f->conf->lc_sign,
				code, strlen(code))))
		{
			break;
		}
/*-
 *		If we're in f_remove, check the old codes also, because
 *		reread may have changed the codes
 */
		if (ls_in_f_remove)
		{
			for (op = f->old_codes; op; op = op->next)
			{
				if (l_keyword_eq(lm_job, op->code, code))
					break;
			}
			if(op)
				break;
		}
		else if (ls_in_vd_info)
		{
			LS_POOLED *pool;
			for (pool = f->pooled; pool; pool = pool->next)
			{
				if (l_keyword_eq(lm_job, pool->code, code))
					break;
			}
			if (pool)
				break;
		}

	}

	if (f && (f->feature == (char *)NULL))
	{
		if (flag)
		{
			LM_SET_ERRNO(lm_job, LM_SERVER_REMOVED, 395, 0)
			if (removed_flag) *removed_flag = flag;
		}
		else
		{
			LM_SET_ERRNO(lm_job, LM_NOFEATURE, 346, 0);
		}
		f = (FEATURE_LIST *) NULL;
	}
	if (f && start_here)
		*start_here = f->next;
	else if (start_here)
		*start_here = (FEATURE_LIST *)0;
	return(f);
}

/*-
 *	f_get_license_handle() - Find a license_handle for a feature
 */

static
USERLIST *
f_get_license_handle(
	char *		feature,	/*- The feature desired */
	int			handle,		/*- User handle */
	USERLIST **	head)		/*- Head of userlist */
{
	FEATURE_LIST *f;
	USERLIST *u;

	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		if (l_keyword_eq(lm_job, f->feature, feature))
		{
			for (u = f->u; u; u = u->next)
			{
				if (u->license_handle == handle)
				{
					*head = u;
					break;
				}
				else if (u->brother)
				{
					USERLIST *j;

					for (j = u->brother; j; j = j->brother)
					{
						if (j->license_handle == handle)
						{
							*head = u;
							u = j;
							break;
						}
					}
					if(j)
						break;
				}
			}
			if(u)
				break;
		}
	}

	if (u == (USERLIST *)NULL)
	{
		LM_SET_ERRNO(lm_job, LM_NOFEATURE, 347, 0)
		f = (FEATURE_LIST *) NULL;
	}
	return(u);
}

/*-
 *	new_user() - Initialize new user data
 */
static
void
new_user(
	int			n,		/*- The number of users */
	USERLIST *	array)	/*- The array of new users to be initialized */
{
	int i;

	(void) bzero((char *) array, n * sizeof(USERLIST));
	for (i=0; i < n; i++)
	{
		array[i].next = &array[i+1];
		array[i].serialno = cur_serialno;
		array[i].seq = 0;
		cur_serialno++;
		if (cur_serialno > MAX_SERIALNO)
			cur_serialno = 1;
	}
	array[n-1].next = NULL;
}

/*-
 *	Recycle_user - Fix up a user from the free list
 */

static
void
recycle_user(USERLIST * u)	/*- The users to be recycled */
{
	u->seq++;
	if (u->seq > 99)
		u->seq = 0;
	u->flags = 0;
	u->license_handle = (u->seq % 100) + (u->serialno * 100);
}

/*-
 *	more_u() - allocate and initialize more USERLIST space
 */
static
void
more_u()
{
	USERLIST *u;

	u = (USERLIST *) LS_MALLOC(sizeof(USERLIST) * NUSERS_INC);
	u_free = u;
	new_user(NUSERS_INC, u);
}

/****************************************************************************/
/**	@brief	Check to see if this user is queued for this particular feature.
 *
 *	@param	pFeature	Feature in question
 *	@param	pUser		User in question
 *
 *	@return	Zero if not queued, else non zero.
 ****************************************************************************/
static
int
sCheckIfQueuedOnParent(
	FEATURE_LIST *	pFeature,
	USERLIST *		pUser)
{
	USERLIST *	pQueued = NULL;
	int			rv = 0;

	if(pFeature == NULL || pUser == NULL)
		goto done;
	for(pQueued = pFeature->queue; pQueued; pQueued = pQueued->next)
	{
		if(pQueued->handle == pUser->handle)
		{
			rv = 1;
			break;
		}
	}

done:
	return rv;
}

/*-
 *	Dequeue a user, if possible.  It may not be possible
 *	if everyone in the queue wants more licenses than we
 *	have.  Returns the number of licenses used by the
 *	dequeued user, if any.
 */
f_dequeue(
	FEATURE_LIST *	f,
	int				freeit)	/*- Free USERLIST struct -- should be 1
								unless file-based */
{
	USERLIST *u = NULL;
	char msg[LM_MSG_LEN+1] = {0};	/*- For ls_client_send */
	char szVendorDef[MAX_VENDOR_CHECKOUT_DATA + 1] = {0};
	int x = 0;
	CLIENT_DATA *cd = NULL;

	memset(msg, 0, sizeof(msg));
	u = f->queue;
	f->queue = (USERLIST *) NULL;	/*- So that f_add will do its work */
	cd = ls_client_by_ulist(u);

	if(!cd)
		return x;
	cd->lastcomm = ls_currtime;

	/*
	 *	sluu 3/10/03
	 *	This fixes bug P7118.
	 *	This is needed because the f_add function will set the vendor_def field
	 *	to that of CLIENT_DATA found in the call to ls_client_by_ulist, which
	 *	might not be the same as that of the USERLIST entry.  This causes
	 *	problems because the USERLIST entry that is added to the FEATURE_LIST
	 *	entry now contains the vendor def for the CLIENT_DATA entry.  When the
	 *	client tries to do a checkin of the license by setting CHECKOUT_DATA
	 *	to the value that the USERLIST entry is, the server can't find the user
	 *	that pertains to this checkout because in the FEATURE_LIST entry, the
	 *	value of vendor_def is that of the last checkout/checkin that had it set,
	 *	as opposed to what's happening here because of queuing.  To reproduce the problem,
	 *	do the following:
	 *	1. Create 2 job handles, job1, job2
	 *	2. Create 2 licenses, 1 for f1 and 1 for f2, only provide 1 license of each
	 *	3. Set checkout data to "1", then checkout "f1" on job1
	 *	4. Set checkout data to "2", then checkout "f1" on job2 with QUEUEING, this will queue
	 *	5. Set checkout data to "3", then checkout "f2" on job1
	 *	6. Set checkout data to "4", then checkout "f2" on job2 with QUEUEING, this will queue
	 *	7. Set checkout data to "3", then checkin "f2" on job1, "f2" with checkout data of "4" on job2 will dequeue
	 *	8. Set checkout data to "4", then checkin "f2" on job2
	 *	9. Set checkout data to "1", then checkin "f1" on job1, "f1" with checkout data of "2" on job2 will dequeue
	 *	HOWEVER, since the last time checkout data for job2 was set to "4", the license server will add a user to
	 *	the FEATURE_LIST entry for "f1" that has a checkout data value of "4" instead of "2".  Therefore, when you
	 *	set checkout data to "2" on job2 and checkin "f1", the server can't find the user because it's looking for "2"
	 *	and it has a user with a checkout data of "4".  To get around this, just change the vendor_def field to that of
	 *	the user before doing the f_add and switch it back afterwards.
	 */
	strncpy(szVendorDef, cd->vendor_def, MAX_VENDOR_CHECKOUT_DATA);
	strncpy(cd->vendor_def, u->vendor_def, MAX_VENDOR_CHECKOUT_DATA);

	if(f_add(cd, f->feature, u->count, u->version,
		       f->dup_select, u->linger, &(u->code[0]), 0, freeit, f) >= 0)
	{

		strncpy(cd->vendor_def, szVendorDef, MAX_VENDOR_CHECKOUT_DATA);
		ls_log_usage(LL_LOGTO_REPORT,
			(long)u->license_handle, (long)0,
			LL_USAGE_DEQUEUED, LL_REASON_REQUEST_SERVICED,
			u->name, u->node, u->display, u->vendor_def,
			f->feature,
			f->code, f->pooled, f->version, u->count, u->count,
			(int) 0, cd, u->group_handle, u->flags, u->linger);
		f->queue = u->next;	/*- Pop him off the queue */
		if (freeit)
			UFREE(u)		/*- Free this one */
		/*
		 *	Check to see if this is a component of a PACKAGE/SUITE
		 */
		(void)strcpy(&msg[MSG_DATA], f->feature);
		if(f->flags & LM_FL_FLAG_SUITE_COMPONENT)
		{
			/*
			 *	sluu:
			 *	If a component, check to see if package itself is queued, if
			 *	yes, don't send any message.  I know that this doesn't work
			 *	in all scenarios, but it's better than our current implementation
			 */
			if(sCheckIfQueuedOnParent(f->package, u) == 0)
			{
				ls_client_send(cd, LM_FEATURE_AVAILABLE, msg);
			}

		}
		else
		{
			if(f->flags & LM_FL_FLAG_SUITE_PARENT)
			{
				msg[MSG_QUEUED_PACKAGE] = 1;
			}
			ls_client_send(cd, LM_FEATURE_AVAILABLE, msg);
		}
		x = u->count;
	}
	else
	{
		strncpy(cd->vendor_def, szVendorDef, MAX_VENDOR_CHECKOUT_DATA);
		f->queue = u;	/*- Put it back */
	}
	return(x);
}

/*-
 *	return_reserved(list, feature) - return the list of reservations
 *	to the option list for the feature.
 */

static
void
return_reserved(
	OPTIONS *		list,
	FEATURE_LIST *	fl)
{
	OPTIONS *last = 0, *o, *next;

	for (o = fl->opt; o; o = o->next)
	{
		last = o;
	}
	for (o = list; o; o = next)
	{
		next = o->next;
		if (fl->reread_reserves)
		{
		  	/* first remove this item from "list" and free */
			free(o);
			/* move one reserve from f->reread_reserves to f->opt */
			if (last)
			{
				last->next = fl->reread_reserves;
				last = last->next;
			}
			else
			{
				last = fl->opt = fl->reread_reserves;
			}
			fl->reread_reserves = fl->reread_reserves->next;
			last->next = 0; /* terminate list */
		}
		else
		{
			if (last)
				last->next = o;
			else
				fl->opt = o;
			break; /* all done */
		}
	}
}

/*-
 *	f_dup_remove() - Remove a user from a duplicate list, for
 *			 servers that support grouping duplicates.
 */
static
USERLIST *
f_dup_remove(
	CLIENT_DATA *	user,
	USERLIST *		u,
	FEATURE_LIST *	fptr,
	USERLIST *		prev,
	int *			numlic,		/*- How many licenses this user REALLY had */
	int *			nback,		/*- How many licenses came back */
	int				freeit)		/*- Free the USERLIST struct  -- should be 1
									unless file-based */
{
	int x, n2, nlic1, res;
	USERLIST *this = u, *last = (USERLIST *) NULL;
	struct options *o;

	*nback = 0;
/*
 *	cycle through u's brothers looking for a dup w/ same user->handle
 */
	while (this)
	{

	if (((this->handle == user->handle) ||
			(this->handle == 0 &&						/* Added this to deal with BUNDLE and situation where */
			 user->handle == LM_LINGERED_HANDLE_VALUE &&	/* on a restart, the client and user don't match because the */
			 this->flags & LM_ULF_BORROW_INIT)) &&			/* way we "recreate" the borrow info.  Without this, the parent */
															/* feature would never have the user removed until server restart */
		    ls_is_duplicate(this, user, this->dup_select))
		{
			break;
		}
		last = this;
		this = this->brother;

	}
	if (!this)
	{
		*numlic = *nback = -1;
		return((USERLIST *) NULL);
	}
	*numlic = this->count;
/*-
 *	Count this feature before and after unlinking this user
 */
	nlic1 = f_count(u, &x, &x);

	if (last)
	{
		last->brother = this->brother;
	}
	else
	{
		int linknext = 1;

		if (this->brother == 0)
		{
			this->brother = this->next;
			linknext = 0;
			u = (USERLIST *) NULL;
		}
		else
			u = this->brother;
		if (prev)
			prev->next = this->brother;
		else
			fptr->u = this->brother;
		if (linknext)
			this->brother->next = this->next;
	}
	(void) f_count(u, &res, &n2);
/*-
 *	NOW, if we give this user back the way he is, we will
 *	be giving back x-res (== this->count - this->normal_count)
 *	reservations.  We may not want to do that.
 */
	if (this->o)
	{
		USERLIST *ul = u;
		struct options *temp;

		o = this->o;
		while (o && res < n2)
		{
/*-
 *			Link these reservations somewhere else
 */
			while (ul && ul->normal_count <= 0)
				ul = ul->brother;
			if (ul == 0)
				break;
			temp = o->next;
			o->next = ul->o;
			ul->o = o;
			o = temp;
			ul->normal_count--;
			res++;
		}
		if (o)			/*- Return what's left */
			return_reserved(o, fptr);
	}
	if (freeit)
		UFREE(this)		/*- Free it (already unlinked) */
	*nback = nlic1 - f_count(u, &x, &x); /*- How many REALLY came back */
	/*printf("t:%d l:%d %s %d this:%x\n", time(0)%100, __LINE__, "nback = ",*nback, this);*/
	return(this);			    /*- His handle */
}


void
f_remove_all_old(int freeit)
{
	FEATURE_LIST *f;

	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
		f_remove_old(f, freeit);
}

/*-
 *	f_remove_old() - Remove "timed-out" users of a feature
 */

static
void
f_remove_old(
	FEATURE_LIST *	f,
	int				freeit)	/*- Free resulting USERLIST struct  -- should be 1
								unless file-based */
{
	USERLIST *u, *nextu, *b, *nextb;
	long d = ls_currtime;


	if (f->timeout)
	{
		d -= f->timeout;
		if (d < 0)
			return; /* P2932 */
/*-
 *		Anyone we haven't heard from since 'd' should be removed.
 */
		for (u = f->u; u; u = nextu)
		{
			nextu = u->next;
			b = u->brother;
			if (!ls_since(u, d))
			{
				CLIENT_DATA *cd;

				cd = ls_client_by_ulist(u);

				f_remove_all(cd, freeit,
							LL_REASON_INACTIVE, 0, NULL);
				f_drop_client(cd);
			}

			while (b)
			{
				nextb = b->brother;
				if (!ls_since(b, d))
				{
				      f_remove_all(
					  ls_client_by_ulist(b),
						freeit, LL_REASON_INACTIVE, 0, NULL);
				      f_drop(b);
				}
				b = nextb;
			}
		}
	}
}

/*-
 *	ls_is_duplicate() - Compare a USERLIST to a CLIENT_DATA for
 *			duplicate.  Returns 1 if it is a duplicate.
 */
int
ls_is_duplicate(
	USERLIST *		user,
	CLIENT_DATA *	client,
	int				duplicate)
{
/*-
 *	If we were called with LM_DUP_NONE, check for the SAME user.
 *	Also, don't share licenses for linger-based borrowing.
 */

	if ((duplicate == LM_DUP_NONE) || client->borrow_seconds)
		duplicate = 	LM_DUP_USER |
				LM_DUP_HOST |
				LM_DUP_DISP |
				LM_DUP_VENDOR;
	else if (duplicate != user->dup_select)
		return 0;

	if ((duplicate & LM_DUP_USER) && strcmp(user->name, client->name))
		return 0;
	if ((duplicate & LM_DUP_HOST) && strcmp(user->node, client->node))
		return 0;
	if ((duplicate & LM_DUP_DISP) && strcmp(user->display, client->display))
		return 0;
	if ((client->use_vendor_def) && (duplicate & LM_DUP_VENDOR) &&
		     strcmp(user->vendor_def, client->vendor_def))
	     return 0;

	return(1);
}

/*-
 *	count_res() - Count the number of reservations in an options listhead
 */
static
count_res(OPTIONS *	o)
{
	int i = 0;

	while (o)
	{
		if (o->type == RESERVE || o->type == DYNAMIC_RESERVE)
			i++;
		o = o->next;
	}
	return(i);
}

/*-
 *	logres() - Log the number of reservations that changed
 */

static
void
logres(
	FEATURE_LIST *	f,
	int				orig,	/*- Original # of reservations */
	long			handle)	/*- Handle from user */
{
	int n = count_res(f->opt);

	if (n > orig)
		ls_log_res_usage(LL_LOGTO_REPORT, handle, LL_USAGE_IN, NULL,
							(int) NULL, n - orig);
	else if (n < orig)
		ls_log_res_usage(LL_LOGTO_REPORT, handle, LL_USAGE_OUT, NULL,
							(int) NULL, orig - n);
}

/*-
 *	f_filter() - Check that the user is INCLUDED, not EXCLUDED, and
 *			his DUP_GROUP mask matches.
 */

static
int
f_filter(
	CLIENT_DATA *	user,	/*- The user */
	FEATURE_LIST *	f,
	int				dup_sel)
{
/*-
 *	First, make sure this guy isn't EXCLUDED, or that he is on
 *	the INCLUDE list for this feature.
 */
	if (ls_exclude(user, f))
	{
		LM_SET_ERRNO(lm_job, LM_FEATEXCLUDE, 348, 0)
		return(lm_job->lm_errno);
	}
	if (!ls_include(user, f))
	{
		LM_SET_ERRNO(lm_job, LM_FEATNOTINCLUDE, 349, 0)
		return(lm_job->lm_errno);
	}
/*-
 *	If we haven't initialized the duplicate selection criteria for
 *	this type of license, do it now, unless the duplicate grouping
 *	was specified in the license.
 * 	If it's specified in the license, then there's no possibility
 *	of conflict between 2 checkouts.
 */
	if (!f->sticky_dup)
	{
		if (f->u == (USERLIST *) NULL)
		{
			f->dup_select = (short) dup_sel;
		}
		else if (f->dup_select != (short) dup_sel)
		{
			static int logged = 0;
			if (!logged)
			{
				logged = 1;
				LOG(( "Multiple dup-groupings in effect for %s:\n", f->feature));
				LOG(( "\t%s%s%s%s%s%s%s%s%s vs. %s%s%s%s%s%s%s\n" ,
					(dup_sel & LM_DUP_NONE) ? "NONE" : "",
					(dup_sel & LM_DUP_NONE) ? " " : "",
					(dup_sel & LM_DUP_USER) ? "USER" : "",
					(dup_sel & LM_DUP_USER) ? " " : "",
					(dup_sel & LM_DUP_HOST) ? "HOST" : "",
					(dup_sel & LM_DUP_HOST) ? " " : "",
					(dup_sel & LM_DUP_DISP) ? "DISPLAY" : "",
					(dup_sel & LM_DUP_DISP) ? " " : "",
					(dup_sel & LM_DUP_VENDOR) ? "VENDOR" : "",
					(f->dup_select & LM_DUP_NONE) ? "NONE" : "",
					(f->dup_select & LM_DUP_NONE) ? " " : "",
					(f->dup_select & LM_DUP_USER) ? "USER" : "",
					(f->dup_select & LM_DUP_USER) ? " " : "",
					(f->dup_select & LM_DUP_HOST) ? "HOST" : "",
					(f->dup_select & LM_DUP_HOST) ? " " : "",
					(f->dup_select & LM_DUP_DISP) ? "DISPLAY" : "",
					(f->dup_select & LM_DUP_DISP) ? " " : "",
					(f->dup_select & LM_DUP_VENDOR) ? "VENDOR":""));
				LOG(( "\tNo further warnings about this.\n"));
			}
		}
	}
	return(0);
}

/*-
 *	Drop a client connection
 */

void
f_drop_client(CLIENT_DATA *	c)
{
	USERLIST tmp;

	if (!c)
		return;
	tmp.handle = c->handle;
	f_drop(&tmp);
}

void
f_drop(USERLIST * him)
{
	int handle = him->handle;
	int linger = handle > 0 ? 0 : 1;
	CLIENT_ADDR *ca;


/*-
 *	A handle < 0 implies a lingering license
 */
	if (handle < 0)
		handle = -handle;

	ca = ls_addr_by_handle(handle);
/*-
*		Remove the sockets that connect this user, if tcp.
*/
	if (ca)
	{
		if (ca->is_fd)
		{
		    int fd = ca->addr.fd;
		    if (!linger)
			    ls_down((LM_SOCKET *)&fd, "removing user");
		}
		ls_oneless();	/*- One fewer client */
		if (!linger)
		{
			ls_delete_client(ca);
		}
	}
}

/*-
 *	Create a CLIENT_DATA struct from the USERLIST stuff
 */

static CLIENT_DATA junk;

static
CLIENT_DATA *
makeclient(USERLIST * u)
{
	bzero((char *) &junk, sizeof(CLIENT_DATA));
	l_zcp(junk.name, u->name, MAX_USER_NAME);	/* LONGNAMES */
	l_zcp(junk.node, u->node, MAX_SERVER_NAME);	/* LONGNAMES */
	l_zcp(junk.display, u->display, MAX_DISPLAY_NAME);	/* LONGNAMES */
	l_zcp(junk.vendor_def, u->vendor_def,MAX_VENDOR_CHECKOUT_DATA);/* LONGNAMES */
	l_zcp(junk.project, u->project,MAX_PROJECT_LEN);/* LONGNAMES */
	junk.use_vendor_def = 1;
	junk.handle = u->handle;
	return(&junk);
}

static void
f_delete_udp_client_if_done(CLIENT_DATA * client)
{

	FEATURE_LIST *f;
	USERLIST *u, *b;
	int fnd=0;

    if (client->lastcomm == LS_DELETE_THIS_CLIENT)
		return; /*already marked*/
	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		for (u = f->u; u; u = u->next)
		{
			if (u->handle == client->handle)
			{
				fnd = 1;
				break;
			}
			for (b = u->brother; b; b = b->brother)
			{
				if (b->handle == client->handle)
				{
					fnd = 1;
					break;
				}
			}
			if (fnd) break;

		}
		if (fnd) break;
	}
#ifndef RELEASE_VERSION
	if (l_real_getenv("DEBUG_HANDLE"))
	{
		(void) printf("f_delete_udp_client_if_done clienthandle%d port%d--%s\n",
			client->handle, ntohs(client->addr.addr.sin.sin_port),
				fnd?"not_deleting" : "deleting");
	}
#endif
		/*mark for deletion*/
	if (!fnd)
		client->lastcomm = LS_DELETE_THIS_CLIENT;
}

static
CLIENT_DATA *
ls_client_by_ulist(USERLIST * ulist)
{
	return ls_client_by_handle(ulist->handle);
}

/*-
 *	f_match_hostids -- return >0, success; 0, failure.
 */
static
int
f_match_hostids(
	HOSTID *		feat_hostid,
	CLIENT_DATA *	client)
{
	HOSTID *h;

	int already_have = 0;

	if (!feat_hostid)
		return 1; /*- always OK */
	for (h = feat_hostid; h; h = h->next)
	{
		if (h->type == HOSTID_ANY ||
			h->type == HOSTID_DEMO ||
			h->type == HOSTID_SERNUM_ID)
			return 1;

		if ( (h->type == HOSTID_INTERNET) ||
			(h->type == HOSTID_USER) ||
			(h->type == HOSTID_DISPLAY) ||
			(h->type == HOSTID_HOSTNAME)) already_have = 1;
	}

	if (!client->hostids && !already_have)
		return 0;
					/*- This may break file-based,
					    Matt, please check! */

	for (h = feat_hostid; h; h = h->next)
	{
		switch (h->type)
		{
		case HOSTID_USER:
			/* fix for P7190 */
			if (ls_hud_hostid_case_sensitive == 0)
			{
				if (L_STREQ_I(h->id.user, client->name))
					return 1;
			}
			else
			{
				if (L_STREQ(h->id.user, client->name))
					return 1;
			}
			break;
		case HOSTID_DISPLAY:
			/* fix for P7190 */
 			if (ls_hud_hostid_case_sensitive == 0)
			{
				if (L_STREQ_I(h->id.display, client->display))
					return 1;
			}
			else
			{
				if (L_STREQ(h->id.display, client->display))
					return 1;
			}

			break;
		case HOSTID_HOSTNAME:
			/* fix for P7190 */
			if (ls_hud_hostid_case_sensitive == 0)
			{
				if (L_STREQ_I(h->id.host, client->node))
					return 1;
			}
			else
			{
				if (L_STREQ(h->id.host, client->node))
					return 1;
			}

			break;
		case HOSTID_INTERNET:
			if (!l_inet_cmp(h->id.internet, client->inet_addr))
				return 1;
			break;
		case HOSTID_DEMO:
		case HOSTID_ANY:
			return 1;
		default: /*- real hostid */
		    {
				int rc;
				char sav_h[MAX_HOSTNAME + 1];

				if (!client->hostids)
				{
					DLOG(("Nil hostid"));
					break;
				}
				gethostname(sav_h, MAX_HOSTNAME);
				lc_set_attr(lm_job, LM_A_HOST_OVERRIDE,
						(LM_A_VAL_TYPE)client->node);
				rc  = l_hostid_cmp(lm_job, client->hostids, h);
				lc_set_attr(lm_job, LM_A_HOST_OVERRIDE,
							(LM_A_VAL_TYPE)sav_h);
				if (rc)
					return 1;
		    }
		}
	}
	return 0;
}

/*
 *	f_brother_head -- get me to the head of this list of brothers
 *	Add to fix P1460
 */

static
USERLIST *
f_brother_head(
	FEATURE_LIST *	fptr,
	USERLIST *		brother)
{
	USERLIST *u, *b;

	for (u = fptr->u; u; u = u->next)
		for (b = u; b; b = b->brother)
			if (b == brother) return u;
	DLOG(("INTERNAL ERROR %s line %d\n", __FILE__, __LINE__));

	/* should never happen! */
	return brother; /* so it doesn't core dump */
}

/*
 *	Return true if this opt list can apply to client
 */
static
int
f_is_res_for_client(
	OPTIONS *		o,
	CLIENT_DATA *	client)	/*- User making request */
{
/*-
 *	This one's a reservation.  If we match
 *	the user, host, or display name, or we
 *	are a member of the group, then
 *	we have one.  If it is free, it is
 *	available to this user.
 */
	if(o->type == DYNAMIC_RESERVE)
	{
		/*
		 *	Check to see what SUITE_DUP_GROUP matches this client
		 */
		if(o->type2 & OPT_BUNDLE)
		{
			if(o->dup_select & LM_DUP_USER && o->pszUser && strcmp(client->name, o->pszUser))
				return 0;
			if(o->dup_select & LM_DUP_HOST && o->pszHost && strcmp(client->node, o->pszHost))
				return 0;
			if(o->dup_select & LM_DUP_DISP && o->pszDisplay && strcmp(client->display, o->pszDisplay))
				return 0;
			if(o->dup_select & LM_DUP_VENDOR && o->pszVendor && strcmp(client->vendor_def, o->pszVendor))
				return 0;
			return 1;
		}
		return 0;
	}
	else if(
			(o->type == RESERVE &&
				(
					(((o->type2 | OPT_HOST) && o->name) && !strcmp(client->node, o->name)) ||
					(((o->type2 | OPT_USER) && o->name) && !strcmp(client->name, o->name)) ||
					(((o->type2 | OPT_GROUP) && o->name) && ls_ingroup(OPT_GROUP, client, o->name)) ||
					(((o->type2 | OPT_HOST_GROUP) && o->name) && ls_ingroup(OPT_HOST_GROUP, client, o->name)) ||
					(((o->type2 | OPT_DISPLAY) && o->name) && !strcmp(client->display, o->name)) ||
					(((o->type2 | OPT_PROJECT) && o->name) && client->project && !strcmp(client->project, o->name)) ||
					(((o->type2 | OPT_INTERNET) ||	(o->type2 == OPT_HOST && !o->name))	&&
						!l_inet_cmp(&client->inet_addr[0], &o->inet_addr[0])
				)
			)
			)
			)
	{
		return 1;
	}
	else
		return 0;
}

/*
 *	make sure we don't have any borrowed features
 */
int
f_borrowed(void)
{
	FEATURE_LIST *l;
	USERLIST *u, *b;
	int ret = 0;
	int t;
	for (l = ls_flist; l; l = l->next)
	{
		for (u = l->u; u; u = u->next)
		{
			if (u->flags & LM_ULF_BORROWED)
			{
				t = u->endtime + u->linger;
				LOG(("BORROWED: %s %s/%s/%s exp: %d minutes\n",
					l->feature, u->name, u->node,
					u->display, (t - ls_currtime) / (60 )));
				ret = 1;
			}
			for (b = u->brother; b; b = b->brother)
			{
				if (b->flags & LM_ULF_BORROWED)
				{
					t = u->endtime + u->linger;
				LOG(("BORROWED: %s %s/%s/%s exp: %d minutes\n",
						l->feature, b->name, b->node,
						b->display,
						(t - ls_currtime)/(60 * 60)));
					ret = 1;
				}
			}
		}
	}
	return ret;
}



/*-
 *	Note about linger optimization in f_remove:
 *	The idea is that if I am lingering and checkin, there is no reason for
 *	me to linger if one of my brothers (who is still checked out) is going
 *	to linger later.
 *
 *	This was an optimization, to keep the data structures from blowing up.
 *	Consider someone doing a 12-hour build, with 15-minute linger.  In 15
 *	minutes, you can probably compile a couple of thousand files on a
 *	fast machine.  And without this optimization, all couple of thousand
 *	of those client structures are hanging around (in the brother list!)
 *	to be processed EACH TIME anyone does anything.
 */

/*-  		THIS NOTE MUST BE THE END OF THIS FILE.


TECHNICAL NOTES ON ls_feature.c 			6/27/96
__________________________________________________________________

Structures:

	There's 2 primary lists in the vendor daemon (vd):

		o (FEATURE_LIST *)ls_flist
		o the client database (see ls_handle.c and ls_client_db.c)

CLIENT DATABASE:

        The CLIENT_DATA database has records corresponding to every client
        application's flexlm "job".  When using TCP, each client
        corresponds to a socket fd.  Each record is called a "client"
        in this doc.

FEATURE_LIST:

	This is a linked list of features supported by the vd.
	A particular line may consist of several INCREMENT
	lines from a license file, "pooled" together.  Also,
	several INCREMENT or FEATURE lines may have independent
	FEATURE_LIST records.

	The layout is

		f1->f2->f2->f3->...

		for (fptr = ls_flist; *fptr; fptr = fptr->next)

	traverses this list, and

		f_get_feat(name, license-key)

	gets an entry given a feature name and 20-char key.

		(void) f_featcount(fptr, &n, &nlic)

	is used to count the number of users and number of licenses
	currently in use by this feature "pool" (FEATURE_LIST *).

		f_list(feature-name, key, &num_users, &num_lic_in_use,
			&num_lic_avail, &queue, &options, [unused_arg], pFeature)

	f_list returns general information about the feature/key.
	It may return NOSERVSUPP it this one has been pooled with
	a different key. f_list() uses f_featcount().

USERLIST:

	When a client checks out a license for a feature, there
	is a USERLIST (user) record linked to the fptr:

		f1->f2->f2->f3
		|       |   |
		u1	u1  u5
		|	|   |
		u2	u4  u5
			|
			u2

		u = f_lookup(client, feature-name,
			&prev, &fptr, LOOKUP_ALL|EXACT, key,
			LOOKUP_ALL|IGNORE_LINGERS, start_here);

        u is the user for this fptr/client.  Note that there may be
	MORE user entries for this same client in this list, if and only if,
        CHECKOUT_DATA (vendor_def) is used, and they each have different
	vendor_def data.  More on vendor_def later...

	The relation between USERLIST and CLIENT_DATA is:

		o user->handle == client->handle, and handle uniquely
		  identifies a client.
		o A single client can have many USERLIST records.

	f_lookup -- given a client, key, it finds the fptr,
	and u (returned value).  If LOOKUP_EXACT, it matches the handle.
	Otherwise, it finds a the first that matches based on dup-group,
	usually the head of a brother list.  If feature is blank, then
	start_here is used, and f_lookup can be used to get to all
	features for this client (to support LM_CI_ALL_FEATURES).

Dup-Grouping:

	The dup-grouping mask is specified in the fptr->dup_select.

		ls_is_duplicate(user, client, dup)

	If dup_select & LM_DUP_NONE, then no dup-grouping is used.
	determines if an existing USERLIST entry, hanging off a
	FEATURE_LIST, and a client sending a message, are considered
	duplicates.  If so, the tree grows as follows:

		f1->f2->...
		|
		u1->u6->u7->...
		|
		u2

	These duplicates are called "brother"s.

		f_count(u1, &tot_res, &max)

	max: 	the highest number of licenses used by any one user
	tot_res: the total number of reservations used up by this feature
	returns: max - tot_res

		Number checked out	1   3   2
					u1->u6->u7

	f_count is used (and needed) only when duplicates are used
	for this feature.  It's used by f_featcount().

	When removing a license if (f->dup_select & LM_DUP_NONE) is true,
	then f_dup_remove() is to correctly handle the duplicate grouping.

Dup-Grouping and vendor-def CHECKOUT_DATA:

	The major complication with dup-grouping is vendor_def.
	Normally, a client has a single USERLIST record for a given key.
	CHECKOUT_DATA is the one exception -- in this case a
	client can have n USERLIST for a given key, each with different
	checkout data.

	When a client exits, if CHECKOUT_DATA is not being used,
	we can free all USERLISTs that share the same client->handle.
	But with CHECKOUT_DATA, we have to proceed DIFFERENTLY if the
	client has been disconnected from when it has called checkin.
	If it's disconnected, then all USERLIST entries for a given
	handle are freed, but if checkin is called, then only those
	with the vendor_def from the checkin call.

RESERVATIONS:

        RESERVATIONS are a list of type (OPTIONS *).  At first
        reservations hang off the fptr->opt field.

		f1->f2
	       /  \
	     u1    r1
	     |     |
	     u2    r2
	     |
	     u3->u4
	     |
	     u5

	If, in the example, all uses are for 1, then 4+2=6 licenses are
	in use.  If a user occupies a reservation, then, the reservation
	is temporarily moved to hang off the user struct (from the
	(OPTIONS *)o field in USERLIST):

		f1------>f2
	       /  \
	     u1    r2
	     |
	     u2
	     |
	     u3->u4
	     |
	     u5
	     |
	     u6
	       \
		r1

	In this example, u6 occupies no NEW licenses, and therefore
	the status quo is largely unchanged.  The counting could be:

		5 (users) + 1 (unused reservation) = 6 (in use)

*/

