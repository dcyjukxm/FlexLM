/*******************************************************************************
 * 
 *	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
 *	This software has been provided pursuant to a License Agreement
 *	containing restrictions on its use.  This software contains
 *	valuable trade secrets and proprietary information of 
 *	Macrovision Corporation. and is protected by law.  It may 
 *	not be copied or distributed in any form or medium, disclosed 
 *	to third parties, reverse engineered or used in any manner not 
 *	provided for in said License Agreement except with the prior 
 *	written authorization from Macrovision Corporation.
 * 
 ******************************************************************************/
/*
 *	Module: $Id: no_s_tests.c,v 1.6 2003/01/13 22:55:15 kmaclean Exp $
 *	
 *	Test suite for configurations with a single-host, no server running.
 *	
 *	Tests performed:
 *	
 *	1   Unlimited licenses (without server running)
 *	2   Expiration date check
 *	3   lm_status() check
 *	4   lm_auth_data() check
 *	5   checkout filter tests
 *	6   vendor-defined hostid tests
 *		
 *	M. Christiano
 *	3/15/88
 *	
 *	Last changed:  12/31/98
 *	
 */

#include "lmachdep.h"
#include <stdio.h>
#ifndef PC
#include <netdb.h>
#else
#include <winsock.h>
#include <pcsock.h>
#endif
#include "lmclient.h"
#include "lm_attr.h"
#include "testsuite.h"
#ifdef OS2
#include <pcsock.h>
#endif /* OS2 */
extern char *getwd();
extern char *l_asc_hostid();
extern HOSTID *lc_gethostid();
#ifdef VMS
extern void cp(), rm();
extern int l_compare_version();
void test_outfilter();
#endif /* VMS */
LM_DATA;
#include "code.h"
#ifdef CF
#undef CF
#endif
#define CF "license.dat"

char *delim = 
	"----------------------------------------------------------------\n";
FILE *ofp;

main(argc, argv)
int argc;
char *argv[];
{
  char gsipath[LM_MAXPATHLEN];
  char hostname[101];
  char ipaddr[101];
  char *hostid;
  HOSTID *id;
  CONFIG *conf;
  int rc;
  int i = 1;
  int porthostplus = 0;
  int liconly = 0;
  int nolic = 0;
  char envbuf[100];
  struct hostent *he;
  FILE *fp;

	ofp = stdout;
	if (argc > 1) 
	{
		while(i < argc)
		{
			if (!strcmp(argv[i], "-php"))
			{
				porthostplus = 1;
				nolic = 1;
			}
			if (!strcmp(argv[i], "-liconly"))
				liconly = 1;
			if (!strcmp(argv[i], "-o"))
			{
				i++;
				if (!(ofp = fopen(argv[i], "w")))
				{
					fprintf(stderr, "%s: ", argv[i]);
					perror("Can't open for writing");
					exit(1);
				}
			}
			i++;
		}
	}

	(void) lm_init("demo", &code, &lm_job);
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_registry(lm_job, "LM_LICENSE_FILE", "");
	lc_set_registry(lm_job, "DEMO_LICENSE_FILE", "");
	x_flexlm_newid();
	gethostname(hostname, 100);
#ifndef VMS
	if ((he = gethostbyname(hostname)) == 0){
		fprintf(ofp, "Invalid hostname, err line %d\n", __LINE__);
	}
	sprintf(ipaddr, "%d.%d.%d.%d", 
	    (unsigned char)he->h_addr_list[0][0],
	    (unsigned char)he->h_addr_list[0][1],
	    (unsigned char)he->h_addr_list[0][2],
	    (unsigned char)he->h_addr_list[0][3]);
#endif /* !VMS */

	id = lc_gethostid(lm_job);
	hostid = l_asc_hostid(lm_job, id);
	if (!nolic)
	{
	  char lic [4096];

/*
 * -----------------------------------------------------------------------------
 *  Preliminary - Create license files
 */

		(void) fprintf(ofp, delim);
		(void) getwd(gsipath, LM_MAXPATHLEN-2);
#ifdef VMS
		gsipath[strlen(gsipath)-1] = '\0';
#endif
		(void) strcat(gsipath, DEMOD);
		(void) fprintf(ofp, "Creating license files for host id %s\n", hostid);
		/*cp(config, backup);*/
		sprintf(lic, "SERVER %s %s 2837\n", 
#ifdef VMS
						hostname,
#else
						ipaddr, 
#endif /* VMS */
							hostid);
				sprintf(lic, "%sDAEMON demo demo\n", lic);
		sprintf(lic, "%sFEATURE f1 demo 1.0 1-jan-2010 0 0 HOSTID=%s\n",
								lic, hostid);
		sprintf(lic, "%sFEATURE f2 demo 1.0 1-jan-93 9 0\n", lic);
		sprintf(lic, "%sFEATURE f3 demo 1.0 1-jan-2010 0 0 HOSTID=EXAMPLE_HOSTID=12345678901234567890123456\n", lic);
		sprintf(lic, "%sFEATURE f4 demo 1.0 1-jan-2010 0 0 HOSTID=EXAMPLE_HOSTID=12345678901234567890123457\n", lic);
		unlink("license.dat");
		ts_lic_pkg("license.dat", lic);
		(void) fprintf(ofp, delim);
		if (liconly) exit(0);
	}
	if (porthostplus) sprintf(envbuf, "2837@%s", hostname);

/*
 * -----------------------------------------------------------------------------
 *	1	Unlimited licenses (without server running)
 */
	(void) fprintf(ofp, delim);
	(void) fprintf(ofp, "1 - Unlimited licenses (without server running)\n");
	(void) fprintf(ofp, delim);
	if (porthostplus)	/* The braces are needed on the PC since */
	{			/* "setenv" is a macro. */
		setenv("LM_LICENSE_FILE", envbuf);
	}
	else
	{
		setenv("LM_LICENSE_FILE", CF);
	}
	l_flush_config(lm_job);
	{
	  static int n[] = 
		{ 1, 2, 5, 10, 100, 1000, 100000, 1000000, 0 };
	  int *nlic;

		for (nlic = n; *nlic; nlic++)
		{
			if (porthostplus && (*nlic > 9999))
				continue;
			rc = lm_checkout(FEATURE, "1.0", *nlic, 0, &code,
							LM_DUP_NONE);
			if (rc)
			{
				(void) fprintf(ofp, "checkout of %d licenses failed\n", 
								*nlic);
				(void) lm_perror("");
			}
			lm_checkin(FEATURE, 0);
		}
	}
/*
 * -----------------------------------------------------------------------------
 *	2	Expiration date check
 */
	(void) fprintf(ofp, delim);
	(void) fprintf(ofp, "2 - Expiration date check\n");
	(void) fprintf(ofp, delim);
	if (porthostplus)
	{
		setenv("LM_LICENSE_FILE", envbuf);
	}
	else
	{
		setenv("LM_LICENSE_FILE", CF);
	}
	l_flush_config(lm_job);
	if (!lm_checkout("f2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "ERROR - Expired features can be checked out line %d\n",
			__LINE__);
	lm_checkin(FEATURE, 0);

/*
 * -----------------------------------------------------------------------------
 *	3	lm_status() check
 */
	(void) fprintf(ofp, delim);
	(void) fprintf(ofp, "3 - lm_status() check\n");
	(void) fprintf(ofp, delim);
	if (porthostplus)
	{
		setenv("LM_LICENSE_FILE", envbuf);
	}
	else
	{
		setenv("LM_LICENSE_FILE", CF);
	}
	l_flush_config(lm_job);
	rc = lm_status(FEATURE);
	if (rc == 0)
		(void) fprintf(ofp, "ERROR - lm_status() returns success with no license\n");
	rc = lm_checkout(FEATURE, "1.0", 1, 1, &code, LM_DUP_NONE);
	if (rc != 0)
		(void) fprintf(ofp, "ERROR - could not check out %s license\n", FEATURE);
	rc = lm_status(FEATURE);
	if (rc != 0)
		(void) lm_perror( "ERROR - lm_status() returns failure after checkout");
	lm_checkin(FEATURE, 0);
/*
 * -----------------------------------------------------------------------------
 *	4	lm_auth_data() check
 */
	(void) fprintf(ofp, delim);
	(void) fprintf(ofp, "4 - lm_auth_data() check\n");
	(void) fprintf(ofp, delim);
	if (porthostplus)
	{
		setenv("LM_LICENSE_FILE", envbuf);
	}
	else
	{
		setenv("LM_LICENSE_FILE", CF);
	}
	l_flush_config(lm_job);
	conf = lm_auth_data(FEATURE);
	if (conf)
		(void) fprintf(ofp, "ERROR - lm_auth_data() returns success with no license\n");
	rc = lm_checkout(FEATURE, "1.0", 1, 1, &code, LM_DUP_NONE);
	if (rc != 0)
		(void) fprintf(ofp, "ERROR - could not check out %s license\n", FEATURE);
	conf = lm_auth_data(FEATURE);
	if (conf == (CONFIG *) NULL)
		(void) lm_perror( "ERROR - lm_auth_data() returns failure after checkout");
	else if (strcmp(conf->feature, FEATURE)
		|| l_compare_version(lm_job, conf->version,  "1.0")
		|| strcmp(conf->daemon, "demo")
		|| strcmp(conf->date, "1-jan-2010")
		|| conf->users != 0
		|| conf->lc_vendor_def
		|| !conf->idptr)
	{
		fprintf(ofp, "ERROR: lm_auth_data returns:\n");
		fprintf(ofp, "	feature: \"%s\"\n", conf->feature);
		fprintf(ofp, "	version: \"%s\"\n", conf->version);
		fprintf(ofp, "	daemon: \"%s\"\n", conf->daemon);
		fprintf(ofp, "	date: \"%s\"\n", conf->date);
		fprintf(ofp, "	users: \"%d\"\n", conf->users);
		fprintf(ofp, "	code: \"%s\"\n", conf->code);
		fprintf(ofp, "	vendor_string: \"%s\"\n", 
				conf->lc_vendor_def ? conf->lc_vendor_def : "");
		fprintf(ofp, "	idptr.type: \"%d\"\n", conf->idptr ? 
				conf->idptr->type : -1);
	}
	lm_checkin(FEATURE, 0);
/*
 * -----------------------------------------------------------------------------
 *	5	checkout filter tests
 */
	(void) fprintf(ofp, delim);
	(void) fprintf(ofp, "5. checkout filter tests\n");
	(void) fprintf(ofp, delim);

	test_outfilter();
/*
 * -----------------------------------------------------------------------------
 *	6	Test vendor-defined hostid
 */
	(void) fprintf(ofp, delim);
	(void) fprintf(ofp, "6. vendor-defined hostid tests\n");
	(void) fprintf(ofp, delim);
	if (lm_checkout("f3", "1.0", 1, 1, &code, LM_DUP_NONE))
		fprintf(ofp, "Error %s line %d\n", lc_errstring(lm_job), __LINE__);
	if (!lm_checkout("f4", "1.0", 1, 1, &code, LM_DUP_NONE))
		fprintf(ofp, "Checkout shd have failed line %d\n", __LINE__);
/*
 * -----------------------------------------------------------------------------
 *	Put things back to "normal"
 */
	(void) fprintf(ofp, delim);
	(void) fprintf(ofp, "Cleaning up -- all errors reported above\n");
	(void) fprintf(ofp, delim);
	/*cp(backup, config);*/
	if (!porthostplus) rm("license.dat");
}

static
int
f1(c)
CONFIG *c;
{
	if (c->feature[0] == 'f') { lc_set_errno(lm_job, LM_LOCALFILTER); return(1); }
	return(0);
}

static
int
f2(c)
CONFIG *c;
{
	if (c->feature[0] == 'f') { lc_set_errno(lm_job, LM_NONETWORK); return(1); }
	return(0);
}

static
int
f3(c)
CONFIG *c;
{
	return(0);
}

#ifdef VMS
void
#endif
test_outfilter()
{
  int rc, (*x)();

	if (lc_get_attr(lm_job, LM_A_CHECKOUTFILTER, (short *) &x))
		lc_perror(lm_job, "Error getting CHECKOUTFILTER");
	if (x)
	{
		(void) fprintf(ofp, "ERROR: checkout filter set at start\n");
	}
	if (lc_set_attr(lm_job, LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)f1))
		lc_perror(lm_job, "Error setting CHECKOUTFILTER");
	lc_set_errno(lm_job, 0);
	if (!(rc = lc_checkout(lm_job, FEATURE, "1.0", 1, 0, &code, LM_DUP_NONE)))
		(void) fprintf(ofp, "ERROR: checkout filter f1 returns success\n");
	if (rc != LM_LOCALFILTER)
	{
		fprintf(ofp, 
		  "ERROR: checkout filter f1 returns wrong _lm_errno: %s, expected %d\n",
							lc_errstring(lm_job), LM_LOCALFILTER);
	}

	if (lc_set_attr(lm_job, LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)f2))
		lc_perror(lm_job, "Error getting CHECKOUTFILTER");
	lc_set_errno(lm_job, 0);
	if (!(rc = lc_checkout(lm_job, FEATURE, "1.0", 1, 0, &code, LM_DUP_NONE)))
	{
		(void) fprintf(ofp, "ERROR: checkout filter f2 returns success\n");
	}
	if (_lm_errno != LM_NONETWORK)
	{
		fprintf(ofp, 
		  "ERROR: checkout filter f2 returns wrong _lm_errno: %s, expected %d\n",
							lc_errstring(lm_job), LM_NONETWORK);
	}

	if (lc_set_attr(lm_job, LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)f3))
		lc_perror(lm_job, "Error getting CHECKOUTFILTER");
	lc_set_errno(lm_job, 0);
	if (rc = lc_checkout(lm_job, FEATURE, "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, 
		  "ERROR: checkout filter f3 returns wrong _lm_errno: %s, expected %d\n",
							lc_errstring(lm_job), 0);
	}

	if (lc_set_attr(lm_job, LM_A_CHECKOUTFILTER, 0))
		lc_perror(lm_job, "Error getting CHECKOUTFILTER");
}
