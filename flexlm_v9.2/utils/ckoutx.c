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
 *	Module: $Id: ckoutx.c,v 1.3 2003/01/13 21:58:46 kmaclean Exp $
 *
 *	Function:	ckoutx main -- tests lmckoutx.c stubs
 *
 *	D. Birns
 *	7/16/96
 *
 *	Last changed:  7/16/96
 *
 */
#include <stdio.h>
#if (defined( __STDC__) || defined(_WINDOWS)) && !defined(apollo)
#include <stdlib.h>
#endif
#include <time.h>
#include <errno.h>
#include "lmclient.h" 
#include "lm_code.h"
#define FEATURE "f1"
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
#ifndef PC
extern int errno;
#endif
#ifdef VMS
extern char *strcpy();
extern int strlen();
#endif

main()
{
  LM_HANDLE *lm_job;
  extern char *l_asc_hostid lm_args((LM_HANDLE *, HOSTID *));


	if ((lc_init((LM_HANDLE *)0, VENDOR_NAME, &code, &lm_job)))
		lc_perror(lm_job, "lm_init failed");

	
	puts("checking out feature f0");
	if (lc_checkout(lm_job, "f0", "1.0", 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
		lc_perror(lm_job, "lc_checkout failed");
}
