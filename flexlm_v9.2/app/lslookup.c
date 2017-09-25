/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lslookup.c,v 1.9 2003/01/13 22:22:33 kmaclean Exp $
 *
 *	Function: 	f_lookup()
 *
 *	D. Birns
 *	9/24/96
 *
 *	Last changed:  1/12/99
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lmselect.h"
#include "lsserver.h"
#include "ls_glob.h"
#include "lsfeatur.h"		/*- Dynamic feature data */
#include "ls_aprot.h"   
/*-
 *	f_lookup() - given client/feature/code, lookup "user" in "featurelist"
 */
USERLIST *
f_lookup(
	CLIENT_DATA *		client,			/*- The user */
	char *				feature,		/*- The feature desired */
	USERLIST **			prev,			/*- The previous user in the list */
	FEATURE_LIST **		fptr,			/*- Pointer to the feature entry */
	int					exactuser,		/*- Return EXACT user if 1, otherwise return any DUP */
	char *				code,			/*- Encryption code from feature */
	int					ignore_lingers,	/*- If true, if handle < 0, don't return it */
	FEATURE_LIST **		start_here,		/*- If feature name is not set, do all */
	int					dup_sel)
{
	FEATURE_LIST *f, *next;
	USERLIST *u;
	int done = 0;
	int dup;

	next = start_here ? *start_here : 0;

	while (1)
	{
		*fptr = (FEATURE_LIST *)NULL;
		f = f_get_feat_next(next ? &next : 0, feature, code);
		*fptr = f;		/*- Return the feature pointer */
		if (!f || !client)
		{
			return 0;
		}
/*-
 *		We have the feature desired, now go get the user
 */

		LM_SET_ERRNO(lm_job, 0, 353, 0); /*- Flag that we found the feature */
		*prev = (USERLIST *) NULL;	/*- In case the first matches */

		for (u = f->u; u; u = u->next)
		{
			USERLIST *firstbrother = u;

			if (f->sticky_dup)
				dup = f->dup_select;
			else
				dup = dup_sel;

			for (; u; u = u->brother)
			{
				/*
				 *	This is needed because the USERLIST entry was created by the
				 *	license server at startup time with data it obtained via
				 *	ls_borrow_init.  That data DOES NOT include the USERLIST.handle
				 *	field, if this weren't here, the "if"
				 *	test would fail and would result in the function returning the 
				 *	same entry forever.
				 */
				if (ignore_lingers &&
						(u->handle < 0 || (u->handle == 0 && u->flags & LM_ULF_BORROW_INIT)))
				{
					continue;
				}
/*-
 *		If we want the exact user, we use LM_DUP_NONE in the
 *		ls_is_duplicate call, since we don't care what the feature's
 *		duplicate grouping mask is (EXACT).  If we don't want
 *		the exact user, however, we use the feature's duplicate
 *		grouping mask if we are grouping duplicates.  Whew!
 */
				if (exactuser)
				{
					if ((u->handle == client->handle) &&
					ls_is_duplicate(u, client, LM_DUP_NONE))
					{
						done = 1; 
						break;
					}
				}
				else if (!(dup & LM_DUP_NONE))
				{
					if (ls_is_duplicate(u, client, dup))
					{
						done = 1; 
						break;
					}
				}
				else
				{			/*- Count duplicates */
					if ((u->handle == client->handle) &&
					ls_is_duplicate(u, client, dup))
					{
						done = 1; 
						break;
					}
				}
			}
			if (!done)
			{
				u = firstbrother;/*- Back to brother list hd*/
				*prev = u;	/*- Update the last one */
			}
			else
				break;
		}
/*
 *		if (!LM_CI_ALL_FEATURES) or we've already checked them all 
 *		then we're done -- don't reset_start until we try this
 *		feature again and it fails. P2765 LM_CI_ALL_FEATURES w/
 *		vendor_def
 */
		if (u || *feature || !*start_here)
			break;

		if (!*feature && start_here)
			*start_here = next;
	}
	return(u);	/*- NULL or non-NULL, "u" is what we want */
}
