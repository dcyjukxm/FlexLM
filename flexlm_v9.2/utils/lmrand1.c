/*****************************************************************************

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
/*
 *	Module: $Id: lmrand1.c,v 1.26 2003/01/13 21:58:47 kmaclean Exp $
 *
 *	Last changed:  6/19/98
 *
 */
#include "stdio.h"
#include "lmclient.h"
#include "l_prot.h"

/*char *l_key_callback;
char *l_key_gen_callback;*/
char *l_api_prefix;

static
void
strip_crnl(buf)
char *buf;
{
  char *cp, *tcp;

  	for (cp = buf; *cp; cp++)
	{
		if (*cp == '\r' )
			for (tcp = cp; *tcp; tcp++)
				*tcp = tcp[1];
	}
}

FILE *ofp, *lsrvendfp, *ifp;
main(argc, argv)
char **argv;
{
#define MAXLINE 2048
  char  *_ge = "_ge";
  char  *dashp = "-p";
  char *line, buf[MAXLINE + 1];
  char *infile = (char *)"lsvendor.c";
  char *outfile = (char *)"lmcode.c";
  char *lsrvendfile = (char *)"lsrvend.c";
  char *ilter = "ilter";
  char  *ase = "ase";
  int i;
  int found = 0;
  char filter_gena[40];
  char phase2a[40];
  int gotkey2 = 0;
  extern char *l_appfile;


	ifp = stdin;
	if (argc < 2) errexit("", 0);
	sprintf(filter_gena, "%c%c%s%s", '-', 'f', ilter, _ge);
	sprintf(phase2a, "%s%c%s%d", dashp, 'h', ase, 2);
	strcat(filter_gena, "n");
	if (!strcmp(argv[1], filter_gena) || !strcmp(argv[1], phase2a))
	{
	  char **cpp;

/*
 *		Strip this arg, and pass args to filter_gen()
 */
		i = l_filter_gen(argc, argv); /* shouldn't return */
		exit(i);
	}

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-seed"))
			l_genseed();
		if (!strcmp(argv[i], "-o"))
		{
			outfile = argv[++i];
			if (++i == argc)
				errexit("Error: -o takes 2 file names", 0);
			lsrvendfile = argv[i];
		}
		else if (!strcmp(argv[i], "-i"))
		{
			infile = argv[++i];
		}
		else errexit( "Error: Unknown argument", argv[i]);

	}
	l_appfile = outfile;


#ifdef PC
#define READMODE "rb"
#else
#define READMODE "r"
#endif
	if (!(ifp = fopen(infile, READMODE)))
	{
		errexit("Cannot open file for reading", infile);
	}
	if (!(ofp = fopen(outfile, "w")))
	{
		errexit("Cannot open file for writing", outfile);
	}
	if (!(lsrvendfp = fopen(lsrvendfile, "w")))
	{
		errexit("Cannot open file for writing", lsrvendfile);
	}
	while (line = fgets(buf, MAXLINE, ifp))
	{
		strip_crnl(line);
		if (*line == '#')
		{
			/*
			 *	Added this check to deal with platform specific code
			 *	that was added to lsvendor.c that doesn't need to be
			 *	included in lmcode.c
			 */
			if(strstr(line, "WINNT") == NULL)
				fputs(line, ofp);
			fputs(line, lsrvendfp);
			continue;
		}
		while (!found && isspace(*line)) line++;

		if (found ||
		!strncmp(line, "VENDORCODE", sizeof("VENDORCODE") - 1))
		{
			found = 1;
			fputs(buf, ofp );
			if (strchr(line, ';'))
			{
				found = 0;
			}
		}
		else if (!strncmp(line, "int keysize",
						sizeof("int keysize") -1))
		{
			fputs(line, ofp);
		}
		else
		{
			fputs(line, lsrvendfp);
		}
	}
	fputs("\
char *vendor_name = VENDOR_NAME;\n\
unsigned long seed1 = ENCRYPTION_SEED1;\n\
unsigned long seed2 = ENCRYPTION_SEED2;\n\
unsigned long seed3 = ENCRYPTION_SEED3;\n\
unsigned long seed4 = ENCRYPTION_SEED4;\n\
unsigned long lmseed1 = LM_SEED1;\n\
unsigned long lmseed2 = LM_SEED2;\n\
unsigned long lmseed3 = LM_SEED3;\n\
int lm_sign_level = LM_SIGN_LEVEL;\n\
int pubkey_strength = LM_STRENGTH;\n\
int l_borrow_ok = LM_BORROW_OK;\n",
	ofp );
	fclose(ofp);
	fclose(ifp);
	fclose(lsrvendfp);
	exit(0);
}
errexit(str1, str2)
char *str1, *str2;
{
	fprintf(stderr,
"%s%s%s\n\
usage: lmrand1 [-i file] -o [outfile1 outfile2] \n\
	default input is lsvendor.c\n\
	default output is lmcode.c, lsrvend.c\n",
	str1, str2 ? ": " : "", str2 ? str2 : "");
	fprintf(stderr,
"\n\
       lmrand1 -seed\n\
        Prints out random numbers for LM_SEEDs for lm_code.h\n");
	exit(1);

}
