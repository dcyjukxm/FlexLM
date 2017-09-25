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
 *	Module: $Id: test_date.c,v 1.2 2003/01/13 22:46:13 kmaclean Exp $
 *
 *	Function: test_date
 *
 *	Description: Tests lm_date 
 *
 *	Paramaters:	(char *) date
 *
 *	Return:		0 if today would not be expired relative to this date.
 *			<> 0 if today would be expired relative to this date.
 *
 *	M. Christiano
 *	3/21/88
 *
 *	Last changed:  10/18/95
 *
 */

#include "lmclient.h"
#ifdef VMS
extern void exit();
extern int l_date();
#endif

main(argc, argv) 
int argc;
char *argv[];
{ 
  char *date = argv[1];
  LM_HANDLE dummy;

	if (argc < 2)
		printf("usage: %s date\n", argv[0]);
	else
		if (l_date(&dummy, date))
		{
			printf("EXPIRED\n");
			exit(1);
		}
		else
		{
			printf("NOT EXPIRED\n"); 
			exit(0);
		}
}
