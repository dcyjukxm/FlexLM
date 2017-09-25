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
#include "lmclient.h"
#include <stdio.h>
#include "lm_code.h"
LM_DATA_STATIC;

LM_CODE(code, 0, 0, VENDOR_KEY1, VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4,
							VENDOR_KEY5);

main()
{
  char feature[MAX_FEATURE_LEN+1];
  LM_SERVER *x, *lc_master_list();
  int i;

	i = lm_init(VENDOR_NAME, &code, &lm_job);
	if (i) lm_perror("lm_init");
	printf("lm_master_list returns:\n");
	lc_set_errno(lm_job, 0);
	x = lm_master_list();
	if (_lm_errno) lm_perror("lm_master_list");
	while (x)
	{
		printf("hostname: \"%s\"  ", x->name);

		if (x->idptr && x->idptr->type == HOSTID_LONG)
			printf("hostid: %x\n", x->idptr->hostid_value);
		else if (x->idptr && x->idptr->type == HOSTID_ETHER)	
		{
			printf("hostid: ");
			for (i = 0; i < ETHER_LEN; i++)
			{
				printf("%s%x", 
					x->idptr->hostid_eth[i] < 16 ? "0" : "", 
					x->idptr->hostid_eth[i] & 0x0ff);
				if (i < ETHER_LEN-1) printf(":");
				else printf("\n");
			}
		}
		else 
			printf("No hostid\n");

		x = x->next;
	}
}
