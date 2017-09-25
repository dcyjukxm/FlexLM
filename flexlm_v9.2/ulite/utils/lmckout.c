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
 *	Module:	lmlite.c v1.1.0.0
 *
 *	Description:	This is a sample application to illustrate 
 *			the use of FLEXlm-ultralite
 *
 *	Last changed:  3/9/98
 *	M. Christiano
 *
 */
#include "lmclient.h" 
#include "lm_code.h"

LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

main(argc, argv)
int argc;
char **argv;
{
  int err;

	if (argc < 3)
	{
		printf("usage: %s feature hostid license_key\n", argv[0]);
		exit(1);
	}

	err = lc_checkit(VENDOR_NAME, argv[1], argv[3], argv[2], &code);
	printf("Return from checkout of feature:%s hostid:%s license:%s is %d\n", 
				argv[1], argv[2], argv[3], err);
}
