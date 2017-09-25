/******************************************************************************

	    COPYRIGHT (c) 1993, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_config.c,v 1.11 2003/01/13 22:41:50 kmaclean Exp $
 *
 *	Function: l_config(conf, type, feature, version, daemon,
 *			   date, users, vendor_def, server)
 *		  l_conf_copy(to, from)
 *
 *
 *	Description: Fills in a CONFIG struct with the most common stuff
 *		     l_conf_copy copies a CONFIG struct
 *
 *	Parameters:	(CONFIG *) conf - CONFIG struct to be filled in
 *			(int) type - Type of CONFIG struct
 *			(char *) feature - feature name
 *			(char *) version - Feature version
 *			(char *) daemon - Feature's daemon.
 *			(char *) date - expiration date
 *			(int) users - # of licenses
 *			(char *) vendor_def - vendor-defined string
 *			(LM_SERVER *) server - list of server hosts
 *
 *	Return:		conf - filled in.
 *
 *	Notes:		l_config() will zero the config struct before starting.
 *
 *	M. Christiano
 *	2/12/93
 *
 *	Last changed:  07 Nov 1998
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include <stdio.h>
#include <errno.h>

static char * malloc_copy lm_args(( LM_HANDLE *job, char *str));
static char ** strlist_copy lm_args(( LM_HANDLE *, char **));


/*
 *	l_conf_copy
 */
void
l_conf_copy(job, to, from)
LM_HANDLE *job;
CONFIG *to, *from;
{
  LM_KEYLIST *k, *nk, *nlast;
	
	memcpy(to, from, sizeof (CONFIG)); /* good enough for now */
	to->idptr = NULL;
	l_hostid_copy(job, &to->idptr, from->idptr);
	to->lc_vendor_def = malloc_copy(job, from->lc_vendor_def);
	to->lc_vendor_info = malloc_copy(job, from->lc_vendor_info);
	to->lc_dist_info = malloc_copy(job, from->lc_dist_info);
	to->lc_user_info = malloc_copy(job, from->lc_user_info);
/*
 *	We have to ensure that the keylist order is unchanged.
 */
	for(nlast = 0, to->lc_keylist = 0, k = from->lc_keylist; k; 
			k = k->next, nlast = nk)
	{
		nk = (LM_KEYLIST *)l_malloc(job, sizeof(LM_KEYLIST));
		nk->key = malloc_copy(job, k->key);
		nk->sign_level = k->sign_level;
		if (nlast) nlast->next = nk;
		else to->lc_keylist = nk;
	}
	to->lc_asset_info = malloc_copy(job, from->lc_asset_info);
	to->lc_issuer = malloc_copy(job, from->lc_issuer);
	to->lc_notice = malloc_copy(job, from->lc_notice);
	to->lc_prereq = malloc_copy(job, from->lc_prereq);
	to->lc_sublic = malloc_copy(job, from->lc_sublic);
	to->lc_dist_constraint = malloc_copy(job, from->lc_dist_constraint);
	to->lc_serial = malloc_copy(job, from->lc_serial);
	to->lc_issued = malloc_copy(job, from->lc_issued);
	to->lc_w_binary = malloc_copy(job, from->lc_w_binary);
	to->lc_w_argv0 = malloc_copy(job, from->lc_w_argv0);
#ifdef SUPPORT_METER_BORROW
	to->lc_meter_borrow_info = malloc_copy(job, from->lc_meter_borrow_info);
	to->meter_borrow_device = malloc_copy(job, from->meter_borrow_device);
#endif /* SUPPORT_METER_BORROW */
	to->lc_platforms = strlist_copy(job, from->lc_platforms);
	to->lc_supersede_list = strlist_copy(job, from->lc_supersede_list);
}
/*
 *	strlist_copy --- see parse_strlist in l_parse_attr.c
 *			copies an array of strings, fixes P3876
 */
static
char **
strlist_copy(job, list)
LM_HANDLE *job;
char **list;
{
 char **listp;
 int cnt;
 int len;
 char **ret, **retp;
 char *buf, *bufp;

 	if (!list) return 0;

 	for (len=0, cnt = 0, listp = list; *listp; listp++)
	{
		cnt++;
		len += strlen(*listp) + 1;
	}
	bufp = buf = (char *)l_malloc(job, len);
	retp = ret = (char **)l_malloc(job, (cnt + 1) * sizeof(char *));
	for (bufp = buf, retp = ret, listp = list; *listp; listp++, retp++)
	{
		strcpy(bufp, *listp);
		*retp = bufp;
		bufp += (strlen(bufp) + 1);
	}
	return ret;
}


/*
 *	malloc_copy
 */
static
char *
malloc_copy(job, str)
LM_HANDLE *job;
char *str;
{
  char *cp = (char *)0;

	if (str)
	{
		if (cp = (char *)l_malloc(job, strlen(str) + 1))
			strcpy(cp, str);
	}
	return cp;
}
