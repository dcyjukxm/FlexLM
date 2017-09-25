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
#define LM_INTERNAL 
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "lm_attr.h"
#include "lm_code.h"
#include <stdio.h>
LM_DATA;
#ifndef ANSI
char *strcpy();
#endif
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
		VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
#ifdef VMS
extern int l_connect();
extern void l_msg_cksum();
#endif

main(argc, argv)
int argc;
char *argv[];
{
  char msg[LM_MSG_LEN+1];
  CONFIG *l_lookup(), *c;
  int d;

	if (argc < 2) 
	{
		printf("Usage: dump featurename\n");
		exit(1);
	}
	d = lm_init("demo", &code, &lm_job);
	if (d && d != DEMOKIT)
	{
		(void) lm_perror("init");
		exit(_lm_errno);
	}
	l_flush_config(lm_job);	/* Read the license file */
	c = l_lookup(lm_job, argv[1]);
	if (c)
	{
		int commtype; 
		d = l_connect(lm_job, c->server, c->daemon, LM_TCP);
		if (d < 0)
			lm_perror("server_connect failed");
		else
		{
			msg[MSG_CMD] = LM_DUMP; 
			strcpy(&msg[MSG_DATA], ""); 
			l_msg_cksum(msg, COMM_NUMREV, LM_TCP);
			network_write(d, msg, LM_MSG_LEN);
		}
	}
	else
		lm_perror("l_lookup");
	exit(_lm_errno);
}
