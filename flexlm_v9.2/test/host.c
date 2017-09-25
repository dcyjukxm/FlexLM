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
#include "lmclient.h"
#ifdef VMS
extern char *strcpy();
extern void l_get_id(), exit();
extern int l_host();
#endif

main(argc, argv)
int argc;
char *argv[];
{
  char hostid[20];
  HOSTID id;
  int status;
  LM_HANDLE dummy;

	if (argc < 2)
	{
		printf("What hostid to you want to try? ");
		gets(hostid);
	}
	else
		strcpy(hostid, argv[1]);

	l_get_id(&dummy, &id, hostid);
	status = l_host(&dummy, &id);
	printf("l_host returns %d for hostid %s\n", status, hostid);
	exit(status);
}
