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
 *	Module: $Id: ls_user_opts.c,v 1.19 2003/06/17 23:32:00 jwong Exp $
 *
 *	Functions: ls_exclude(user, feature) - Is user EXCLUDEd from feat.
 *		   ls_include(user, feature) - Is user INCLUDEd in feature.
 *
 *	Description: These routines manipulate the server's internal
 *			user options database.
 *
 *	Note that all users are included in a feature if the feature has
 *	no include list.
 *
 *	Parameters:	(CLIENT_DATA *) user - client data structure
 *			(FEATURE_LIST *) feature - The feature pointer
 *
 *	Return:		0 - condition not true, 1 if true.
 *
 *	M. Christiano
 *	6/14/90
 *
 *	Last changed:  11/16/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsfeatur.h"		/* Dynamic feature data */
#include "ls_aprot.h"
#ifdef MOTO_88K
#include <sys/file.h>
#include <sys/uio.h>
#endif

OPTIONS *includeall = (OPTIONS *) NULL;		/* INCLUDEALL structs */
OPTIONS *excludeall = (OPTIONS *) NULL;		/* EXCLUDEALL structs */
OPTIONS *b_includeall = (OPTIONS *) NULL;	/* INCLUDEALL structs */
OPTIONS *b_excludeall = (OPTIONS *) NULL;	/* EXCLUDEALL structs */

/* Fix for P7208 */
int getgroupcaseinsensitiveflag();

static
inoption lm_args(( CLIENT_DATA *, OPTIONS *, OPTIONS *));

/*
 *	ls_exclude() - Is user excluded from feature
 */
ls_exclude(CLIENT_DATA *user, FEATURE_LIST *feature)
{
  OPTIONS *excl = user->borrow_seconds ? feature->b_exclude : feature->exclude;
  OPTIONS *exclall = user->borrow_seconds ? b_excludeall : excludeall;
	return(inoption(user, excl, exclall));
}

/*
 *	ls_include() - Is user included in feature
 */
ls_include(CLIENT_DATA *user, FEATURE_LIST *feature)
{
  OPTIONS *incl = user->borrow_seconds ? feature->b_include : feature->include;
  OPTIONS *inclall = user->borrow_seconds ? b_includeall : includeall;
	if (!incl && !inclall)
		return 1;
	else
		return(inoption(user, incl, inclall));
}

/*
 *	Support functions
 */


/*
 *	inoption() - Is the user in the specified option class.
 */
static
inoption(CLIENT_DATA *user, OPTIONS *option, OPTIONS *option2)
{
	OPTIONS *o, *o2;
	int i;

	for (i=0; i < 2; i++)
	{
		if (i == 0) 	o2 = option;
		else 		o2 = option2;

		for (o = o2; o; o = o->next)
		{
			if (o->type2 == OPT_HOST && o->name)
			{
				if (L_STREQ(o->name, user->node)) return(1);
			}
			else if (o->type2 == OPT_USER)
			{
				if (L_STREQ(o->name, user->name)) return(1);
			}
			else if (o->type2 == OPT_DISPLAY)
			{
				if (L_STREQ(o->name, user->display)) return(1);
			}
			else if (o->type2 == OPT_HOST_GROUP)
			{
				if (ls_ingroup(OPT_HOST_GROUP, user, o->name))
					return(1);
			}
			else if (o->type2 == OPT_GROUP)
			{
				if (ls_ingroup(OPT_GROUP, user, o->name))
					return(1);
			}
			else if ((o->type2 == OPT_INTERNET) ||
					((o->type2 == OPT_HOST) && !o->name))
			{
			  int j;
				for (j=0; j<=3; j++)
				{
					if ((o->inet_addr[j] != (short) -1 &&
						o->inet_addr[j] !=
							user->inet_addr[j]))
						break;
				}
				if (j == 4) return(1);	/* All 4 bytes match */
			}
			else if (o->type2 == OPT_PROJECT)
			{
				if (user->project &&
					L_STREQ(o->name, user->project))
				return(1);
			}
		}
	}
	return(0);
}

/*
 *	ls_ingroup() - Is the user in the specified group.
 *	If so, it returns the string that matches
 */

GROUP *groups = (GROUP *) NULL;			/* Server's group definitions */
GROUP *hostgroups = (GROUP *) NULL;		/* Server's group definitions */

int
ls_ingroup(int type, CLIENT_DATA *c, char *group)
{
	GROUP *	g = NULL;
	char *	str = (type == OPT_GROUP) ? c->name : c->node;
	char *	cp = NULL, * next = NULL;
	char *	szEnvReturn = NULL;

	/*
	 *	First, find the group
	 */
	for (g = (type == OPT_GROUP) ? groups : hostgroups; g; g = g->next)
		if (L_STREQ(g->name, group)) break;
	if (g)
	{

		for (cp = g->list; cp && *cp; cp = next)
		{
			short inet[4];
			char t;
			int putback = 0;

			for (next = cp; *next && !isspace(*next); next++)
				;

			if (*next)
			{
				putback = 1;
				*next = 0; /* null terminate, but put back later */
			}
			if (l_is_inet(cp))
			{

				l_inet_to_addr(cp, &t, &inet[0]);
				if (!l_inet_cmp(&c->inet_addr[0], &inet[0]))
				{
					if (putback)
						*next = ' ';
					return 1;
				}
			}

			/* Fix for P7208 */
			/* default setting for ls_group_case_insensitve == 0 */
			if ((getgroupcaseinsensitiveflag()) == 0)
			{
				if (L_STREQ(cp, str))
				{
					if(putback)
						*next = ' ';
					return 1;
				}
			}
			else
			{
				if (L_STREQ_I(cp, str))
				{
					if (putback)
						*next = ' ';
					return 1;
				}
			}
			if (putback)
				*next = ' ';
			while ((isspace(*next)) && (*next))
				next++;
		}
	}
	return(0);
}
