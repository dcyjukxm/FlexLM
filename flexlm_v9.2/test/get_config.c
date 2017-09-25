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

/*****************************************************************************/
/*	
 *	Module: $Id: get_config.c,v 1.2 2003/01/13 22:46:13 kmaclean Exp $
 *
 *	Function: get_config()
 *
 *	Description: Gets a configuration line from license.dat
 *
 *	Paramaters:	feature name
 *
 *	Return:		Configuration data
 *
 *	M. Christiano
 *	2/18/88
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_code.h"
#include "lm_attr.h"
#include <stdio.h>
LM_DATA_STATIC;

LM_CODE(code, 0, 0, VENDOR_KEY1, VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4,
							VENDOR_KEY5);

main(argc, argv)
int argc;
char *argv[];
{
  char feature[MAX_FEATURE_LEN+1];
  CONFIG *x, *lc_get_config();
  char *license_file = NULL;
  int i;

	(void) lm_init(VENDOR_NAME, &code, &lm_job);
	if (argc > 1) 
	{
		strcpy(feature, argv[1]);
		if (argc > 2)
		{
			license_file = argv[2];
		}
	}
	else
	{
		printf("What feature to you want to try? ");
		gets(feature);
	}
	if (license_file)
	{
		if (lc_set_attr(lm_job, LM_A_LICENSE_FILE, 
					(LM_A_VAL_TYPE) license_file))
		{
			lm_perror("set license file");
			exit(_lm_errno);
		}
	}
	x = lc_get_config(lm_job, feature);
	if (x != (CONFIG *)NULL)
	{
		printf("Feature name:\t\"%s\"\n", x->feature);
		printf("Daemon program:\t\"%s\"\n", x->daemon);
		printf("Version:\t%.2f\n", x->version);
		printf("Exp. date:\t\"%s\"\n", x->date);
		printf("# users:\t%d\n", x->users);
		printf("Encryption:\t%s\n", x->code);
		if (x->lc_vendor_def)
			printf("Vendor-defined string:\t\"%s\"\n", 
							x->lc_vendor_def);
		if (x->idptr && x->idptr->type == HOSTID_LONG)
			printf("hostid:\t%x\n", x->idptr->hostid_value);
		else if (x->idptr && x->idptr->type == HOSTID_ETHER)
		{
			printf("hostid:\t");
			for (i = 0; i < ETHER_LEN; i++)
			{
				printf("%x", x->idptr->hostid_eth[i] & 0x0ff);
				if (i < ETHER_LEN-1) printf(":");
				else printf("\n");
			}
		}
		exit(0);
	}
	else
	{
		lm_perror("lm_get_config");
		exit(_lm_errno);
	}
}
