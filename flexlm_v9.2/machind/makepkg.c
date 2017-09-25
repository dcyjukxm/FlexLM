/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: makepkg.c,v 1.16 2003/05/05 15:58:04 sluu Exp $
 *
 *	Function:	create a PACKAGE line
 *
 *	Description: 	Prompts for information, and generates a PACKAGE line
 *
 *	Parameters:	Package name,  version, components and options
 *
 *	Return:		PACKAGE line
 *	
 *			Normally, lmcrypt is a better tool for making
 *			licenses.
 *
 *	D. Birns
 *	10/8/94
 *
 *	Last changed:  11/2/98
 *
 */
#include "lmprikey.h"
#include "lmclient.h"
#include "lm_code.h"
#include "lmseeds.h"
#include <ctype.h>
#include <stdio.h>
#if (defined(PC) || defined(__STDC__)) && !defined(apollo)
#include <stdlib.h>
#include <string.h>
#else
#endif /* defined(PC) || defined(__STDC__) */

#define START_EXP_DATE "1-jan-0"
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

void get_string 	lm_args((char *, int, char *));
int get_suite 		lm_args((lm_noargs));
void get_components 	lm_args((char *, int));
void banner		lm_args((lm_noargs));
void usage		lm_args((lm_noargs));
void output		lm_args((char *, int, char **));
int get_one_component	lm_args((char *, char *, int));
FILE *file_open		lm_args((char *));
FILE *ofp ;
int valid_version	(char *ver);

char resp[MAX_CONFIG_LINE+1];

int
main(argc, argv)
char **argv;
{
  char feature[MAX_FEATURE_LEN + 1];
  char version[MAX_VER_LEN + 1];
  char components[MAX_CONFIG_LINE + 1];
  int suite;
  LM_HANDLE *job;
  char result[MAX_CONFIG_LINE+1];
  char *return_str;
  char *errors;
  int rc;

	ofp = stdout;

	if ((argc == 2 && !strcmp(argv[1], "-h")) || argc > 2)
	{
		usage();
		exit(0);
	}
	if (argc == 2) ofp = file_open(argv[1]);

	LM_CODE_GEN_INIT(&code);	
	if (lc_init(0, VENDOR_NAME, &code, &job) && lc_get_errno(job) != LM_DEMOKIT)
	{
		lc_perror(job, "lc_init");
		exit(lc_get_errno(job));
	}
	banner();
	get_string(feature, MAX_FEATURE_LEN, "Package Name");
	while (1)
	{
		get_string(version, MAX_VER_LEN, "Package Version");
		if (!valid_version(version))
			fprintf(ofp, "\tInvalid Version: %s, please reenter\n\t", 
				version);
		else break;
	}
	suite = get_suite();
	sprintf(result, "PACKAGE %s %s %s ", feature, VENDOR_NAME, version);
	if (suite) 
	{
		printf("\tOPTIONS=SUITE\n");
		strcat (result, " OPTIONS=SUITE");
	}


	memset(components, 0, sizeof(components));
	get_components(components, suite);
	sprintf(result, "%s COMPONENTS=\"%s\"", result, components);
	sprintf(result, "%s SIGN=0", result);
	rc = lc_cryptstr(job, result, &return_str, &code, 
			LM_CRYPT_FORCE, 0, &errors);
	if (errors || rc)
	{
		if (errors) fputs(errors, ofp);
		if (rc) fprintf(ofp, "Error: %s\n", lc_errstring(job));
	}
	if (return_str) fputs(return_str, ofp);
	exit(rc);
	return 0;
}
void
get_string(buf, len, title)
char *buf;
int len;
char *title;
{
	while (1)
	{
		printf("%-40s: ", title);
		*resp = 0;
		fgets(resp, MAX_CONFIG_LINE, stdin);	/* overrun checked */
		if (!*resp || *resp == '\n')
			break;
		resp[strlen(resp) - 1] = 0; /* trailing newline */

		

		if ((int)strlen(resp) > len)
		{
			printf("\tEntry is too long, please limit to %d\n",
				len);
			continue;
		}
		/* validate */
		if (strchr(resp, ' '))
		{
			printf("\tNo spaces allowed in entry\n");
			continue;
		}
		break;
	}
	strncpy(buf, resp, len);
	buf[len] = '\0';
}

int
get_suite()
{
   char buf[2];
	printf("PACKAGE has 2 different modes:  \n");
	printf("\ta) convenience tool for distribution\n");
	printf("\tb) selling Features in a SUITE\n");

	*buf = '\0';
	while (1)
	{
		get_string(buf, 1, "Which PACKAGE mode (a or b)?");
		if (*buf == 'a' || *buf == 'b' || *buf == 'A' || *buf == 'B')
			break;
		puts("\tPlease enter one character, a or b");
	}
	switch (*buf)
	{
		case 'a':
		case 'A':	return 0;
		case 'B':	
		case 'b':	return 1;
		/* can't get here */
	}
	return 0;
}
void
get_components(buf, suite_flag)
char *buf;
int suite_flag;
{
  char component[MAX_CONFIG_LINE + 1];

	puts("\nYou will be prompted for components, one at a time");
	puts("Press <Return> for a component feature name to end\n");
	while (get_one_component(buf, component, suite_flag))
	{
		if (*buf) 	sprintf(buf, "%s %s", buf, component);
		else 		strcpy(buf, component);
	}
}
int 
get_one_component(all, this, suite_flag)
char *all, *this;
int suite_flag;
{
  char feature[MAX_FEATURE_LEN + 1];
  char version[MAX_VER_LEN + 1];
  char count[MAX_LONG_LEN + 1];
  int num;
	
	if (all && *all)
		printf("\n\tCOMPONENTS=\"%s\"\n", all);

	get_string(feature, MAX_FEATURE_LEN, 
			"    Feature Name or return to end");
	if (*feature == '\n')
		return 0;
	while (1)
	{
		get_string(version, MAX_VER_LEN, 
			"    Feature Version");
		if (*version && !valid_version(version))
			fprintf(ofp, "    Invalid Version: %s, please re-enter\n", 
				version);
		else break;
	}

	if (!suite_flag && *version)
		while (1)
		{
			get_string(count, MAX_LONG_LEN, 
					"    Number Users");
			if (!*count || (sscanf(count, "%d", &num) == 1))	/* overrun checked */
				break;
			puts("Invalid integer\n");
		}
	else
		*count = '\0';
	strcpy(this, feature);
	if (*version) 	sprintf(this, "%s:%s", this, version);
	if (*count) 	sprintf(this, "%s:%s", this, count);
	return 1;
}


void
banner()
{
	printf("makepkg -\n"COPYRIGHT_STRING(1988)"\n\n");
}

void
usage()
{
	puts("usage: makepkg [ file ] -- output goes to stdout or appends to file");
}
FILE *
file_open(name)
char *name;
{
	FILE *ofp;
	if (!(ofp = fopen(name, "a")))
	{
		perror("fopen failed");
		exit(-1);
	}
	return ofp;
}

valid_version(ver)
char *ver;
{
  char *cp = ver;
  int got_decimal = 0;

        if (!ver) return 0;

        if (strlen(ver) > MAX_VER_LEN) return 0;

        for (cp = ver; *cp; cp++)
        {
                if (isdigit(*cp))
                        continue;
                if (*cp == '.' && !got_decimal) /* only allow 1 decimal */
                {
                        got_decimal = 1;
                        continue;
                }
/* 
 *      if we got here, there's an invalid character
 */
                return 0;
        }
        return 1;
}
