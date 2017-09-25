/******************************************************************************

	    COPYRIGHT (c) 1994, 2003  by Macrovision Corporation.
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
 *	Module: $Id: lsapi_tests.c,v 1.5 2003/01/13 22:55:15 kmaclean Exp $
 *
 *	Function: lsapi_tests
 *
 *	Description: Test suite for configurations with a single-host 
 *							(server running).
 *
 *	Tests performed:
 *
 *	1	Unlimited licenses (with server running)
 *	2	N licenses (all at once, in two pieces, one at a time)
 *	2.1	Multiple features from a single application
 *	3	Make sure several applications servers can run at once.
 *	3.1	Multiple checkouts of a single feature with no checkin
 *	4	Kill server, make sure application die
 *	5	Pause application, make sure server and client both know
 *	9	Version expiration tests
 *	11	Checkout/checkin cycle tests
 *	12	Feature start date tests
 *	15	mixed floating/demo licenses
 *	16	FEATURESET tests
 *	17	lmreread tests
 *	18	INCLUDE/EXCLUDE/RESERVE INTERNET tests
 *	22	INCREMENT tests
 *		
 *
 *	Parameters:	id (int) - The host id
 *
 *	Return:		(int) - 0 - OK, ie. we are running on the specified
 *					host.
 *				NOTTHISHOST - We are not running on the
 *						specified host.
 *
 *	M. Christiano
 *	2/12/94 - Adapted from one_s_tests.c
 *
 *	Last changed:  6/18/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_attr.h"
#include "lm_lsapi.h"
#include "code.h"
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>	/* for X_OK */
#if defined (MOTO_88K) || defined (sco)
#include <sys/unistd.h>	/* for X_OK */
#endif
#ifdef SVR4
#include <unistd.h>
#endif
#include "testsuite.h"

#ifndef ANSI
extern char *strcpy(), *getwd(), *strrchr();
#endif
extern char *l_bin_date();
static test_date();
extern char *l_asc_hostid();
extern HOSTID *lc_gethostid();
extern char *lc_username();
extern int portinc;		/* Increment for port # in ts_lic_file */
LM_DATA;

static LS_HANDLE good();
static LS_HANDLE breakc();
static LS_HANDLE h1, h2, h3, h4, h5, h6, h7, h8, h[10];
static LS_STR provider[256];

#define LOG "_log"	/* Log file for the daemons */
#define CF3 "license.dat3"
/* CF2 is the same as 4,5,6,11,21,23 except for port number*/
#define CF4 "lic2.4.dat"
#define CF5 "lic2.5.dat"
#define CF9 "lic2.9.dat"
#define CF11 "lic2.11.dat"
#define CF12 "license.dat12"
#define CF15 "license.dat15"
#define CF15b "license.dat15b"
#define CF16 "license.dat16"
#define CF16b "license.dat16b"
#define CF16c "license.dat16c"
#define CF17 "license.dat17"
#define CF17b "license.dat17b"
#define CF17c "license.dat17c"
#define CF17d "license.dat17d"
#define CF17e "license.dat17e"
#define CF17x "license.dat17x"
#define CF18 "license.dat18"
#define CF18b "license.dat18b"
#define CF18c "license.dat18c"
#define CF18d "license.dat18d"
#define CF18e "license.dat18e"
#define CF18f "license.dat18f"
#define CF18g "license.dat18g"
#define CF18h "license.dat18h"
#define CF22 "license.dat22"
#define OPTSFILE "/tmp/opts"
#define OPTSFILE18 "/tmp/opts2"
#define OPTSFILE18b "/tmp/opts3"
#define OPTSFILE18c "/tmp/opts4"
#define OPTSFILE18d "/tmp/opts5"
#define OPTSFILE18e "/tmp/opts6"
#define OPTSFILE18f "/tmp/opts7"
#define OPTSFILE18g "/tmp/opts8"
#define OPTSFILE18h "/tmp/opts9"

LM_CODE_GLOBAL(lmcode, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, VENDOR_KEY2, VENDOR_KEY3,VENDOR_KEY4,
						VENDOR_KEY5);
LS_CHALLENGE_FLEXLM challenge;

#define VER "1.0"			/* Version number we usually use */
#define OVER "0.0"		/* Old Version number (only for UPGRADE) */
#define VENDORSTRING "Vendor String Goes Here"

char *ts_Progname;
char *delim =
     "---------------------------------------------------------------------\n";

#define strsave(x) { char *_p = malloc(strlen(x)+1); strcpy(_p, x); x = _p; }
char *argv_0;

main(argc, argv)
int argc;
char *argv[];
{
  char demopath[LM_MAXPATHLEN];
  char demofpath[LM_MAXPATHLEN];
  char demo2path[LM_MAXPATHLEN];
  char hostname[101];
  char cmd[LM_MAXPATHLEN];
  char *vendord= (char *) NULL;
  char *hostid;
  HOSTID *id;
  int rc;
  unsigned long lsrc;
  int i;
  int t;
  int dotest = -1;
  int subtest = -1;
  char *us;
  extern int ts_use_sleep;

	argv_0 = argv[0];
	setlinebuf(stdout);
	setlinebuf(stderr);

#ifndef NO_SIGCLD
	/*signal(SIGCLD,SIG_IGN);*/
#endif

	challenge.Protocol = LS_FLEXLM_PROTOCOL;
	strcpy(challenge.ChallengeData.VendorName, VENDOR_NAME);
	challenge.ChallengeData.VendorCode = lmcode;
	challenge.Size = sizeof(challenge);

/*
 *	We need this lm_init() here, since the LSAPI library won't call
 *	lm_init() until we do a checkout.
 */
	/* ts_use_sleep = 1;*/
	lm_init("demo", &code, &lm_job);
	lsrc = LSEnumProviders(0, provider);
	if (lsrc)
	{
		(void) printf("LSEnumProviders failed: %d\n", lsrc);
		exit((int)lsrc);
	}
	ts_Progname = strrchr(argv[0],'/');
	if (ts_Progname) ts_Progname++;
	else ts_Progname=argv[0];

	for (i=1; i<argc; i++) 
	{
		if (strcmp("-vendord",argv[i])==0) 
		{
			vendord = argv[++i];
		}
		else if (atoi(argv[i]) >= 0 && dotest == -1) 
			dotest = atoi(argv[i]);
		else if (*argv[i] >= 'a' && *argv[i] <= 'z')
			subtest = *argv[i];

	}
	gethostname(hostname, 100);
	id = lm_gethostid();
	hostid = l_asc_hostid(lm_job, id);
	strsave(hostid);

#if 0
/*
 * -----------------------------------------------------------------------------
 *	Preliminary - Create license files
 */

	if (dotest < 0 || dotest == 0)
	{
		if (!vendord) 
		{
			(void) getwd(demopath, LM_MAXPATHLEN-2);
#ifdef VMS
			demopath[strlen(demopath)-1] = '\0';
#endif
			(void) strcat(demopath, DEMOD);
			vendord = demopath;
		}
		(void) getwd(demo2path, LM_MAXPATHLEN-2);
#ifdef VMS
		demopath[strlen(demo2path)-1] = '\0';
#endif
		strcat(demo2path, "/demo2");
	
		(void) getwd(demofpath, LM_MAXPATHLEN-2);
#ifdef VMS
		demopath[strlen(demofpath)-1] = '\0';
#endif
		strcat(demofpath, "/demof");

		t = access(vendord, X_OK);
		if (t) {
			fprintf(stderr,"Vendor daemon %s is not executable: ",
				vendord);
			perror("");
			exit(1);
		}

		(void) printf(delim);
		(void) printf("Creating license files for host id %s\n",hostid);
		(void) printf(delim);

		cp(config, backup);
		(void) ts_lic_file(CF, hostname, CONFIG_FEATURE, hostid, 
			    vendord, "", FEATURE, "demo", VER, 0, 
			    "01-jan-99", (char *) NULL, "", hostid, (char **)0);
		(void) ts_lic_file(CF2, hostname, CONFIG_FEATURE, hostid, 
			    vendord, "", FEATURE, "demo", VER, 9, 
			    "01-jan-99", (char *) NULL, VENDORSTRING, NULL, 
								(char **)0);
		(void) ts_lic_append(CF2, "f2", CONFIG_FEATURE, "demo", VER, 
			OVER, 9, "01-jan-99", (char *) NULL, "", NULL);
		(void) ts_lic_append(CF2, "f3", CONFIG_FEATURE, "demo", VER, 
			OVER, 9, "01-jan-99", (char *) NULL, "", NULL);
		(void) ts_lic_append(CF2, FEATURE, CONFIG_FEATURE, "demo", VER,
			OVER, 0, "01-jan-90", (char *) NULL, "", hostid);

		(void) ts_lic_file(CF3, hostname, CONFIG_FEATURE, hostid, 
			    vendord, "", FEATURE, "demo", VER, 4, 
			    "01-jan-99", (char *) NULL, "", NULL, (char **)0);
		(void) ts_lic_append_vendor(CF3, "demo2", demo2path);
		(void) ts_lic_append(CF3, "f2", CONFIG_FEATURE, "demo2", VER, 
						OVER, 4, "01-jan-99", 
						(char *) NULL, "", NULL);
		(void) ts_lic_file(CF12, hostname, CONFIG_FEATURE, hostid, 
				    vendord, "", FEATURE, "demo", VER, 9, 
				    "01-jan-99", l_bin_date("01-jan-97"), 
					"", NULL, (char **)0);
		(void) ts_lic_file(CF15, hostname, CONFIG_FEATURE, hostid, 
			    vendord, "", FEATURE, "demo", VER, 9, 
			    "01-jan-99", (char *) NULL, "", NULL, (char **)0);
		(void) ts_lic_append(CF15, "f2", CONFIG_FEATURE, "demo", VER, 
							OVER, 0, "01-jan-99", 
						(char *) NULL, "", "DEMO");
		(void) ts_lic_file(CF15b, hostname, CONFIG_FEATURE, hostid, 
			    vendord, "", FEATURE, "demo", VER, 0, 
			    "01-jan-99", (char *) NULL, "", "DEMO", (char **)0);
		(void) ts_lic_append(CF15b, "f2", CONFIG_FEATURE, "demo", VER, 
							OVER, 9, "01-jan-99", 
						(char *) NULL, "", NULL);
		(void) ts_lic_file(CF16, hostname, CONFIG_FEATURE, hostid, 
			    demofpath, "", FEATURE, "demo", VER, 5, 
			    "01-jan-99", (char *) NULL, "", NULL, (char **)0);
		(void) ts_lic_append(CF16, "f2", CONFIG_FEATURE, "demo", VER, 
							OVER, 9, "01-jan-99", 
						(char *) NULL, "", NULL);
		(void) ts_lic_file(CF16b, hostname, CONFIG_FEATURE, hostid, 
				    demofpath, "", FEATURE, "demo", VER, 9, 
				    "01-jan-99", (char *) NULL, "", NULL,
				    (char **)0);
		sprintf(cmd, "./lmfeats demo -c %s -n >> %s\n", CF16b, CF16b);
		system(cmd);
		(void) ts_lic_append(CF16b, "f2", CONFIG_FEATURE, "demo", VER, 
							OVER, 3, "01-jan-99", 
						(char *) NULL, "", NULL);
		(void) ts_lic_file(CF16c, hostname, CONFIG_FEATURE, hostid, 
			    demofpath, "", FEATURE, "demo", VER, 5, 
			    "01-jan-99", (char *) NULL, "", NULL, (char **)0);
		(void) ts_lic_append(CF16c, "f2", CONFIG_FEATURE, "demo", VER, 
							OVER, 9, "01-jan-99", 
						(char *) NULL, "", NULL);
		sprintf(cmd, "./lmfeats demo -c %s -n >> %s\n", CF16c, CF16c);
		system(cmd);

		us = lm_username(1);
		sprintf(cmd, "echo RESERVE 2 f1 USER %s > %s", us, OPTSFILE);
		system(cmd);
		portinc = 0;
		(void) ts_lic_file(CF17, hostname, CONFIG_FEATURE, hostid, 
				    demo2path, OPTSFILE, FEATURE, "demo2", 
				    VER, 3, "01-jan-99", (char *) NULL, 
				    "", NULL, (char **)0);
		(void) ts_lic_file(CF17b, hostname, CONFIG_FEATURE, hostid, 
			    demo2path, "", FEATURE, "demo2", VER, 5, 
			    "01-jan-99", (char *) NULL, "", NULL, (char **)0);
		(void) ts_lic_append(CF17b, "f2", CONFIG_FEATURE, "demo2", 
						VER, OVER, 9, "01-jan-99", 
						(char *) NULL, "", NULL);
		(void) ts_lic_append(CF17b, "f3", CONFIG_FEATURE, "demo2", 
						VER, OVER, 9, "01-jan-99", 
						(char *) NULL, "", NULL);
		(void) ts_lic_file(CF17c, hostname, CONFIG_FEATURE, hostid, 
			    demo2path, "", FEATURE, "demo2", VER, 5, 
			    "01-jan-99", (char *) NULL, "", NULL, (char **)0);
		(void) ts_lic_append(CF17c, "f4", CONFIG_FEATURE, "demo2", 
						VER, OVER, 9, "01-jan-99", 
						(char *) NULL, "", NULL);
		(void) ts_lic_append(CF17c, "f5", CONFIG_FEATURE, "demo2", 
						VER, OVER, 9, "01-jan-99", 
						(char *) NULL, "", NULL);
		(void) ts_lic_file(CF17d, hostname, CONFIG_FEATURE, hostid, 
			    demo2path, "", FEATURE, "demo2", VER, 
			    4, "01-jan-99", (char *) NULL, "", NULL, 
			    (char **)0);
		portinc = 100;
		(void) ts_lic_file(CF17e, hostname, CONFIG_FEATURE, hostid, 
			    demo2path, "", "f2", "demo2", VER, 4, 
			    "01-jan-99", (char *) NULL, "", NULL, (char **)0);


		sprintf(cmd, "echo GROUP x a%s b%s %s ' ' > %s", us, us, 
							 us, OPTSFILE18);
		system(cmd);
		sprintf(cmd, "echo GROUP x c%s ' ' >> %s", us, OPTSFILE18);
		system(cmd);
		sprintf(cmd, "echo INCLUDE f1 GROUP x >> %s", OPTSFILE18);
		system(cmd);
		(void) ts_lic_file(CF18, hostname, CONFIG_FEATURE, hostid,
				    demo2path, OPTSFILE18, FEATURE, "demo2", 
				    VER, 3, "01-jan-99", (char *) NULL, "", 
				    NULL, (char **)0);

		sprintf(cmd, "echo GROUP x a%s b%s c%s > %s", us, us, us,
								OPTSFILE18b);
		system(cmd);
		sprintf(cmd, "echo GROUP x %s >> %s", us, OPTSFILE18b);
		system(cmd);
		sprintf(cmd, "echo INCLUDE f1 GROUP x >> %s", OPTSFILE18b);
		system(cmd);
		(void) ts_lic_file(CF18b, hostname, CONFIG_FEATURE, hostid, 
				    demo2path, OPTSFILE18b, FEATURE, "demo2", 
				    VER, 3, "01-jan-99", (char *) NULL, "", 
				    NULL, (char **)0);

		sprintf(cmd, "echo GROUP x a%s b%s %s c%s > %s", us, us, us, us,
								OPTSFILE18c);
		system(cmd);
		sprintf(cmd, "echo EXCLUDE f1 GROUP x >> %s", OPTSFILE18c);
		system(cmd);
		(void) ts_lic_file(CF18c, hostname, CONFIG_FEATURE, hostid, 
				    demo2path, OPTSFILE18c, FEATURE, "demo2", 
				    VER, 3, "01-jan-99", (char *) NULL, "", 
				    NULL, (char **)0);

		sprintf(cmd, "echo GROUP x a%s b%s c%s %s ' ' > %s", us, us, 
							us, us, OPTSFILE18d);
		system(cmd);
		sprintf(cmd, "echo EXCLUDE f1 GROUP x >> %s", OPTSFILE18d);
		system(cmd);
		(void) ts_lic_file(CF18d, hostname, CONFIG_FEATURE, hostid, 
				    demo2path, OPTSFILE18d, FEATURE, "demo2", 
				    VER, 3, "01-jan-99", (char *) NULL, "", 
				    NULL, (char **)0);

		sprintf(cmd, "echo GROUP x a%s b%s c%s ' ' > %s", us, us, us, 
							OPTSFILE18e);
		system(cmd);
		sprintf(cmd, "echo INCLUDE f1 GROUP x >> %s", OPTSFILE18e);
		system(cmd);
		(void) ts_lic_file(CF18e, hostname, CONFIG_FEATURE, hostid, 
				    demo2path, OPTSFILE18d, FEATURE, "demo2", 
				    VER, 3, "01-jan-99", (char *) NULL, "", 
				    NULL, (char **)0);
	
		sprintf(cmd, "echo GROUP x a%s b%s c%s > %s", us, us, us, 
							OPTSFILE18f);
		system(cmd);
		sprintf(cmd, "echo EXCLUDE f1 GROUP x >> %s", OPTSFILE18f);
		system(cmd);
		(void) ts_lic_file(CF18f, hostname, CONFIG_FEATURE, hostid, 
				    demo2path, OPTSFILE18f, FEATURE, "demo2", 
				    VER, 3, "01-jan-99", (char *) NULL, "", 
				    NULL, (char **)0);

		{
			int i;
			struct hostent *he;
			if (he=gethostbyname(hostname)) 
			{
				sprintf(cmd, "echo RESERVE 3 f1 INTERNET ");
				for (i=0;i< he->h_length;i++)
				{
					if (i) sprintf(cmd, "%s.", cmd);
					sprintf(cmd,"%s%d", cmd,
					(unsigned char)he->h_addr_list[0][i]);
				}
				sprintf(cmd, "%s >> %s", cmd, OPTSFILE18g);
				system(cmd);
			} else perror("gethostbyname failed");
		}
		(void) ts_lic_file(CF18g, hostname, CONFIG_FEATURE, hostid, 
				    demo2path, OPTSFILE18g, FEATURE, "demo2", 
				    VER, 3, "01-jan-99", (char *) NULL, "", 
				    NULL, (char **)0);

		(void)sprintf(cmd, "echo RESERVE 3 f1 INTERNET 192.156.100.100 >> %s", 
			OPTSFILE18h);
		system(cmd);

		(void) ts_lic_file(CF18h, hostname, CONFIG_FEATURE, hostid, 
				    demo2path, OPTSFILE18h, FEATURE, "demo2", 
				    VER, 3, "01-jan-99", (char *) NULL, "", 
				    NULL, (char **)0);

		(void) ts_lic_file(CF22, hostname, CONFIG_FEATURE, hostid, 
			    vendord, "", FEATURE, "demo", VER, 1, "01-jan-99", 
			    (char *) NULL, "", NULL, (char **)0);
		(void) ts_lic_append(CF22, "f1", CONFIG_INCREMENT, "demo", 
						VER, OVER, 1, "01-jan-99", 
						(char *) NULL, "", NULL);

		cp(CF, config);
		system(cmd);

 /* CF2 is the same as 4,5,6,11, and 21, except for port number*/
		cp(CF2, CF4); 
		cp(CF4, CF5); 
		cp(CF5, CF9); 
		cp(CF9, CF11); 

		rm(LOG);
		rm("demo.lock");
		rm("demo2.lock");
	}

#endif

    if (dotest < 0 || dotest == 1)
    {
/*
 * -----------------------------------------------------------------------------
 *	1	Unlimited licenses (with server running)
 */
	printf(delim);
	printf("1 - Unlimited licenses (with server running)\n");
	printf(delim);
	{
	  static int n[] = { 1, 1, 2, 5, 10, 1, 100, 1, 1000, 1000000, 0 };
	  int *nlic;

		for (nlic = n; *nlic; nlic++)
		{
			rc = lm_checkout("f0", VER, *nlic, 0, &code,
				LM_DUP_NONE);
			if (rc)
			{
			  char string[100];
				(void) sprintf(string, 
					"Checkout of %d license%s failed\n",
						*nlic, (*nlic==1?"":"s"));
				(void) lm_perror(string);
			}
		}
		for (nlic = n; *nlic; nlic++)
			lm_checkin("f0", 0);
	}

    }
    if (dotest < 0 || dotest == 2)
    {
/*
 * -----------------------------------------------------------------------------
 *	2	N licenses (all at once, in two pieces, one at a time)
 */
#if 0
	printf(delim);
	printf("2 - N licenses (all at once, in two pieces, one at a time)\n");
	printf(delim);

/* 
 *	All at once
 */
	h1 = good(FEATURE, VER, 9, "", __LINE__);
	h2 = breakc(FEATURE, VER, 1, "", __LINE__);
	test_misc_funcs(h1);
	release(h1);
	release(h2);
/* 
 *	All in 2 pieces 
 */
	h1 = good(FEATURE, VER, 4, "", __LINE__);
	h2 = good(FEATURE, VER, 5, "", __LINE__);
	h3 = breakc(FEATURE, VER, 1, "", __LINE__);
	release(h1);
	release(h2);
	release(h3);
/* 
 *	One at a time (3 max)
 */
	{
	  static int n[] = { 1, 1, 1, 0 };
	  int *nlic;
	  int i, j;

		for (i=0,nlic = n; *nlic; nlic++, i++)
		{
			char msg[100];
			sprintf(msg, "i = %d\n", i);
			h[i] = good(FEATURE, VER, *nlic, msg, __LINE__);
		}
		h[i] = breakc(FEATURE, VER, 1, "", __LINE__);
		for (j=0; j<i; j++)  release(h[j]);
	}
#endif

/* 
 * -----------------------------------------------------------------------------
 *	2.1	Multiple features from a single application
 */
	printf(delim);
	printf("2.1 - Multiple features from a single application\n");
	printf(delim);

	{
	  static char *features[] = { FEATURE, "f2", "f3", 0 };
	  char **x;
	  int j;
		
		for (i = 0, x = features; *x; i++, x++)
		{
			h[i] = good(*x, VER, 1, "", __LINE__);
		}
		for (j=0; j<i; j++)  release(h[j]);
	}
    }
    if (dotest < 0 || dotest == 3)
    {

/* 
 * -----------------------------------------------------------------------------
 *	3	Make sure several applications servers can run at once.
 */
	printf(delim);
	printf("3 - Make sure several applications servers can run at once.\n");
	printf(delim);


	h1 = good(FEATURE, VER, 1, "", __LINE__);
	h2 = good("f6", VER, 1, "", __LINE__);
	release(h1);
	release(h2);

    }
#if 0 /* does nothing with udp, at least as written */
    if (dotest < 0 || dotest == 5)
    {
/* 
 * -----------------------------------------------------------------------------
 *	5	Kill application, make sure server knows
 */
	printf(delim);
	printf("5 - Kill application, make sure server knows\n");
	printf(delim);

	h1 = good(FEATURE, VER, 9, "", __LINE__);
	release(h1);
	lsapi_sleep(20);		/* Give it time to happen */
	h1 = good(FEATURE, VER, 9, "", __LINE__);
	release(h1);

    }
#endif
    if (dotest < 0 || dotest == 9)
    {
/* 
 * -----------------------------------------------------------------------------
 *	9	Version expiration tests
 */
	printf(delim);
	printf("9 - Version expiration tests\n");
	printf(delim);

	{
	  static char * n[] = { "0.1", "0.2", "0.5", "0.9", "0.99", VER, 0 };
	  char **ver;
	  int j;

		for (i = 0, ver = n; *ver; i++, ver++)
		{
		    h[i] = good("f2", *ver, 1, "", __LINE__);
		}
		for (j=0; j<i; j++)  release(h[j]);
	}
	{
	  static char * n[] = { "1.1", "1.0510000001", "2.0", "9.0", "99.9", 
			0 };
	  char **ver;

		for (ver = n; *ver; ver++)
		{
		    h1 = breakc("f2", *ver, 1, 
			    "ERROR - CAN checkout for bad version" , __LINE__);
		    release(h1);
		}
	}

    }
    if (dotest < 0 || dotest == 11)
    {
/* 
 * -----------------------------------------------------------------------------
 *	11	Checkout/checkin cycle tests
 */
	printf(delim);
	printf("11 - Checkout/checkin cycle test\n");
	printf(delim);


	for (i = 0; i < 30; i++)
	{
		h1 = good(FEATURE, VER, 1, "Cannot cycle licenses", __LINE__);
		release(h1);
	}

    }
    if (dotest < 0 || dotest == 12)
    {
/* 
 * -----------------------------------------------------------------------------
 *	12	Feature start date
 */
	printf(delim);
	printf("12 - Feature start date test\n");
	printf(delim);


	h1 = breakc("f8", VER, 1, 
	    "license for version not enabled yet can be checked out", __LINE__);
	release(h1);

    }
    if (dotest < 0 || dotest == 15)
    {
/* 
 * -----------------------------------------------------------------------------
 *	15	- Mixed floating/demo licenses
 */
	printf(delim);
	printf("15 - Mixed floating/demo licenses\n");
	printf(delim);

	h1 = good("f15-1a", VER, 1, "(1) checkout real f1 failed", __LINE__);
	h2 = good("f15-1b", VER, 1, "(2) checkout DEMO f2 failed", __LINE__);
	release(h1); release(h2);
	h1 = good("f15-1b", VER, 1, "(3) checkout DEMO f2 failed", __LINE__);
	h2 = good("f15-1a", VER, 1, "checkout real f1 failed", __LINE__);
	release(h1); release(h2);

	h1 = good("f15-2a", VER, 1, "(4) checkout DEMO f1 failed", __LINE__);
	h2 = good("f15-2b", VER, 1, "(5) checkout real f2 failed", __LINE__);
	release(h1); release(h2);
	h1 = good("f15-2b", VER, 1, "(6) checkout real f2 failed", __LINE__);
	h2 = good("f15-2a", VER, 1, "(7) checkout DEMO f1 failed", __LINE__);
	release(h1); release(h2);

    }
    if (dotest < 0 || dotest == 16)
    {
/* 
 * -----------------------------------------------------------------------------
 *	16	- FEATURESET tests
 */
	printf(delim);
	printf("16 - FEATURESET tests\n");
	printf(delim);

	h1 = breakc("f16-1", VER, 1, "daemon came up with no FEATURESET line", 
								__LINE__);
	release(h1);

	h1 = good("f6", VER, 1, "Daemon failed with a good FEATURESET line",
								__LINE__);
	release(h1);

    }
#if 0
    if (dotest < 0 || dotest == 17)
    {

/* 
 * -----------------------------------------------------------------------------
 *	17	- lmreread tests
 */
      LM_USERS *users, *lc_userlist();
      int gotit;

	printf(delim);
	printf("17 - lmreread tests - reread same license file\n");
	printf(delim);



	gotit = 0;
	users = lm_userlist(FEATURE);
	while (users)
	{
	    if (*users->name == '\0')
	    {
		if (users->nlic != 3) 
			(void) 
			 printf("ERROR - we don't start with 3 licenses (%d)\n",
								users->nlic);
		gotit++;
		break;
	    }
	    users = users->next;
	}
	if (gotit == 0) 
		(void) printf(
		  "ERROR - lm_userlist() didn't return total after lmreread\n");


	unlink("/tmp/lmreread.log");
	unlink("/tmp/lmreread.log2");
#ifdef SVR4
	svr4_system("lmreread", "/tmp/lmreread.log");
#else
	system("./lmreread 1>/tmp/lmreread.log 2>&1");
#endif
	lsapi_sleep(2);

	gotit = 0;
	users = lm_userlist(FEATURE);
	while (users)
	{
	    if (*users->name == '\0')
	    {
		if (users->nlic != 3) 
			(void) 
			printf("ERROR - we don't have 3 licenses after lmreread (%d)\n",
								users->nlic);
		gotit++;
		break;
	    }
	    users++;
	}
	if (gotit == 0)
		(void) printf("ERROR - lm_userlist() didn't return total\n");

	lm_disconn(1);	/* drop the connection that lm_userlist() created */
#if 0	/*- Removed 7/10/92 */
	lsapi_sleep(10);	/* Give time to TCP-IP to shutdown the socket */
#endif

	h1 = good(FEATURE, VER, 1, "can't checkout first license", __LINE__);
	h2 = good(FEATURE, VER, 1, "can't checkout second license", __LINE__);
	h3 = good(FEATURE, VER, 1, "can't checkout third license", __LINE__);
/*-
 *	Now make sure we can only check out the 3 licenses
 */
	h4 = breakc(FEATURE, VER, 1, "Got more than 3 licenses", __LINE__);
	release(h1); release(h2); release(h3); release(h4);

#ifndef SVR4		/* SVR4 systems can't rename the license file! */
/*- ****************	FIX THIS TO WORK ON SVR4 */
/*
 *	17b - read new license file with 2 new features, 2 missing
 */
	printf(delim);
	printf("17b - lmreread tests - remove/add feature support\n");
	printf(delim);

	rm(CF17x);
	cp(CF17b, CF17x);
	lsapi_sleep(1);

	h1 = good(FEATURE, VER, 1, "can't checkout f1 license", __LINE__);
	h2 = good("f2", VER, 1, "can't checkout f2 license", __LINE__);
	h3 = good("f3", VER, 1, "can't checkout f3 license", __LINE__);
	h4 = breakc("f4", VER, 1, "CAN checkout f4 license", __LINE__);
	h5 = breakc("f5", VER, 1, "CAN checkout f5 license", __LINE__);
	release(h1); release(h2); release(h3); release(h4); release(h5);


#ifdef SVR4
	rename(CF17c, CF17x);
	svr4_system("lmreread", "/tmp/lmreread.log2");
#else
	rm(CF17x);
	cp(CF17c, CF17x);
	system("./lmreread 1>/tmp/lmreread.log2 2>&1");
#endif
	lsapi_sleep(2);


	h1 = good(FEATURE, VER, 1, "can't checkout f1 license", __LINE__);
	h2 = good("f4", VER, 1, "can't checkout f4 license", __LINE__);
	h3 = good("f5", VER, 1, "can't checkout f5 license", __LINE__);
	h4 = breakc("f2", VER, 1, "CAN checkout f2 license", __LINE__);
	h5 = breakc("f3", VER, 1, "CAN checkout f3 license", __LINE__);
	release(h1); release(h2); release(h3); release(h4); release(h5);
#endif	/* !SVR4 */
#ifndef SVR4		/* SVR4 systems can't rename the license file! */
/*- ****************	FIX THIS TO WORK ON SVR4 */
/*
 *	17c - read new license file with different count of 1 feature
 */
	printf(delim);
	printf("17c - lmreread tests - change feature count supported\n");
	printf(delim);

	rm(CF17x);
	cp(CF17d, CF17x);
	lsapi_sleep(1);

	h1 = good(FEATURE, VER, 1, "can't checkout f1 license", __LINE__);
	h2 = good(FEATURE, VER, 1, "can't checkout second f1 license",__LINE__);
	h3 = good(FEATURE, VER, 1, "can't checkout third f1 license", __LINE__);
	h4 = good(FEATURE, VER, 1, "can't checkout fourth f1 license",__LINE__);
	h5 = breakc(FEATURE, VER, 1, "can checkout fifth f1 license", __LINE__);
#ifdef SVR4
	rename(CF17, CF17x);
	svr4_system("lmreread", "/tmp/lmreread.log2");
#else
	rm(CF17x);
	cp(CF17, CF17x);
	system("./lmreread 1>/tmp/lmreread.log2 2>&1");
#endif
	lsapi_sleep(2);

	release(h4); release(h5);
	h4 = breakc(FEATURE, VER, 1, 
		     "can checkout fourth f1 license after lmreread", __LINE__);
	release(h3); 
	release(h4);
	h3 = good(FEATURE, VER, 1, 
		    "can't checkout third f1 license after lmreread", __LINE__);
	h4 = breakc(FEATURE, VER, 1, 
		     "can checkout fourth f1 license after lmreread", __LINE__);
	release(h4);
#ifdef SVR4
	rename(CF17d, CF17x);
	svr4_system("lmreread", "/tmp/lmreread.log2");
#else
	rm(CF17x);
	cp(CF17d, CF17x);
	system("./lmreread 1>/tmp/lmreread.log2 2>&1");
#endif
	lsapi_sleep(2);

	h4 = good(FEATURE, VER, 1, 
	       "can't checkout fourth f1 license after 2nd lmreread", __LINE__);
	h5 = breakc(FEATURE, VER, 1, 
		  "can checkout fifth f1 license after 2nd lmreread", __LINE__);
	release(h1); release(h2); release(h3); release(h4); release(h5);
#endif	/* !SVR4 */
#ifndef SVR4		/* SVR4 systems can't rename the license file! */
/*- ****************	FIX THIS TO WORK ON SVR4 */
/*
 *	17d - read new license file without feature, add feature back
 */
	printf(delim);
	printf("17d - lmreread tests - remove feature and add back\n");
	printf(delim);

	rm(CF17x);
	cp(CF17d, CF17x);
	lsapi_sleep(1);

	h1 = good(FEATURE, VER, 1, "can't checkout f1 license", __LINE__);
	h2 = good(FEATURE, VER, 1, "can't checkout second f1 license",__LINE__);
	h3 = good(FEATURE, VER, 1, "can't checkout third f1 license", __LINE__);
	h4 = good(FEATURE, VER, 1, "can't checkout fourth f1 license",__LINE__);
#ifdef SVR4
	rename(CF17e, CF17x);
	svr4_system("lmreread", "/tmp/lmreread.log2");
#else
	rm(CF17x);
	cp(CF17e, CF17x);
	system("./lmreread 1>/tmp/lmreread.log2 2>&1");
#endif
	lsapi_sleep(2);

	h5 = breakc(FEATURE, VER, 1, "can checkout f1 license after lmreread", 
								__LINE__);

#ifdef SVR4
	rename(CF17d, CF17x);
	svr4_system("lmreread", "/tmp/lmreread.log2");
#else
	rm(CF17x);
	cp(CF17d, CF17x);
	system("./lmreread 1>/tmp/lmreread.log2 2>&1");
#endif
	lsapi_sleep(2);

	h6 = breakc(FEATURE, VER, 1, 
		  "(43) - can checkout f1 license after 2nd lmreread", __LINE__);
	release(h6);
	release(h4); release(h5);
	h4 = good(FEATURE, VER, 1, 
		    "can't checkout fourth license after lmreread", __LINE__);
	h5 = breakc(FEATURE, VER, 1, 
		  "can checkout f1 license again after 2nd lmreread", __LINE__);
	release(h1); release(h2); release(h3); release(h4); release(h5);
#endif	/* !SVR4 */

    }
#endif
    if (dotest < 0 || dotest == 18)
    {

/* 
 * -----------------------------------------------------------------------------
 *	18	- INCLUDE/EXCLUDE/RESERVE INTERNET/ tests
 */

	printf(delim);
	printf("18 - INCLUDE/EXCLUDE/RESERVE INTERNET/ tests\n");
	printf(delim);
	

	h1 = good("f18-1", VER, 1, "can't checkout license", __LINE__);
	release(h1);

	h1 = good("f18-2", VER, 1, "can't checkout license", __LINE__);
	release(h1);


	h1 = breakc("f18-3", VER, 1, "can checkout license", __LINE__);
	release(h1);

	h1 = breakc("f18-4", VER, 1, "can checkout license", __LINE__);
	release(h1);

	h1 = breakc("f18-5", VER, 1, "can checkout license", __LINE__);
	release(h1);

	h1 = good("f18-6", VER, 1, "can't checkout license", __LINE__);
	release(h1);

	h1 = good("f18-7", VER, 1, "can't checkout license", __LINE__);
	release(h1);

	lsapi_sleep(1);

	h1 = breakc("f18-8", VER, 1, "can checkout license", __LINE__);
	release(h1);

    }
    if (dotest < 0 || dotest == 22)
    {
/* 
 *----------------------------------------------------------------------------
 *	22	INCREMENT tests
 */
      LM_USERS *users;

	printf(delim);
	printf("22 - INCREMENT tests - make sure INCREMENT line works\n");
	printf(delim);
	

	h1 = good("f22", VER, 1, "can't checkout license with INCREMENT line",
								__LINE__);
	h2 = good("f22", VER, 1, 
		"can't checkout second license with INCREMENT line", __LINE__);
	h3 = breakc("f22", VER, 1, "can checkout third (or second) license",
								__LINE__);
	release(h1); release(h2); release(h3);
    }

    if (dotest < 0 || dotest > 22)
    {
/*
 * -----------------------------------------------------------------------------
 *	Put things back to "normal"
 */
	(void) printf(delim);
	(void) printf("Cleaning up -- all errors reported above\n");
	(void) printf(delim);
    }
    if (dotest <= 22)
	exit(0);
    else
	exit(-1);
}

usage()
{
	fprintf(stderr,"usage: %s [-vendord path]\n",ts_Progname);
}

static
LS_HANDLE
good(feature, ver, num, msg, lineno)
char *feature;
char *ver;
int num;
char *msg;
int lineno;
{
  LS_HANDLE hand;
  unsigned long rc;
  unsigned long nunits = num;

	rc = LSRequest(provider, (LS_STR *) "demo", 
			(LS_STR *) feature, (LS_STR *) ver,
			nunits, (LS_STR *) NULL, (LS_CHALLENGE *)&challenge,
			&nunits, &hand);
	if (rc != LS_SUCCESS)
	{
		(void) printf("LSRequest failed: %s (line %d)\n", msg, lineno);
		(void) printf("LSAPI status: %x\n", rc);
	}
	return(hand);
}

static
LS_HANDLE
breakc(feature, ver, num, msg, lineno)
char *feature;
char  *ver;
int num;
char *msg;
int lineno;
{
  LS_HANDLE hand;
  unsigned long rc;
  unsigned long nunits = num;

	rc = LSRequest(provider, (LS_STR *) "demo", (LS_STR *) feature, 
			(LS_STR *) ver, nunits, (LS_STR *) NULL, 
			(LS_CHALLENGE *)&challenge, &nunits, &hand);
	if (rc == LS_SUCCESS)
	{
		(void) printf("LSRequest succeeded: %s (line %d)\n", msg, 
									lineno);
	}
	return(hand);
}

status(h)
LS_HANDLE h;
{
  unsigned long x;

	x = LSUpdate(h, 0, 0, "", (LS_CHALLENGE *)&challenge, &x);
	return((int) x);
}

release(h)
LS_HANDLE h;
{
	(void) LSRelease(h, 0, "");
	(void) LSFreeHandle(h);
}

test_misc_funcs(h)
LS_HANDLE h;
{
  LS_STR x[256];
  unsigned long y;
  struct foo { unsigned long size; char data[256]; } foo;
  struct foo2 { unsigned long size; LM_HANDLE *data; } foo2;
  LS_STATUS_CODE rc;
  LS_ULONG z;
	
	if (h == (LS_HANDLE) NULL)
	{
		(void) printf("ERROR: test_misc_funcs() called with NULL handle\n");
		return;
	}
	rc = LSQuery(h, LS_INFO_SYSTEM, x, (unsigned long) 256, &z);
	if (rc) decode(h, rc);
	else if (strcmp(x, provider))
	{
		(void) printf("ERROR: LSQuery doesn't return right provider\nExpected: \"%s\", Got: \"%s\"\n", provider, x);
	}

	foo.data[0] = '\0';
	rc = LSQuery(h, LS_INFO_DATA, (LS_STR *) &foo, 
					(unsigned long) sizeof(foo), &z);
	if (rc) decode(h, rc);
	else if (strcmp(foo.data, VENDORSTRING))
	{
		(void) printf("ERROR: LSQuery(..., LS_INFO_DATA,...) failed\nExpected: \"%s\", Got: \"%s\"\n",
				VENDORSTRING, foo.data);
	}

	rc = LSQuery(h, LS_UPDATE_PERIOD, (LS_STR *) &y, 
				(unsigned long) sizeof(unsigned long), &z);
	if (rc) decode(h, rc);
	else if (y < 1 || y > 60)
	{
		(void) printf("Questionable return from LSQuery(.., LS_UDATE_PERIOD,...) = %d\n",
									y, &z);
	}

	foo.data[0] = '\0';
	rc = LSQuery(h, LS_LICENSE_CONTEXT, (LS_STR *) &foo2, 
					(unsigned long) sizeof(foo2), &z);
	if (rc) decode(h, rc);
	else
	{
		if (foo2.data == (LM_HANDLE *) NULL)
			printf("Error: LSQuery(..., LS_LICENSE_CONTEXT,...) returns NULL handle\n");
		else if (foo2.data->type != LM_JOB_HANDLE_TYPE)
			printf("Error: LSQuery(..., LS_LICENSE_CONTEXT,...) returns non-JOB handle\n");
	}
}

decode(lshandle, rc)
LS_HANDLE lshandle;
LS_STATUS_CODE rc;
{
  LS_STATUS_CODE rcsave = rc;
  LS_STR errbuf[256];

	switch(rc)
	{
		case LS_BAD_HANDLE:
		case LS_INSUFFICIENT_UNITS:
		case LS_SYSTEM_UNAVAILABLE:
		case LS_LICENSE_TERMINATED:
		case LS_AUTHORIZATION_UNAVAILABLE:
		case LS_LICENSE_UNAVAILABLE:
		case LS_RESOURCES_UNAVAILABLE:
		case LS_NETWORK_UNAVAILABLE:
		case LS_TEXT_UNAVAILABLE:
		case LS_UNKNOWN_STATUS:
		case LS_BAD_INDEX:
		case LS_LICENSE_EXPIRED:
		case LS_BUFFER_TOO_SMALL:
		case LS_BAD_ARG:
		case LS_OTHER_FLEX_ERROR:
			rc = LSGetMessage(lshandle, rc, errbuf, 
							(unsigned long) 256);
			(void) printf("LSAPI status on request: %s (%x)\n", 
								errbuf, rcsave);
			(void) printf("LSAPI status on LSGetMessage: %x\n", rc);
		break;
	    default:
			(void) printf("default case taken\n");
			rc = LSGetMessage(lshandle, rc, errbuf, 
							(unsigned long) 256);
			(void) printf("LSAPI status on request: %s\n", errbuf);
	}
}
lsapi_sleep(seconds)
{
	sleep(seconds);
}
#if 0
last_test()
{
    if (dotest < 0 || dotest == 4)
    {
/* 
 * -----------------------------------------------------------------------------
 *	4	Kill server, make sure application dies
 */
	printf(delim);
	printf("4 - Kill server, make sure applications die\n");
	printf(delim);

	h1 = good(FEATURE, VER, 1, "", __LINE__);
	lsapi_sleep(60);		
	(void) status(h1); (void) status(h1);
	i = status(h1);
	if (i == 0) 
	{
		printf("Killed server - application kept on running\n");
	}
	release(h1);

    }
}
#endif
