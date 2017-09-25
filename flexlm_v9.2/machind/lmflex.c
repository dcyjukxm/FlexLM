/******************************************************************************

	    COPYRIGHT (c) 1988, 2001 by Macrovision Corporation
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation

 *****************************************************************************/
/*	
 *	Module: $Id: lmflex.c,v 1.46 2003/06/16 18:05:13 jwong Exp $
 *
 *	Description:	This is a sample application program, to illustrate 
 *			the use of the Flexible License Manager.
 *
 *	Last changed:  06 Dec 1998
 *	D. Birns
 *
 */

#include "lmclient.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include "lm_attr.h"
#ifdef PC
#define LICPATH "license.dat;."
#else
#define LICPATH "@localhost:license.dat:."
#endif /* PC */

#define FEATURE "f1"
VENDORCODE code;
LM_HANDLE *lm_job;

void
main(int argc, char * argv[])
{
	char feature[MAX_FEATURE_LEN * 2] = {'\0'};

	if (lc_new_job(0, lc_new_job_arg2, &code, &lm_job))
	{
		lc_perror(lm_job, "lc_new_job failed");
		exit(lc_get_errno(lm_job));
	}

#ifdef PC
	LM_USE_FLEXLOCK();

	printf("Enter \"lcm\" to demo new LCM functionality\n");
	printf("Enter \"flexlock\" to demo FLEXlock functionality\n\n");
#endif
	printf("Enter \"f1\" to demo floating functionality\n");
	printf("Enter \"f2\" to node-locked functionality\n");
	printf("Enter feature to checkout [default: \"%s\"]: ", FEATURE);


	fgets(feature, MAX_FEATURE_LEN + 2, stdin);	/*	add 2 for \n and \0	*/
	feature[strlen(feature) - 1] = '\0';
	if(!*feature)
		strcpy(feature, FEATURE);
#ifdef PC
	if (!strcmp(feature, "lcm"))
		printf("Use key: OR10660-2377213\n");
#endif
/* 
 *      ISVs with high security requirements should not set LM_A_FLEXLOCK
 */
#ifdef PC
	lc_set_attr ( lm_job, LM_A_FLEXLOCK , (LM_A_VAL_TYPE) 1 );
#endif
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)LICPATH);
	if (lc_checkout(lm_job, feature, "1.0", 1, LM_CO_NOWAIT, &code, 
								LM_DUP_NONE))
	{
		lc_perror(lm_job, "checkout failed");
		exit (lc_get_errno(lm_job));
        }
	printf("%s checked out...", feature);
	printf("press return to exit..."); 
/*
 *	Wait till user hits return
 */
	getchar();
	lc_checkin(lm_job, feature ,0);

	lc_free_job(lm_job);
	exit(0);
}
