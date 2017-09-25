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
 *	Module: $Id: bigfile.c,v 1.4 2003/01/13 22:55:13 kmaclean Exp $
 *
 *	Description:  Make a big license file
 *
 *	Parameters:	default is 10000 lines, arg is num of lines
 *
 *	D. Birns
 *	7/14/95
 *
 *	Last changed:  9/24/98
 *
 */

#include "lmclient.h"
#include "lm_code.h"
#ifdef PC
#include "pcsock.h"	/* For gethostname() */
#endif /* PC */
#define MAXSIZ 1000000
#define LINES 10000

LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

usage()
{
	puts("Usage:  bigfile [-nolic] [-liconly] [-size]");
	exit(1);
}
main(argc, argv)
int argc;
char **argv;
{
  char file[MAXSIZ]; /* No bigger than 1 MB */
  int lines = LINES;
  int i;
  char *cp = file, *return_str, *errors;
  FILE *fp;
  char hostname[101];
  LM_HANDLE *job;
  int liconly = 0;
  int create_lic = 1;
  VENDORCODE vc;
	
	for (i = 1; i< argc; i++)
	{
		if (!strcmp(argv[i], "-liconly"))
			liconly = 1;
		else if (!strcmp(argv[i], "-nolic"))
			create_lic = 0;
		else if (*argv[i] == '-' && isdigit(argv[i][1]))
			lines = atoi(&argv[i][1]);
		else
			usage();

	}


	if (lines > (MAXSIZ/80))
	{
		printf("Warning: using %d lines, max file size is 1MB\n", 
								LINES);
		lines = LINES;
	}
	gethostname(hostname, 100);
	if (!hostname)
	{
		perror("Can't get hostname");
		exit(1);
	}
	if (create_lic)
	{
		if (!(fp = fopen("bigfile.dat", "w")))
		{
			perror("Can't write to bigfile.dat");
			exit(1);
		}
	
		sprintf(cp, "SERVER %s ANY 2837\nDAEMON demo demo big.opts\nUSE_SERVER\n", hostname);
		cp += strlen(cp);
			
		for (i = 0; i < lines; i++)
		{
			sprintf(cp, "FEATURE f%d demo 1.0 1-jan-0 1 0\n", i);
			cp += strlen(cp);
		}
		lc_init((LM_HANDLE *)0, VENDOR_NAME, &code, &job);
		memcpy(&vc, &code, sizeof(code));
		vc.data[0] ^= VENDOR_KEY5;
		vc.data[1] ^= VENDOR_KEY5;

		if (lc_cryptstr(job, file, &return_str, &vc, 0, "string", &errors))
		{
			lc_perror(job, "error making license");
			exit(1);
		}
		fprintf(fp, return_str);
		fclose(fp);
		if (!(fp = fopen("big.opts", "w")))
		{
			perror("Can't write to big.opts");
			exit(1);
		}
		fprintf(fp, "REPORTLOG bigreport.log\n");
		fclose(fp);
		if (liconly) exit(0);
	}
}
