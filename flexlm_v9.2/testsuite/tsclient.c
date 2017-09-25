/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
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
 *	Module: $Id: tsclient.c,v 1.54 2003/01/13 22:55:18 kmaclean Exp $
 *
 *	Description:	This is a sample application program, to illustrate 
 *			the use of the Flexible License Manager.
 *
 *	Last changed:  06 Dec 1998
 *	D. Birns
 *
 */

#include "lmclient.h" 
#include "code.h"
/*#include "testsuite.h" */
lm_extern int ts_ds_app lm_args((char *, unsigned char *, unsigned int,
					unsigned char *, unsigned int));
FILE *ofp;
#include <stdio.h>
#ifdef ANSI
#include <stdlib.h>
#endif
#include <time.h>
#include "lm_attr.h"
#ifdef PC
#define LICPATH "license.dat"
#else
#define LICPATH "@localhost:license.dat:."
#endif /* PC */

#define FEATURE "f1"
VENDORCODE vcode;
LM_HANDLE *lm_job;
LM_HANDLE *lm_job2;

void
main(argc, argv)
int argc;
char **argv;
{
  char feature[MAX_FEATURE_LEN+1];
  CONFIG *confptr;
  int i;
  char **cpp;
  char buf[2048];
  int t = time(0);


  
	if (lc_new_job((LM_HANDLE *)0, lc_new_job_arg2, &vcode, &lm_job))
	{
		lc_perror(lm_job, "lc_new_job failed");
		exit(lc_get_errno(lm_job));
	}
	if (argc == 2)
		strcpy(feature, argv[1]);
	else
		strcpy(feature, FEATURE);
#ifdef PC
	if (!strcmp(feature, "lcm")) printf("Use key: OR6802-4864392\n");
#endif
/* 
 *      ISVs with high security requirements should not set LM_A_FLEXLOCK
 */
	lc_set_attr ( lm_job, LM_A_FLEXLOCK , (LM_A_VAL_TYPE)1 );
	lc_set_attr ( lm_job, LM_A_RETRY_CHECKOUT , (LM_A_VAL_TYPE)0 );
	x_flexlm_newid();
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)LICPATH);

	if (lc_checkout(lm_job, feature, "1.0", 1, LM_CO_NOWAIT, &vcode, 
								LM_DUP_NONE))
	{							
		lc_perror(lm_job, "checkout failed");
		exit (lc_get_errno(lm_job));
	}
	printf("press return to continue\n");
	getchar();
	lc_checkin(lm_job, "f1" ,0);
	lc_free_job(lm_job);


        exit(0);

}
