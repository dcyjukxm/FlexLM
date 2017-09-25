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
 *	Module: $Id: ls_ubase.c,v 1.6 2003/01/13 22:22:38 kmaclean Exp $
 *
 *	Function: ls_user_based
 *
 *	Description: 	enforce user/host-based licensing
 *
 *	Parameters:	none
 *
 *	Return:		0 success
 *			1 failure
 *
 *	D. Birns
 *	9/1/95
 *
 *	Last changed:  7/15/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "lsfeatur.h"
int cntgrp lm_args(( char *, GROUP *));
int cntlist lm_args((char *));
static void missing lm_args((char *,FEATURE_LIST *,char *));

extern FEATURE_LIST *ls_flist;
extern GROUP *groups;
extern GROUP *hostgroups;

void
ls_user_based(options, vendor_name)
char *options;
char *vendor_name;
{
  FEATURE_LIST *fl;
  OPTIONS *o;
  int cnt;
  int printed = 0;

	for (fl = ls_flist; fl; fl = fl->next)
	{
		if ((fl->type_mask & LM_TYPE_USER_BASED) ||
		    (fl->type_mask & LM_TYPE_HOST_BASED))
		{
			for (cnt = 0, o = fl->include; o; o = o->next)
			{
				if (o->type2 == OPT_GROUP && (fl->type_mask &
							LM_TYPE_USER_BASED))
				{
					cnt += cntgrp(o->name, groups);
				}
				else if (o->type2 == OPT_HOST_GROUP && 
					(fl->type_mask & LM_TYPE_HOST_BASED))
				{
					cnt += cntgrp(o->name, hostgroups);
				}
				else if (((fl->type_mask & LM_TYPE_USER_BASED)&&
						(o->type2 == OPT_USER)) ||
					((fl->type_mask & LM_TYPE_HOST_BASED) &&
						(o->type2 == OPT_HOST)))
					cnt ++;
			}
			if (!cnt || ((fl->type_mask & LM_TYPE_USER_BASED) &&
				cnt > fl->user_based) ||
				(((fl->type_mask & LM_TYPE_HOST_BASED) &&
				cnt > fl->host_based))) 
			{
				LOG((lmtext( "%s license error for %s "),
					fl->type_mask & LM_TYPE_USER_BASED ? 
					LM_LICENSE_USER_BASED :
					LM_LICENSE_HOST_BASED ,
					fl->feature));
                /*
                 *  TODO    555
                 *  Set number of licenses to 0 here to let user know that we're
                 *  not serving licenses.
                 */
				if (!cnt)
				{
					_LOG((lmtext("(INCLUDE missing)\n")));
					if (!printed)
					{
						printed = 1;
						missing(options, fl, vendor_name);
					}
				}
				else
				{
                    _LOG((lmtext("--\n")));
                LOG((lmtext("   Number of INCLUDE names (%d) exceeds limit of %d\n"),
					    cnt, 
                        fl->type_mask & LM_TYPE_USER_BASED ? 
						fl->user_based :
						fl->host_based));
				}
				fl->flags |= LM_FL_FLAG_USER_BASED_ERR;
			}
            else    /*  Let user know how close they are to USER_BASED limit */
            {
                LOG((lmtext("Number of INCLUDED entries for %s "
                     "feature %s is %d,\n"),
                     fl->type_mask & LM_TYPE_USER_BASED ?
                        LM_LICENSE_USER_BASED : LM_LICENSE_HOST_BASED,
                        fl->feature, cnt));
                LOG((lmtext("  maximum is %d\n"),
                    fl->type_mask & LM_TYPE_USER_BASED ?
                        fl->user_based : fl->host_based));

            }
		}
	}
}
int
cntgrp(name, group)
char *name;
GROUP *group;
{
  GROUP *g;

	for (g = group; g; g = g->next)
	{
		if (l_keyword_eq(lm_job, g->name, name))
			return cntlist(g->list);
	}
	return 0;
}
int
cntlist(list)
char *list;
{
  char *cp;
  int cnt = 0;
	
	for (cp = list; cp && (cp = strchr(cp, ' ')); cnt++)
		cp++;
	return ++cnt;
}
static
void
missing(options, fl, vendor_name)
char *options;
FEATURE_LIST *fl;
char *vendor_name;
{
	LOG((
	lmtext("Please ensure that no more than %d %s for %s \n"),
			fl->type_mask & LM_TYPE_USER_BASED ? 
				fl->user_based : fl->host_based, 
			fl->type_mask & LM_TYPE_USER_BASED ? "users" : "hosts",
				fl->feature));
	LOG((lmtext( "  are in an INCLUDE list in the daemon options file.\n")));
	if (!options && !*options)
	{
		LOG((lmtext("To set up your options file, modify the line:\n")));
		LOG((lmtext("             DAEMON %s .../%s\n"), vendor_name,
			vendor_name));
		LOG((lmtext("to read:\n")));
		LOG((lmtext("             DAEMON %s .../%s <pathname>/%s.opts\n"),
			vendor_name, vendor_name, vendor_name));
		LOG((
		lmtext(
		"   where <pathname> is the directory where you place \n")));
		LOG((lmtext("   %s.opts\n"), vendor_name));
		LOG((
		lmtext("The options file should contain lines like this:\n")));
		LOG((lmtext("  INCLUDE %s %s <username>\n"), fl->feature,
					fl->type_mask & LM_TYPE_USER_BASED ? 
					"USER" :
					"HOST"));
	}
}
