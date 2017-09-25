/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lg_genvd.h,v 1.2 2003/01/13 22:22:33 kmaclean Exp $
 *
 *	D. Birns
 *	1/4/94
 *
 *	Last changed:  10/23/96
 *
 */

	
typedef struct _gen_feature_list {
	struct _gen_feature_list *next; /* list of FEATURE_LISTS */
	char *vd_name; 			/* Vendor daemon name */
	VENDORCODE vc; 			/* contains encryption seeds */
	FEATURE_LIST *fl;
	int lock_desc;
	char lock_name[MAX_DAEMON_NAME + 20];
	long lock_check_time; 		/* last time checked */
} GEN_FEATURE_LIST;


void			lg_add_fl 	lm_args((char *, VENDORCODE *,
						FEATURE_LIST *));
GEN_FEATURE_LIST * 	lg_new_gfl 	lm_args((char *, VENDORCODE *));
void 			lg_free_gfl lm_args((GEN_FEATURE_LIST *));
GEN_FEATURE_LIST * 	lg_lookup_fl lm_args((char *));
FEATURE_LIST * 		lg_flist 	lm_args((CLIENT_DATA *, char *));
