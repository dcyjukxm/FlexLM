/******************************************************************************

	COPYRIGHT (c) 1998, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_njob.c,v 1.11 2003/01/13 22:41:47 kmaclean Exp $
 *
 *	Function:
 *
 *	Description: 
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	7/30/95
 *
 *	Last changed:  9/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_attr.h"
#include "lgetattr.h"
int
lc_new_job(oldjob, l_new_job, vcode, newjobp )
LM_HANDLE *oldjob;
lc_new_job_cb l_new_job; 
VENDORCODE *vcode;
LM_HANDLE **newjobp;
{
#if defined( FLEX_STATIC) || !defined(PC)
  extern int (*L_NEW_JOB)();
#endif /* FLEX_STATIC */
  char vendor_name[MAX_DAEMON_NAME + 1];
  int ret;
  int sign_level;

#if defined( FLEX_STATIC) || !defined(PC)
        l_new_job = L_NEW_JOB;
#else /* DLL */
	{
	  extern char *l_borrow_decrypt(void *, char *, int, int);
	  extern char *(*l_borrow_dptr)(void *, char *, int, int);
		l_borrow_dptr = l_borrow_decrypt;
	}
#endif /* FLEX_STATIC */
	(*l_new_job)(vendor_name, vcode, 0, 0, 0, &sign_level);
	(*l_new_job)(0, 0, 0, 0, 0, 0);
	if (!(ret =  lc_init(oldjob, vendor_name, vcode, newjobp)))
	{
		(*newjobp)->options->flags |= LM_OPTFLAG_CUSTOM_KEY5;
		(*newjobp)->l_new_job = (char *)l_new_job;
		if (((*newjobp)->attrs[REAL_KEYS] != REAL_KEYS_VAL) ||
					((*l_new_job)(0, 0, 2, 0, 0, 0)))
			(*newjobp)->options->flags |= LM_OPTFLAG_BORROW_OK;
		if (sign_level > (int)(*newjobp)->L_SIGN_LEVEL)
			(*newjobp)->L_SIGN_LEVEL = (char *)sign_level;
			
	}
	return ret;
}

