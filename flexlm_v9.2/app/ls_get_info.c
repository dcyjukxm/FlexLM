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
 *	Module: $Id: ls_get_info.c,v 1.4 2003/01/13 22:22:35 kmaclean Exp $
 *
 *	Function:	ls_get_info() - Get info about vendor daemon
 *
 *	Description: 	Gets info for the LS_A_VD_INFO command
 *
 *	Parameters:	
 *
 *	Return:
 *
 *	M. Christiano
 *	8/13/95	 - Separated out from ls_vd_info.c
 *
 *	Last changed:  10/19/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "ls_sprot.h"
#include "lsserver.h"
#include "lsfeatur.h" /* Dynamic feature data */
#include "ls_aprot.h"

FEATURE_LIST *
ls_get_info(feature, code, num_users, float_lic_used, tot_lic_used,
				num_lic, q_cnt)
char *feature;
char *code;
int *num_users;
int *float_lic_used;
int *tot_lic_used;
int *num_lic;
int *q_cnt;
{
  USERLIST *q, *u;
  OPTIONS *options;
  FEATURE_LIST *f;
  int unused;
  int nusers = 0;
  extern FEATURE_LIST *ls_flist;

	*num_users = -1;
	*float_lic_used = -1;
	*tot_lic_used = -1;
	*num_lic = -1;
	*q_cnt = -1;

/*
 *	get the FEATURE_LIST for this feature
 */
	f = f_get_feat(feature, code, 1);

	if (!f  || (f->flags & LM_FL_FLAG_NONSUITE_PARENT))
		return 0;

/*
 *		f_list() gets almost everything we need, but we don't need
 *			the returned USERLIST *. num_users is computed
 *			incorrectly
 */
	(void) f_list(feature, code, &unused, float_lic_used, 
					num_lic, &q, &options, 1, NULL);

/*
 *		compute the number of users in the queue for this feature
 */
	for (*q_cnt = 0; q; q = q->next) (*q_cnt)++;
/*
 *		compute the num of reserved users for this feature
 */
	for (*tot_lic_used = 0, u = f->u; u; u = u->next)
	{
		int tot, x;
		f_count(u, &x, &tot);
		*tot_lic_used += tot;
	}
/* 		
 *	Bug P1144 -- count num_users by hand 
 */
	for (f = ls_flist;f && f->feature && *(f->feature); f = f->next)
	{
	  USERLIST *b;
		if (l_keyword_eq(lm_job, f->feature, feature) && 
			l_keyword_eq(lm_job, f->code, code))
		{
			for (u = f->u; u ; u = u->next)
			{
				for (b = u; b ; b = b->brother)
					nusers++;
			}
			break;
		}
	}
	*num_users = nusers;
	return(f);
}

