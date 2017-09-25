/******************************************************************************

	    COPYRIGHT (c) 1995, 2000 by Macrovision Corporation.
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
 *	Module: $Id: lmsimple.c,v 1.16 2003/01/17 19:43:58 jwong Exp $
 *
 *	Description: 	Simple API example
 *
 *	D. Birns
 *	11/30/95
 *
 *	Last changed:  11/6/97
 *
 */

#include "lmpolicy.h"
#include <stdio.h>
#define FEATURE "f1"
#include <stdlib.h>
#include <string.h>

#ifdef PC
#define LICPATH "license.dat;."
#else
#define LICPATH "@localhost:license.dat:."
#endif /* PC */

int
main()
{
  char feature[MAX_FEATURE_LEN+1];
  LP_HANDLE *lp_handle;

#ifdef PC
	printf("Enter \"lcm\" to demo new LCM functionality\n");
	printf("Enter \"flexlock\" to demo FLEXlock functionality\n\n");
	printf("Enter \"f1\" to demo floating functionality\n");
	printf("Enter \"f2\" to node-locked functionality\n");
#endif
	printf("Enter feature to checkout [default: \"%s\"]: ", FEATURE);

	fgets(feature, MAX_FEATURE_LEN, stdin); 
	if (*feature == '\n') 
		strcpy(feature, FEATURE);
	else
		feature[strlen(feature)-1] = '\0'; /* truncate */
#ifdef PC
	if (!strcmp(feature, "lcm")) 
        {
                printf("(Note: only works with lm_code.h set to LM_STRENGTH_DEFAULT\n");
                printf("Use key: OR10660-2377213\n");
        }
#endif
/* 
 *      ISVs with high security requirements should not include LM_FLEXLOCK
 *      in lp_checkout()
 */
	LM_USE_FLEXLOCK();

	if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_FLEXLOCK, feature, "1.0", 1, 
					LICPATH, &lp_handle))
	{
		lp_perror(lp_handle, "Checkout failed");
		exit(1);
	}
	printf("%s checked out...press return to exit...", feature); 
	getchar();
	lp_checkin(lp_handle); 
	exit(0);
	return 0;
}

