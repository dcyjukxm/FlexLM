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
 *	Module: $Id: ping_vd.c,v 1.2 2003/01/13 22:46:13 kmaclean Exp $
 *
 *	Function: ping_vd
 *
 *	Description:  call l_ping_vd
 *
 *	Parameters:
 *
 *	Return:
 *
 *	D. Birns
 *	7/30/95
 *
 *	Last changed:  2/7/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_code.h"
#include "lm_attr.h"
#include <sys/types.h>
#include <netinet/in.h>

#if !defined(htons) && !defined(PC) && !defined (OSF)
extern u_short htons();
#endif

LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
main()
{
  LM_HANDLE *lm_job;
  int port;
  char *host;
  int rc;
  COMM_ENDPOINT e;
  LM_SERVER *s, *sp;


	if ((rc = lc_init((LM_HANDLE *)0, VENDOR_NAME, &code, &lm_job)) && 
							rc != LM_DEMOKIT)
	{
		lc_perror(lm_job, "lm_init failed");
		exit(rc);
	}
	if (lc_set_attr(lm_job, LM_A_NO_TRAFFIC_ENCRYPT, (LM_A_VAL_TYPE) 1))
		lc_perror(lm_job, "Turning off traffic encryption failed");
	s = lc_master_list(lm_job);
	e.transport = LM_TCP;
	for (sp = s; sp; sp = sp->next)
	{
		e.transport_addr.port = htons(sp->port);
		printf("trying %s,%d\n", s->name, sp->port);
		if ((l_connect_host_or_list(lm_job, &e, s->name, 
					sp, VENDOR_NAME, 0)) < 0)
		{
			puts("connect failed\n");
		}
		else
		{
			puts("connect succeeded\n");
		}
		
		lc_disconn(lm_job, 0);
	}
}

