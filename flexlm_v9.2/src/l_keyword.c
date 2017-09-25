/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_keyword.c,v 1.4 2003/01/13 22:41:52 kmaclean Exp $
 *
 *	Function:	l_keyword_eq
 *
 *	Description: 	Match keywords -- handle case stuff.
 *	D. Birns
 *	4/6/97
 *
 *	Last changed:  9/17/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
/*
 *	l_keyword_eq -- case insensitivity is set in job
 * 	return 1 if str equal keyword, 0 if not
 *	Mostly to handle case-sensitivity...
 */
int API_ENTRY
l_keyword_eq(job, str1, str2)
LM_HANDLE *job;
char *str1;
char *str2;
{
  char buf1[MAX_CONFIG_LINE];
  char buf2[MAX_CONFIG_LINE];
  int l1, l2;
	/* handle odd cases */
	if (!str1 && !str2) return 1;
	if (!str1 || !str2) return 0;
	l1 = strlen(str1);
	l2 = strlen(str2);
	if ((l1 != l2) || (l1 >= MAX_CONFIG_LINE)) return 0;
	strcpy(buf1, str1);
	strcpy(buf2, str2);
	if (!(job->options->flags & LM_OPTFLAG_STRINGS_CASE_SENSITIVE))
	{
		l_uppercase(buf1);
		l_uppercase(buf2);
		return L_STREQ(buf1, buf2);
	}
	else return L_STREQ(str1, str2);
}
int API_ENTRY
l_keyword_eq_n(job, str1, str2, n)
LM_HANDLE *job;
char *str1;
char *str2;
{
  char buf1[MAX_CONFIG_LINE];
  char buf2[MAX_CONFIG_LINE];
	/* handle odd cases */
	if (!str1 && !str2) return 1;
	if (!str1 || !str2) return 0;
	l_zcp(buf1, str1, n);
	l_zcp(buf2, str2, n);
	if (job->options && !(job->options->flags & LM_OPTFLAG_STRINGS_CASE_SENSITIVE))
	{
		l_uppercase(buf1);
		l_uppercase(buf2);
		return L_STREQ_N(buf1, buf2, n);
	}
	else return L_STREQ_N(str1, str2, n);
}
