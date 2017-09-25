/******************************************************************************

	    COPYRIGHT (c) 1995, 2003  by Macrovision Corporation.
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
 *	Module: $Id: reptest.c,v 1.2 2003/01/13 22:55:15 kmaclean Exp $
 *
 *	Function:	reptest -- stress test report writer
 *
 *	D. Birns
 *	7/30/95
 *
 *	Last changed:  3/20/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_code.h"
#include "lm_attr.h"
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

#define MAXJOBS 500
LM_HANDLE *jobs[MAXJOBS];
main(argc, argv)
char *argv[];
{
  int i, cnt, repeats = 2, repeat, rc;
  char **feats, **cpp;
  char display[MAXJOBS];
  char user[MAXJOBS];
  char host[MAXJOBS];
  int users = MAXJOBS;

	for (i = 1; i < argc; i++)
	{
		if (!strncmp(argv[i], "-users", 2))
		{
			i++;
			users = atoi(argv[i]);
			if (users > MAXJOBS) users = MAXJOBS;
			continue;
		}
		if (!strncmp(argv[i], "-repeats", 2))
		{
			i++;
			repeats = atoi(argv[i]);
			continue;
		}
		printf("Usage: reptest [-users n] [-repeats n]\n");
		exit(1);
	}
		
	for (i=0;i<users;i++)
	{
	  LM_HANDLE *ojob = (LM_HANDLE *)0;
		if ((rc = lc_init(ojob, VENDOR_NAME, &code, &jobs[i])) && 
								rc != LM_DEMOKIT)
		{
			lc_perror(jobs[i], "lm_init failed");
			exit(rc);
		}
	}
	feats = lc_feat_list(jobs[0], 0, 0);
	cpp = feats;
	cnt = 0;

	for (repeat = 0; repeat < repeats; repeat++)
	{
		for (i=0;i<users;i++)
		{
			lc_set_attr(jobs[i], LM_A_CHECK_INTERVAL, 
				(LM_A_VAL_TYPE)-1);
			sprintf(display, "display%d", i);
			sprintf(user, "user%d", i);
			sprintf(host, "host%d", i);
			lc_set_attr(jobs[i], LM_A_DISPLAY_OVERRIDE, 
				(LM_A_VAL_TYPE)display);
			lc_set_attr(jobs[i], LM_A_USER_OVERRIDE, 
				(LM_A_VAL_TYPE)user);
			lc_set_attr(jobs[i], LM_A_HOST_OVERRIDE, 
				(LM_A_VAL_TYPE)host);
			for (cnt = 0, cpp = feats; *cpp; cpp++, cnt++)
			{
			  CONFIG *conf;
				conf = lc_get_config(jobs[i], *cpp);
				if (!conf)
				{
					printf("getconfig failed for %s\n", 
						*cpp);
					continue;
				}
				if ((conf->package_mask &&
				!(conf->package_mask & LM_LICENSE_PKG_COMPONENT))
				|| strcmp(conf->daemon, VENDOR_NAME))
					continue;
				cnt++;
				(void)lc_checkout(jobs[i], *cpp, "1.0", 
						(cnt%10) ? 1 : 2, 
						LM_CO_NOWAIT, 
						&code, LM_DUP_NONE);
				l_select_one(0, -1, 50);
				if ((cnt%10) < 7) 
				{
					lc_checkin(jobs[i], *cpp, 0);
					l_select_one(0, -1, 50);
				}
			}
		}
		for (i=0;i<users;i++)
		{
			lc_disconn(jobs[i], 1);
		}
	}
}
