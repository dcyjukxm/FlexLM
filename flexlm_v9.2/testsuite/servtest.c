/******************************************************************************

	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
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
 *	Module: $Id: servtest.c,v 1.131 2003/04/18 23:48:09 sluu Exp $
 *	Function: servtest
 *
 *	Description: Test suite for configurations with a single-host 
 *			no child processes
 *			Adapted from one_s_tests
 *
 *	Tests performed:
 *
 *	1	Unlimited licenses (with server running)
 *	3	Make sure several applications servers can run at once.
 *	7	Expiration date check
 *	9	Version expiration tests
 *	10	Queueing tests
 *	11	Checkout/checkin cycle tests
 *	12	Feature start date tests
 *	13	lm_checkout() MAXUSERS return test
 *	15	mixed floating/demo licenses
 *	18	INCLUDE/EXCLUDE/RESERVE INTERNET tests
 *	19	Duplicate license checkin tests
 *	20	Lingering license tests
 *	21	Vendor-data change test
 *	22	INCREMENT tests
 *	23	UDP tests
 *	24	UDP lost and duplicated test
 *	25	Job tests
 *	26	extra checkout with lm_status
 *	27	lm_vsend() tests
 *	28  	LM_A_ALT_ENCRYPTION tests
 *	29	TIMEOUT tests
 *	30	LM_A_VD_* tests
 *	32 	PACKAGE tests
 *	33 	BADCODE tests
 *	34 	QUEUEing tests
 *	36	UPGRADE
 *	37	UPGRADE PACKAGE tests
 *	38 	+port@host test
 *	39 	vendor-defined hostid
 *	40 	v5 attributes
 *	41 	Simple and Trivial APIs
 *	42 	Bug tests
 *	43 	v5.1 attributes
 *	44 	path tests
 *	98	N licenses (all at once, in two pieces, one at a time)
 *	98.1	Multiple features from a single application
 *	last	checkout, kill server, make sure exitcall is called
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
 *	8/28/89 
 *
 *	Last changed:  12/14/98
 *
 */

#ifndef LM_INTERNAL
#define LM_INTERNAL
#endif /* LM_INTERNAL */
#include "lmprikey.h"
#include "lmpubkey.h"
#include "lmachdep.h"
#include "lmclient.h"
#include "laclient.h"
#include "lmpolicy.h"
#include "lm_attr.h"
#include "sys/stat.h"
#if !defined( _MSC_VER) && defined(ANSI)
#include "unistd.h"
#endif
#include "code.h"
#define TEST_DEMOF
#include "l_prot.h"
#ifdef PC		 
#include <io.h>		    
#include <direct.h>
#ifndef WINNT
#include <stdlib.h>
#ifndef OS2
#include <lzexpand.h>
#endif /* OS2 */
#endif /* WINNT */	    
	  extern int l_make_regs_testsuite; /* added for v8 */
#endif /* PC */
#ifndef WINNT
#include <string.h>
#endif
#include <time.h>
int this_mach_speed;

#ifdef NECSX4 /* L_STREQ blows up C compiler only in servtest on NECSX5 */
#undef L_STREQ
int L_STREQ(char *s1, char *s2){
    return ((*(s1) == *(s2)) && !strcmp((s1), (s2)));
}
#endif

char MAIN_LICENSE[30]; /* "servtest" */
char PKG_LICENSE[30];  /* "package" */
char ALT1_LICENSE[30]; /* "z_alt1" */
char ALT4_LICENSE[30]; /* "z_alt1" */
char ALT2_LICENSE[30]; /* "alt2" */
char ALT3_LICENSE[30]; /* "alt3" */

#ifdef USE_WINSOCK	    
#include <pcsock.h>	    
#else			    
#include <netdb.h>
#endif /* USE_WINSOCK */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>	/* for X_OK */
#if defined (MOTO_88K) || defined (sco)
#include <sys/unistd.h>	/* for X_OK */
#endif
#ifdef SVR4
#include <unistd.h>
#endif
#include "testsuite.h"
int quick = 0;
LM_HANDLE_PTR lm_job;

#ifdef VMS
#define PORT 200
#define PORT2 210
#else
#define PORT 2837
#define PORT2 9876
#endif /* VMS */
#ifdef __DECC
#define lc_sleep(j,t) sleep(t)
#endif /* __DECC */
#define LOG "_log"	/* Log file for the daemons */
#define CFTMP "license.dat.tmp"
#define OPTSFILE "opts"

#ifdef lm_sleep
#undef lm_sleep
#define lm_sleep(x) l_select_one(0, -1, 1000 * (x)); /* sleep x secs */
#endif
#define lc_sleep(j, x) l_select_one(0, -1, 1000 * (x)); /* sleep x secs */

struct _vendors {
	char *name;
	VENDORCODE *vc;
	unsigned long sig;
	} vendors[] = {

#ifdef TEST_DEMOF
	{"demo", &code, VENDOR_KEY5 },
	{"demof", &codef, VENDOR_KEYf_5 },
	{"demof2", &codef2, VENDOR_KEYf2_5 },
#else
	{"demo", &code, VENDOR_KEY5 },
#endif

	{ (char *)0, (VENDORCODE *)0, 0}};
#ifdef PC
	l_make_regs_testsuite = 1;
#define TWOPATH ".;..\\testsuite\\pathtest.dat"
#define MISSINGPATH ".;xyzzy.zzyyx;98761@globes;..\\testsuite\\pathtest.dat"
#define BADPATH ".;C:\\Windows;9876@foobar;..\\testsuite\\pathtest.dat"
#define BADPATH2 "C:\\Windows;.;C:\\Windows\\System;9876@foobar;..\\testsuite\\pathtest.dat"
#define LONGPATH "\\a\\b\\c\\d\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\e\\"
#else /* PC */

#define TWOPATH ".:../testsuite/pathtest.dat"
#define MISSINGPATH ".:xyzzy.zzyyx:98761@globes:../testsuite/pathtest.dat"
#define BADPATH ".:/etc/passwd:9876@foobar:../testsuite/pathtest.dat"
#define BADPATH2 "/etc/hosts:.:/etc/passwd:9876@foobar:../testsuite/pathtest.dat"
#define LONGPATH "/a/b/c/d/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/e/"
#endif /* not PC */


char p1981_serial[2];
char *path_list[] = { TWOPATH, MISSINGPATH, BADPATH, BADPATH2, 0};

#ifdef PC16
extern HINSTANCE _hInstance;
#endif

FILE *ofp;

#define VER "1.05"		/* Version number we usually use */
#define OVER "0.0"		/* Old Version number (only for UPGRADE) */
int got_exitcall, got_reconnect;
void CB_LOAD_DATA LM_CALLBACK_TYPE user_exitcall(f) char * f; { got_exitcall=1; }
void CB_LOAD_DATA LM_CALLBACK_TYPE user_reconnect(f, p, t, i) 
char *f;
int p;
int t;
int i;
{
	if (!f || !*f) 
		fprintf(ofp, "reconnect error line %d\n", __LINE__);
	got_reconnect = 1;
}

char hostname[101];

static int primes[] = {2,3,5,7,11,13,17,19,23,29,0};
char ** featlist_test lm_args((LM_HANDLE *, int));

int
gcd(i, j)
int i, j;
{
  int prime = 1; int *p;

	for(p = primes; *p && *p < i && *p < j; p++)
	{
		if (i > j && !(i % j)) return j;
		if (i < j && !(j % i)) return i;
		if (!(i % *p ) && !(j % *p))/* it's a divisor */
			prime *= *p;
	}
	return prime;
}

char *ts_Progname;
char *delim =
     "---------------------------------------------------------------------\n";
HOSTTYPE *hosttype;
LM_HANDLE *main_job;

LM_VD_FEATURE_INFO fi;
LM_VD_GENERIC_INFO gi;
#define strsave(x) { char *_p = malloc(strlen(x)+1); strcpy(_p, x); x = _p; }
char *argv_0;
static int file_based = 0;	/* Serverless */
int gotplus = 0;
int maxtest  = -1;
int dotest = -1;
int bugnumber = 0;
static int pc16=0;
LM_HANDLE *job[NUM_JOBS];
int win95 = 0;
int subtest = -1;
/*
 * Forward declarations
 */

void create_optsf();
VENDORCODE vc, lic_vc;
CONFIG *conf, *pos;
char hostid[MAX_HOSTID_LEN + 1];
char servtest_lic[257];

/*
 * Main
 */

int
main(argc, argv)
int argc;
char *argv[];
{
  char optline[LM_MAXPATHLEN];
  char *vendord= (char *) NULL;
  HOSTID *id;
  int rc;
  int i;
  char *us;
  int noadd = 1;	/* We default to the "non-additive" daemon */
  int server_only = 0;
  int use_server = 0;
  int make_license = 0;
  char *license_file = CF;
  char license_file_template[LM_MAXPATHLEN];
#ifdef PC
  WSADATA wsadata;
  OSVERSIONINFO p;
#endif
        ofp = stdout;

#ifdef PC
        

        p.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&p);
        if (p.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
                win95 = 1;
        if (WSAStartup((WORD)WINSOCK_VERSION, &wsadata))
        {
                fprintf( stderr, "ERROR; could not initialize Winsock\n" );
                exit( -1 );
        }

	l_putenv("FLEXLM_INTERVAL_OK=1");
#else 
	if (!getenv("FLEXLM_INTERVAL_OK")) 
	{
		fprintf(ofp, "$FLEXLM_INTERVAL_OK not set, exiting\n");
		exit(1);
	}
#endif

#ifdef PC
        l_putenv("LM_DEBUG_HOSTID=1");
        l_putenv("FLEXLM_BATCH=1");
#else
        setenv("LM_DEBUG_HOSTID","1");
#endif
	lm_job = (LM_HANDLE *)0;
	argv_0 = argv[0];
#ifndef PC	
	setlinebuf(stdout);
	setlinebuf(stderr);
#else
        setvbuf(stdout, 0, _IONBF, 0);
        setvbuf(stderr, 0, _IONBF, 0);
#endif /* PC */
#if 0 /*def UNIX*/
	{
	  static char rcname[100];
	  static char borrowname[100];
	  char hname[100] ;
		gethostname(hname, 99);
		sprintf(rcname, "%s/.flexlmrc_%s", l_real_getenv("HOME"), 
			hname);
		setenv("FLEXLM_RC", rcname); 
		sprintf(borrowname, "%s/.flexlmborrow_%s", 
			l_real_getenv("HOME"), hname);
		setenv("FLEXLM_BORROWFILE", borrowname); 
	}
#endif

	memcpy(&lic_vc, &code, sizeof(VENDORCODE));
	ts_lm_init("demo", &code, &job[0], __LINE__);
	lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)2);
	lm_job = job[0];
	x_flexlm_newid();
	hosttype = lm_hosttype(0);
	license_file_template[0] = '\0';
	sprintf(MAIN_LICENSE, "servtest.%s", LM_LICENSE_FILE_SUFFIX);
	sprintf(PKG_LICENSE, "package.%s", LM_LICENSE_FILE_SUFFIX);
	sprintf(ALT1_LICENSE, "z_alt1.%s", LM_LICENSE_FILE_SUFFIX);
	sprintf(ALT4_LICENSE, "z_alt2.%s", LM_LICENSE_FILE_SUFFIX);
	sprintf(ALT2_LICENSE, "alt.2.%s", LM_LICENSE_FILE_SUFFIX);
	sprintf(ALT3_LICENSE, "alt3.%s", LM_LICENSE_FILE_SUFFIX);


	for (i=1; i<argc; i++) 
	{
		if (strcmp("-o",argv[i])==0) 
		{
			i++;
			if (!(ofp = fopen(argv[i], "w")))
			{
				fprintf(stderr, "%s: ", argv[i]);
				perror("Can't open for writing");
				exit(1);
			}
#ifndef PC	
                        setlinebuf(ofp);
#else
                        setvbuf(ofp, 0, _IONBF, 0);
#endif
			continue;
		}
		if (strcmp("-hostname",argv[i])==0) 
		{
			strcpy(hostname, argv[++i]);
		}
		if (strcmp("-hostid",argv[i])==0) 
		{
			strcpy(hostid, argv[++i]);
		}
		if (strcmp("-vendord",argv[i])==0) 
		{
			vendord = argv[++i];
		}
		else if (strcmp(argv[i], "-add") == 0)
		{
			noadd = 0;
		}
		else if (strcmp("-file", argv[i]) == 0)
		{
			file_based = 1;
		}
                else if (strcmp("-nolic", argv[i]) == 0)
                {
                        make_license = 0;
                }
                else if (strcmp("-dolic", argv[i]) == 0)
                {
                        make_license = 1;
                }
                else if (strcmp("-quick", argv[i]) == 0)
                {
                        quick = 1;
			fprintf(ofp, "quick test, skipping some tests\n");
                }
		else if (strcmp("-server", argv[i]) == 0)
                {
                        use_server = 1;
                }
		else if (strcmp("-so", argv[i]) == 0)
		{
			server_only = 1;
		}
		else if (strcmp("-bug", argv[i]) == 0)
		{
			if (dotest == -1) dotest = 42;
			bugnumber = atoi(argv[++i]);
		}
                else if (strcmp("-c", argv[i]) == 0)
                {
			license_file = argv[++i];
                        make_license = 0;;
                }
                else if (strcmp("-pc", argv[i]) == 0)
                {
                         pc16 = 1;
                }
		else if (strncmp("-F", argv[i], 2) == 0)
		{
			strcpy(license_file_template, argv[i]+2);
		}
		else if (*argv[i] >= 'a' && *argv[i] <= 'z')
			subtest = *argv[i];
		else if (atoi(argv[i]) >= 0 && dotest == -1) 
		{
		  int k;
			dotest = atoi(argv[i]);
			for (k = 0;argv[i][k]; k++)
			{
				if (argv[i][k] == '+')
				{
					gotplus = 1;
					break;
				}
				else if (argv[i][k] == '-')
				{
					maxtest = atoi(&argv[i][k+1]);
					break;
				}
			}
		}

	}
	if (!*hostname)
		gethostname(hostname, 100);
	{
	  char buf[MAX_CONFIG_LINE + 1] = {'0'};

		if (!*hostid)
		{
			lc_hostid(lm_job, HOSTID_DEFAULT, buf);
#ifdef HP_LONG_HOSTID_STRING
			l_zcp(hostid, buf, strlen(buf) + 1);
#else /* !HP_LONG_HOSTID_STRING */
			l_zcp(hostid, buf, MAX_HOSTID_LEN);
#endif
		}
	}

#if defined(WINNT) || !defined(_WINDOWS)

/*
 * -----------------------------------------------------------------------------
 *	Preliminary - Create license files
 */
	/* calculate this machine speed */
	{
 	  time_t t = time(0);
	  FILE *sfp;
	  struct stat s;
	  char filename[50];
#define PC_SPEED 7
		sprintf(filename, "%s_speed.dat", l_platform_name());
	 	if (!stat(filename, &s))
		{
			sfp = fopen(filename, "r");
			fscanf(sfp, "%d", &this_mach_speed);
		}
		if (!this_mach_speed)
		{
			fprintf(ofp, "determining machine speed...");
			fflush(ofp);
			for (i = 0; i < 1000000000; i++) ;
			this_mach_speed = time(0) - t;
			sfp = fopen(filename, "w");
			fprintf(sfp, "%d\n", this_mach_speed);
		}
		fclose(sfp);
		this_mach_speed *= 100;
		this_mach_speed /= PC_SPEED;
		fprintf(ofp, "this_mach_speed = %d (2001 PC=100)\n", this_mach_speed);
	}

	if ((dotest == 0) || make_license)
	{
		if (getenv("LM_LICENSE_FILE"))  /* FEATURESET fails if set */
		{
			unsetenv("LM_LICENSE_FILE");
		}
		fprintf(ofp, delim);
		fprintf(ofp, "Creating license files for host id %s\n",hostid);
		fprintf(ofp, delim);
		unlink("servtest.err");
#define DAEMON_LINE "VENDOR demo  OPTIONS=opts\nVENDOR demof \nVENDOR demof2 demof2\n"
		makelic(lm_job, LM_RESERVED_THIS_HOST, hostid, PORT, 
			license_file_template, MAIN_LICENSE,
#ifdef VMS
                        vms_daemon_line(str, port)
#else
			DAEMON_LINE
#endif
			);
			
		makelic(lm_job, 0, 0, 0, "package.tem", 
						PKG_LICENSE, "");
		{
		  FILE *f;
			f = fopen("alt.2.tem", "w");
			fprintf(f, 
			"FEATURE alt demo 1.0 01-jan-2020 9 0 SIGN=0 \n");
			fclose(f);
			f = fopen("alt3.tem", "w");
			fprintf(f, "FEATURE g1 demo 1.0 01-jan-2020 9 0 SIGN=0 \n");
			fprintf(f, "FEATURE g2 demo 1.0 01-jan-2020 9 0 SIGN=0 \n");
			fprintf(f, "FEATURE g2 demo 1.0 01-jan-2020 9 0 SIGN=0 \n");
			fclose(f);
		}

		makelic(lm_job, hostname, "EXAMPLE_HOSTID2=VENDORID2", PORT, "alt.2.tem", 
					ALT2_LICENSE, DAEMON_LINE);
		makelic(lm_job, hostname, "EXAMPLE_HOSTID2=VENDORID3", PORT, "alt3.tem", 
					ALT3_LICENSE, DAEMON_LINE);
		{
		  FILE *f;
			f = fopen("z_alt2.tem", "w");
			fprintf(f, 
"INCREMENT f22 demo 1.05 1-jan-2020 20 0 ISSUER=GSI SIGN=0\n");
			fclose(f);
		}

		makelic(lm_job, hostname, "HOSTNAME=wronghost", PORT, "z_alt2.tem", 
					ALT4_LICENSE, DAEMON_LINE);
		unlink("alt.2.tem");
		unlink("alt3.tem");
		unlink("z_alt2.tem");
		makelic(lm_job, hostname, hostid, PORT2, "pathtest.tem", 
			"pathtest.dat", "DAEMON demo pathd pathtest.opt\n");
#if 0
		makelic(lm_job, hostname, hostid, 0, "borrow.tem", 
						"borrow.dat", "VENDOR demo\n");
#endif
/* 
 *		Make options files
 */
                us = lm_username(1);
                if ( pc16 == 1) us=hostname;
                sprintf(optline, "INCLUDE user_based USER %s", us);
		ts_opt_file(OPTSFILE, optline);
                sprintf(optline, "RESERVE 2 f1 USER %s", us);
                sprintf(optline, "RESERVE 1 colon:test USER %s", us);
		ts_opt_file_append(OPTSFILE, optline);
		ts_opt_file_append(OPTSFILE, 
			"RESERVE 1 bug1652:VERSION=0.5 USER nouser");
                sprintf(optline, "RESERVE 1 bug1652:VERSION=1.0 USER %s", us);
		ts_opt_file_append(OPTSFILE, optline);
                sprintf(optline, "RESERVE 1 p2493 USER %s", us);
		ts_opt_file_append(OPTSFILE, optline);
                sprintf(optline, "GROUP x1 a%s b%s %s \237%s", us, us, us, us);
                ts_opt_file_append(OPTSFILE, optline);
                sprintf(optline, "GROUP x1 c%s %s\236", us, us);
                ts_opt_file_append(OPTSFILE, optline);
		sprintf(optline, "HOST_GROUP xh %s", hostname);
		ts_opt_file_append(OPTSFILE, optline);
		sprintf(optline, "GROUP x2 a%s b%s %s c%s, %s\235", us, us, us, us, us);
		ts_opt_file_append(OPTSFILE, optline);
		sprintf(optline, "GROUP x3 a%s b%s c%s %s", us, us, us, us);
		ts_opt_file_append(OPTSFILE, optline);
		sprintf(optline, "GROUP x4 a%s b%s c%s", us, us, us);
                ts_opt_file_append(OPTSFILE, optline);
                sprintf(optline, "GROUP x5 a%s b%s c%s", us, us, us);
                ts_opt_file_append(OPTSFILE, optline);
                sprintf(optline, "GROUP bit8 %s %s %s %s %s %s %s %s %s %s",
		"\200\201\202\203\204\205\206\207\210", 
		"\211\212\213\214\215\216\217\220\221", 
		"\222\223\224\225\226\227\230\231\232", 
		"\233\234\235\236\237\241\242\243\244",
		"\245\246\247\250\251\252\253\254\255",
		"\256\257\260\261\262\263\264\265\266",
		"\267\270\271\272\273\274\275\276\277",
		"\300\301\302\303\304\305\306\307\310",
		"\311\312\313\314\315\316\317\320\321", 
		"\322\323\324\325\326\327\330\331\332");

                ts_opt_file_append(OPTSFILE, optline);
                ts_opt_file_append(OPTSFILE, "NOLOG IN");
                ts_opt_file_append(OPTSFILE, "NOLOG OUT");
                ts_opt_file_append(OPTSFILE, "NOLOG DENIED");
		if (file_based)
		{
                	sprintf(optline, "INCLUDE f18-1 USER %s", us);
                	ts_opt_file_append(OPTSFILE, optline);
                	sprintf(optline, "INCLUDE f18-2 HOST %s", hostname);
                	ts_opt_file_append(OPTSFILE, optline);
                	sprintf(optline, "EXCLUDE f18-3 USER %s", us);
                	ts_opt_file_append(OPTSFILE, optline);
                	sprintf(optline, "EXCLUDE f18-4 USER %s", us);
                	ts_opt_file_append(OPTSFILE, optline);
                	sprintf(optline, "INCLUDE f18-5 USER abc%s", us);
                	ts_opt_file_append(OPTSFILE, optline);
                	sprintf(optline, "EXCLUDE f18-6 USER abc%s", us);
                	ts_opt_file_append(OPTSFILE, optline);
		}
		else
		{
                	ts_opt_file_append(OPTSFILE, "INCLUDE f18-1 GROUP x1");
			ts_opt_file_append(OPTSFILE, "INCLUDE f18-2 HOST_GROUP xh");
			ts_opt_file_append(OPTSFILE, "EXCLUDE f18-3 GROUP x2");
			ts_opt_file_append(OPTSFILE, "EXCLUDE f18-4 GROUP x3");
                	ts_opt_file_append(OPTSFILE, "INCLUDE f18-5 GROUP x4");
                	ts_opt_file_append(OPTSFILE, "EXCLUDE f18-6 GROUP x5");
		}
#ifndef VMS
		sprintf(optline, "%s", "RESERVE 3 f18-7 INTERNET ");
		{
                        int i;
                        struct hostent *he;
                        if (he=gethostbyname(hostname))
                        {
                                for (i=0;i< he->h_length;i++)
                                {
                                        if (i) sprintf(optline, "%s.", optline);
                                        sprintf(optline,"%s%d", optline,
                                        (unsigned char)he->h_addr_list[0][i]);
                                }
                        } else perror("gethostbyname failed");
		}
#endif /* !VMS */
		ts_opt_file_append(OPTSFILE, optline);
		ts_opt_file_append(OPTSFILE, 
				"RESERVE 3 f18-8 INTERNET 192.156.100.100");
		ts_opt_file_append(OPTSFILE, "TIMEOUT f29 10");
		ts_opt_file_append(OPTSFILE, "TIMEOUT bug9999 10");
		ts_opt_file_append(OPTSFILE, "TIMEOUT bug1304_1 10");
		ts_opt_file_append(OPTSFILE, "TIMEOUT bug1304_2 10");
/*
 *		v5.1 features
 */
		ts_opt_file_append(OPTSFILE, "MAX 1 max GROUP x1");
                sprintf(optline, "RESERVE 1 euop1 USER %s", us);
			ts_opt_file_append(OPTSFILE, optline);
		ts_opt_file_append(OPTSFILE, "RESERVE 1 euop2 USER notme");
                sprintf(optline, "RESERVE 1 euop3 USER %s", us);
			ts_opt_file_append(OPTSFILE, optline);
		ts_opt_file_append(OPTSFILE, "RESERVE 1 euop4 USER notme");
		ts_opt_file_append(OPTSFILE, "LINGER p5910 5");
		ts_opt_file_append(OPTSFILE, "REPORTLOG +report.log");
		unlink("report.log");
/*
 *		V8 borrow
 */
                sprintf(optline, "INCLUDE_BORROW f1 USER %s", us);
			ts_opt_file_append(OPTSFILE, optline);
                sprintf(optline, "EXCLUDE_BORROW f4 USER %s", us);
			ts_opt_file_append(OPTSFILE, optline);
		ts_opt_file_append(OPTSFILE, "EXCLUDE_BORROW borrowok USER notme");
		ts_opt_file_append(OPTSFILE, "INCLUDE_BORROW borrowfail USER notme");
		ts_opt_file_append(OPTSFILE, "BORROW_LOWWATER lowwater 2");
		l_set_registry(lm_job, "HOSTID", "testing123", 0, 0);
		create_optsf();
#ifndef VMS
		rm(LOG);
#endif /* !VMS */
		if (file_based)
		{
			system("./lminstall > /dev/null\n");
		}
	}


    if (use_server)
    {
#ifndef VMS
        run_server(license_file, "all", LOG);	/* For test 2, test #1 doesn't care */
#endif /* !VMS */
	if (server_only)
	{
	    exit (0);
        }
    }

#endif /* defined(WINNT) || !defined(_WINDOWS) */

    if (!getenv(LM_DEFAULT_ENV_SPEC))
    {
	setenv(LM_DEFAULT_ENV_SPEC, license_file);
	strcpy(servtest_lic, license_file);
    }
    else strcpy(servtest_lic, getenv(LM_DEFAULT_ENV_SPEC));
            
    setenv("FLEXLM_INTERVAL_OK", "1");
    lc_free_job(job[0]);
    lc_new_job((LM_HANDLE *)0, 0, &code, &main_job);
    st_set_attr(main_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE) -1, __LINE__);
    st_set_attr(main_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0, __LINE__);
    st_set_attr(main_job, LM_A_MT_HEARTBEAT, (LM_A_VAL_TYPE) 0, __LINE__);
    for (i = 0; i < l_pubseedcnt; i++)
    {
      int j;
    	for (j =0; j < lm_pubsize[i][j]; j++)
	{
	    memcpy(&code.pubkeyinfo[i].pubkey[j], &lm_pubkey[i][j], 
	    				sizeof(lm_pubkey[i][j]));
	}
        code.pubkeyinfo[i].pubkey_fptr = l_pubkey_verify;
        memcpy(&code.pubkeyinfo[i].pubkeysize, &lm_pubsize, sizeof(lm_pubsize));
    	for (j =0; j < lm_pubsize[i][j]; j++)
	{
	    memcpy(&codef.pubkeyinfo[i].pubkey[j], &lm_pubkey[i][j], 
	    				sizeof(lm_pubkey[i][j]));
	}
        codef.pubkeyinfo[i].pubkey_fptr = l_pubkey_verify;
        memcpy(&codef.pubkeyinfo[i].pubkeysize, &lm_pubsize, sizeof(lm_pubsize));
    }


    if (dotest < 0 || dotest == 1)
    {
/*
 * -----------------------------------------------------------------------------
 *	1	Unlimited licenses (with server running)
 */
	serv_log("1 - Unlimited licenses (with server running)\n");
	{
	  static int n[] = { 1, 1, 2, 5, 10, 1, 100, 1, 1000, 9999, 0 };
	  int *nlic;

		ts_lc_new_job(&code, &job[0], __LINE__);
		/*lm_job = job[0];*/
		for (nlic = n; *nlic; nlic++)
		{
			rc = lc_checkout(job[0], "f0", VER, *nlic, 0, &code,
				LM_DUP_NONE);
			if (rc)
			{
				printf(
					"Checkout of %d license%s failed, %s\n",
				*nlic, (*nlic==1?"":"s"), lc_errstring(job[0]));
			}
		}
		for (nlic = n; *nlic; nlic++)
			lc_checkin(job[0], "f0", 0); 
		lc_free_job(job[0]);
	}
    }
    if ((dotest < 0 || dotest == 3) && !file_based || (dotest < 3 && maxtest >= 3))
    {


/* 
 * -----------------------------------------------------------------------------
 *	3	Make sure several applications servers can run at once.
 */
	serv_log("3 - Make sure several applications servers can run at once.\n");
	
	lm_job = 0;

	ts_lc_new_job( &code, &job[4], __LINE__);
#ifdef TEST_DEMOF
	ts_lm_init("demof", &codef, &job[5], __LINE__);
	lc_set_attr(job[5], LM_A_INTERNAL2, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[5], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
#endif
	if (lc_checkout(job[4], "f5", VER, 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "error line %d %s\n", __LINE__, 
			lc_errstring(job[4]));
	}
#ifdef TEST_DEMOF

	fprintf(ofp, "Expect error with USE_SERVER\n");
	if (lc_checkout(job[5], "f6", VER, 1, 0, &codef, LM_DUP_NONE))
	{
		fprintf(ofp, "error line %d %s\n", __LINE__, 
			lc_errstring(job[5]));
	}
	lc_checkin(job[5], "f6", 0);
	fprintf(ofp, "End Errors\n");
#endif /* DEMOF*/
	lc_checkin(job[4], "f5", 0);
	lc_free_job(job[4]);
	lc_free_job(job[5]);
	if (!quick)
	{
	  CONFIG *c;
/* 
 * -----------------------------------------------------------------------------
 *	3.1	Multiple checkouts of a single feature with no checkin
 */
		serv_log("3.1 - multiple checkouts of a single feature, then program dies\n");

		ts_lc_new_job( &code, &job[0], __LINE__);
		ts_lc_new_job( &code, &job[1], __LINE__);
		ts_lc_new_job( &code, &job[2], __LINE__);
		ts_lc_new_job( &code, &job[3], __LINE__);
		if (lc_checkout(job[0], "f5", VER, 1, 0, &code, LM_DUP_NONE))
		{
			fprintf(ofp, "error line %d %s\n", __LINE__, 
				lc_errstring(job[0]));
		}
		if (lc_checkout(job[1], "f5", VER, 1, 0, &code, LM_DUP_NONE))
		{
			fprintf(ofp, "error line %d %s\n", __LINE__, 
				lc_errstring(job[1]));
		}
		if (lc_checkout(job[2], "f5", VER, 1, 0, &code, LM_DUP_NONE))
		{
			fprintf(ofp, "error line %d %s\n", __LINE__, 
				lc_errstring(job[2]));
		}
		if (!(c = lc_auth_data(job[2], "f5")))
			fprintf(ofp, "lc_auth_data failed line %d, err %s\n", 
				__LINE__, lc_errstring(job[3]));
		lc_disconn(job[0], 1);
		lc_disconn(job[1], 1);
		lc_disconn(job[2], 1);
		l_select_one(0, -1, 1000); /* sleep 1 sec */
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = c;
		if (lc_get_attr(job[3], LM_A_VD_FEATURE_INFO, (short *)&fi)
			|| fi.tot_lic_in_use != 0)
		{
			
			fprintf(ofp, "error line %d exp %d got %d, err %s\n", 
				__LINE__, 0, fi.tot_lic_in_use, lc_errstring(job[3]));
			lc_checkin(job[0], "f5", 0);
			lc_checkin(job[1], "f5", 0);
			lc_checkin(job[2], "f5", 0);
		}
		lc_free_job(job[0]);
		lc_free_job(job[1]);
		lc_free_job(job[2]);
		lc_free_job(job[3]);
	}
    }
    if (dotest < 0 || dotest == 5 || (dotest < 5 && maxtest >=5) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	5	If applications closes socket, make sure daemon knows
 */
	serv_log("5 - client closes socket - make sure daemon knows\n");

	ts_lc_new_job( &code, &lm_job, __LINE__);
	if (lm_checkout("f1", VER, 1, 1, &code, LM_DUP_NONE))
		fprintf(ofp, "error line %d: %s\n", __LINE__, lc_errstring(lm_job));
	if (get_feat_info("f1", __LINE__, &fi, lm_job))
	{
		if (fi.tot_lic_in_use != 1)
			fprintf(ofp, "error exp %d got %d line %d\n", 1,
				fi.tot_lic_in_use, __LINE__);
	}
	if (!file_based)	/* No need to test this for file-based */
	{
		lc_disconn(lm_job, 1);
		lm_sleep(1);
		if (get_feat_info("f1", __LINE__, &fi, lm_job))
		{
			if (fi.tot_lic_in_use != 0)
				fprintf(ofp, "error line %d\n", __LINE__);
		}
	}
	lc_free_job(lm_job);
    }

    if ((dotest < 0 || dotest == 7)|| (dotest < 7 && maxtest >=7) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	7	Expiration date check
 */
	serv_log("7 - Expiration date check\n");

	ts_lc_new_job( &code, &job[0], __LINE__);
	lm_job = job[0];
	if (lm_checkout("f7", VER, 1, 1, &code, LM_DUP_NONE) == 0)
	{
		fprintf(ofp, "ERROR - Expired features can be checked out\n");
		ts_dump(FEATURE);
	}
	lm_checkin("f7", 0);
	lm_free_job(job[0]);
    }

    if ((dotest < 0 || dotest == 8)|| (dotest < 8 && maxtest >=8) )
    {
      char buf[1024];

/* 
 * -----------------------------------------------------------------------------
 *	8	Find license file with "-c"
 *
 */
	serv_log("8 - LM_A_LICENSE_FILE\n");

	ts_lc_new_job( &code, &job[0], __LINE__);
	st_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)MAIN_LICENSE, __LINE__);
	if (lc_checkout(job[0], "f9", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "error line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	}
	lc_checkin(job[0], "f9", 0);
	lc_free_job(job[0]);
	ts_lc_new_job( &code, &job[0], __LINE__);
	st_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1, __LINE__);
	if (!lc_set_attr(job[0], LM_A_LICENSE_FILE, 
				(LM_A_VAL_TYPE)"nosuch.foo"))
		fprintf(ofp, "LM_A_LICENSE_FILE error line %d\n", __LINE__);
	if (!lc_checkout(job[0], "f9", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "error line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	}
	lc_free_job(job[0]);
    }

    if ((dotest < 0 || dotest == 9)|| (dotest < 9 && maxtest >=9) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	9	Version expiration tests
 */
	serv_log("9 - Version expiration tests\n");

	ts_lc_new_job( &code, &job[0], __LINE__);

	{
	  static char * n[] = { "0.1", "0.2", "0.5", "0.9", "0.99", ".999", "1.00001", 
		"1.0", "4.0", "4.5", "4.50", "004.5", "004",
		"4.050", "4.06", "4.01", "1", "2", "", "0.0", "0", "0.", ".0",
		"001.000", 0 };
	  char **ver;

	        lm_job = job[0];
		for (ver = n; *ver; ver++)
		{
			if (lm_checkout("f9", *ver, 1, LM_CO_NOWAIT, &code, 
				LM_DUP_NONE))
			
				fprintf(ofp, "ERROR - Can't checkout ver %s: %s\n", 
					*ver, lc_errstring(lm_job));
			else
				lm_checkin("f9", 0);
		}
	}
	{
	  static char * n[] = { "10", "39.9", "4.51", "4.6", "4.60", 
		"5.0", "40", "41", "04.5001", "0004.6", "005", "19970321.0", 
		"1997032100", 0};
	  char **ver;

		for (ver = n; *ver; ver++)
		{
			if (!lm_checkout("f9", *ver, 1, LM_CO_NOWAIT, &code, 
				LM_DUP_NONE))
			
				fprintf(ofp, "ERROR - CAN checkout ver %s\n", *ver);
			else
				lm_checkin("f9", 0);
		}
	}
	lm_free_job(job[0]);
    }


    if ((dotest < 0 || dotest == 10)|| (dotest < 10 && maxtest >=10) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	10	Queueing tests
 */
	serv_log("10 - Queueing tests\n");

	ts_lc_new_job( &code, &job[0], __LINE__);
	ts_lc_new_job( &code, &job[1], __LINE__);
	ts_lc_new_job( &code, &job[2], __LINE__);
	lm_job = job[0];
	good_client_line_nc("f2", VER, 5, &code, LM_DUP_NONE, argv_0, 
				0, __LINE__);
/* 
 *	Now, add 5 to the queue, and make sure that no one can get in.
 */
	lm_job = job[1];
	queue_client_nc("f2", VER, 5, &code, LM_DUP_NONE, argv_0);
	if (rc = lm_status("f2") != LM_FEATQUEUE)
	{
		ts_error(lm_job, "lm_status for queue");
		lm_checkin("f2", 0);
	}
	lm_sleep(5);
/* 
 *	Now, make sure we can't get any
 */
	lm_job = job[2];
	if (lm_checkout("f2", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE) 
									== 0)
	{
		fprintf(ofp, "ERROR - we got licenses when queued users were ahead of us\n");
		ts_dump("f2");
		lm_checkin("f2", 0);
	}
	lm_job = job[0];
	lm_checkin("f2", 0);
	lm_job = job[1];
	lm_checkin("f2", 0);
	lm_free_job(job[0]);
	lm_free_job(job[1]);
	lm_free_job(job[2]);
    }

    if ((dotest < 0 || dotest == 11)|| (dotest < 11 && maxtest >=11) )
    {
      long t = time(0);
      long d;
      long elapsed;
/* 
 * -----------------------------------------------------------------------------
 *	11	Checkout/checkin cycle tests
 */
	serv_log("11 - Checkout/checkin cycle test\n");
#define CYCLE_CNT 300

	ts_lc_new_job( &code, &job[0], __LINE__);
	l_select_one(0, -1, 3000);
	t = time(0);
	for (i = 0; (elapsed = time(0) - t) < 10; i++)
	{
		rc = lc_checkout(job[0], "f40", "1.0", 1, LM_CO_NOWAIT, &code, LM_DUP_NONE);
		if (rc)
		{
            i++;
			fprintf(ofp, "Can't checkout f40 attempt %d, LINE %d %s\n", i, __LINE__,
				lc_errstring(job[0]));
			break;
		}  
		lc_checkin(job[0], "f40", 0);
	}
	i++;
        lc_free_job(job[0]);
	d = gcd(i, elapsed);
	if (elapsed)
	fprintf(ofp, "%s %d milliseconds/ckout, %d ckouts per second (%d/%d)\n", 
			l_platform_name(), 
		(elapsed * 1000 )/i,
			i/(time(0) - t),
			d ? i/d : -1, d ? elapsed/d : -1);
    }
#ifdef LM_UNIX_MT
    if ((dotest < 0 || dotest == 12)|| (dotest < 12 && maxtest >=12) )
    {
      long t = time(0);
      long d;
      long elapsed;
/* 
 * -----------------------------------------------------------------------------
 *	12	Checkout/checkin cycle tests -- single-threaded
 */
	serv_log("12 - non-mt Checkout/checkin cycle test\n");
#define CYCLE_CNT 300

	ts_lc_new_job( &code, &job[0], __LINE__);
	lc_set_attr(job[0], LM_A_MT_HEARTBEAT, (LM_A_VAL_TYPE)0);
	l_select_one(0, -1, 3000);
	t = time(0);
	for (i = 0; (elapsed = time(0) - t) < 10; i++)
	{
		rc = lc_checkout(job[0], "f40", "1.0", 1, LM_CO_NOWAIT, &code, LM_DUP_NONE);
		if (rc)
		{
			i++;
			fprintf(ofp, "Can't checkout f40 attempt %d, LINE %d %s\n", i, __LINE__,
				lc_errstring(job[0]));
			break;
		}  
		lc_checkin(job[0], "f40", 0);
	}
	i++;
        lc_free_job(job[0]);
	d = gcd(i, elapsed);
	if (elapsed)
	fprintf(ofp, "%s %d milliseconds/ckout, %d ckouts per second (%d/%d)\n", 
			l_platform_name(), 
		(elapsed * 1000 )/i,
			i/(time(0) - t),
			d ? i/d : -1, d ? elapsed/d : -1);
    }
#endif


    if ((dotest < 0 || dotest == 13)|| (dotest < 13 && maxtest >=13) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	13	lm_checkout() MAXUSERS test
 */
        serv_log("13 - lm_checkout() MAXUSERS test\n");

	ts_lc_new_job( &code, &job[0], __LINE__);
	lm_job = job[0];
	good_client_nc("f2", VER, 9, &code, LM_DUP_NONE, argv_0, 0);
	ts_lc_new_job( &code, &job[1], __LINE__);
	lm_job = job[1];
	rc = lm_checkout("f2", VER, 1, 0, &code, LM_DUP_NONE);
	if (rc != MAXUSERS)
	{
	  char string[100];
		sprintf(string, 
			"Checkout of 1 license did not return MAXUSERS");
		ts_error(lm_job, string);
	}
	lm_checkin("f2", 0);
	lm_job = job[0];
        lm_checkin("f2", 0);
	lm_free_job(job[0]);
	lm_free_job(job[1]);
    }


    if ((dotest < 0 || dotest == 15)|| (dotest < 15 && maxtest >=15) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	15	- Mixed floating/demo licenses
 */
	serv_log("15 - Mixed floating/demo licenses\n");

	ts_lc_new_job( &code, &job[0], __LINE__);
	lm_job = job[0];
	good_client_nc("f15-1a", VER, 1, &code, LM_DUP_NONE, argv_0, 
					"(1) checkout real f15-1 failed");
	good_client_nc("f15-1b", VER, 1, &code, LM_DUP_NONE, argv_0, 
					"(2) checkout DEMO f15-1 failed");
	lm_checkin("f15-1a", 0);
	lm_checkin("f15-1b", 0);

	good_client_nc("f15-1b", VER, 1, &code, LM_DUP_NONE, argv_0, 
					"(3) checkout DEMO f15-1 failed");
	good_client_nc("f15-1a", VER, 1, &code, LM_DUP_NONE, argv_0, 
					"checkout real f15-1 failed");
	lm_checkin("f15-1b", 0);
	lm_checkin("f15-1a", 0);

	good_client_nc("f15-2a", VER, 1, &code, LM_DUP_NONE, argv_0, 
					"(4) checkout DEMO f15-2 failed");
	good_client_nc("f15-2b", VER, 1, &code, LM_DUP_NONE, argv_0, 
					"(5) checkout real f15-2 failed");
	lm_checkin("f15-2a", 0);
	lm_checkin("f15-2b", 0);

	good_client_nc("f15-2b", VER, 1, &code, LM_DUP_NONE, argv_0,
					"(6) checkout real f15-2 failed)");
	good_client_nc("f15-2a", VER, 1, &code, LM_DUP_NONE, argv_0, 
					"(7) checkout DEMO f15-2 failed");
	lm_checkin("f15-2b", 0);
	lm_checkin("f15-2a", 0);
	lm_free_job(job[0]);
    }

#ifdef TEST_DEMOF
    if (((dotest < 0 || dotest == 16) && !file_based)|| (dotest < 16 && maxtest >=16) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	16	- FEATURESET tests
 */
	serv_log("16 - FEATURESET tests\n");


	ts_lm_init("demof2", &codef2, &job[0], __LINE__);
	lc_set_attr(job[0], LM_A_INTERNAL2, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lm_job = job[0];
	i = break_client_nc("f16-1", VER, 1, &codef2, LM_DUP_NONE, argv_0, 0);
	if (i == 0)
	{
		fprintf(ofp, "ERROR - Daemon came up with no FEATURESET line\n");
	}
	lc_free_job(lm_job);
	ts_lm_init("demof", &codef, &job[0], __LINE__);
	lc_set_attr(job[0], LM_A_INTERNAL2, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lm_job = job[0];
	st_set_attr(lm_job, LM_A_LKEY_START_DATE , (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(lm_job, LM_A_LKEY_LONG , (LM_A_VAL_TYPE)1, __LINE__);
	fprintf(ofp, "Expect error with USE_SERVER\n");
	i = good_client_nc("f6", VER, 1, &codef, LM_DUP_NONE, argv_0, 0);
	if (i != 0)
	{
		fprintf(ofp, "ERROR - Daemon failed with a good FEATURESET line\n");
	}
	fprintf(ofp, "End Errors\n");
	lc_free_job(lm_job);

    }
#endif /* TEST DEMOF */
    if ((dotest < 0 || dotest == 17)|| (dotest < 17 && maxtest >=17) )
    {
	reread_tests(subtest);
    }


    if ((dotest < 0 || dotest == 18)|| (dotest < 18 && maxtest >=18) )
    {

/* 
 * -----------------------------------------------------------------------------
 *	18	- INCLUDE/EXCLUDE/RESERVE INTERNET/ tests
 */

	if (file_based) serv_log( "18 - RESERVE INTERNET tests\n");
	else serv_log( "18 - INCLUDE/EXCLUDE/RESERVE INTERNET/ tests\n");

	if (!file_based)
	{
		ts_lc_new_job( &code, &lm_job, __LINE__);

		good_client_line_nc("f18-1", VER, 1, &code, LM_DUP_NONE, argv_0, 
				"(46) - can't checkout f18-1 license", __LINE__);
		lm_free_job(lm_job);

		ts_lc_new_job( &code, &lm_job, __LINE__);
		good_client_line_nc("f18-2", VER, 1, &code, LM_DUP_NONE, argv_0, 
				"(47) - can't checkout f18-2 license",__LINE__);
		lm_free_job(lm_job);
		ts_lc_new_job( &code, &lm_job, __LINE__);
		break_client_line_nc("f18-3", VER, 1, &code, LM_DUP_NONE, argv_0, 
			"(48) - can checkout f18-3 license", __LINE__);
		lm_free_job(lm_job);

		ts_lc_new_job( &code, &lm_job, __LINE__);
		if (!lc_checkout(lm_job, "f18-4", VER, 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "can checkout f18-4 license\n", __LINE__);

		lm_free_job(lm_job);

		ts_lc_new_job( &code, &lm_job, __LINE__);
		break_client_nc("f18-5", VER, 1, &code, LM_DUP_NONE, argv_0, 
						"(50) - can checkout f18-5 license");
		lm_free_job(lm_job);

		ts_lc_new_job( &code, &lm_job, __LINE__);
		good_client_nc("f18-6", VER, 1, &code, LM_DUP_NONE, argv_0, 
					"(51) - can't checkout f18-6 license");
		lm_free_job(lm_job);
	}

	ts_lc_new_job( &code, &lm_job, __LINE__);
	good_client_nc("f18-7", VER, 1, &code, LM_DUP_NONE, argv_0, 
					"(51b) - can't checkout f18-7 license");
	lm_free_job(lm_job);

	ts_lc_new_job( &code, &lm_job, __LINE__);
	if (!lc_checkout(lm_job, "f18-8", VER, 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "Checkout succeeded for f18-8, line %d\n", __LINE__);
	lm_free_job(lm_job);
    }
    if ((dotest < 0 || dotest == 19)|| (dotest < 19 && maxtest >=19) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	19	- Duplicate license checkin tests
 */

	serv_log( "19 - Duplicate license checkin tests\n");
	
	ts_lc_new_job( &code, &lm_job, __LINE__);
	good_client_line_nc("f2", VER, 1, &code, LM_DUP_DISP, argv_0, 
				"(52) - can't checkout DISP license", __LINE__);
	rc = lm_checkout("f2", VER, 1, 0, &code, LM_DUP_DISP);
	if (rc)
	{
		ts_error(lm_job, "Can't checkout second license (DISP)");
	}
	lm_checkin("f2", 0);

	good_client_nc("f2", VER, 1, &code, LM_DUP_USER, argv_0, 
					"(53) - can't checkout USER license");
	rc = lm_checkout("f2", VER, 1, 0, &code, LM_DUP_USER);
	if (rc)
	{
		ts_error(lm_job, "Can't checkout second license (USER)");
	}
	lm_checkin("f2", 0);

	good_client_nc("f2", VER, 1, &code, LM_DUP_HOST, argv_0, 
					"(54) - can't checkout HOST license");
	rc = lm_checkout("f2", VER, 1, 0, &code, LM_DUP_HOST);
	if (rc)
	{
		ts_error(lm_job, "Can't checkout second license (HOST)");
	}
	lm_checkin("f2", 0);
	if (get_feat_info("dup_19_1", __LINE__, &fi, lm_job))
	{
		if (fi.dup_select != LM_DUP_HOST)
			fprintf(ofp, "dup_19_1 group wrong, exp 0x%x, got 0x%x line %d\n", 
			LM_DUP_HOST, fi.dup_select,__LINE__);
	}
	if (get_feat_info("dup_19_2", __LINE__, &fi, lm_job))
	{
		if (fi.dup_select != (LM_DUP_HOST|LM_DUP_DISP))
			fprintf(ofp, "dup_19_2 group wrong, exp 0x%x got 0x%x line %d\n", 
				LM_DUP_HOST|LM_DUP_DISP,
				fi.dup_select,__LINE__);
	}
	if (get_feat_info("dup_19_3", __LINE__, &fi, lm_job))
	{
		if (fi.dup_select != (LM_DUP_HOST|LM_DUP_DISP|LM_DUP_USER))
			fprintf(ofp, "dup_19_3 group wrong, exp 0x%x got 0x%x line %d\n", 
				LM_DUP_HOST|LM_DUP_DISP|LM_DUP_USER,
				fi.dup_select,__LINE__);
	}
	if (get_feat_info("dup_19_4", __LINE__, &fi, lm_job))
	{
		if (fi.dup_select != (LM_DUP_HOST|LM_DUP_DISP|LM_DUP_USER|LM_DUP_VENDOR))
			fprintf(ofp, "dup_19_4 group wrong, exp 0x%x got 0x%x line %d\n", 
				LM_DUP_HOST|LM_DUP_DISP|LM_DUP_USER|LM_DUP_VENDOR,
				fi.dup_select,__LINE__);
	}
	if (get_feat_info("dup_19_5", __LINE__, &fi, lm_job))
	{
		if (fi.dup_select != (LM_DUP_HOST|LM_DUP_USER))
			fprintf(ofp, "dup_19_5 group wrong, exp 0x%x got 0x%x line %d\n", 
				LM_DUP_HOST|LM_DUP_USER,
				fi.dup_select,__LINE__);
	}
/* 
 *	P1135
 */
	ts_lc_new_job( &code, &job[0], __LINE__);
	ts_lc_new_job( &code, &job[1], __LINE__);
	if (lc_checkout(job[0], "dup_19_1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "Error, DUP_GROUP failed line %d\n", __LINE__);
	if (lc_checkout(job[1], "dup_19_1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "Error, DUP_GROUP failed line %d\n", __LINE__);
	if (get_feat_info("dup_19_1", __LINE__, &fi, lm_job))
	{
		if (fi.float_in_use != 1)
			fprintf(ofp, "dup_19_1 float_in_use wrong, exp 1 got %d line %d\n", 
				fi.float_in_use, __LINE__);
#if 1
		if (fi.user_cnt != 2)
			fprintf(ofp, "dup_19_1 user count wrong, exp 2 got %d line %d\n", 
				fi.user_cnt, __LINE__);
#endif
	}
	lc_disconn(job[0], 1);
	l_select_one(0, -1, 500); /* sleep 1/2 sec */
	lc_disconn(job[1], 1);
	l_select_one(0, -1, 500); /* sleep 1/2 sec */
	if (get_feat_info("dup_19_1", __LINE__, &fi, lm_job))
	{
		if (fi.float_in_use != 0)
		fprintf(ofp, "dup_19_1 float_in_use wrong, exp 0 got %d line %d\n", 
				fi.float_in_use, __LINE__);
		if (fi.user_cnt != 0)
		fprintf(ofp, "dup_19_1 user count wrong, exp 0 got %d line %d\n", 
				fi.user_cnt, __LINE__);
	}
/* 
 *	P1135 -- do it again, disconnecting in the other order
 */
	if (lc_checkout(job[0], "dup_19_1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "Error, DUP_GROUP failed line %d\n", __LINE__);
	if (lc_checkout(job[1], "dup_19_1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "Error, DUP_GROUP failed line %d\n", __LINE__);
	lc_disconn(job[1], 1);
	l_select_one(0, -1, 500); /* sleep 1/2 sec */
	lc_disconn(job[0], 1);
	l_select_one(0, -1, 500); /* sleep 1/2 sec */
	if (get_feat_info("dup_19_1", __LINE__, &fi, lm_job))
	{
		if (fi.float_in_use != 0)
		fprintf(ofp, "dup_19_1 float_in_use wrong, exp 0 got %d line %d\n", 
				fi.float_in_use, __LINE__);
		if (fi.user_cnt != 0)
		fprintf(ofp, "dup_19_1 user count wrong, exp 0 got %d line %d\n", 
				fi.user_cnt, __LINE__);
	}

	lm_free_job(job[0]);
	lm_free_job(job[1]);
	lm_free_job(lm_job);
    }
    if (((dotest < 0 || dotest == 20) )|| (dotest < 20 && maxtest >=20) )
    {
/* 
 *-----------------------------------------------------------------------------
 *	20	- Lingering license tests
 */
      LM_USERS *users;

	serv_log( "20 - Lingering license tests\n");
	
	ts_lc_new_job( &code, &lm_job, __LINE__);
	ts_lc_new_job( &code, &job[0], __LINE__);
	st_set_attr(lm_job, LM_A_LINGER, (LM_A_VAL_TYPE)5, __LINE__);
	rc = lm_checkout("f2", VER, 1, 0, &code, LM_DUP_NONE);
	if (rc)
	{
		ts_error(lm_job, "Can't checkout license");
	}
	lm_checkin("f2", 1);
	lm_sleep(3);
	if (get_feat_info("f2", __LINE__, &fi, lm_job))
	{
		if (fi.tot_lic_in_use != 1)
			fprintf(ofp, "linger error, exp %d, got %d line %d\n", 
					1, fi.tot_lic_in_use, __LINE__);
	}
	l_select_one(0, -1, 3000);
	lc_checkout(job[0], "f2", VER, 1, 0, &code, LM_DUP_NONE);
	ts_flush_server(job[0], __LINE__);
	lc_checkin(job[0], "f2", 1);
	if (get_feat_info("f2", __LINE__, &fi, job[0]))
	{
		if (fi.tot_lic_in_use != 0)
			fprintf(ofp, "linger error, line %d, exp 0, got %d\n", 
				__LINE__, fi.tot_lic_in_use);
	}
	lc_free_job(job[0]);
	lm_free_job(lm_job);
    }

    if ((dotest < 0 || dotest == 21)|| (dotest < 21 && maxtest >=21) )
    {
/* 
 *----------------------------------------------------------------------------
 *	21	Vendor-data change tests 
 */
      LM_USERS *users;
      int gotfirst, gotthird, gotsecond;
/*
 *	Bug #347
 */
	serv_log( "21 - Vendor-data change test - checkout 2 features and exit\n");
	
	ts_lc_new_job( &code, &lm_job, __LINE__);
	st_set_attr(lm_job, LM_A_CHECKOUT_DATA, 
				(LM_A_VAL_TYPE)"first", __LINE__);
	rc = lm_checkout("f21", VER, 1, 0, &code, LM_DUP_VENDOR);
	if (rc)
	{
		ts_error(lm_job, "Can't checkout first license (55)");
	}
	st_set_attr(lm_job, LM_A_CHECKOUT_DATA, 
				(LM_A_VAL_TYPE)"second", __LINE__);
	rc = lm_checkout("f21", VER, 1, 0, &code, LM_DUP_VENDOR);
	if (rc)
	{
		ts_error(lm_job, "Can't checkout second license (56)");
	}
	lm_disconn(1);
	if (get_feat_info("f21", __LINE__, &fi, lm_job))
	{
		if (fi.user_cnt != 0)
			fprintf(ofp, "license still out exp 0, got %d, line %d\n", 
				fi.user_cnt, __LINE__);
	}
/*
 *	Now see if we can check in only one of these licenses
 *	Bug #405
 */
	serv_log( "21b - Vendor-data change test\n");
	fprintf(ofp, "      checkout 3, then check them in, 1 at a time\n");
	st_set_attr(lm_job, LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"first", __LINE__);
	rc = lm_checkout("f21", VER, 1, 0, &code, LM_DUP_VENDOR);
	if (rc)
	{
		ts_error(lm_job, "Can't checkout first license (57)");
	}
	st_set_attr(lm_job, LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"second", __LINE__);
	rc = lm_checkout("f21", VER, 1, 0, &code, LM_DUP_VENDOR);
	if (rc)
	{
		ts_error(lm_job, "Can't checkout second license (58)");
	}
	st_set_attr(lm_job, LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"third", 
								__LINE__);
	rc = lm_checkout("f21", VER, 1, 0, &code, LM_DUP_VENDOR);
	if (rc)
	{
		ts_error(lm_job, "Can't checkout third license (59)");
	}
	st_set_attr(lm_job, LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"second", 
								__LINE__);
	lm_checkin("f21", 0);
	users = lm_userlist("f21");
	gotfirst = gotthird = 0;
	while (users)
	{
		if (*users->name != '\0')
		{
			if (!strcmp(users->vendor_def, "first")) 
				gotfirst = 1;
			else if (!strcmp(users->vendor_def, "third"))
				gotthird = 1;
			else
			{
				fprintf(ofp, 
	"EEKS: got other than 1st/3rd user: U:%s H:%s D:%s V:%s (60)\n",
					users->name, users->node, 
					users->display, users->vendor_def);
			}
		}
		users = users->next;
	}
	if (!gotfirst) 
		fprintf(ofp, "ERROR: First checkout not still out (61)\n");
	if (!gotthird) 
		fprintf(ofp, "ERROR: Third checkout not still out (62)\n");


/*
 *	Do the same for "first"
 */
	st_set_attr(lm_job, LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"first", __LINE__);
	lm_checkin("f21", 0);
	users = lm_userlist("f21");
	gotthird = 0;
	while (users)
	{
		if (*users->name != '\0')
		{
			if (!strcmp(users->vendor_def, "third"))
				gotthird = 1;
			else
			{
				fprintf(ofp, 
	"EEKS: got other than 3rd user: U:%s H:%s D:%s V:%s (60a)\n",
					users->name, users->node, 
					users->display, users->vendor_def);
			}
		}
		users = users->next;
	}
	if (!gotthird) 
		fprintf(ofp, "ERROR: Third checkout not still out (62a)\n");
/*
 *	Do the same for "third"
 */
	st_set_attr(lm_job, LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"third", __LINE__);
	lm_checkin("f21", 0);
	users = lm_userlist("f21");
	while (users)
	{
		if (*users->name != '\0')
		{
			fprintf(ofp, 
	"EEKS: got feature -- not expecting one: U:%s H:%s D:%s V:%s (60b)\n",
				users->name, users->node, 
				users->display, users->vendor_def);
		}
		users = users->next;
	}
	lm_disconn(1);
	lm_free_job (lm_job);

    }

    if ((dotest < 0 || dotest == 22)|| (dotest < 22 && maxtest >=22) )
    {
/* 
 *----------------------------------------------------------------------------
 *	22	INCREMENT tests
 */
	serv_log( "22 - INCREMENT tests - make sure INCREMENT line works\n");
	
	ts_lc_new_job( &code, &job[0], __LINE__);
	ts_lc_new_job( &code, &job[1], __LINE__);
	ts_lc_new_job( &code, &job[2], __LINE__);
	lm_job = job[0];
	good_client_nc("f22", VER, 1, &code, LM_DUP_NONE, argv_0, 
		    "(63) - can't checkout license with INCREMENT line");
	lm_job = job[1];
	good_client_nc("f22", VER, 1, &code, LM_DUP_NONE, argv_0, 
		    "(64) - can't checkout second license with INCREMENT line");
	lm_job = job[2];
	break_client_nc("f22", VER, 1, &code, LM_DUP_NONE, argv_0, 
		  	"(65) - can checkout third (or second) license");
	if (get_feat_info("overdraft_22", __LINE__, &fi, job[2]))
	{
		if (fi.overdraft != 2 || fi.num_lic != 5)
			fprintf(ofp, "overdraft_22 not incrementing, line %d\n", 
							__LINE__);
	}
	lm_checkin("f22", 0);
	lm_job = job[1];
	lm_checkin("f22", 0);
	lm_job = job[0];
	lm_checkin("f22", 0);

	lm_free_job(job[0]);
	lm_free_job(job[1]);
	lm_free_job(job[2]);


    }

#if 0
    if (((dotest < 0 || dotest == 23) && !file_based)|| (dotest < 23 && maxtest >=23) )
    {
/* 
 *----------------------------------------------------------------------------
 *	23	UDP tests
 */

	
	if (subtest == -1 || subtest == 'a')
	{

	

	    serv_log( "23a - Basic UDP tests (10 seconds)\n");
	    
	    ts_lc_new_job( &code, &job[0], __LINE__);
	    ts_lc_new_job( &code, &job[1], __LINE__);
	    ts_lc_new_job( &code, &job[2], __LINE__);
	    ts_lc_new_job( &code, &job[3], __LINE__);
	    st_set_attr(job[0], LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)-1, __LINE__);
            st_set_attr(job[1], LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)-1, __LINE__);
            st_set_attr(job[2], LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)-1, __LINE__);
            st_set_attr(job[3], LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)-1, __LINE__);
	    st_set_attr(job[0], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5, __LINE__);
            st_set_attr(job[1], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5, __LINE__);
            st_set_attr(job[2], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5, __LINE__);
            st_set_attr(job[3], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5, __LINE__);
	    st_set_attr(job[0], LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)LM_UDP, __LINE__);
	    st_set_attr(job[1], LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)LM_UDP, __LINE__);
	    st_set_attr(job[2], LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)LM_UDP, __LINE__);
	    st_set_attr(job[3], LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)LM_UDP, __LINE__);
	    if (lc_set_attr(job[0], LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)40))
		ts_error(job[0], "job[0] set_attr");
	    if (lc_set_attr(job[1], LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)40))
		ts_error(job[1], "job[0] set_attr");
	    if (lc_set_attr(job[2], LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)40))
		ts_error(job[2], "job[0] set_attr");
	    if (lc_set_attr(job[3], LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)40))
		ts_error(job[3], "job[0] set_attr");

	    if (lc_checkout(job[0], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "error line %d\n", __LINE__);
	    lc_sleep(job[0],10); /*let some heartbeats happen*/
	    if (lc_checkout(job[1], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "error line %d\n", __LINE__);
	    if (lc_checkout(job[2], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "error line %d\n", __LINE__);
	    if (lc_checkout(job[3], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "error line %d\n", __LINE__);
	    lc_checkin(job[0], "f1", 0);
	    lc_checkin(job[1], "f1", 0);
	    lc_checkin(job[2], "f1", 0);
	    lc_checkin(job[3], "f1", 0);
	    lc_free_job(job[0]);
	    lc_free_job(job[1]);
	    lc_free_job(job[2]);
	    lc_free_job(job[3]);
	}
	if (subtest == -1 || subtest == 'b')
	{
	
	  int num_reconnects, i, j;

            serv_log( "23b -  UDP tests -- timeout UDP clients (35 seconds)\n");
	    ts_lc_new_job( &code, &job[0], __LINE__); 
	    ts_lc_new_job( &code, &job[1], __LINE__); 
	    ts_lc_new_job( &code, &job[2], __LINE__);
	    if (lc_set_attr(job[0], LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)40))
		ts_error(job[0], "job[0] set_attr");
	    if (lc_set_attr(job[1], LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)3))
		ts_error(job[0], "job[0] set_attr");
	    if (lc_set_attr(job[2], LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)40))
		ts_error(job[0], "job[0] set_attr");
	    st_set_attr(job[0], LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)-1, __LINE__);
	    st_set_attr(job[1], LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)-1, __LINE__);
	    st_set_attr(job[2], LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)-1, __LINE__);
	    st_set_attr(job[0], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)-1, __LINE__);
	    st_set_attr(job[1], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)-1, __LINE__);
	    st_set_attr(job[2], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)-1, __LINE__);
	    st_set_attr(job[0], LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)LM_UDP, 
								__LINE__);
	    st_set_attr(job[1], LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)LM_UDP, 
								__LINE__);
	    st_set_attr(job[2], LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)LM_UDP, 
								__LINE__);
	    if (lc_checkout(job[0], "f1", "1.0", 4, 0, &code, LM_DUP_NONE))
	    {
		ts_error(job[0], "checkout");
		fprintf(ofp, "UDP error line %d\n", __LINE__);
	    }
	    if (lc_checkout(job[1], "f1", "1.0", 3, 0, &code, LM_DUP_NONE))
	    {
		ts_error(job[1], "checkout");
		fprintf(ofp, "UDP error line %d\n", __LINE__);
	    }
/*
 *	Time out 2nd job
 */
	    l_select_one(0, -1, 6000);
	    ts_flush_server(job[0], __LINE__);
	    ts_flush_server(job[0], __LINE__);
	    get_feat_info("f1", __LINE__, &fi, job[0]);
	    if (fi.tot_lic_in_use != 4)
		fprintf(ofp, "UDP timeout not working, line %d, exp %d got %d\n",
				__LINE__, 4, fi.tot_lic_in_use);
	    lc_checkin(job[0], "f1", 0);
/* 	
 *		keep sending heartbeats till we reconnect and num_reconnects
 * 		gets set correctly
 */
	    j = 0;
	    while ((i = lc_heartbeat(job[1], &num_reconnects, 1) 
						|| !num_reconnects) && j < 5)
	    {
		lc_sleep(job[0], 1);
		j++;
	    }
	    if (num_reconnects != 1)
		fprintf(ofp, "lc_heartbeat not working, line %d, exp %d got %d %s i=%d\n",
				__LINE__, 1, num_reconnects, 
				lc_errstring(job[1]), i);
	    lc_checkin(job[1], "f1", 0);
	    lm_free_job(job[0]);
	    lm_free_job(job[1]);
	    lm_free_job(job[2]);
	}
    }
    if ((((dotest < 0 || dotest == 24) && !quick) && !file_based)|| (dotest < 24 && maxtest >=24) )
    {
	    serv_log( "24 -  UDP tests -- duplicate and lost messages (15 seconds)\n");
	    if (!getenv("TEST_UDP_SERNUM"))
		fprintf(ofp, 
	    "Warning: $TEST_UDP_SERNUM not set, UDP test may not be valid\n");
		
/* 		
 *		force every third (random) message to be lost or duplicated.
 */
	    if (!getenv("TEST_UDP_SERNUM")) {
		    setenv("TEST_UDP_SERNUM", "3"); 
	    }
	    ts_lc_new_job( &code, &job[0], __LINE__); lm_job = job[0];
	    st_set_attr(lm_job, LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)120, __LINE__);
	    st_set_attr(lm_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)10, __LINE__);
	    good_client_line_nc("f2", VER, 1, &code, LM_DUP_NONE, 
				argv_0, 0, __LINE__);
	    ts_lc_new_job( &code, &job[1], __LINE__); lm_job = job[1];
	    st_set_attr(lm_job, LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)120, __LINE__);
	    st_set_attr(lm_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)10, __LINE__);
	    good_client_udp_nc("f2", VER, 1, &code, LM_DUP_NONE, 
				argv_0, 0, __LINE__);
	    ts_lc_new_job( &code, &job[2], __LINE__); lm_job = job[2];
	    st_set_attr(lm_job, LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)120, __LINE__);
	    st_set_attr(lm_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)10, __LINE__);
	    good_client_udp_nc("f3", VER, 1, &code, LM_DUP_NONE, 
				argv_0, 0, __LINE__);
	    ts_lc_new_job( &code, &job[3], __LINE__); lm_job = job[3];
	    st_set_attr(lm_job, LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)120, __LINE__);
	    st_set_attr(lm_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)10, __LINE__);
	    good_client_udp_nc("f3", VER, 1, &code, LM_DUP_NONE, 
				argv_0, 0, __LINE__);
	    ts_lc_new_job( &code, &job[4], __LINE__); lm_job = job[4];
	    st_set_attr(lm_job, LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)120, __LINE__);
	    st_set_attr(lm_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)10, __LINE__);
	    break_client_udp_nc("f3", VER, 9, &code, LM_DUP_NONE, 
				argv_0, 0, __LINE__);
	    lm_sleep(15); /* let some heartbeats happen*/
	    ts_lc_new_job( &code, &job[5], __LINE__); lm_job = job[5];
	    st_set_attr(lm_job, LM_A_UDP_TIMEOUT, (LM_A_VAL_TYPE)120, __LINE__);
	    break_client_udp_nc("f3", VER, 9, &code, LM_DUP_NONE, 
				argv_0, 0, __LINE__);
	    lm_checkin("f3", 0);
	    lm_job = job[4]; lm_checkin("f3", 0);
	    lm_job = job[3]; lm_checkin("f3", 0);
	    lm_job = job[2]; lm_checkin("f3", 0);
	    lm_job = job[1]; lm_checkin("f2", 0);
	    lm_job = job[0]; lm_checkin("f2", 0);
	    unsetenv("TEST_UDP_SERNUM"); 
	    lm_free_job(job[0]);
	    lm_free_job(job[1]);
	    lm_free_job(job[2]);
	    lm_free_job(job[3]);
	    lm_free_job(job[4]);
	    lm_free_job(job[5]);

    }
#endif /* 0*/

    if ((dotest < 0 || dotest == 25)|| (dotest < 25 && maxtest >=25) )
    {
/* 
 *----------------------------------------------------------------------------
 *	25	Job tests
 */
      char msg[1024];

	    serv_log( "25 - Job Tests\n");
	    
	    ts_lc_new_job( &code, &job[0], __LINE__);
	    if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
	    {
		    sprintf(msg, "Can't checkout f1 (line %d)", __LINE__);
		    ts_error(lm_job, msg);
	    }
#ifdef TEST_DEMOF
	    ts_lm_init("demof", &codef, &job[1], __LINE__);
	    lc_set_attr(job[1], LM_A_INTERNAL2, (LM_A_VAL_TYPE)1);
	    lc_set_attr(job[1], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	    if (!lc_checkout(job[0], "f6", "1.0", 1, LM_CO_NOWAIT, &codef, LM_DUP_NONE))
	    {
		    sprintf(msg, "checkout of f6 succeeded - shouldn't(line %d)", __LINE__);
		    ts_error(lm_job, msg);
	    }
	    fprintf(ofp, "Expect Error with USE_SERVER\n");
	    if (lc_checkout(job[1], "f6", "1.0", 1, LM_CO_NOWAIT, &codef, LM_DUP_NONE))
	    {
		    sprintf(msg, "Can't checkout f6 (line %d)", __LINE__);
		    ts_error(job[1], msg);
	    }
	    lc_checkin(job[1], "f6", 0);
	    lc_free_job(job[1]);
	    fprintf(ofp, "End errors\n");
#endif
	    lc_checkin(job[0], "f1", 0);
	    lm_free_job(job[0]);

    }

    if ((dotest < 0 || dotest == 26)|| (dotest < 26 && maxtest >=26) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	26	Bug P437 extra checkout with lm_status
 */
	serv_log( "26 - Extra checkout with lm_status\n");

	ts_lc_new_job( &code, &lm_job, __LINE__);

	if (lm_checkout("f1", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
		ts_error(lm_job, "checkout f1");
	if (rc = lm_status("f1")) 
	{
		fprintf(ofp, "lm_status should be 0, linenum %d\n", __LINE__);
	}
	if (!(rc = lm_status("f2"))) 
	{
		fprintf(ofp, "lm_status shouldn't be 0, linenum %d\n", __LINE__);
	}
	if (lm_checkout("f2", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
		ts_error(lm_job, "checkout f2");
	if (rc = lm_status("f1")) 
	{
		fprintf(ofp, "lm_status should be 0, linenum %d\n", __LINE__);
	}
	if (rc = lm_status("f2")) 
	{
		fprintf(ofp, "lm_status should be 0, linenum %d\n", __LINE__);
	}
	lm_checkin("f1", 0);
	if ((rc = lm_status("f1"))==0) 
	{
		fprintf(ofp, "lm_status shouldn't be 0, linenum %d\n", __LINE__);
	}
	if (rc = lm_status("f2")) 
	{
		fprintf(ofp, "lm_status should be 0, linenum %d\n", __LINE__);
	}
	if (lm_checkout("f2", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
		ts_error(lm_job, "checkout f2");
	if ((rc = lm_status("f1")) == 0) 
	{
		fprintf(ofp, "lm_status shouldn't be 0, linenum %d\n", __LINE__);
	}
	if (rc = lm_status("f2")) 
	{
		fprintf(ofp, "lm_status should be 0, linenum %d\n", __LINE__);
	}
	lm_checkin("f2", 0);
	if ((rc = lm_status("f1"))==0) 
	{
		fprintf(ofp, "lm_status shouldn't be 0, linenum %d\n", __LINE__);
	}
	if ((rc = lm_status("f2"))==0)
	{
		fprintf(ofp, "lm_status shouldn't be 0, linenum %d\n", __LINE__);
	}
	lm_free_job(lm_job);

    }

    if (((dotest < 0 || dotest == 27) && !file_based)|| (dotest < 27 && maxtest >=27) )
    {
      char *ret;
/* 
 * -----------------------------------------------------------------------------
 *	27 - lm_vsend() tests.
 */
	serv_log( "27 - lm_vsend() tests\n");

	ts_lc_new_job( &code, &lm_job, __LINE__);

	if (lm_checkout("f1", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
		ts_error(lm_job, "checkout f1");
	ret = lm_vsend("test");
	if (!ret || strcmp(ret, "ok"))
	{
		fprintf(ofp, "Error - lm_vsend() doesn't work: return: \"%s\"\n",ret);
		ts_error(lm_job, "lm_vsend");
	}
	lm_checkin("f1", 0);
	lm_disconn(1);
/*
 *	Make sure we get a 2nd server started
 */
	good_client_line_nc("f1", VER, 1, &code, LM_DUP_NONE, argv_0, 
				0, __LINE__);
	good_client_line_nc("f1", VER, 1, &code, LM_DUP_NONE, argv_0, 
				0, __LINE__);
	good_client_line_nc("f1", VER, 1, &code, LM_DUP_NONE, argv_0, 
				0, __LINE__);
	good_client_line_nc("f1", VER, 1, &code, LM_DUP_NONE, argv_0, 
				0, __LINE__);
	good_client_line_nc("f1", VER, 1, &code, LM_DUP_NONE, argv_0, 
				0, __LINE__);
	good_client_line_nc("f1", VER, 1, &code, LM_DUP_NONE, argv_0, 
				0, __LINE__);

	if (lm_checkout("f1", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
		ts_error(lm_job, "checkout f1");
	ret = lm_vsend("test");
	if (!ret || strcmp(ret, "ok"))
	{
		fprintf(ofp, "Error - lm_vsend() doesn't work with 2nd server: return: \"%s\"\n",
									ret);
		ts_error(lm_job, "lm_vsend");
	}
	lm_checkin("f1", 0);
	lm_free_job(lm_job);

    }
#ifndef FREE_VERSION
/* 
 * -----------------------------------------------------------------------------
 *	28 - LM_A_ALT_ENCRYPTION tests
 */
    if ((dotest < 0 || dotest == 28)|| (dotest < 28 && maxtest >=28) )
    {
	serv_log( "28 -  LM_A_ALT_ENCRYPTION tests \n");

	ts_lc_new_job( &code, &job[0], __LINE__);
	ts_lc_new_job( &code, &job[1], __LINE__);
/*
 *	make sure LM_A_ALT_ENCRYPTION doesn't break things
 */
	lc_set_attr(job[0], LM_A_ALT_ENCRYPTION, 
			(LM_A_VAL_TYPE)(LM_CHAR_PTR) &altcode);
        lc_set_attr(job[1], LM_A_ALT_ENCRYPTION, 
			(LM_A_VAL_TYPE)(LM_CHAR_PTR) &altcode);
/* 
 *	All at once
 */
	lm_job = job[0];
	good_client_line_nc("f28", VER, 1, &code, LM_DUP_NONE, argv_0, 0,__LINE__);
	lm_job = job[1];
	break_client_line_nc("f28", VER, 1, &code, LM_DUP_NONE, argv_0, 0, __LINE__);
	lm_checkin("f28", 0);
	lm_job = job[0];
	lm_free_job(job[0]);
	lm_free_job(job[1]);
    }
#endif /* FREE_VERSION */
/* 
 * -----------------------------------------------------------------------------
 *	29 - TIMEOUT tests
 */
    if (((dotest < 0 || dotest == 29)  && !file_based)|| (dotest < 29 && maxtest >=29) )
    {
	serv_log( "29 -  TIMEOUT tests (12 seconds)\n");

	ts_lc_new_job( &code, &job[0], __LINE__);
	ts_lc_new_job( &code, &job[1], __LINE__);
        st_set_attr(job[0], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)-1, __LINE__);
	if (lc_checkout(job[0], "f29", VER, 1,LM_CO_NOWAIT, &code, LM_DUP_NONE))
	{
		ts_error(job[0], "checkout failed");
		goto exit29;
	} 
	/* This one should fail */
	get_feat_info("f29", __LINE__, &fi, job[0]);
	if (fi.tot_lic_in_use != 1)
			fprintf(ofp, "TIMEOUT not working, line %d, exp %d got %d",
				__LINE__, 1, fi.tot_lic_in_use);
	
			
	l_select_one(0, -1, 11000);
	ts_flush_server(job[1], __LINE__);

	get_feat_info("f29", __LINE__, &fi, job[1]);
	if (fi.tot_lic_in_use != 0)
			fprintf(ofp, "TIMEOUT not working, line %d, exp %d got %d",
				__LINE__, 0, fi.tot_lic_in_use);
		
exit29:
	lc_checkin(job[0], "f29", 0);
	lm_free_job(job[0]);
    }

#if 0 /* these aren't needed anymore -- we test these all over the place */

/* 
 * -----------------------------------------------------------------------------
 *	30 - LM_A_VD_* tests
 */
    if ((dotest < 0 || dotest == 30)|| (dotest < 30 && maxtest >=30) )
    {
      LM_HANDLE *h1;
	serv_log( "30 -  LM_A_VD_* tests \n");

	ts_lc_new_job( &code, &h1, __LINE__);
	if (lc_checkout(h1, "f1", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "line %d: ", __LINE__);
		ts_error(h1, "checkout failed");
	} 
	if (lm_vd_msg(h1, "f1"))
	{
		fprintf(ofp, "line %d: ", __LINE__);
		ts_error(h1, "LM_A_VD_* failed");
	} 

		/* f6 is only in demof, so should fail */
	if (!lm_vd_msg(h1, "f6")) 
	{
		fprintf(ofp, "line %d: ", __LINE__);
		ts_error(h1, "LM_A_VD_* succeeded, but should have failed");
	} 
	lc_checkin(h1, "f1", 0);
	lm_free_job(h1);
    }
#endif

#ifdef LM_SUPPORT_MT
/* 
 * -----------------------------------------------------------------------------
 *	31 - purify test
 *
 *	1) start server and do a checkout
 *	2) while heartbeats go off, do lm_userlist and malloc/free calls
 *	3) call lmremove to have the server remove the client
 *	4) while client reconnects, do lm_userlist and malloc/free calls
 */
#ifdef VMS
#define MALLOCSIZE 10
#else
#define MALLOCSIZE 100
#endif

    if (((dotest < 0 || dotest == 31) && !quick)|| (dotest < 31 && maxtest >=31) )
    {
      long t, t2;        
      char *u, *h, *d;

	serv_log( "31 -  malloc/free reentrancy, purify test (70 seconds)\n");

	ts_lc_new_job( &code, &lm_job, __LINE__);
	
	st_set_attr(lm_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)15, __LINE__);
	st_set_attr(lm_job, LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)15, __LINE__);
	if (lm_checkout(FEATURE, VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
	{
	   char c[1024];
		sprintf(c, "checkout failed line %d", __LINE__);
		ts_error(lm_job, c);
	} 
		
	t = time(0);
        while ((time(0) - t) < 30)
        {
          int i = 1;
		t2 = time(0);
		/* 5 seconds worth of free/malloc calls */
		while ((time(0) - t2) < 5) free(malloc(i++ * MALLOCSIZE));
                if (!lm_userlist("f1"))
			fprintf(ofp, "error line %d: %s\n", __LINE__, 
							lc_errstring(lm_job));
		
		l_select_one(0, -1, 2000);
        }
	ts_lc_new_job( &code, &job[0], __LINE__);
	u = lm_username(1);
	h = lm_hostname(1); 
	d = lm_display(1);
	if (lc_remove(job[0], "f1", u, h, d))
		fprintf(ofp, "error line %d %s\n", __LINE__, lc_errstring(job[0]));
		
	t = time(0);
        while ((time(0) - t)  < 30)  /* should check it back out again */
        {
          int i = 1;
		t2 = time(0);
		/* 5 seconds worth of free/malloc calls */
		while ((time(0) - t2) < 5) free(malloc(i++ * MALLOCSIZE));
                if (!lc_userlist(job[0], "f1"))
			fprintf(ofp, "error line %d: %s\n", __LINE__, 
							lc_errstring(job[0]));
		l_select_one(0, -1, 2000);
        }
        lm_checkin("f1", 0);

	lc_free_job(job[0]);
	lc_free_job(lm_job);
    }
#endif

/* 
 * -----------------------------------------------------------------------------
 *	32 - PACKAGE tests
 *
 */
    if ((dotest < 0 || dotest == 32)|| (dotest < 32 && maxtest >=32) )
    {
      int cnt;

	serv_log( "32 -  PACKAGE tests\n");

	ts_lc_new_job( &code, &lm_job, __LINE__);
	if (lm_checkout("comp1", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
	{
	   char c[1024];
		sprintf(c, "checkout failed line %d", __LINE__);
		ts_error(lm_job, c);
	} 
	/* same job, so it will work */
	if (lm_checkout("comp2", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
	{
	   char c[1024];
		sprintf(c, "checkout failed line %d", __LINE__);
		ts_error(lm_job, c);
	} 
	lm_checkin("comp2", 1);
	lm_checkin("comp1", 1);
	if (!lm_checkout("suite", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
	{
	   char c[1024];
		sprintf(c, "checkout should have failed, line %d", __LINE__);
		ts_error(lm_job, c);
	}
	lm_checkin("suite", 0);
/*
 *	pkg -- it should be in job, but not server
 */
	conf = lc_get_config(lm_job, "pkg");
	if (!conf && lm_job->lm_errno != LM_NOSERVSUPP) 
		fprintf(ofp, "can't find pkg1comp1 line %d %s\n", __LINE__,
			lc_errstring(lm_job));
	else
	{
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = conf;
		if (!lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi) &&
			(lm_job->lm_errno != LM_NOSERVSUPP))
			fprintf(ofp, "pkg shouldn't be in server line %d\n", 
							__LINE__);
	}
/*
 *	pkg1
 */
	conf = lc_get_config(lm_job, "pkg1comp1");
	if (!conf) fprintf(ofp, "can't find pkg1comp1 line %d\n", __LINE__);
	else
	{
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = conf;
		lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi);
		if (fi.num_lic != 4) 
			fprintf(ofp, "Wrong pkg numlic (%d) line %d\n", 
				fi.num_lic, __LINE__);
		if (strncmp(conf->version, "1.0", 3)) 
			fprintf(ofp, "Wrong pkg ver line %d\n", __LINE__);
	}
/*
 *	pkg2 issues are already tested earlier
 *	pkg3 is already tested earlier
 */
	conf = lc_get_config(lm_job, "pkg3comp1");
	if (!conf) fprintf(ofp, "can't find pkg3comp1 line %d\n", __LINE__);
	else
	{
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = conf;
		lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi);
		if (fi.num_lic != 10) 
			fprintf(ofp, "Wrong pkg numlic (%d) line %d\n", 
				fi.num_lic, __LINE__);
	}
/*
 *	pkg5
 */
	conf = lc_get_config(lm_job, "pkg5comp1");
	if (!conf) fprintf(ofp, "can't find pkg5comp1 line %d\n", __LINE__);
	else
	{
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = conf;
		lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi);
		if (fi.num_lic != 5) fprintf(ofp, "Wrong pkg numlic line %d\n", 
								__LINE__);
	}
	conf = lc_get_config(lm_job, "pkg5");
	if (!conf) fprintf(ofp, "can't find pkg5 line %d %s\n", __LINE__, lc_errstring(lm_job));
	else
	{
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = conf;
		lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi);
		if (fi.num_lic != 5) 
			fprintf(ofp, "Wrong pkg numlic line %d exp 5, got %d\n", 
						__LINE__, fi.num_lic);
	}
/*
 *	pkg6
 */
	cnt = 0;
	pos = (CONFIG *)0;
	while (conf = lc_next_conf(lm_job, "pkg6comp1", &pos))
	{
		cnt++;
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = conf;
		lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi);
		if (cnt == 1)
		{
			if (fi.num_lic != 2) 
				fprintf(ofp, "Wrong pkg numlic line %d\n", __LINE__);
			if (strcmp(conf->version, "1.05"))
				fprintf(ofp, "pkg6comp1 wrong version line %d\n", 
								__LINE__);
			break;
		}
		else
		{
			if (fi.num_lic != 4) 
				fprintf(ofp, "Wrong pkg numlic (%d) line %d\n", 
					fi.num_lic, __LINE__);
			if (strcmp(conf->version, "1.050"))
				fprintf(ofp, "pkg6comp1 wrong version %s line %d\n", 
						conf->version, __LINE__);
		}
	}
	if (!cnt) fprintf(ofp, "can't find pkg6comp1 line %d\n", __LINE__);
	cnt = 0;
	while (conf = lc_next_conf(lm_job, "pkg6comp2", &pos))
	{
		cnt++;
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = conf;
		lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi);
		switch (cnt) 
		{
		case 2:
			if (fi.num_lic != 6) 
				fprintf(ofp, "Wrong pkg numlic (%d) line %d\n", 
					fi.num_lic, __LINE__);
			if (l_compare_version(lm_job, "2.0", conf->version))
				fprintf(ofp, 
				"pkg6comp2 wrong ver (%s) cnt %d line %d\n", 
						conf->version, cnt, __LINE__);
			break;
		case 1:
			if (fi.num_lic != 2) 
				fprintf(ofp, "Wrong pkg numlic line %d\n", __LINE__);
			if (l_compare_version(lm_job, "1.050", conf->version))
				fprintf(ofp, 
				"pkg6comp2 wrong ver (%s) cnt %d line %d\n", 
						conf->version, cnt, __LINE__);
			break;
		}
	}
	if (!cnt) fprintf(ofp, "can't find pkg6comp2 line %d\n", __LINE__);
/* 
 *	pkgs 7-9 are similar to 6
 *	pkg 10 
 */
	cnt = 0;
	pos = (CONFIG *)0;
	while (conf = lc_next_conf(lm_job, "pkg10comp1", &pos))
	{
		cnt++;
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = conf;
		lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi);
		if (cnt == 1)
		{
			if (fi.num_lic != 3) 
				fprintf(ofp, "Wrong pkg numlic line %d exp 3 got %d\n", __LINE__, fi.num_lic);
			if (l_compare_version(lm_job,conf->version, "1.000"))
				fprintf(ofp, "pkg10comp1 wrong version line %d\n", 
								__LINE__);
			break;
		}
		else if (cnt == 2)
		{
			if (fi.num_lic != 1) 
				fprintf(ofp, "Wrong pkg numlic (%d) line %d\n", 
					fi.num_lic, __LINE__);
			if (l_compare_version(lm_job,conf->version, "1.5"))
				fprintf(ofp, "pkg10comp1 wrong version %s line %d\n", 
						conf->version, __LINE__);
		}
		else 
			fprintf(ofp, "error line %d\n", __LINE__);
			
	}
	if (!cnt) fprintf(ofp, "can't find pkg6comp1 line %d\n", __LINE__);

	if (conf = lc_get_config(lm_job, "pkg13comp1"))
	{
		if (conf->lc_dup_group != LM_DUP_HOST) 
			fprintf(ofp, "dup_group error line %d\n", __LINE__);
		if (conf->lc_overdraft != 1) 
			fprintf(ofp, "overdraft error line %d\n", __LINE__);
		if (!conf->lc_vendor_info || strcmp(conf->lc_vendor_info, "vi"))
			fprintf(ofp, "vendor_info error line %d\n", __LINE__);
		if (strcmp(conf->lc_dist_info, "di"))
			fprintf(ofp, "dist_info error line %d\n", __LINE__);
		if (strcmp(conf->lc_user_info, "ui"))
			fprintf(ofp, "user_info error line %d\n", __LINE__);
		if (strcmp(conf->lc_asset_info, "ai"))
			fprintf(ofp, "asset_info error line %d\n", __LINE__);
		if (strcmp(conf->lc_issuer, "issuer"))
			fprintf(ofp, "issuer_info error line %d\n", __LINE__);
		if (strcmp(conf->lc_notice, "notice"))
			fprintf(ofp, "notice error line %d\n", __LINE__);
	} 
	else
		fprintf(ofp, "getconfig error line %d\n", __LINE__);
	lc_free_job(lm_job);

    }
/* 
 * -----------------------------------------------------------------------------
 *	33 - BADCODE tests
 *
 */
    if ((dotest < 0 || dotest == 33)|| (dotest < 33 && maxtest >=33) )
    {

     int rc;
	serv_log( "33 -  BADCODE tests\n");
	ts_lc_new_job( &code, &lm_job, __LINE__);
/*
 *	1. changed feature name
 */
	if ((rc = lm_checkout("e1", VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE)) !=
		LM_BADCODE && (rc != LM_NOSERVSUPP))
	{
		fprintf(ofp, "checkout didn't fail with NOFEATURE line %d\n", 
					__LINE__);
		if (lm_job->lm_errno) ts_error(lm_job, "checkout");
	}
/*
 *	2. changed version
 */

	/* This makes a local test only */
	if ((rc = lm_checkout("f1", "1.051", 1, LM_CO_NOWAIT, &code, LM_DUP_NONE)) 
		!= LM_OLDVER  && (rc != LM_SERVLONGGONE))
	{
		fprintf(ofp, "checkout didn't fail with NOFEATURE line %d\n", 
					__LINE__);
		if (lm_job->lm_errno) 
		{
		  char **cpp, **files;
			ts_error(lm_job, "\tcheckout");
			lc_get_attr(lm_job, LM_A_LF_LIST, (short *)&files);
			for (cpp = files; *cpp; cpp++)
				fprintf(ofp, "%s ", *cpp);
			fputs("", ofp);
		}

	}
/*
 *	3. changed exp-date ( there's no way to test this, since
 *		the code is already used )
 */
/*
 *	4. changed count ( there's no way to test this, since
 *		the code is already used, and it's another FEATURE line
 *		which is ignored anyway)
 */
/*
 *	5. bad enabling feature line for package 
 *	   server side first:
 */
	if (conf = lc_get_config(lm_job, "pkg11comp1"))
	{
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE;
		fi.feat = conf;
		if (!lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi))
		fprintf(ofp, "bad enabling feature comp in server %d\n", __LINE__);
/*
 *	client side
 */
		if (!lm_checkout("pkg11comp1", VER, 1, LM_CO_NOWAIT,&code,
							LM_DUP_NONE) 
			|| (lm_job->lm_errno != LM_BADCODE 
			&& lm_job->lm_errno != LM_NOSERVSUPP
			&& lm_job->lm_errno != LM_SIGN_REQ
			) ) 
		{
			fprintf(ofp, 
			"checkout succeeded, bad enabling feature line %d\n", 
								__LINE__);
			if (lm_job->lm_errno) ts_error(lm_job, "checkout");
		}
	}
/*
 *	6. bad package, good feature line for package 
 *	   uncounted, so can't query the server
 *	   client side:
 */
	conf = lc_get_config(lm_job, "pkg12comp1");
	if (!conf)
		fprintf(ofp, "can't find pkg12comp1 line %d\n", __LINE__);
	if (!lm_checkout("pkg12comp1", VER, 1, LM_CO_NOWAIT,&code,LM_DUP_NONE) 
		|| (lm_job->lm_errno != LM_BADCODE
		&& lm_job->lm_errno != LM_NOSERVSUPP 
		&& lm_job->lm_errno != LM_SIGN_REQ))
	{
		fprintf(ofp, "checkout succeeded, bad enabling feature line %d %s\n", 
					__LINE__, lc_errstring(lm_job));
		if (lm_job->lm_errno) ts_error(lm_job, "checkout");
	}
	lc_free_job(lm_job);

    }
/* 
 * -----------------------------------------------------------------------------
 *	34 - QUEUE tests
 *
 */
    if ((dotest < 0 || dotest == 34)|| (dotest < 34 && maxtest >=34) )
    {
      LM_HANDLE *h1, *h2;
	serv_log( "34 -  Queueing tests\n");

	ts_lc_new_job( &code, &h1, __LINE__);
	ts_lc_new_job( &code, &h2, __LINE__);

	if (lc_checkout(h1, "f1", VER, 7, LM_CO_QUEUE, &code, LM_DUP_NONE))
		fprintf(ofp, "Error line %d: %s\n", __LINE__, lc_errstring(h1));
	if (lc_checkout(h2, "f1", VER, 7, LM_CO_QUEUE, &code, LM_DUP_NONE)
		!= LM_FEATQUEUE)
		fprintf(ofp, "Error line %d: %s\n", __LINE__, lc_errstring(h1));
						
	lc_checkin(h1, "f1", 0);
	lc_sleep(h2, 1);
	if (lc_status(h2, "f1"))
	{
		fprintf(ofp, "Queueing Error line %d: %s\n", __LINE__, 
							lc_errstring(h2));
	}

	lc_free_job(h1);
	lc_free_job(h2);
    }
		
    if ((dotest < 0 || dotest == 36)|| (dotest < 36 && maxtest >=36) )
    {
/* 
 * -----------------------------------------------------------------------------
 *	36 UPGRADE
 */
	serv_log( "36 -- UPGRADE\n");
	{
/* 
 *		The license should have 4 pools, with 1@2.0 1@3.0 1@4.0
 *		versions
 */
		ts_lc_new_job( &code, &job[0], __LINE__); 
		ts_lc_new_job( &code, &job[1], __LINE__);
		ts_lc_new_job( &code, &job[2], __LINE__);
		if (lc_checkout(job[0], "f36", "3.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade failure line %d (%s)\n", __LINE__, 
				lc_errstring(job[0]));
		lc_checkin(job[0], "f36", 1);
		if (lc_checkout(job[0], "f36", "4.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade failure line %d %s\n", __LINE__, 
				lc_errstring(job[0]));
		lc_checkin(job[0], "f36", 1);
		if (!lc_checkout(job[0], "f36", "5.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade failure line %d\n", __LINE__);
		lc_checkin(job[0], "f36", 1);
		if (lc_checkout(job[0], "f36", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade failure line %d\n", __LINE__);
		if (lc_checkout(job[1], "f36", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade failure line %d\n", __LINE__);
		if (lc_checkout(job[2],"f36", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade failure line %d\n", __LINE__);
		lc_checkin(job[0], "f36", 1);
		lc_checkin(job[1], "f36", 1);
		lc_checkin(job[2], "f36", 1);
		if (!lc_checkout(job[0], "f36", "3.0", 3, 0, &code, 
								LM_DUP_NONE))
			fprintf(ofp, "Upgrade failure line %d\n", __LINE__);
		lc_checkin(job[0], "f36", 0);
		lc_free_job(job[0]); lc_free_job(job[1]); lc_free_job(job[2]);
	}
    }
	
/* 
 * -----------------------------------------------------------------------------
 *	37 - UPGRADE PACKAGE
 *
 */
    if ((dotest < 0 || dotest == 37)|| (dotest < 37 && maxtest >=37) )
    {

	serv_log( "37 - UPGRADE PACKAGE tests\n");
	fprintf(ofp, "\t37a - UPGRADE regular PACKAGE \n");

	ts_lc_new_job( &code, &job[0], __LINE__); lm_job = job[0];
	ts_lc_new_job( &code, &job[1], __LINE__); lm_job = job[1];
	if (lc_checkout(job[0], "pkg14comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	if (lc_checkout(job[1], "pkg14comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg14comp1", 0);
	lc_checkin(job[1], "pkg14comp1", 0);

	if (lc_checkout(job[0], "pkg14comp1", "2.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	if (!lc_checkout(job[1], "pkg14comp1", "2.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg14comp1", 0);
	lc_checkin(job[1], "pkg14comp1", 0);

	if (lc_checkout(job[0], "pkg14comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	if (lc_checkout(job[1], "pkg14comp1", "2.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg14comp1", 0);
	lc_checkin(job[1], "pkg14comp1", 0);
	lc_free_job(job[0]);
	lc_free_job(job[1]);

	serv_log( "\t37b - UPGRADE SUITE\n");
	ts_lc_new_job( &code, &job[0], __LINE__); lm_job = job[0];
	ts_lc_new_job( &code, &job[1], __LINE__); lm_job = job[1];
	if (lc_checkout(job[0], "pkg15comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	if (lc_checkout(job[1], "pkg15comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg15comp1", 0);
	lc_checkin(job[1], "pkg15comp1", 0);

	if (lc_checkout(job[0], "pkg15comp1", "2.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	if (!lc_checkout(job[1], "pkg15comp1", "2.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg15comp1", 0);
	lc_checkin(job[1], "pkg15comp1", 0);

	if (lc_checkout(job[0], "pkg15comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	if (lc_checkout(job[1], "pkg15comp1", "2.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg15comp1", 0);
	lc_checkin(job[1], "pkg15comp1", 0);
	lc_free_job(job[0]);
	lc_free_job(job[1]);

	serv_log( "\t37c - UPGRADE a COMPONENT\n");
	ts_lc_new_job( &code, &job[0], __LINE__); lm_job = job[0];
	ts_lc_new_job( &code, &job[1], __LINE__); lm_job = job[1];
	if (lc_checkout(job[0], "pkg16comp1", "2.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	if (!lc_checkout(job[1], "pkg16comp2", "2.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Upgrade pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg15comp1", 0);
	lc_checkin(job[1], "pkg16comp2", 0);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	serv_log( "\t37d - DUPLICATE PACKAGE LINES\n");
	ts_lc_new_job( &code, &job[0], __LINE__); lm_job = job[0];
	ts_lc_new_job( &code, &job[1], __LINE__); lm_job = job[1];
	if (lc_checkout(job[0], "pkg17comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Dup pkg failure line %d\n", __LINE__);
	if (!lc_checkout(job[1], "pkg17comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Dup pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg17comp1", 0);
	lc_checkin(job[1], "pkg17comp1", 0);

	if (lc_checkout(job[0], "pkg18comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Dup pkg failure line %d\n", __LINE__);
	if (!lc_checkout(job[1], "pkg18comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Dup pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg18comp1", 0);
	lc_checkin(job[1], "pkg18comp1", 0);
#if 0 /* have to write a test for pkg19 */
	if (lc_checkout(job[0], "pkg19comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "INCREMENT pkg failure line %d\n", __LINE__);
	if (!lc_checkout(job[1], "pkg19comp1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "INCREMENT pkg failure line %d\n", __LINE__);
	lc_checkin(job[0], "pkg19comp1", 0);
	lc_checkin(job[1], "pkg19comp1", 0);
#endif
	lc_free_job(job[0]);
	lc_free_job(job[1]);
    }
/* 
 * -----------------------------------------------------------------------------
 *	38 - +port@host test
 *
 */
    if (((dotest < 0 || dotest == 38) && file_based == 0)|| (dotest < 38 && maxtest >=38) )
    {

      char addr[100];
      char savfile[1024];
      char *savfilep;
      CONFIG *conf;
      CONFIG *pos;
      char **featlist;
      int cnt1, cnt2;
      LM_USERS *u, *up;
      FILE *fp;
      char **cpp;
      int gotone = 0;

	serv_log( "38 - +port@host test\n");

	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	/*sprintf(addr, "+%d@%s", PORT, hostname);*/
	sprintf(addr, "@%s", hostname);
	setenv("LM_LICENSE_FILE", addr);
	ts_lc_new_job( &code, &job[0], __LINE__); 
	if (!(featlist = featlist_test(job[0], __LINE__)))
		fprintf(ofp, "featlist failed %d:%s\n", __LINE__,
				lc_errstring(job[0]));
	for (; featlist && *featlist; featlist++)
	{
		pos = 0;
		while (conf = lc_next_conf(job[0], *featlist, &pos))
		{
			if (!L_STREQ(conf->feature, *featlist))
				fprintf(ofp, "nextconf failed %d %s:%s\n", __LINE__,
					*featlist, lc_errstring(job[0]));
		}
			gotone ++;
	}
	if (!gotone)
		fprintf(ofp, "next_conf failed %d:%s\n", __LINE__,
			lc_errstring(job[0]));
	pos = 0;
	if (conf = lc_next_conf(job[0], "no_such_feat", &pos))
		fprintf(ofp, "next_conf succeeded no_such_feat %d\n", __LINE__);

	if (lc_checkout(job[0], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "+port@host failed line %d: %s\n", __LINE__,
						lc_errstring(job[0]));
	else
		lc_checkin(job[0], "f1", 0);
	if (lc_checkout(job[0], "f2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "+port@host failed line %d: %s\n", __LINE__,
						lc_errstring(job[0]));
	else
	{
		featlist_test(job[0], __LINE__);
		conf = lc_auth_data(job[0], "f2");
		if (!conf || (!conf->lc_notice) || 
					(strlen(conf->lc_notice) < 200))
			fprintf(ofp, "auth_data on long license failing %d %s\n",
					__LINE__, lc_errstring(job[0]));
		lc_checkin(job[0], "f1", 0);
	}

	lc_set_errno(job[0], 0);
	cnt1 = 0;
	pos = (CONFIG *)0;

	while (conf = lc_next_conf(job[0], "f36", &pos))
	{
		cnt1 ++;
	}

	lc_free_job(job[0]);
	if (savfile)
		setenv("LM_LICENSE_FILE", savfile);
	ts_lc_new_job( &code, &job[0], __LINE__); 
	cnt2 = 0;
	pos = (CONFIG *)0;
	while (conf = lc_next_conf(job[0], "f36", &pos))
	{
		cnt2 ++;
	}
	if (cnt1 != cnt2) 
		fprintf(ofp, "next_conf +port@host error (%d!= %d), %d\n", 
			cnt1, cnt2, __LINE__);
	lc_free_job(job[0]);

	setenv("LM_LICENSE_FILE", addr);
	ts_lc_new_job( &code, &job[0], __LINE__); 
#ifdef PC16
	{
	  FARPROC lpfn;
		lpfn  = MakeProcInstance((FARPROC) checkout_filter,  _hInstance);
		lc_set_attr(job[0], LM_A_CHECKOUTFILTER, 
					(LM_A_VAL_TYPE) lpfn);
	}
#else /* PC16 */
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, 
					(LM_A_VAL_TYPE) checkout_filter);
#endif /* PC16 */
	if (!lc_checkout(job[0], "f2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "checkout_filter failed line %d: %s\n", __LINE__,
						lc_errstring(job[0]));
	if (lc_get_errno(job[0]) != LM_LOCALFILTER)
		fprintf(ofp, 
		"checkout_filter gives wrong errno line %d: exp %d, got %d\n", 
				__LINE__, LM_LOCALFILTER, lc_get_errno(job[0]));
	if (lc_checkout(job[0], "node-locked-counted", "1.0", 1, 0, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "checkout_filter failed line %d: %s\n", __LINE__,
						lc_errstring(job[0]));
	lc_free_job(job[0]);
	ts_lc_new_job( &code, &job[0], __LINE__); 
	ts_lc_new_job( &code, &job[1], __LINE__); 
/*
 *	Bug P1190
 */
	if (lc_checkout(job[0], "f2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "checkout_filter failed line %d: %s\n", __LINE__,
						lc_errstring(job[0]));
	for (cnt1 = 0, u = lc_userlist(job[1], "f2"); u; u = u->next)
		cnt1++;
	if (cnt1 != 2) fprintf(ofp, "userlist failed line %d exp %d got %d\n",
		__LINE__, 2, cnt1);
	for (cnt1 = 0, u = lc_userlist(job[1], "f2"); u; u = u->next)
		cnt1++;
	if (cnt1 != 2) fprintf(ofp, "userlist failed line %d exp %d got %d\n",
		__LINE__, 2, cnt1);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
/*
 *	P1981
 */
	ts_lc_new_job( &code, &job[0], __LINE__); 
	strcpy(p1981_serial, "1");
#ifdef PC16
	{
	  FARPROC lpfn;
		lpfn  = MakeProcInstance((FARPROC) checkout_filter2,  _hInstance);
		lc_set_attr(job[0], LM_A_CHECKOUTFILTER, 
					(LM_A_VAL_TYPE) lpfn);
	}
#else /* PC16 */
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER,
					(LM_A_VAL_TYPE) checkout_filter2);
#endif /* PC16 */
	if (lc_checkout(job[0], "p1981", "1.0", 1, LM_CO_LOCALTEST, &code, 
		LM_DUP_NONE))
		fprintf(ofp, "checkout_filter2 failed line %d: %s\n", __LINE__,
			lc_errstring(job[0]));
	if (lc_checkout(job[0], "p1981_2", "1.0", 1, LM_CO_LOCALTEST, &code, 
		LM_DUP_NONE))
		fprintf(ofp, "checkout_filter2 failed line %d: %s\n", __LINE__,
			lc_errstring(job[0]));
	strcpy(p1981_serial, "2");
	if (lc_checkout(job[0], "p1981", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "checkout_filter2 failed line %d: %s\n", __LINE__,
			lc_errstring(job[0]));
	lc_free_job(job[0]);

	/* P2122 */

	if (!(fp = fopen("featlist.dat", "w")))
		perror("Can't open featlist.dat");
	fprintf(fp, "SERVER %s %s\nUSE_SERVER\n", hostname, hostid);
	fclose(fp);
	setenv("LM_LICENSE_FILE", "featlist.dat");
	ts_lc_new_job( &code, &job[0], __LINE__); 
	featlist_test(job[0], __LINE__);
	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
		LM_DUP_NONE))
		fprintf(ofp, "failed line %d: %s\n", __LINE__, lc_errstring(job[0]));
	featlist_test(job[0], __LINE__);
	unlink ("featlist.dat");
		
	lc_free_job(job[0]);
	setenv("LM_LICENSE_FILE", savfile);
    }
/* 
 * -----------------------------------------------------------------------------
 *	39 - vendor-defined hostid
 *
 */
    if ((dotest < 0 || dotest == 39)|| (dotest < 39 && maxtest >=39) )
    {

	HOSTID *tmphostid;
	serv_log( "39 - vendor-defined hostid\n");
	fprintf(ofp, "\tIgnore the following messages: \n");
	fprintf(ofp, "\t\tLicense recovered\n");
	fprintf(ofp, "\t\tMissing security device\n");
#ifdef PC
	l_putenv("LM_PC_DEBUG_VDHOSTID=1");

#endif /* PC */
	ts_lc_new_job( &code, &job[0], __LINE__); 
	lm_job = job[0];
	x_flexlm_newid();
	tmphostid = lc_getid_type(job[0], HOSTID_VENDOR + 3);
	tmphostid = lc_getid_type(job[0], HOSTID_VENDOR + 2);
	tmphostid = lc_getid_type(job[0], HOSTID_VENDOR + 3);

	if (lc_checkout(job[0], "vdhostid", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "vendor-defined hostid failed line %d: %s\n", __LINE__,
						lc_errstring(job[0]));
	if (lc_checkout(job[0], "vdhostid3", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "vendor-defined hostid failed line %d: %s\n", __LINE__,
						lc_errstring(job[0]));
	if (lc_checkout(job[0], "vdhostid3_1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "vendor-defined hostid failed line %d: %s\n", __LINE__,
						lc_errstring(job[0]));
	
/*
 *	Test removing dongle
 */
	{
	  FILE *fp;
		if (!(fp = fopen("hostid.txt", "w")))
			fprintf(ofp, "Can't open file_hostid, line %d", __LINE__);
		fprintf(fp, "FILE_HOSTID_VAL\n");
		fclose(fp);
	}
	test_dongles(job, "vdhostid5");
	test_dongles(job, "vdhostid4");
/*
 *	Check invalid vendor-defined hostid 
 */
	if (!lc_checkout(job[0], "badvdhostid", "1.0", 1, 0, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "vendor-defined hostid shd have failed line %d: %s\n", 
					__LINE__, lc_errstring(job[0]));
	lc_free_job(job[0]);
    }
/* 
 * -----------------------------------------------------------------------------
 *	40 - v5 attributes
 *
 */
    if ((dotest < 0 || dotest == 40)|| (dotest < 40 && maxtest >=40) )
    {
      CONFIG *conf, *pos = (CONFIG *)0;
      int cnt = 0;
      char savfile[1024];
      char *savfilep;
      char addr[100];

	serv_log( "40 - v5 attributes\n");
        setenv("LM_PROJECT", "project_test");

	ts_lc_new_job( &code, &job[0], __LINE__); 
	lm_job = job[0];
	lc_set_attr(job[0], LM_A_CAPACITY, (LM_A_VAL_TYPE)2);
/*
 *	SUPERSEDE and ISSUED
 */	
	while (conf = lc_next_conf(job[0], "f40", &pos))
	{
		cnt += conf->users;
	}
	if (cnt != 4)
		fprintf(ofp, "Error, SUPERSEDE failed line %d expected %d, got %d\n", 
			__LINE__, 4, cnt);
	if (lc_checkout(job[0], "f40-a", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "f40-a failed line %d: %s\n", __LINE__, 
						lc_errstring(job[0]));
	if (get_feat_info("f40-a", __LINE__, &fi, job[0]))
	{
		if (fi.tot_lic_in_use != 2)
			fprintf(ofp, "CAPACITY not working, line %d, exp %d got %d\n",
				__LINE__, 2, fi.tot_lic_in_use);
	}
	ts_lc_new_job( &code, &job[1], __LINE__); 
	if (lc_checkout(job[1], "internet-hostid", "1.0", 1, 0, &code, 
			LM_DUP_NONE))
		fprintf(ofp, "internet-hostid failed line %d: %s\n", __LINE__, 
			lc_errstring(job[1]));
	/* USER_BASED */
	if (lc_checkout(job[1], "user_based", "1.0", 1, 0, &code, 
			LM_DUP_NONE))
		fprintf(ofp, "user_based failed line %d: %s\n", __LINE__, 
			lc_errstring(job[1]));
	/* P4949 */
	if (lc_checkout(job[1], "bad_user_based", "1.0", 1, 0, &code,  
			LM_DUP_NONE) != LM_USER_BASED)
		fprintf(ofp, "user_based should have failed line %d\n", 
			__LINE__);
/*
 *	PLATFORMS
 */	
	if (!lc_checkout(job[1], "platform", "1.0", 1, 0, &code, 
			LM_DUP_NONE) || (lc_get_errno(job[1]) != LM_PLATNOTLIC))
		fprintf(ofp, "platform test failed line %d. exp %d, got %d\n", 
			__LINE__, lc_get_errno(job[1]));
	if (lc_checkout(job[1], "good_plat", "1.0", 1, 0, &code, LM_DUP_NONE) )
		fprintf(ofp, "platform test failed line %d\n", __LINE__);
	lc_checkin(job[1], "good_plat", 1);
/*
 *	Same platform test with port@host -- make sure plat is validated
 *	on server side also...
 */
	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	/*sprintf(addr, "+%d@%s", PORT, hostname);*/
	sprintf(addr, "@%s", hostname);
	setenv("LM_LICENSE_FILE", addr);
	ts_lc_new_job( &code, &job[2], __LINE__); 
	if (!lc_checkout(job[2], "platform", "1.0", 1, 0, &code, 
			LM_DUP_NONE) || (lc_get_errno(job[2]) != LM_PLATNOTLIC))
		fprintf(ofp, "platform test failed line %d. exp %d, got %d\n", 
			__LINE__, LM_PLATNOTLIC, lc_get_errno(job[2]));
	if (lc_checkout(job[1], "good_plat", "1.0", 1, 0, &code, LM_DUP_NONE) )
		fprintf(ofp, "platform test failed line %d\n", __LINE__);
	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	/*sprintf(addr, "+%d@%s", PORT, hostname);*/
	sprintf(addr, "@%s", hostname);
	setenv("LM_LICENSE_FILE", savfile);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
    }
/* 
 * -----------------------------------------------------------------------------
 *	41 - New APIs 
 *
 */
    if ((dotest < 0 || dotest == 41)|| (dotest < 41 && maxtest >=41) )
    {
      LP_HANDLE *lp_handle1;
      LP_HANDLE *lp_handle2;
      LP_HANDLE *lp_handle3;
      LP_HANDLE *lp_handle4;
      LP_HANDLE *lp_handle5;
      CONFIG *conf, *pos = (CONFIG *)0;
      int cnt = 0;

	serv_log( "41 - Simple and Trivial APIs\n");
        setenv(LM_DEFAULT_ENV_SPEC, "");
	if (CHECKOUT(LM_RESTRICTIVE, "f2", "1.0", MAIN_LICENSE))
	{
		fprintf(ofp, "err LINE %d %s\n", __LINE__, ERRSTRING());
	}
	CHECKIN();
	if (lp_checkout(LPCODE, LM_RESTRICTIVE, "f2", "1.0", 1, 
					MAIN_LICENSE, &lp_handle1))
	{
		fprintf(ofp, "err LINE %d %s\n", __LINE__, lp_errstring(lp_handle1));
	}
	if (lp_checkout(LPCODE, LM_RESTRICTIVE, "f2", "1.0", 1, 
					MAIN_LICENSE, &lp_handle2))
	{
		fprintf(ofp, "err LINE %d %s\n", __LINE__, lp_errstring(lp_handle2));
	}
	if (lp_checkout(LPCODE, LM_RESTRICTIVE, "f2", "1.0", 9, 
				MAIN_LICENSE, &lp_handle3) != LM_MAXUSERS)
	{
		fprintf(ofp, "err LINE %d\n", __LINE__);
		lp_checkin(lp_handle3);
		lp_handle3 = NULL;
	}
	lp_checkin(lp_handle1);
	lp_checkin(lp_handle2);
	lp_checkin(lp_handle3);
/*
 *	Test policies
 */
	if (lp_checkout(LPCODE, LM_RESTRICTIVE, "f28", "1.0", 1, 
					MAIN_LICENSE, &lp_handle1))
	{
		fprintf(ofp, "err LINE %d %s\n", __LINE__, lp_errstring(lp_handle1));
	}
	if (lp_checkout(LPCODE, LM_RESTRICTIVE, "f28", "1.0", 1, 
				MAIN_LICENSE, &lp_handle2) != LM_MAXUSERS)
	{
		fprintf(ofp, "err LINE %d %s\n", __LINE__, lp_errstring(lp_handle2));
	}
	if (lp_checkout(LPCODE, LM_LENIENT, "f28", "1.0", 1, 
				MAIN_LICENSE, &lp_handle3) != LM_MAXUSERS)
	{
		fprintf(ofp, "err LINE %d %s\n", __LINE__, lp_errstring(lp_handle3));
	}
	if (lp_checkout(LPCODE, LM_FAILSAFE, "no_such_feature", 
		"1.0", 1, MAIN_LICENSE, &lp_handle4) )
	{
		fprintf(ofp, "err LINE %d %s\n", __LINE__, lp_errstring(lp_handle4));
	}
	if (lp_checkout(LPCODE, LM_LENIENT, "no_such_feature", 
		"1.0", 1, MAIN_LICENSE, &lp_handle5) )
	{
		fprintf(ofp, "err LINE %d %s\n", __LINE__, lp_errstring(lp_handle5));
	}
	lp_checkin(lp_handle1);
	lp_checkin(lp_handle2);
	lp_checkin(lp_handle3);
	lp_checkin(lp_handle4);
	lp_checkin(lp_handle5);
	
        setenv(LM_DEFAULT_ENV_SPEC, license_file);

    }
/* 
 * -----------------------------------------------------------------------------
 *	42 - Bug tests -- source in bugtest.c
 *
 */
    if ((dotest < 0 || dotest == 42)|| (dotest < 42 && maxtest >=42) || (bugnumber > 0))
    {
	serv_log( "42 -- Bug tests\n");

	bugtests();
    }
/* 
 * -----------------------------------------------------------------------------
 *	43 - v5.1 attributes
 *
 */
    if ((dotest < 0 || dotest == 43)|| (dotest < 43 && maxtest >=43))
    {
      CONFIG *c;
      char hostid_buf[MAX_CONFIG_LINE];
      LM_HANDLE *sav_job = lm_job;
      char savfilep[200];


	serv_log( "43 -- v5.1 attributes \n");

	ts_lc_new_job( &code, &job[0], __LINE__); 
	ts_lc_new_job( &code, &job[1], __LINE__); 
	ts_lc_new_job( &code, &job[2], __LINE__); 
	ts_lc_new_job( &code, &job[3], __LINE__); 
	lm_job = job[3];
	x_flexlm_newid();
	lm_job = job[0];
	x_flexlm_newid();
	lm_job = sav_job;
#if 1
	if (c = lc_get_config(job[0], "supersede1"))
		fprintf(ofp, "supersede failed line %d\n", __LINE__);
	if (c = lc_get_config(job[0], "supersede2"))
		fprintf(ofp, "supersede failed line %d\n", __LINE__);
	if (!(c = lc_get_config(job[0], "supersede3")))
		fprintf(ofp, "supersede failed line %d\n", __LINE__);
	fprintf(ofp, "Hostids: \n");
	if (lc_hostid(job[0], HOSTID_DEFAULT, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
	if (lc_hostid(job[0], HOSTID_USER, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
	if (lc_hostid(job[0], HOSTID_HOSTNAME, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
	if (lc_hostid(job[0], HOSTID_DISPLAY, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
	if (lc_hostid(job[0], HOSTID_INTERNET, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
	if (lc_hostid(job[0], HOSTID_VENDOR + 1, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
	if (strcmp(hostid_buf, "EXAMPLE_HOSTID=12345678901234567890123456"))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__);
	if (lc_hostid(job[0], HOSTID_VENDOR + 2, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
	if (strcmp(hostid_buf, "EXAMPLE_HOSTID2=VENDORID2"))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__);
	if (lc_hostid(job[0], HOSTID_VENDOR + 3, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
	if (strcmp(hostid_buf, "\"EXAMPLE_HOSTID3=ID3 EXAMPLE_HOSTID3=ID3_1\""))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__);
	if (lc_hostid(job[0], HOSTID_VENDOR + 4, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
	if (strcmp(hostid_buf, "FILE_HOSTID=FILE_HOSTID_VAL"))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__,
			lc_errstring(job[0]));
#ifdef PC
	if (lc_hostid(job[0], HOSTID_ETHER, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s", hostid_buf);
        l_putenv("LM_USE_SNMP=1");
	if (lc_hostid(job[0], HOSTID_ETHER, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s", hostid_buf);
	unsetenv("LM_USE_SNMP");
        l_putenv("LM_NO_NETBIOS=1");
	if (lc_hostid(job[0], HOSTID_ETHER, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s", hostid_buf);
	unsetenv("LM_NO_NETBIOS");
	if (lc_hostid(job[0], HOSTID_DISK_SERIAL_NUM, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s", hostid_buf);
	if (lc_hostid(job[0], HOSTID_FLEXID1_KEY, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s", hostid_buf);
	if (lc_hostid(job[0], HOSTID_FLEXID2_KEY, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s", hostid_buf);
	if (lc_hostid(job[0], HOSTID_FLEXID5_KEY, hostid_buf))
		fprintf(ofp, "lc_hostid failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	else fprintf(ofp, "\t%s\n", hostid_buf);
#endif

	if (lc_checkout(job[0], "max", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "MAX test failed line %d\n", __LINE__);
	if (!lc_checkout(job[1], "max", "1.0", 1, 0, &code, LM_DUP_NONE) 
				|| (lc_get_errno(job[1]) != LM_MAXLIMIT))
		fprintf(ofp, "MAX test failed line %d. exp %d, got %d/%d\n", 
				__LINE__, LM_MAXLIMIT, lc_get_errno(job[1]), 
						job[1]->errno_minor);
	if (lc_checkout(job[0], "ser_num", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "ID= test failed line %d\n", __LINE__);
	else
	{
	  CONFIG *c = lc_auth_data(job[0], "ser_num");
		if (!c->idptr || c->idptr->type != HOSTID_SERNUM_ID ||
				strcmp("223-456-789-0", c->idptr->id.string))
			fprintf(ofp, "ID= test failed line %d expected %s, got %s\n",
					       __LINE__, "223-456-789-0",  
					       l_asc_hostid(job[0], c->idptr));

	}
	if (get_feat_info("ser_num", __LINE__, &fi, job[0]) && 
					(fi.tot_lic_in_use != 1))
		fprintf(ofp,  "error line %d, exp %d got %d\n",
						__LINE__, 1, fi.tot_lic_in_use);
	lc_checkin(job[0], LM_CI_ALL_FEATURES, 1);
	if (get_feat_info("ser_num", __LINE__, &fi, job[0]) &&
						(fi.tot_lic_in_use != 0))
		fprintf(ofp, 
		"LM_CI_ALL_FEATURES not working, line %d, exp %d got %d\n",
						__LINE__, 0, fi.tot_lic_in_use);

#endif

/*
 *	Test LM_A_TCP_TIMEOUT
 */
	lc_set_attr(job[2], LM_A_TCP_TIMEOUT, (LM_A_VAL_TYPE) 1);
	lc_set_attr(job[2], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE) 0);
	if (lc_checkout(job[2], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "TCP_TIMEOUT test failed line %d\n", __LINE__);
	if (get_feat_info("f1", __LINE__, &fi, job[2]) &&
						(fi.tot_lic_in_use != 1))
		fprintf(ofp, 
		"TCP_TIMEOUT not working, line %d, exp %d got %d\n",
						__LINE__, 1, fi.tot_lic_in_use);
	l_select_one(0, -1, 2000);
	ts_flush_server(job[0], __LINE__);
	ts_flush_server(job[0], __LINE__);
	if (get_feat_info_err("f1", __LINE__, &fi, job[0], 0, 0) &&
						(fi.tot_lic_in_use != 0))
		fprintf(ofp, 
		"TCP_TIMEOUT not working, line %d, exp %d got %d\n",
						__LINE__, 0, fi.tot_lic_in_use);
/*
 *	Test reserve package names
 */
	if (lc_checkout(job[0], "euop1c1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "reserve package name test failed line %d\n", __LINE__);

	if (!lc_checkout(job[0], "euop2c2", "1.0", 1, 0, &code, LM_DUP_NONE) ||
		job[0]->lm_errno != LM_RESVFOROTHERS)
		fprintf(ofp, 
		"reserve package name test failed line %d exp %d, got %d\n", 
		__LINE__, LM_RESVFOROTHERS, job[0]->lm_errno);
		

/* 
 *	test l_baddate 
 *	1) FLEXLM_DEBUG_BADDATE tells l_baddate() to always fail
 *	2) Only test uncounted licenses, since we can't easily pass this 
 *	   env var to the server.
 */
#ifndef VMS
        setenv("FLEXLM_DEBUG_BADDATE", "1");
	lc_set_attr(job[3], LM_A_CHECK_BADDATE, (LM_A_VAL_TYPE)1);
	fprintf(ofp, "\t(Expect l_baddate failure with USE_SERVER or port@host)\n");
	if (!lc_checkout(job[3], "f4", "1.0", 1, 0, &code, LM_DUP_NONE) ||
		job[3]->lm_errno != LM_BADSYSDATE)
		fprintf(ofp, "l_baddate test failed line %d exp %d, got %d\n", 
			__LINE__, LM_BADSYSDATE, job[3]->lm_errno);
	if (lc_checkout(job[3], "vdhostid5", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "l_baddate test failed line %d %s\n", __LINE__,
			lc_errstring(job[3]));
        unsetenv("FLEXLM_DEBUG_BADDATE");
#endif /* VMS */

/*
 *	lc_expire_days
 */
	c = lc_get_config(job[0], "f0");
	if (!c || lc_expire_days(job[0], c) < 360)
		fprintf(ofp, "lc_expire_days failed %d expected > 360, got %d \n", 
			__LINE__, c?lc_expire_days(job[0],c) : -1);
	c = lc_get_config(job[0], "f40");
	if (!c || lc_expire_days(job[0], c) != LM_FOREVER)
		fprintf(ofp, "lc_expire_days failed %d expected %d, got %d \n", 
			__LINE__, LM_FOREVER,  c?lc_expire_days(job[0],c) : -1);
		

	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
	lc_free_job(job[3]);

#ifndef VMS
	/* should fail */

	strcpy(savfilep, getenv("LM_LICENSE_FILE"));
	setenv ("LM_LICENSE_FILE", "nofile.dat");

	if (!(CHECKOUT(LM_RESTRICTIVE, "lic_string", "1.0", MAIN_LICENSE)))
		fprintf(ofp, "lic_string succeeded, error line %d \n", __LINE__);
	CHECKIN();

	/* should succeed, with license in string */

	{
	  char buf[1024];
		sprintf(buf, "START_LICENSE\nFEATURE lic_string demo 1.0 \
			permanent uncounted 2CC6523BF4D1 \
		        HOSTID=ANY SIGN2=\"0053 90DB AEE6 B4EE B9D1 03D5 \
			3B61 9800 87F0 46F1 319F 8334 FB71 D5C8 F7C3\"\
			\nEND_LICENSE%c.", 
#ifdef PC
		';'
#else
		':'
#endif
		);
		if (CHECKOUT(LM_RESTRICTIVE, "lic_string", "1.0", buf))
			fprintf(ofp, "lic_string failed, error line %d %s\n", __LINE__,
			ERRSTRING());
	}
	CHECKIN();

	/* should succeed, using file, AFTER string */

#ifdef PC
	if (CHECKOUT(LM_RESTRICTIVE, "f1", "1.0", 
		"START_LICENSE\nFEATURE lic_string demo 1.0 permanent uncounted E5DEABDD3A2F HOSTID=ANY\nEND_LICENSE;."))
#else
        if (CHECKOUT(LM_RESTRICTIVE, "f1", "1.0",
                "START_LICENSE\nFEATURE lic_string demo 1.0 permanent uncounted E5DEABDD3A2F HOSTID=ANY\nEND_LICENSE:."))

#endif
		fprintf(ofp, "f1 failed, error line %d %s\n", __LINE__,
			ERRSTRING());
	CHECKIN();
	setenv ("LM_LICENSE_FILE", savfilep);
#endif /* VMS */

    }
#ifndef VMS
    if ((dotest < 0 || dotest == 44)|| (dotest < 44 && maxtest >=44))
    {
      CONFIG *c;
      char hostid_buf[MAX_CONFIG_LINE];
      LM_HANDLE *sav_job = lm_job;
      char *savfilep;
      char savfile[1024];
      char **path;
      char *f = "pathtest1";
      char twopath2[LM_MAXPATHLEN+1], twopath3[LM_MAXPATHLEN+1], twopath4[LM_MAXPATHLEN+1];
      char TWOPATH2[100];
      char TWOPATH3[100];
      char TWOPATH4[100];

#ifdef PC
        sprintf(TWOPATH2, "@%%s;..\\testsuite\\pathtest.dat");
        sprintf(TWOPATH3, ".;%d@%%s", LMGRD_PORT_START +2);
        sprintf(TWOPATH4, "@%%s;%d@%%s", LMGRD_PORT_START + 2);
#else

	sprintf(TWOPATH2, "@%%s:../testsuite/pathtest.dat");
	sprintf(TWOPATH3, ".:%d@%%s", LMGRD_PORT_START + 2);
	sprintf(TWOPATH4, "@%%s:%d@%%s", LMGRD_PORT_START + 2);
#endif
	sprintf(twopath2, TWOPATH2, hostname);
	sprintf(twopath3, TWOPATH3, hostname);
	sprintf(twopath4, TWOPATH4, hostname, hostname);
	savfilep = getenv("LM_LICENSE_FILE");

	strcpy(savfile, savfilep);

	serv_log( "44 -- Path Tests\n");

	for (path = path_list; *path; path++)
	{
		fprintf(ofp, "  PATH:%s\n", *path);
		setenv(LM_DEFAULT_ENV_SPEC, *path);
		ts_lc_new_job( &code, &job[0], __LINE__); 
		ts_lc_new_job( &code, &job[1], __LINE__); 
		ts_lc_new_job( &code, &job[2], __LINE__); 
/*
 *	checkout both from 1st license file.
 */
		if (lc_checkout(job[0], f, "1.0", 2, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "pathtest test failed line %d %s\n", __LINE__,
				lc_errstring(job[0]));
		if (lc_checkout(job[1], f, "1.0", 2, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "pathtest test failed line %d %s\n", __LINE__,
				lc_errstring(job[1]));
		if (!lc_checkout(job[2], f, "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "pathtest test failed line %d exp 0 got %d\n", 
				__LINE__, lc_get_errno(job[2]));
		lc_checkin(job[0], LM_CI_ALL_FEATURES, 0);
		lc_checkin(job[1], LM_CI_ALL_FEATURES, 0);
		lc_checkin(job[2], LM_CI_ALL_FEATURES, 0);
		lc_free_job(job[0]);
		lc_free_job(job[1]);
		lc_free_job(job[2]);
	}
/*
 * 
 *	-----------------------------------------------------
 */
	setenv(LM_DEFAULT_ENV_SPEC, twopath2);
	ts_lc_new_job( &code, &job[0], __LINE__); 
	fprintf(ofp, "  PATH:%s\n", twopath2);
        if (lc_checkout(job[0], f, "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "pathtest test failed line %d exp 0 got %d\n", 
				__LINE__, lc_get_errno(job[0]));
        lc_checkin(job[0], f, 1);
        if (lc_checkout(job[0], "pathtest2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "can't switch servers, line %d exp 0 got %d\n", 
				__LINE__, lc_get_errno(job[0]));
	lc_free_job(job[0]);

	setenv(LM_DEFAULT_ENV_SPEC, twopath3);
	fprintf(ofp, "  PATH:%s\n", twopath3);
	ts_lc_new_job( &code, &job[0], __LINE__); 
/*
 *	pathtest2 only in 2nd file
 */
        if (lc_checkout(job[0], "pathtest2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "pathtest test failed line %d exp 0 got %s\n", __LINE__, 
				lc_errstring(job[0]));
        lc_checkin(job[0], "pathtest2", 1);
/*
 *	switch to uncounted feature only in 1st file
 */
        if (lc_checkout(job[0], "f0", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "pathtest test failed line %d exp 0 got %d\n", 
			__LINE__, lc_get_errno(job[0]));
        lc_checkin(job[0], "pathtest2", 1);
	lc_free_job(job[0]);
/*
 *	-----------------------------------------------------
 * 
 */
	setenv(LM_DEFAULT_ENV_SPEC, twopath4);
	fprintf(ofp, "  PATH:%s\n", twopath4);
	ts_lc_new_job( &code, &job[0], __LINE__); 
/*
 *	locked2 is uncounted only in 2nd file
 */
        if (lc_checkout(job[0], "locked2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "pathtest test failed line %d exp 0 got %d\n", 
				__LINE__, lc_get_errno(job[0]));
        lc_checkin(job[0], "locked2", 1);
        if (lc_checkout(job[0], f, "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "pathtest test failed line %d exp 0 got %d\n", 
				__LINE__, lc_get_errno(job[0]));
	lc_free_job(job[0]);
/*
 *	-----------------------------------------------------
 * 
 */
	setenv(LM_DEFAULT_ENV_SPEC, TWOPATH);
	fprintf(ofp, "  PATH:%s\n", TWOPATH);
	ts_lc_new_job( &code, &job[0], __LINE__); 
/* 
 *	f1 is only in first file
 */
        if (lc_checkout(job[0], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "pathtest test failed line %d exp 0 got %d\n", 
				__LINE__, lc_get_errno(job[0]));
        lc_checkin(job[0], "f1", 1);
/* 
 *	pathtest2 is only in second file -- have to switch daemons
 */
        if (lc_checkout(job[0], "pathtest2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "pathtest test failed line %d exp 0 got %d\n", 
				__LINE__, lc_get_errno(job[0]));
	lc_free_job(job[0]);

/*
 *	-----------------------------------------------------
 * 	Make sure one job can't switch servers
 *	-----------------------------------------------------
 */
	setenv(LM_DEFAULT_ENV_SPEC, TWOPATH);
	fprintf(ofp, "  PATH:%s\n", TWOPATH);
	ts_lc_new_job( &code, &job[0], __LINE__); 
/* 
 *	f1 is only in first file
 */
        if (lc_checkout(job[0], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "pathtest test failed line %d exp 0 got %d\n", 
				__LINE__, lc_get_errno(job[0]));
/* 
 *	pathtest2 is only in second file -- have to switch daemons
 */
        if ((rc = lc_checkout(job[0], "pathtest2", "1.0", 1, 0, &code, 
		LM_DUP_NONE))
					!= LM_NOSERVSUPP && rc != LM_NOFEATURE)
		fprintf(ofp, "pathtest test failed line %d exp %d got %d\n", __LINE__,
			LM_NOSERVSUPP, lc_get_errno(job[0]));
	lc_free_job(job[0]);


        setenv(LM_DEFAULT_ENV_SPEC, savfile);
    }

#endif /* VMS */
/* 
 * -----------------------------------------------------------------------------
 *	45 - v6 attributes
 *
 */
    if ((dotest < 0 || dotest == 45)|| (dotest < 45 && maxtest >=45))
    {
	test45();
    }
/* 
 * -----------------------------------------------------------------------------
 *	46 - v6.1 attributes
 *
 */
    if ((dotest < 0 || dotest == 46)|| (dotest < 46 && maxtest >=46))
    {
	test46();
    }
/* 
 * -----------------------------------------------------------------------------
 *	47 - v7 attributes
 *
 */
    if ((dotest < 0 || dotest == 47)|| (dotest < 47 && maxtest >=47))
    {
	test47();
    }
/* 
 * -----------------------------------------------------------------------------
 *	48 - environent, registry, and license installation
 *
 */
    if ((dotest < 0 || dotest == 48)|| (dotest < 48 && maxtest >=48))
    {
	test48();
    }
/* 
 * -----------------------------------------------------------------------------
 *	49 - v7.1 security
 *
 */
    if ((dotest < 0 || dotest == 49)|| (dotest < 49 && maxtest >=49))
    {
	test49();
    }
    if ((dotest < 0 || dotest == 50)|| (dotest < 50 && maxtest >=50))
    {
	test_8bit(0);
    }
    if ((dotest < 0 || dotest == 51)|| (dotest < 51 && maxtest >=51))
    {
	test_8bit(1);
    }
    if ((dotest < 0 || dotest == 52)|| (dotest < 52 && maxtest >=52))
    {
	test52(subtest);
    }
    if ((dotest < 0 || dotest == 53)|| (dotest < 53 && maxtest >=53))
    {
	test53();
    }

    if (((dotest < 0 || dotest == 97) && !quick)|| (dotest < 97 && maxtest >=97) )
    {
/*
 * -----------------------------------------------------------------------------
 *	97	multi-spawn
 */
#ifdef SGI4
#define MULTI_CNT 5
#else
#ifdef PC32
#define MULTI_CNT 80
#else
#define MULTI_CNT 25
#endif /* PC32 */
#endif /* SGI4 */

      int i;
      LM_HANDLE *h[MULTI_CNT];
      int multi_cnt = MULTI_CNT;

#ifdef WINNT
        if (win95) multi_cnt = 30; /* most it can handle */
#endif

	serv_log( "97 concurrent users test\n");
	fprintf(ofp, "\t%d users\n", MULTI_CNT);
	for (i = 0; i < multi_cnt; i++)
	{
		ts_lc_new_job( &code, &h[i], __LINE__);
		lc_set_attr(h[i], LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	}
	for (i = 0; i < multi_cnt; i++)
	{
		if (lc_checkout(h[i], "f97", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "failure, client %d line %d: %s\n", i, __LINE__,
				lc_errstring(h[i]));
	}
	for (i = 0; i < multi_cnt; i++)
	{
		/*lc_checkin(h[i], "f97", 0);*/
		lc_free_job(h[i]);
	}
    }

    if (((dotest < 0 || dotest == 98) && !quick)|| (dotest < 98 && maxtest >=98) )
    {
/*
 * -----------------------------------------------------------------------------
 *	98	N licenses (all at once, in two pieces, one at a time)
 */
	serv_log( "98 - N licenses (all at once, in two pieces, one at a time)\n");
	{



/* 
 *	All at once
 */
		for (i=0;i<NUM_JOBS;i++) 
		{
			ts_lc_new_job( &code, &job[i], __LINE__);
			lm_job = job[i];			
			st_set_attr(lm_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)10, __LINE__);
		}

		lm_job = job[0];
		if (lm_checkout("f2", VER, 9, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "checkout failed line %d\n", __LINE__);
		lm_job = job[1];
		if (!lm_checkout("f2", VER, 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "checkout should have failed line %d\n", 
					__LINE__);
		lm_job = job[0];
		lm_checkin("f2", 0); 
		lm_job = job[1];
		lm_checkin("f2", 0);
/* 
 *	All in 2 pieces 
 */
			
		lm_job = job[0];
		if (lm_checkout("f2", VER, 4, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "checkout failed line %d %s\n", __LINE__,
						lc_errstring(lm_job));
		lm_job = job[1];
		if (lm_checkout("f2", VER, 5, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "checkout failed line %d %s\n", __LINE__,
						lc_errstring(lm_job));
		lm_job = job[2];
		if (!lm_checkout("f2", VER, 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "checkout should have failed line %d\n", 
								__LINE__);
		lm_job = job[0];
		lm_checkin("f2", 0);
		lm_job = job[1];
		lm_checkin("f2", 0);
		lm_job = job[2];
		lm_checkin("f2", 0);
		lm_job = job[0];

/* 
 *	One at a time (9 max)
 */
		{
		  static int n[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 };
		  int *nlic;
		  int i;

			for (i=0,nlic = n; *nlic; nlic++, i++)
			{
				lm_job = job[i];
				if (lm_checkout("f2", VER, 1, 0, &code, 
								LM_DUP_NONE))
				    fprintf(ofp, 
				    "checkout failed line %d i = %d (%s)\n", 
					__LINE__, i, lc_errstring(lm_job));
			}
			lm_job = job[i];
			if (!lm_checkout("f2", VER, 1, 0, &code, 
								LM_DUP_NONE))
				fprintf(ofp, "checkout should have failed line %d\n", 
								__LINE__);
			for (i=0,nlic = n; *nlic; nlic++, i++)
			{
				lm_job = job[i];
				lm_checkin("f2", 0);
			}
			lm_job = job[i];
			lm_checkin("f2", 0);
		}

	}
/* 
 * -----------------------------------------------------------------------------
 *	98.1	Multiple features from a single application
 */
	serv_log( "98.1 - Multiple features from a single application\n");
	{
		static char *features[] = { "f2", "f3", "f4", 0 };
		char **x;
		
		for (x = features; *x; x++)
		{
			rc = lm_checkout(*x, VER, 1, 0, &code, LM_DUP_NONE);
			if (rc) 
			{
			   fprintf(ofp, "checkout of %s license failed, line %d\n", 
				*x, __LINE__);
				ts_dump(*x);
			}
		}
		for (x = features; *x; x++)
		{
			lm_checkin(*x, 0);
		}
		for (i=0;i<NUM_JOBS;i++) 
		{
			lm_free_job(job[i]);
		}
	}
    }
    lm_job = lc_first_job(main_job);
    while (lm_job)
    {
	if (lm_job != main_job)
		fprintf(ofp, "job is %x\n", lm_job);
	lm_job = lc_next_job(lm_job);
    }


    if ((dotest < 0 || dotest > 98)|| (dotest < 98 && maxtest >=98) )
    {
	last_test();
/*
 * -----------------------------------------------------------------------------
 *	Put things back to "normal"
 */
#if 0 /* All these cleanup things involve those nasty child processes... */
	fprintf(ofp, "Cleaning up -- all errors reported above\n");
	cp(backup, config);
	rm(CF);
	rm(OPTSFILE); 
#endif /* All these... */
    }
    lm_job = lc_first_job(main_job);
    while (lm_job)
    {
      LM_HANDLE *next;
	if (lm_job != main_job)
		fprintf(ofp, "job is %x\n", lm_job);
	
	next = lc_next_job(lm_job);
	lc_free_job(lm_job);
	lm_job = next;
    }
#ifdef PC
	WSACleanup();
#endif
    if (dotest <= 100)
	exit(0);
    else
	exit(-1);
    return 0;
}

void
usage()
{
	fprintf(stderr,"usage: %s [-vendord path]\n","" /*ts_Progname*/);
}

int
lm_vd_msg(job, feature)
LM_HANDLE *job;
char *feature;
{
  CONFIG *c, *conf;
  int first = 1;
	
	c = (CONFIG *)0;

	for (conf = lc_next_conf(job, feature, &c);conf; 
		conf=lc_next_conf(job, feature, &c))
	{
		if (!conf) break;
		if (first)
		{
			gi.type = LM_VD_GENINFO_HANDLE_TYPE;
			gi.feat = conf;
			if (lc_get_attr(job,LM_A_VD_GENERIC_INFO, (short *)&gi))
				return(-1);
			else 
			{
				/*fprintf(ofp, "conn-timeout = %d\n", gi.conn_timeout);*/
			}
			first = 0;
		}
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE;
		fi.feat = conf;
		if (lc_get_attr(job, LM_A_VD_FEATURE_INFO, (short *)&fi) &&
			lc_get_errno(job) != LM_NOFEATURE)
		{
			
			ts_error(job, "LM_A_VD_FEATURE_INFO");
			return -1;
		}
		else
		{
			/*fprintf(ofp, "lowwater %d\n", fi.lowwater);*/
		}
	}
	return 0;
}

void
ts_lm_init(vendor_id, vendor_key, job_id, line)
char *vendor_id;		/* vendor ID */
VENDORCODE *vendor_key;		/* Vendor's encryption code */
LM_HANDLE **job_id;
int line;
{
  int rc;

	
	if ((rc = lc_init(main_job, vendor_id, vendor_key, job_id)) && 
						rc != LM_DEMOKIT  &&
						rc != LM_DEFAULT_SEEDS) 	
	{
		fprintf(ofp, "error line %d: %s\n", line, lc_errstring(*job_id));
	}
	st_set_attr(*job_id, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0, __LINE__);
	st_set_attr(*job_id, LM_A_PROMPT_FOR_FILE, (LM_A_VAL_TYPE) 0, __LINE__);
	lc_set_registry(*job_id, "DEMO_LICENSE_FILE", 0);
	lc_set_registry(*job_id, "DEMOF_LICENSE_FILE", 0);
	lc_set_registry(*job_id, "DEMOF2_LICENSE_FILE", 0);
	lc_set_registry(*job_id, "LM_LICENSE_FILE", 0);
}
void
ts_lc_new_job(vendor_key, job_id, line)
VENDORCODE *vendor_key;		/* Vendor's encryption code */
LM_HANDLE **job_id;
int line;
{
  int rc;

	
	if ((rc = lc_new_job(main_job, 0, vendor_key, job_id)) && 
						rc != LM_DEMOKIT) 	
	{
		fprintf(ofp, "error line %d: %s\n", line, lc_errstring(*job_id));
	}
	st_set_attr(*job_id, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0, __LINE__);
	st_set_attr(*job_id, LM_A_PROMPT_FOR_FILE, (LM_A_VAL_TYPE) 0, __LINE__);
	lc_set_registry(*job_id, "DEMO_LICENSE_FILE", 0);
	lc_set_registry(*job_id, "LM_LICENSE_FILE", 0);
	st_set_attr(*job_id, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE) 1, __LINE__);
}
void
makelic(job, hostname, hostid, port, infile, outfile, daemon_line)
LM_HANDLE *job;
char *hostname;
char *hostid;
int port;
char *infile;
char *outfile;
char *daemon_line;
{
#define MAXLINE 500
#define MAXSIZE (MAXLINE * 500)
#ifdef PC
  static			/* so that the next line won't overflow 
				 * the stack on Windows.
				 */
#endif
#ifndef PC16
  char license[MAXSIZE];
#else
  char license[11]; /* this never gets called */
#endif
  FILE *ifp, *lic_ofp, *efp;
  char str[MAXLINE +1];
  char *cp = license;
  char *return_str;
  char *errors;
  VENDORCODE vc;
  char *badline, *templine;
  struct _vendors *vendor;
  LM_HANDLE *fs_job;
  char *featset;
  char *template_file;
  char server_line[200];
  int i;
	
	*server_line = 0;


	if (hostid)
	{
#ifdef VMS
		sprintf(server_line, "SERVER %s %s\n", hostname, hostid);
#else
		/*sprintf(server_line, "SERVER %s %s %d\n", hostname, hostid, port);*/
		if (L_STREQ(infile, "pathtest.tem"))
			sprintf(server_line, "SERVER this_host %s %d\n", 
				hostid, LMGRD_PORT_START + 2);
		else
			sprintf(server_line, "SERVER %s %s\n", hostname, 
							hostid);
#endif /* VMS */
	}
	strcpy(cp, server_line);
	cp += strlen(cp);
	strcpy(cp, daemon_line);
	cp += strlen(cp);
	if (!strcmp(outfile, MAIN_LICENSE))
	{
	  char domain[MAX_CONFIG_LINE + 1];
	  	unlink("demo.lic");
		sprintf(cp, "FEATURE node-locked-counted demo 1.0 1-jan-0 1 0 SIGN=0 HOSTID=%s ck=0\n", hostid);
		cp += strlen(cp);
		sprintf(cp, "FEATURE internet-hostid demo 1.0 1-jan-0 1 0 SIGN=0 HOSTID=INTERNET=*.*.*.* ck=0\n");
		cp += strlen(cp);
		if (1 != pc16)
			sprintf(cp, "FEATURE good_plat demo 1.0 1-jan-0 1 0 SIGN=0 PLATFORMS=%s\n", l_platform_name());
		else
                	sprintf(cp, "FEATURE good_plat demo 1.0 1-jan-0 1 0 SIGN=0 PLATFORMS=%s\n", "i86_w" );
		cp += strlen(cp);
		sprintf(cp, "FEATURE dongle1 demo 1.0 31-dec-2010 uncounted FLOAT_OK=%s HOSTID=FLEXID=FILE-testing123 SIGN=0\n", hostid);
		cp += strlen(cp);
		sprintf(cp, "FEATURE dongle1 demo 1.0 31-dec-2010 uncounted FLOAT_OK HOSTID=FLEXID=FILE-testing456 SIGN=0\n", hostid);
		cp += strlen(cp);
		sprintf(cp, "FEATURE dongle1 demo 1.0 31-dec-2010 uncounted FLOAT_OK=12345678 HOSTID=FLEXID=FILE-testing789 SIGN=0\n", hostid);
		cp += strlen(cp);

#if 0
#ifndef PC
		lc_hostid(job, HOSTID_DOMAIN, domain);
		sprintf(cp, "FEATURE domain demo 1.0 permanent 1 0 SIGN=0 HOSTID=%s\n", domain);
		cp += strlen(cp);
#endif
#endif
	}

	if (*infile)
	{
		template_file = infile;
	}
	else
        {
                if ( pc16==1 )
                        template_file = "servtest16.tem";
                else
			template_file = "servtest.tem";
	}
	if (!(ifp = fopen(template_file, "r")))
	{
		fprintf(stderr, "Can't read %s, exiting\n", template_file);
		exit(1);
	}
	if (!(lic_ofp = fopen(outfile, "w")))
	{
		fprintf(stderr, "Can't write to %s, exiting\n", outfile);
		exit(1);
	}
	if (!(efp = fopen("servtest.err", "a")))
	{
		fprintf(stderr, "Can't append to servtest.err, exiting\n");
		exit(1);
	}
/*
 *	There's no server line in the file
 */
	while (fgets(str, MAXLINE, ifp))
	{
		strcpy(cp, str);
		{
		  char *r, *end;
		  char patt[50], repl[50];
		  int mult, len;
			if (r = strstr(cp, "$PCSECOND"))
			{
				end = r + strlen("$PCSECOND");
				/* back up to start of multiplier */
				for (r-=2; isdigit(*r); r--) ;
				r++;
				sscanf(r, "%d", &mult);
				mult *= this_mach_speed;
				mult /= 100;
				len = end - r;
				l_zcp(patt, r, len);
				sprintf(repl, "%d", mult);
				end = replace(r, patt, repl);
				for (i = 0; i < len; i++)
					r[i] = ' ';
				strncpy(r, end, strlen(end));
			}
		}
		cp += strlen(str);
	}
	if (L_STREQ(infile, "pathtest.tem"))
	{
		sprintf(cp, "\tHOSTID=%s\n", hostid);
	}
/*
 *	encrypt for every possible vendor name
 */
	for (vendor = vendors; vendor->name; vendor++)
	{
		if (L_STREQ(vendor->name, "demof"))
		{
			lc_set_attr(job, LM_A_LKEY_START_DATE, (LM_A_VAL_TYPE)1);
			lc_set_attr(job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1);
		}
		strcpy(job->vendor, vendor->name);
		memcpy(&vc, (char *)vendor->vc, sizeof(VENDORCODE));
		vc.data[0] = (vendor->vc->data[0])^(vendor->sig);
		vc.data[1] = (vendor->vc->data[1])^(vendor->sig);
		vc.strength = LM_STRENGTH;
		for (i = 0; i < LM_MAXSIGNS; i++)
		{
		  int j, pcnt;
		  LM_VENDORCODE_PUBKEYINFO *p; 
		  int sign_level = l_priseedcnt; 
		  /* from LM_CODE_GEN_INIT */
			for (pcnt = 0; pcnt <= l_priseedcnt; pcnt++) 
			{
				memcpy(&vc.pubkeyinfo[pcnt].pubkeysize, 
					lm_prisize[pcnt], 
					sizeof(lm_prisize[pcnt])); 
				vc.pubkeyinfo[pcnt].pubkey_fptr = l_prikey_sign;
				vc.pubkeyinfo[pcnt].sign_level = sign_level--; 
				{ 
				  int i; 
					for (i = 0; i < LM_PUBKEYS; i++) 
					{ 
						memcpy(
						vc.pubkeyinfo[pcnt].pubkey[i], 
							lm_prikey[pcnt][i], 
							lm_prisize[pcnt][i]); 
					} 
				} 
			}
		}
		if (lc_cryptstr(job, license, &return_str, &vc, 
				LM_CRYPT_FORCE,
				infile, &errors))
		{
			fprintf(efp, "%s\n", errors);
			lc_free_mem(job, errors);
		}
		strcpy(license, return_str);
		lc_free_mem(job, return_str);
		if (L_STREQ(vendor->name, "demof"))
		{
			lc_set_attr(job, LM_A_LKEY_START_DATE, (LM_A_VAL_TYPE)0);
			lc_set_attr(job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)0);
		}
	}
	fwrite(license, strlen(license), 1, lic_ofp);
	fflush(lic_ofp);
	fclose(ifp);
	fclose(efp);
	fclose(lic_ofp);
	if (!strcmp(outfile, MAIN_LICENSE))
	{
#ifdef TEST_DEMOF
/*
 *		Add necessary FEATURESET line(s)
 */
		/*
		fprintf(ofp, "%x %x %x\n", ENCRYPTION_SEED1, VENDOR_KEYf_5, ENCRYPTION_SEED1 ^ VENDOR_KEYf_5);
		fflush(ofp);
		codef.data[0] ^= VENDOR_KEYf_5;
		codef.data[1] ^= VENDOR_KEYf_5;*/
		lc_init(0, "demof", &codef, &fs_job);
		lc_set_errno(fs_job, 0);
		st_set_attr(fs_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)MAIN_LICENSE, __LINE__);
		st_set_attr(fs_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1, __LINE__);
		st_set_attr(fs_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)0, __LINE__);
		featset = lc_feat_set(fs_job, "demof", &codef, 0);
		if (!(lic_ofp = fopen(MAIN_LICENSE, "a")))
		{
			fprintf(stderr, "Can't write to %s, exiting (line %d)\n", MAIN_LICENSE, __LINE__);
			exit(1);
		}
		if (featset)
			fprintf(lic_ofp, "FEATURESET demof %s\n", featset);
		else
		{
			fprintf(stderr, "line %d: ", 
				__LINE__);
			ts_error(fs_job, "Error Generating FEATURESET for demof");
		}
		lc_free_job(fs_job);
/* 
 *	A bogus line
 */
		fprintf(lic_ofp, "FEATURESET demof2 1234567887654321\n", featset);

		fclose(lic_ofp);
#endif /* TEST_DEMOF */
		if (!(lic_ofp = fopen(ALT1_LICENSE, "w")))
		{
			fprintf(stderr, "Can't write to %s, exiting (%d)\n", ALT1_LICENSE, __LINE__);
			exit(1);
		}

        	fprintf(lic_ofp, server_line);
        	fprintf(lic_ofp, daemon_line);
	
/*
 *		Make some bad lines by copying good ones and changing others
 *		1. change feature name
 */
		templine = get_line(" f3 ", license);
		badline = replace(templine, "f3", "e1");
		fwrite(badline, strlen(badline), 1, lic_ofp);
/*
 *		2. change version (also change the code to identify)
 */
		badline = replace(templine, "1.05", "1.06");
		strcpy(str, badline);
		badline = replace(str, " 9 ", " 9 111111");
		fwrite(badline, strlen(badline), 1, lic_ofp);
/*
 *		3. change expiration date
 */
		badline = replace(templine, "1-jan-2020", "2-jan-2020");
		fwrite(badline, strlen(badline), 1, lic_ofp);
/*
 *		4. change count
 */
		badline = replace(templine, " 9 ", " 8 ");
		fwrite(badline, strlen(badline), 1, lic_ofp);
/*
 *		5. Bad enabling FEATURE for PACKAGE 
 *	  	(the good one is already in the file)
 */
		badline = 
	  "FEATURE pkg11 demo 1.050 01-jan-0 2 11111111111111111111 \n";
		fwrite(badline, strlen(badline), 1, lic_ofp);
/*
 *		6. Bad PACKAGE line, good enabling FEATURE
 *	   Since this is an uncounted feature, we can only query the client,
 *	   not the server
 */
		badline = 
	"PACKAGE pkg12 demo 1.050 11111111111111111111 COMPONENTS=pkg12comp1\n";
		fwrite(badline, strlen(badline), 1, lic_ofp);
		templine = 
	"FEATURE pkg12 demo 1.050 01-jan-2020 0 B18131413BD8E53CB8FE \"\" DEMO\n";
		fwrite(templine, strlen(templine), 1, lic_ofp);

/*
 *		Badcode 
 */
		badline = 
	"FEATURE badcodefeat demo 1.0 permanent 1 123456789012\n";
		fwrite(badline, strlen(badline), 1, lic_ofp);
/*
 *		P5552
 */

		badline = 
	"FEATURE P5552 demo 1.0 permanent uncounted HOSTID=ANY\n";
		fwrite(badline, strlen(badline), 1, lic_ofp);
		badline = 
	"FEATURE P5552_sign demo 1.0 permanent uncounted HOSTID=ANY SIGN=\n";
		fwrite(badline, strlen(badline), 1, lic_ofp);

		fclose(lic_ofp);
		lic_ofp = fopen("nosuch.dat", "w");
		fprintf(lic_ofp, "SERVER %s any\nVENDOR nosuch\nFEATURE alt nosuch 1.0 01-jan-2020 9 0\n", hostname);
		fclose(lic_ofp);
	}
	else if (!strcmp(infile, "pathtest.tem"))
	{

		if (!(lic_ofp = fopen("pathtest.opt", "w")))
		{
			perror("Can't open pathtest.opt");
			return;
		}
		fprintf(lic_ofp, "REPORTLOG pathtest.rl\n");
		fclose(lic_ofp);
	}
}

void
st_license(lm_job, lic, line)
LM_HANDLE *lm_job;
char *lic;
int line;
{
  VENDORCODE vc;
  char *result;
  char *errors;
  FILE *fp;
  char str[500];
  LM_HANDLE *j;
  int i;
  int sign_level = l_priseedcnt;;

#if 1
	/*memset(&vc, 0, sizeof(vc));*/
	memcpy((char *)&vc, (char *)&lic_vc, sizeof(vc));
	for (i = 0; i < LM_MAXSIGNS; i++)
	{
	  int j;
		vc.pubkeyinfo[i].pubkey_fptr = l_prikey_sign;
		vc.pubkeyinfo[i].pubkey_fptr = l_prikey_sign;
		vc.pubkeyinfo[i].sign_level = sign_level--; \
		memcpy(&vc.pubkeyinfo[i].pubkeysize, &lm_prisize, sizeof(lm_prisize));
		for (j = 0; j < LM_PUBKEYS; j++) 
			memcpy(&vc.pubkeyinfo[i].pubkey[j], &lm_prikey[i][j],  lm_prisize[i][j]); 
	}
	vc.data[0] ^= VENDOR_KEY5;
	vc.data[1] ^= VENDOR_KEY5;
	vc.strength = LM_STRENGTH;
#endif
	ts_lm_init("demo", &vc, &j, __LINE__);

	sprintf(str, 
        "SERVER %s ANY\nVENDOR demo\n%s\n", hostname, lic);
	if (lc_cryptstr(j, str, &result, &vc, LM_CRYPT_FORCE,
						MAIN_LICENSE, &errors))
	{
		fprintf(stderr, "line %d: ", line);
		ts_error(j, errors); 
		lc_free_mem(j, errors);
		exit(1);
	}
	if (!(fp = fopen(MAIN_LICENSE, "w")))
	{
		fprintf(stderr, "file %s line %d: ", MAIN_LICENSE, line);
		perror ("Can't open file for write");
		exit(1);
	}
	fputs(result, fp);
	lc_free_mem(j, result);
	fclose(fp);
	lc_free_job(j);
}

#ifdef TRUE_BSD
static
char *
strstr(str, patt)
char *str;
char *patt;
{
  int i, len, plen;
	len = strlen(str);
	plen = strlen(patt);
	for (i = 0; str[i]; i++)
	{
		if (((len - i) >= plen) && !strncmp(&str[i], patt, plen))
			return &str[i];
	}
	return(char *)0;
}
#endif /* TRUE_BSD */
void
serv_log(str)
char *str;
{
  CONFIG *conf, *pos = (CONFIG *)0;
  LM_HANDLE *log_job;
  time_t t = time(0);
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  struct tm *tm;

LM_CODE(logcode, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

#ifdef THREAD_SAFE_TIME
	localtime_r(&t, &tst);
	tm = &tst;
#else /* !THREAD_SAFE_TIME */
	tm = localtime(&t);
#endif
	if (gotplus) dotest = -1;
	fprintf(ofp, "Test %s", str);


	if (!file_based)
	{
	 	lc_init(0, VENDOR_NAME, &logcode, &log_job);
		lc_set_registry(log_job, "LM_LICENSE_FILE", 0);
		lc_set_registry(log_job, "DEMO_LICENSE_FILE", 0);
		st_set_attr(log_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0, __LINE__);
		conf = l_next_conf_or_marker(log_job, "f1", &pos, 1,(char *)0);
		if (!conf)
		{
			lc_free_job(log_job);
			return;
		}
		if (l_connect(log_job, conf->server, conf->daemon, LM_TCP) < 0)
		{
			lc_free_job(log_job);
			return;
		}
		lc_set_errno(log_job, 0);
		lc_log(log_job, str);
		if (lc_get_errno(log_job))
			ts_error(log_job, "lc_log");
		lc_free_job(log_job);
	}
}

#if defined(PC) && !defined(WINNT) && !defined(OS2)
void Sleep(mSeconds)
int mSeconds;
{
	DWORD end_time = GetTickCount()+mSeconds;

	while( GetTickCount() < end_time )
		Yield();
}

void CopyFile(char *from, char *to, int ignored )
{
	OFSTRUCT ofStrSrc;
	OFSTRUCT ofStrDest;
	HFILE hfSrcFile, hfDstFile;

	LZStart();

	/* Open, copy, and then close the files. */

	hfSrcFile = LZOpenFile(from, &ofStrSrc, OF_READ);
	hfDstFile = LZOpenFile(to, &ofStrDest, OF_CREATE);
	CopyLZFile(hfSrcFile, hfDstFile);
	LZClose(hfSrcFile);
	LZClose(hfDstFile);

	LZDone(); /* free the internal buffers */
}

#endif /* defined(PC) && !defined(WINNT) && !defined(OS2) */

char *
get_line(feature, str)
char *feature;
char *str;
{
  char *curr, *next, *cp, last;
  static char ret[2048];
  int i;
  int cont = 0;

	curr = next = str;
	while (1)
	{
		while (*next && ((*next != '\n') || cont))
		{
			if (*next == '\\') 
				cont = 1;
			if ((*next == '\n') && cont) 
				cont = 0;
			next++;
		}
		for (i = 0,cp = curr; cp <= next; cp++)
			ret[i++] = *cp;
		ret[i] = '\0';
		if (strstr(ret, feature))
		{
			return ret;
		}
		if (!*next) return (char *)0;
		curr = ++next;
	}
}
char *
replace(str, opatt, npatt)
char *str, *opatt, *npatt;
{
  char *cp;
  static char ret[2048];
  int i, len;
	
	strcpy(ret, str);

	cp = strstr(ret, opatt);
	len = strlen(npatt);
	for (i = 0; i < len; i++)
		cp[i] = npatt[i];
	return(ret);
}
get_feat_info(feat, line, fi, job)
char * feat;
int line;
LM_VD_FEATURE_INFO *fi;
LM_HANDLE *job;
{
	return (get_feat_info_err(feat, line, fi, job, 1, 0));
}
get_feat_info_err( char * feat, int line, LM_VD_FEATURE_INFO *fi, LM_HANDLE *job, int report_errs, CONFIG *conf)
{
  CONFIG *pos = (CONFIG *)0;
  int found = 0;

	memset(fi, 0, sizeof(LM_VD_FEATURE_INFO));
	if (!conf)
	{
		while (conf = lc_next_conf(job, feat, &pos))
		{
			fi->type = LM_VD_FEATINFO_HANDLE_TYPE; fi->feat = conf;
			if (!lc_get_attr(job, LM_A_VD_FEATURE_INFO, 
						(short *)fi))
				return 1; /* return the first pool */
			if (lc_get_errno(job) != LM_NOSERVSUPP) /* real error */
				break;
		}
	}
	else
	{
		fi->type = LM_VD_FEATINFO_HANDLE_TYPE; fi->feat = conf;
		if (!lc_get_attr(job, LM_A_VD_FEATURE_INFO, (short *)fi))
				return 1; 
	}
	if (report_errs)
		fprintf(ofp, "%s (%s) not in server, line %d: %s\n", feat, 
		conf ? conf->code : "" , line,
		lc_errstring(job));
	return 0;
}

reread_tests(subtest)
{
  char buf[200];
  int rc;
  LM_USERS *users;	
/* 
 * -----------------------------------------------------------------------------
 *	lmreread tests
 */

	if (subtest == -1 || subtest == 'a')
	{
		serv_log("17a lmreread tests - reread same license file\n");
		reread_same_lic();

	}

	if (subtest == -1 || subtest == 'b')
	{
		st_rename(MAIN_LICENSE, "license.sav",__LINE__);
		serv_log("17b lmreread tests - remove/add feature support\n");
		ts_lm_init("demo", &code, &job[0], __LINE__); 
		st_license(job[0], "FEATURE f1 demo 1.0 1-jan-0 9 0 SIGN=0", __LINE__);
		st_reread();
		lc_free_job(job[0]);
#ifdef VMS
		sleep(10);
#endif /* VMS */
		ts_lc_new_job(&code, &job[0], __LINE__); 
		lc_set_attr(job[0], LM_A_SIGN_LEVEL, 0);
		if (lc_checkout(job[0], "f1", "1.0", 9, 0, &code, LM_DUP_NONE))
		{
			fprintf(ofp, "reread error line %d %s\n", __LINE__, 
				lc_errstring(job[0]));
		}
		lc_checkin(job[0], "f1", 0);

		st_license(job[0], 
	"FEATURE f1 demo 1.0 1-jan-0 5 0 SIGN=0\nFEATURE f2 demo 1.0 1-jan-0 9 0 SIGN=0\nFEATURE f3 demo 1.0 1-jan-0 9 0 SIGN=0", __LINE__);
		st_reread();
		lc_free_job(job[0]);
		ts_lc_new_job(&code, &job[0], __LINE__); 
		lc_set_attr(job[0], LM_A_SIGN_LEVEL, 0);
		if (lc_checkout(job[0], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		{
			fprintf(ofp, "reread error line %d %s\n", __LINE__, 
				lc_errstring(job[0]));
		}
		if (lc_checkout(job[0], "f2", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "reread error line %d\n", __LINE__);
		if (lc_checkout(job[0], "f3", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "reread error line %d\n", __LINE__);
		lc_disconn(job[0], 1);
		st_license(job[0], 
	"FEATURE f1 demo 1.0 1-jan-0 5 0 SIGN=0\nFEATURE f4 demo 1.0 1-jan-0 9 0 SIGN=0\nFEATURE f5 demo 1.0 1-jan-0 9 0 SIGN=0", __LINE__);
		st_reread();
		lc_free_job(job[0]);
#ifdef VMS
		sleep(10);
#endif /* VMS */
		ts_lc_new_job(&code, &job[0], __LINE__); 
		lc_set_attr(job[0], LM_A_SIGN_LEVEL, 0);
		if (lc_checkout(job[0], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "reread error line %d: \"%s\"\n", __LINE__,
			lc_errstring(job[0]));
		if (lc_checkout(job[0], "f4", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "reread error line %d: \"%s\"\n", __LINE__,
			lc_errstring(job[0]));
		if (lc_checkout(job[0], "f5", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "reread error line %d: \"%s\"\n", __LINE__,
			lc_errstring(job[0]));
		lc_disconn(job[0], 1);
		ts_lc_new_job(&code, &job[1], __LINE__); 
		lc_set_attr(job[1], LM_A_SIGN_LEVEL, 0);
		st_set_attr(job[1], LM_A_LICENSE_DEFAULT , (LM_A_VAL_TYPE)"license.sav", __LINE__);
		if (!lc_checkout(job[1], "f2", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "reread error line %d\n", __LINE__);
		if (!lc_checkout(job[1], "f3", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "reread error line %d\n", __LINE__);
		lc_checkin(job[1], "f2", 0);
		lc_checkin(job[1], "f3", 0);
		lc_free_job(job[1]);
		lc_free_job(job[0]);
	}
	if (subtest == -1 || subtest == 'c')
	{
	/*
	 *	read new license file with different count of 1 feature
	 */
		serv_log("17c lmreread tests - change feature count supported\n");

		ts_lc_new_job(&code, &job[0], __LINE__); 
		lc_set_attr(job[0], LM_A_SIGN_LEVEL, 0);
		st_license(job[0], "FEATURE f1 demo 1.0 1-jan-0 4 0 SIGN=0", __LINE__);
		st_reread();
		lc_free_job(job[0]);
#ifdef VMS
		sleep(10);
#endif /* VMS */
		ts_lc_new_job(&code, &job[0], __LINE__); 
		lc_set_attr(job[0], LM_A_SIGN_LEVEL, 0);
		if (get_feat_info("f1", __LINE__, &fi, job[0]))
		{
			if (fi.num_lic != 4)
				fprintf(ofp, "reread error line %d\n", __LINE__);
		}
		st_license(job[0], "FEATURE f1 demo 1.0 1-jan-0 3 0 SIGN=0", __LINE__);
		st_reread();
		lc_free_job(job[0]);
#ifdef VMS
		sleep(10);
#endif /* VMS */
		ts_lc_new_job(&code, &job[0], __LINE__); 
		lc_set_attr(job[0], LM_A_SIGN_LEVEL, 0);
		if (get_feat_info("f1", __LINE__, &fi, job[0]))
		{
			if (fi.num_lic != 3)
				fprintf(ofp, "reread error line %d\n", __LINE__);
		}
	}
	if (subtest == -1 || subtest == 'd')
	{
		serv_log( "17d - lmreread tests - remove feature and add back\n");
		st_license(job[0], "FEATURE f2 demo 1.0 1-jan-0 3 0 SIGN=0", __LINE__);
		st_reread();
		lc_free_job(job[0]);
#ifdef VMS
		sleep(10);
#endif /* VMS */
		ts_lc_new_job(&code, &job[1], __LINE__); 
		lc_set_attr(job[1], LM_A_SIGN_LEVEL, 0);
		if (!lc_checkout(job[1], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "reread error line %d [but okay if using port@host]\n", 
						__LINE__);
		lc_checkin(job[1], "f1", 0);
		lc_free_job(job[1]);
	}
#if 1 /*- removed per matt */
	if (subtest == -1 || subtest == 'e')
	{
	  FILE *fp;
		ts_lm_init("demo", &code, &job[0], __LINE__); 
		ts_lm_init("demo", &code, &job[1], __LINE__); 
		ts_lm_init("demo", &code, &job[2], __LINE__); 
		serv_log( "17e - lmreread tests - reread reserve options\n");
		st_license(job[0], "FEATURE f1 demo 1.0 permanent 4 0 SIGN=0\
			\nFEATURE f2 demo 1.0 permanent 4 0 SIGN=0", __LINE__);
		st_rename(OPTSFILE, "opts.sav", __LINE__);
		fp = fopen("opts", "w"); 
		fprintf(fp, "RESERVE 3 f1 USER user1\n");
		fprintf(fp, "RESERVE 3 f2 USER user1\n");
		fprintf(fp, "EXCLUDE f1 USER user3\nREPORTLOG +report.log\n");
		fclose(fp);
		st_reread();
		lc_free_job(job[0]);
		lc_free_job(job[1]);
		lc_free_job(job[2]);
		ts_lc_new_job(&code, &job[0], __LINE__); 
		ts_lc_new_job(&code, &job[1], __LINE__); 
		ts_lc_new_job(&code, &job[2], __LINE__); 
		st_set_attr(job[0], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"user1", __LINE__);
		st_set_attr(job[1], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"user2", __LINE__);
		st_set_attr(job[2], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"user3", __LINE__);
		st_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0, __LINE__);
		st_set_attr(job[1], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0, __LINE__);
		st_set_attr(job[2], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0, __LINE__);
		if (lc_checkout(job[0], "f1", "1.0", 4, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Checkout failed, line %d:\"%s\"\n", 
						__LINE__, lc_errstring(job[0]));
		if (!lc_checkout(job[2], "f1", "1.0", 4, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Checkout shd have excluded user3 l:%d\n", 
								__LINE__);
		if (lc_checkout(job[0], "f2", "1.0", 2, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Checkout failed, line %d:\"%s\"\n", 
						__LINE__, lc_errstring(job[0]));
		if (!lc_checkout(job[2], "f2", "1.0", 2, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Checkout shd have excluded user3 l:%d\n", 
								__LINE__);
		fp = fopen("opts", "w"); 
		fprintf(fp, "RESERVE 3 f1 USER user2\n");
		fprintf(fp, "RESERVE 3 f2 USER user2\nREPORTLOG +report.log\n");
		fclose(fp);
		st_reread();
		lc_checkin(job[0], "f1", 0);
		if (lc_checkout(job[1], "f1","1.0", 4, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Checkout failed, line %d:\"%s\"\n", 
				__LINE__, lc_errstring(job[1]));
		lc_checkin(job[1], "f1", 0);
		if (lc_checkout(job[2], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "user3 excluded line %d: \"%s\"\n", 
						__LINE__, lc_errstring(job[2]));
		lc_checkin(job[0], "f2", 0);
		if (lc_checkout(job[1], "f2","1.0", 2, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "Checkout failed, line %d:\"%s\"\n", 
				__LINE__, lc_errstring(job[1]));
		lc_checkin(job[1], "f2", 0);
		if (lc_checkout(job[2], "f2", "1.0", 1, 0, &code, LM_DUP_NONE))
			fprintf(ofp, "user3 excluded line %d: \"%s\"\n", 
						__LINE__, lc_errstring(job[2]));
		lc_free_job(job[0]);
		lc_free_job(job[1]);
		lc_free_job(job[2]);
	}
#endif
	if (subtest == -1 || subtest == 'f')
	{
	 struct stat s;
		serv_log("17f - reset files\n");
	 	if (!stat("license.sav", &s))
			st_rename("license.sav", MAIN_LICENSE, __LINE__);
	 	if (!stat("opts.sav", &s))
			st_rename("opts.sav", OPTSFILE, __LINE__);
		st_reread();
	}
}

void
last_test()
{
  int rc;
  char buf[200];
/* 
 * -----------------------------------------------------------------------------
 *	last	Kill server, make sure application dies
 */
	serv_log( "last - Kill server, make sure applications die (15 seconds)\n");
/*
 *	shutdown nosuch.dat and pathtest.dat servers
 */
	ts_lm_init("demo", &code, &job[0], __LINE__);
	st_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"pathtest.dat", __LINE__);
	if ((rc = lc_shutdown(job[0], 0, 0)) <= 0)
		fprintf(ofp, "shutdown failed %d %s got %d\n", __LINE__, 
			lc_errstring(job[0]), rc);
	lc_free_job(job[0]);
	ts_lm_init("demo", &code, &job[0], __LINE__);
		
	sprintf(buf, "-%d@%s", LMGRD_PORT_START, hostname);
	st_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf, __LINE__);
	strcpy(job[0]->vendor, "nosuch");
	if ((rc = lc_shutdown(job[0], 0, 0)) <= 0)
		fprintf(ofp, "shutdown failed %d %s got %d\n", __LINE__, 
			lc_errstring(job[0]), rc);
	lc_free_job(job[0]);

	ts_lm_init("demo", &code, &lm_job, __LINE__);
	st_set_attr(lm_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5, __LINE__);
	st_set_attr(lm_job, LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(lm_job, LM_A_USER_EXITCALL, (LM_A_VAL_TYPE)user_exitcall, __LINE__);
	st_set_attr(lm_job, LM_A_USER_RECONNECT, (LM_A_VAL_TYPE)user_reconnect, __LINE__);
	good_client_line_nc("f1", VER, 1, &code, LM_DUP_NONE, argv_0, 0,
							__LINE__);
	ts_lm_init("demo", &code, &job[0], __LINE__);
	if ((rc = lc_shutdown(job[0], 0, 0)) <= 0)
		fprintf(ofp, "shutdown failed %d %s got %d\n", __LINE__, 
			lc_errstring(job[0]), rc);
	lc_free_job(job[0]);
#ifdef WINNT
	lc_sleep(lm_job,40);
#endif
#if  defined(PC16)
	/*
	 * Windows NT console applicatins such as servtest must call
 	 * lc_timer directly.
	 */
        Sleep(3000);
        lc_timer(lm_job);
        Sleep(3000);
        lc_timer(lm_job);
#endif
#if !defined(PC16) && ! defined (WINNT)
	l_select_one(0, -1, 15000); /* sleep 15 sec */
#endif /* WINNT */
	if (!got_exitcall)
		fprintf(ofp, "Error--server died, and child didn't detect, line %d\n",
			__LINE__);
	if (!got_reconnect)
		fprintf(ofp, "Error--server died, and child didn't detect, line %d\n",
			__LINE__);
}

reread_same_lic()
{
	/*ts_lm_init("demo", &code, &job[0], __LINE__);*/
	ts_lc_new_job(&code, &job[0], __LINE__);
	lc_set_attr(job[0], LM_A_SIGN_LEVEL, 0);
	if (lc_checkout(job[0], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "reread error line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	}
	if (get_feat_info("f1", __LINE__, &fi, job[0]))
	{
		if (fi.num_lic != 9)
			fprintf(ofp, "reread error line %d\n", __LINE__);
		if (fi.tot_lic_in_use != 1)
			fprintf(ofp, "reread error line %d\n", __LINE__);
	}
	st_reread();
	if (lc_checkout(job[0], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "reread error line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	}
	if (get_feat_info("f1", __LINE__, &fi, job[0]))
	{
		if (fi.num_lic != 9)
			fprintf(ofp, "reread error line %d\n", __LINE__);
		if (fi.tot_lic_in_use != 1)
			fprintf(ofp, "reread error line %d\n", __LINE__);
	}
	lc_free_job(job[0]);
}
	

#ifdef VMS
/*
 * DAEMON name path opts => DAEMON name path port opts
 * DAEMON name path      => DAEMON name path port
 */
static int nextvmsport = 0;
static int first = 1;
void
vms_daemon_line(line, port)
char *line;
int port;
{
    char *p, f1[MAXLINE], f2[MAXLINE], f3[MAXLINE], f4[MAXLINE];
    int nfields = 0;

        if (first)
        {
                nextvmsport = port;
                first = 0;
        }
        nfields = sscanf(line, "%s %s %s %s", f1, f2, f3, f4);
        if (nfields > 3)                /* 4th field is an opts file */
        {
/*
 *              If there is leading path information in the opts field,
 *              get rid of it.
 */
                if (p = strrchr(f4, '/')) strcpy(f4, p+1);
                sprintf(line, "%s %s %s %d %s\n", f1, f2, f3, nextvmsport, f4);
        }
        else
        {
                sprintf(line, "%s %s %s %d\n", f1, f2, f3, nextvmsport);
        }
        nextvmsport++;
}
#endif /* VMS */

#ifndef unix
#define unlink remove
#endif /* VMS */
void 
st_rename(from, to, line)
char *from;
char *to;
int line;
{
	unlink(to);
	if (rename(from, to))
	{
		fprintf(stderr, "%s %s line %d: ", from, to, line);
		perror("rename(), exiting");
		exit(1);
	}
}

void
st_set_attr(job, attr, val, line)
LM_HANDLE *job;
int attr;
char * val;
int line;
{
	if (lc_set_attr(job, attr, (LM_A_VAL_TYPE) val))
		fprintf(ofp, "set_attr error line %d: %s\n", line,
			lc_errstring(job));
}
int
LM_CALLBACK_TYPE
checkout_filter(c)
CONFIG *c;
{
	if (c->feature[0] == 'f') 
		return(1); 
	return(0);
}
int
CB_LOAD_DATA
LM_CALLBACK_TYPE
checkout_filter2(c)
CONFIG *c;
{
	if (!c->lc_serial) return 1;
	if (!strcmp(c->lc_serial, p1981_serial))
		return(0); 
	return(1);
}
test_dongles(job, featname)
LM_HANDLE **job;
char *featname;
{
  int rc;

	if (lc_checkout(job[0], featname, "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "%s  failed line %d: %s\n", 
				featname, __LINE__, lc_errstring(job[0]));

	if ((rc = lc_timer(job[0])) != 0)
		fprintf(ofp, "%s exp 0 got %d %d %s\n", 
				featname, rc, __LINE__, lc_errstring(job[0]));
/*
 *	Remove "dongle"
 */
	fprintf(ofp, "\t(Ignore next few errors if using USE_SERVER or port@host)\n");
        if (rename("hostid.txt", "hostid.sav"))
        { /* error condition */
                fprintf(ofp, "Error in Renaming hostid.txt\n");
                if (remove("hostid.txt"))
			fprintf(ofp, "Error in Removing hostid.txt\n");

        }
	l_select_one(0, -1, 3000); /* sleep 3 sec */
	if ((rc = lc_timer(job[0])) != 1)
		fprintf(ofp, "%s exp 1 got %d %d %s\n", 
				featname, rc, __LINE__, lc_errstring(job[0]));
	if ((rc = lc_timer(job[0])) != 2)
		fprintf(ofp, "%s exp 2 got %d %d %s\n", 
				featname, rc, __LINE__, lc_errstring(job[0]));
/*
 *	Replace "dongle"
 */
	rename("hostid.sav", "hostid.txt");
	l_select_one(0, -1, 3000); /* sleep 3 sec */
	if ((rc = lc_timer(job[0])) != 0)
		fprintf(ofp, "%s exp 0 got %d %d %s\n", 
				featname, rc, __LINE__, lc_errstring(job[0]));
/*
 *	Remove "dongle"
 */
	rename("hostid.txt", "hostid.sav");
	l_select_one(0, -1, 3000); /* sleep 3 sec */
	if ((rc = lc_timer(job[0])) != 1)
		fprintf(ofp, "%s exp 1 got %d %d %s\n", 
				featname, rc, __LINE__, lc_errstring(job[0]));
/*
 *	Replace "dongle"
 */
	rename("hostid.sav", "hostid.txt");
	l_select_one(0, -1, 3000); /* sleep 3 sec */
	if ((rc = lc_timer(job[0])) != 0)
		fprintf(ofp, "%s exp 0 got %d %d %s\n", 
				featname, rc, __LINE__, lc_errstring(job[0]));
	fprintf(ofp, "\t(End ignore errors----- )\n");
}
void
create_optsf()
{
  FILE *fp;

#if 0
	if (!(fp = fopen("demof.opt", "w")))
	{
		perror("Cannot open demof.opt");
		return;
	}
	fprintf(fp, "COMM_TRANSPORT UDP\n");
	fclose(fp);
#endif
	return;
}


char **
featlist_test(job, line)
LM_HANDLE *job;
int line;
{
 char **featlist;

	featlist = lc_feat_list(job, 0, 0);
	if (!featlist || strcmp(featlist[0], "alt"))
		fprintf(ofp,  "lc_feat_list failed line %d: %s\n", 
					line, lc_errstring(job));
	return featlist;
}
st_reread()
{
  LM_HANDLE *job;
  char buf[50];
  int i;

#if 0
	puts("do reread"); gets(buf); 
#else
	lc_init((LM_HANDLE *)0, "demo", &code, &job);
	lc_set_attr(job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	sprintf(buf, "%d@localhost", LMGRD_PORT_START + 1);
	lc_set_attr(job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	if (la_reread(job, 0, 0, 0) <= 0) lc_perror(job, "la_reread failed");
	if (getenv("REREAD_PAUSE"))
	{
		sscanf(getenv("REREAD_PAUSE"), "%d", &i);
		fprintf(ofp, "sleeping %d seconds for reread\n", i);
		sleep(i);
	}
	lc_free_job(job);
#endif

}
