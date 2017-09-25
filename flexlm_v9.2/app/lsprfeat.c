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
 *	Module: $Id: lsprfeat.c,v 1.12.12.3 2003/07/01 17:04:21 sluu Exp $
 *
 *	Function: 	ls_print_feats
 *
 *	Description: 	lists supported features
 *
 *	Parameters:
 *
 *	Return:
 *
 *	D. Birns
 *	10/23/95
 *
 *	Last changed:  1/6/99
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
#include "flex_utils.h"

extern int ls_i_am_lmgrd;		/* We are not lmgrd */
extern char *prog;			/* Our program path */
extern int *server_tab;		/* Table of server PIDs */
extern char ls_our_hostname[];
extern int ls_allow_lmremove;
extern int ls_imaster;
extern int ls_flexlmd;
extern int ls_user_based_reread_delay;
#define FEAT_PER_LINE 3		/* Number of features to print per line */

static void ls_print_one_feat lm_args(( FEATURE_LIST *, char *, int *, int, int *));

void
ls_print_feats()
{
  FEATURE_LIST *f, *f2;
  int gotone = 0;
  int gotone_counted = 0;
  int featcnt = 2;
  int user_based = 0;


	/* top dog */

	if (ls_flist->feature == (char *) NULL)
	{
		LOG((lmtext("Server started on %s\n"), ls_our_hostname));

	}
	else
	{
	  char string [25 + ((MAX_FEATURE_LEN+4) * 4)];

		sprintf(string,
			lmtext("Server started on %s for:\t"), ls_our_hostname);
		for (f = ls_flist; f && f->feature && *(f->feature);
							f = f->next)
		{
			if (((f->nlic >= 0 || f->res >= 0)
				&& !(f->flags & LM_FL_FLAG_NONSUITE_PARENT)) ||
			(f->flags & (LM_FL_FLAG_LS_HOST_REMOVED |
					LM_FL_FLAG_REREAD_REMOVED)))
			{
/*
 *				List only independent features or
 *				SUITE parents
 */
				if ( !(f->flags & LM_FL_FLAG_SUITE_COMPONENT)||
					(f->flags & LM_FL_FLAG_SUITEBUNDLE_PARENT))
				{
					ls_print_one_feat(f, string,
						&featcnt, 0, &user_based);
					if (!(f->flags & LM_FL_FLAG_UNCOUNTED))
						gotone_counted++;
					gotone++;
				}
/*
 *				List components for this if it's
 *				a SUITE parent
 */
				if (f->flags & LM_FL_FLAG_SUITEBUNDLE_PARENT)
				{
				  FEATURE_LIST *savpkg, *printed_pkg ;

					savpkg = printed_pkg =
						(FEATURE_LIST *)0;
					for (f2 = ls_flist;
						f2 && f2->feature &&
							*(f2->feature);
							f2 = f2->next)
					{
						if (f2->package == f)
						{
							if (savpkg !=
							printed_pkg)
							{
							ls_print_one_feat(
							savpkg, string,
							&featcnt, 0, &user_based);
							printed_pkg =
							savpkg;
							}
							savpkg = f2;
						}
					}
					if (savpkg)
						ls_print_one_feat( savpkg,
						string, &featcnt, 1, &user_based);
					else
					{
						strcat(string,
							   "*NONE*)\n");
						LOG((string));
						*string = '\0';
						featcnt = 0;
					}

				}
			}
		}
		if (gotone)
		{
			strcat(string, "\n");
			LOG((string));
		}
	}
	if ((ls_imaster && !gotone) ||
		(!gotone_counted && (ls_s_qnum() > 1)))
	{
		if (gotone && !gotone_counted && (ls_s_qnum() > 1))
		{
			LOG((lmtext("Redundant servers require some counted features, exiting\n")));
		}
		else
		{
			LOG((lmtext("No features to serve, exiting\n")));
		}
		ls_go_down(EXIT_NOFEATURES);
	}
	if (user_based && (user_based != -1))
	{
		LOG(("NOTE: Some features are USER_BASED or HOST_BASED\n"));
		LOG(("  If a reread is done, and the INCLUDE list changes for these\n"));
		LOG(("  there is a delay of %d hours for this to take effect.\n",
			ls_user_based_reread_delay/(60*60)));
	}
	{
	  extern int ls_use_all_feature_lines;
	  extern int ls_a_license_case_sensitive;;

		if (ls_use_all_feature_lines)
		{
			LOG(("\n"));
			LOG(("All FEATURE lines for this vendor behave like INCREMENT lines\n"));
			LOG(("\n"));
		}
		if (ls_a_license_case_sensitive)
		{
			LOG(("\n"));
			LOG(("Licenses are case sensitive for this vendor\n"));
			LOG(("\n"));
		}
	}
}

/*
 *	ls_print_one_feat -- list one feature
 */
static
void
ls_print_one_feat(f, string, featcnt, package_end, user_based)
FEATURE_LIST *f;
char *string;
int *featcnt;
int package_end;  /* if true, ends package */
int *user_based;
{

	if ((f->type_mask & LM_TYPE_USER_BASED) ||
	 (f->type_mask & LM_TYPE_HOST_BASED))
		*user_based = 1;
	if (f->flags & LM_FL_FLAG_SUITEBUNDLE_PARENT)
	{
		if ((*featcnt != FEAT_PER_LINE ) && (strlen(string)> 4))
		{
			strcat(string, "\n");
			LOG((string));
		}
		else if (*string)
		{
			strcat(string, "\n");
			LOG((string));
		}
		sprintf(string, lmtext( "%s (consisting of:\t\t"),
				f->feature);
		*featcnt = 2;
	}
	else
	{
		if (*featcnt == FEAT_PER_LINE )
		{
			strcat(string, "\n");
			LOG((string));
			*string = '\0';
			*featcnt = 0;
		}
		strcat(string, f->feature);
		if (package_end)
			strcat(string, ")");
		if (strlen(f->feature) < 8)
			strcat(string, "\t\t");
		else if (strlen(f->feature) < 15)
			strcat(string, "\t");
		else
			strcat(string, " ");
		(*featcnt)++;
	}
}
