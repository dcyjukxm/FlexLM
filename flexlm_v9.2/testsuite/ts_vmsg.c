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
 *	Module: $Id: ts_vmsg.c,v 1.16 2003/01/13 22:55:19 kmaclean Exp $
 *
 *	Function: ts_vmsg(str)
 *
 *	Description: Vendor-defined message processing routine for testsuite.
 *
 *	Paramaters:	(char *) str - the input string
 *
 *	Return:		
 *
 *	M. Christiano
 *	1/4/94
 *
 *	Last changed:  06 Jul 1996
 *
 */

static char *sccsid = "@(#) ts_vmsg.c:v1.4";
#include "lmachdep.h"
#include "lmclient.h"
#include "lsserver.h"
#include "lsfeatur.h"
#include "ls_attr.h"

char *
ts_vmsg(str)
char *str;
{
  char *ret = "bad";
  extern int timer_expired;
#ifdef SUPPORT_METER_BORROW
  extern int ls_last_borrow_check;
#endif /* SUPPORT_METER_BORROW */
  extern ls_feat_dump();
  extern int ls_borrow_minutes_is_seconds;

	if (!strcmp(str, "test")) ret = "ok";
	else if (!strcmp(str, "flush")) 
	{
		ret = "flushed";
		timer_expired = 1;
		LOG(("server flushing...\n"));
#ifdef SUPPORT_METER_BORROW
		ls_last_borrow_check = 0;
#endif /* SUPPORT_METER_BORROW */
	}
	else if (!strcmp(str, "seconds")) 
	{
		printf("Setting borrow minutes to seconds \n");
		ls_borrow_minutes_is_seconds = 1;
		ret = "okseconds";
	}
	else if (!strcmp(str, "dump")) 
	{
		fprintf(stderr, "NOT Calling ls_feat_dump()\n");
		/* ls_feat_dump(); */
		ret = "ok";
	}
	return(ret);
}
outfilter()
{
  extern LM_HANDLE *lm_job;
  char *f;
  char nlic[MAX_LONG_LEN+1], flag[MAX_LONG_LEN+1];
  char *version, dup_sel[MAX_LONG_LEN+1];
  char linger[MAX_LONG_LEN+1];
  char *code;


  CLIENT_DATA *cd;
  int nlicd, flagd, dup_seld, lingerd;

	ls_get_attr(LS_ATTR_FEATURE, &f);
	if (!strcmp(f, "outfilter"))
	{
		ls_get_attr(LS_ATTR_NLIC, &nlicd);
	/*
	 *	convert to string
	 */
		sprintf(nlic, "%d", nlicd);
		ls_get_attr(LS_ATTR_FLAG, &flagd);
	/*
	 *	convert to string
	 */
		sprintf(flag, "%d", flagd);
		ls_get_attr(LS_ATTR_VERSION, &version);
		ls_get_attr(LS_ATTR_CLIENT, &cd);
		ls_get_attr(LS_ATTR_DUP_SEL, &dup_seld);
	/*
	 *	convert to string
	 */
		sprintf(dup_sel, "%d", dup_seld);
		ls_get_attr(LS_ATTR_LINGER, &lingerd);
	/*
	 *	convert to string
	 */
		sprintf(linger, "%d", lingerd);
		ls_get_attr(LS_ATTR_CODE, &code);
		if (ls_checkout(f, nlic, flag, cd, version, dup_sel,
			linger, code, 0, 1) == 1)
			return -1;
		return 0; /* this means we've done the ls_checkout call */
	}
	else return 1;
}
