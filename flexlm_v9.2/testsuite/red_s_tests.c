/******************************************************************************

	    COPYRIGHT (c) 1993, 2003 by Macrovision Corporation.
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
 *	Module: $Id: red_s_tests.c,v 1.12 2003/01/13 22:55:15 kmaclean Exp $
 *
 *	Function: red_s_tests
 *
 *	Description: Test suite for redundant configurations 
 *
 *	Tests performed:
 *
 *	D. Birns
 *	11/24/93 - Adapted from one_server_tests.c
 *
 *	test 1 - start 3 servers, checkout a license.
 *
 *	test 2 - kill master, checkout a license
 *
 *	test 3 - checkout a license, kill master, make sure client connects to
 *		 new server, and license remains checked out
 *
 *	test 4 - same as 3, but restart old master, kill new master, and make
 *		 sure client reconnects and remains checked out.
 *
 *	test 5 - lose quorum, make sure client dies.
 *
 *	test 6 - same as test 5, but add one more server so quorum is 
 *		 reinstated, and checkout license
 *
 *	test 7 - make master server in license file unreachable, start 
 *		 remaining two servers, checkout license
 *
 *
 *	Last changed:  1/6/99
 *
 */
#ifndef lint
static char *sccsid = "@(#) red_s_tests.c:v1.29";
#endif

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_attr.h"
#include "code.h"
#include "lmprikey.h"
#include <stdio.h>
#include <sys/types.h> /* for pid_t */
#include "testsuite.h"
#define HOST1 "flexlm_one"
FILE *ofp;
#define HOST2 "flexlm_two"
#define HOST3 "flexlm_three"
#define HOSTBAD "flexlm_nowhere"
#define LOCK1 "/usr/tmp/lockone"
#define LOCK2 "/usr/tmp/locktwo"
#define LOCK3 "/usr/tmp/lockthree"
#ifdef SUNOS5
#define SLEEP_MULTIPLIER 2.0
#else
#ifdef HP
#define SLEEP_MULTIPLIER 1.0
#else
#define SLEEP_MULTIPLIER 0.25
#endif
#endif
LM_DATA;
LM_HANDLE *job;


LM_VD_FEATURE_INFO fi;
char *hosts1[] = { HOST1, HOST2, HOST3, 0};
char *hosts2[] = { HOSTBAD, HOST2, HOST3, 0};
char *hosts3[] = { HOST2, HOST1, HOST3, 0};



char * replace lm_args(( char *str, char *, char *));
#define VER "1.0"
#define RCF1 "rlicense.dat1"
#define RCF2 "rlicense.dat2"
#define RCF3 "rlicense.dat3"
#define RLOG1 "_rlog1" /*log files for daemons*/
#define RLOG2 "_rlog2"
#define RLOG3 "_rlog3"
VENDORCODE ckout_code;

char *ts_Progname;
char *delim =
     "---------------------------------------------------------------------\n";
HOSTTYPE *hosttype;
#define strsave(x) { char *_p = malloc(strlen(x)+1); strcpy(_p, x); x = _p; }

char *argv_0;
usage()
{
	fprintf(stderr,"usage: %s [-vendord path]\n",ts_Progname);
}
static 
void
checkhosts_exit()
{
#if 1 /*ndef PC*/
	fputs("redundant hosts missing from /etc/hosts, exiting", stderr);
	fputs("Add the following lines to /etc/hosts:", stderr);
	fputs("   127.0.0.1	localhost loghost flexlm_one flexlm_two flexlm_three", stderr);
	fputs("   192.156.198.254 flexlm_nowhere", stderr);
	exit(1);
#endif /* PC */
}
static
void
checkhosts(host)
char *host;
{
#ifdef PC
  static int init;
  WORD WinsockVerRequested;
  WSADATA wsadata;

 	if (!init)
	{
		init = 1;
                WinsockVerRequested=MAKEWORD(2, 0);
		WSAStartup(WinsockVerRequested, &wsadata);
	}
#endif

	if (!gethostbyname(host)) 
		checkhosts_exit();
}

void red_sleep(units)
{
	(void)l_select_one (0, -1, units * 1000);
}
void outahere(sig)
{
	printf("red_s_tests timed out, exiting...\n");
	exit(1);
}
	

long server_pids[3];

int got_callback;
void
callback(i)
int i;
{
	got_callback = 1;
}

main(argc, argv)
int argc;
char *argv[];
{
  char demopath[LM_MAXPATHLEN];
  char *vendord= (char *) NULL;
  char *hostid, *l_asc_hostid();
  HOSTID *id;
  int rc;
  int i;
  int t;
  int dotest = -1;
  int subtest = -1;
  char *us;
  extern int do_dump;
  FILE *fp;
  VENDORCODE vc;


	do_dump = 0;
	setenv("NO_FLEXLM_HOSTNAME_ALIAS", "");
	setenv("LMGRD", "lmgrd_r");
	argv_0 = argv[0];
#ifndef PC
	setlinebuf(stdout);
	setlinebuf(stderr);
#endif /* PC */
	if (argc > 1) 
	{
		sscanf(argv[1], "%d", &dotest);
	}

	LM_CODE_GEN_INIT(&code);
	lm_init("demo", &code, &lm_job);
	lc_set_registry(lm_job, "DEMO_LICENSE_FILE", 0);
	lc_set_registry(lm_job, "LM_LICENSE_FILE", 0);
	lc_alarm(lm_job, outahere, 0, 15 *60 * 1000);
	hosttype = lm_hosttype(0);

	ts_Progname = strrchr(argv[0],'/');
	if (ts_Progname) ts_Progname++;
	else ts_Progname=argv[0];

	id = lm_gethostid();
	hostid = l_asc_hostid(lm_job, id);
	strsave(hostid);

/*
 * -----------------------------------------------------------------------------
 *	Preliminary - Create license files
 */

	if (dotest < 0 || dotest == 0)
	{
		(void) getwd(demopath, LM_MAXPATHLEN-2);
		strcat(demopath, "/demo_r");
		vendord = demopath;

		checkhosts(HOST1);
		checkhosts(HOST2);
		checkhosts(HOST3);
		checkhosts(HOSTBAD);
		(void) printf(delim);
		(void) printf("Creating license files for host id %s\n",hostid);
		(void) printf(delim);
		if (!(fp = fopen("opt1", "w")))
		{
			perror("Can't open opt1");
			exit(1);
		}
		fprintf(fp, "REPORTLOG +redserv1.log\n");
		fclose(fp);
		if (!(fp = fopen("opt2", "w")))
		{
			perror("Can't open opt2");
			exit(1);
		}
		fprintf(fp, "REPORTLOG +redserv2.log\n");
		fclose(fp);
		if (!(fp = fopen("opt3", "w")))
		{
			perror("Can't open opt3");
			exit(1);
		}
		fprintf(fp, "REPORTLOG +redserv3.log\n");
		fclose(fp);
		{
		  char lic[2000];
		  char *cp;
		  char *licstr, *err;
		  FILE *fp;
			sprintf(lic, 
"SERVER flexlm_one %s 2837\n\
SERVER flexlm_two %s 2937\n\
SERVER flexlm_three %s 3037\n\
VENDOR demo ./demo_r opt1\n\
FEATURE f1 demo 1.0 01-jan-2010 9 0 SIGN=0 SIGN2=0 HOSTID=%s\n",
			hostid, hostid, hostid, hostid);
			if (lc_cryptstr(lm_job, lic, &licstr, &code, 
					LM_CRYPT_FORCE, "string", &err))
			{
				printf(
				"cryptstr failed line %d %s %s sig 0x%x\n", 
					__LINE__, err ? err : "", 
					lc_errstring(lm_job), VENDOR_KEY5);
			}
			fp = fopen(RCF1, "w");
			fputs(licstr, fp);
			fclose(fp);
			cp = replace(licstr, "opt1", "opt2");
			fp = fopen(RCF2, "w");
			fputs(cp, fp);
			fclose(fp);
			cp  = replace(licstr, "opt1", "opt3");
			fp = fopen(RCF3, "w");
			fputs(cp, fp);
			fclose(fp);
		}	
#if 0
		/* make rlicense.dat1b same as rlicense.dat1 except order */
		system("sed -n 2p < rlicense.dat1 > rlicense.dat3"); 
		system("sed 2d < rlicense.dat1 >> rlicense.dat3"); 
#endif
		rm(RLOG1);
		rm(RLOG2);
		rm(RLOG3);
		rm(LOCK1);
		rm(LOCK2);
		rm(LOCK3);
	}

#if 0
    if (dotest < 0 || dotest == 1)
    {
     long start;
/*
 * -----------------------------------------------------------------------------
 *	1	Basic running -- start servers, checkout a license
 */
	lc_new_job(0, 0, &ckout_code, &job);
	printf(delim);
	printf("1 - Basic running -- start servers, checkout a license\n");
	printf(delim);
	setenv("DEBUG_HOSTNAME", "flexlm_one");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock1");
	server_pids[0] = run_server_nowait(RCF1, "1 server1", RLOG1);

	setenv("DEBUG_HOSTNAME", "flexlm_two");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock2");
	server_pids[1] = run_server_nowait(RCF2, "1 server2", RLOG2);

	setenv("DEBUG_HOSTNAME", "flexlm_three");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock3");
	server_pids[2] = run_server(RCF2, "1 server3", RLOG3);
	/*setenv("LM_LICENSE_FILE", "rlicense.dat1");*/
	setenv("LM_LICENSE_FILE", "2837@flexlm_one,2937@flexlm_two,3037@flexlm_three");
	red_sleep(10); /*Give it time to respond*/
	lc_set_attr(job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5);
	if (lc_checkout(job, FEATURE, VER, 1, 0, &ckout_code, LM_DUP_NONE)) 
	{
		printf("checkout failed, _lm_errno = %d %s, line %d\n", 
			_lm_errno, lc_errstring(job), __LINE__);
	}
	lc_checkin(job, FEATURE, 1);
	printf(delim);
	printf("1A - checkout/in stress test ~70 seconds\n");
	printf(delim);
	start = time(0);
	lc_free_job(job);
/*
 *	assumes 5 second timer on servers
 */
	while ((time(0) - start) < 70)
	{
		if (lc_new_job(0, 0, &ckout_code, &job))
		{
			printf("init error %s, line %d\n", 
				lc_errstring(job), __LINE__);
			continue;
		}
		if (lc_checkout(job, FEATURE, VER,1, 0, &ckout_code, LM_DUP_NONE)) 
		{
			printf("checkout error %s, line %d\n", 
				lc_errstring(job), __LINE__);
			continue;
		}
		lc_checkin(job, FEATURE, 1);
		lc_free_job(job);
	}

	kill_server();
    }

/*
 * -----------------------------------------------------------------------------
 *	2	-- start servers, kill master, checkout license
 */
    if (dotest < 0 || dotest == 2)
    {
	printf(delim);
	printf("2 - start servers, kill master, checkout license\n");
	printf(delim);
	setenv("DEBUG_HOSTNAME", "flexlm_one");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock1");
	server_pids[0] = run_server_nowait(RCF1, "2 server1", RLOG1);

	setenv("DEBUG_HOSTNAME", "flexlm_two");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock2");
	server_pids[1] = run_server_nowait(RCF2, "2 server2", RLOG2);

	setenv("DEBUG_HOSTNAME", "flexlm_three");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock3");
	server_pids[2] = run_server(RCF2, "2 server3", RLOG3);

	red_sleep(10); /*Give it time to respond*/
	kill_server_pid(server_pids[0]);
	red_sleep(25); /*Give it time to respond*/

	setenv("LM_LICENSE_FILE", "rlicense.dat1");
	l_flush_config(lm_job);
	if (lm_checkout(FEATURE, VER, 1, 0, &code, LM_DUP_NONE)) {
		printf("checkout failed, _lm_errno = %d %s, line %d\n", 
			_lm_errno, lc_errstring(lm_job), __LINE__);
	}

	kill_server();
    }


/*
 * -----------------------------------------------------------------------------
 *	3	-- checkout a license, kill master, make sure client still
 *		   checked out.
 */
    if (dotest < 0 || dotest == 3)
    {
	printf(delim);
	printf("3 - checkout a license, kill master, make sure client still");
	printf(" checked out.\n");
	printf(delim);

	test3("3");

	kill_server();
	kill_clients();
    }

/*
 * -----------------------------------------------------------------------------
 *	test 4 - same as 3, but restart old master, kill new master, and make
 *		 sure client reconnects and remains checked out.
 */
    if (dotest < 0 || dotest == 4)
    {
	char str[100];
	printf(delim);
	printf("4 - same as 3, but restart old master, kill new master, \n");
	printf("    and make sure client reconnects and remains ");
	printf("checked out.\n");
	printf(delim);

	test3("4");

/*
 *	start server 1 again
 */
	setenv("DEBUG_HOSTNAME", "flexlm_one");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock1");
	server_pids[0] = run_server(RCF1, "4, part 2", RLOG1);
	red_sleep(15); /*Give it time to respond*/

/*
 *	kill second/master server -- server1 should now be assigned master
 */
	kill_server_pid(server_pids[1]);

	red_sleep(20); /*Give it time to respond*/
	if (!r_get_feat_info(FEATURE, __LINE__, &fi))
	{
		printf("Server not supporting %s?\n", FEATURE);
		return;
	}
	if (fi.tot_lic_in_use != 1)
	{
		printf("Error, Expected %d, got %d line %d\n",
			1, fi.tot_lic_in_use, __LINE__);
	}

	kill_server();
	kill_clients();
    }
/*
 * -----------------------------------------------------------------------------
 *	test 5 - lose quorum -- make sure child dies.
 */
    if (dotest < 0 || dotest == 5)
    {
	int i;
	LM_HANDLE *job5;
	got_callback = 0;
	printf(delim);
	printf("5 - lose quorum -- make sure child dies.\n");
	printf("    you will see \"Lost license...\" -- ignore\n");
	printf(delim);

/*
 *	start 3 servers
 */
	setenv("DEBUG_HOSTNAME", "flexlm_one");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock1");
	server_pids[0] = run_server_nowait(RCF1, "5 server1", RLOG1);

	setenv("DEBUG_HOSTNAME", "flexlm_two");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock2");
	server_pids[1] = run_server_nowait(RCF2, "5 server2", RLOG2);

	setenv("DEBUG_HOSTNAME", "flexlm_three");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock3");
	server_pids[2] = run_server(RCF3, "5 server3", RLOG3);

	l_select_one(0, -1, 10000);
/*
 *	start client
 */
	if (lc_new_job(0, 0, &ckout_code, &job5))
	{
		printf("lc_init failed %d %s\n", __LINE__, lc_errstring(job5));
	}
	lc_set_attr(job5, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)-1);
	lc_set_attr(job5, LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)-1);
	lc_set_attr(job5, LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)2);
	lc_set_attr(job5, LM_A_USER_EXITCALL, (LM_A_VAL_TYPE)callback);
	if (lc_checkout(job5, "f1", "1.0", 1, 0, &ckout_code, LM_DUP_NONE))
	{
		printf("error line %d %s\n", __LINE__, 
			lc_errstring(job5));
	}
/*	
 *	lose quorum
 */
	kill_server_pid(server_pids[0]);
	kill_server_pid(server_pids[1]);
	for (i = 0; (i < 15) && !got_callback; i++)
	{
		printf("i is %d\n", i);
		l_select_one(0, -1, 2000);
		lc_timer(job5);
	}
	if (!got_callback)
	{
		printf("lost quorum, client didn't die, line %d\n", __LINE__);
		
	}
	kill_server_pid(server_pids[2]);
	lc_free_job(job5);
	
    }

/*
 * -----------------------------------------------------------------------------
 *	test 6 - lose quorum -- make sure child dies, then  restart
 *		 server, checkout successfully
 */
    if (dotest < 0 || dotest == 6)
    {
	printf(delim);
	printf("6 - lose quorum -- make sure child dies.\n");
	printf("    (ignore \"lost license...\" message\n");
	printf("    then restart server, checkout successfully\n");
	printf(delim);

/*
 *	start 3 servers
 */
	setenv("DEBUG_HOSTNAME", "flexlm_one");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock1");
	server_pids[0] = run_server_nowait(RCF1, "6 server1", RLOG1);

	setenv("DEBUG_HOSTNAME", "flexlm_two");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock2");
	server_pids[1] = run_server_nowait(RCF2, "6 server2", RLOG2);

	setenv("DEBUG_HOSTNAME", "flexlm_three");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock3");
	server_pids[2] = run_server(RCF3, "6 server3", RLOG3);

	red_sleep(10); /*Give it time to respond*/
/*
 *	start client
 */
	lm_set_attr(LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5);
	lm_set_attr(LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)5);
	good_client_line(FEATURE, VER, 5, LM_DUP_NONE, argv_0, 
						"ckout failed", __LINE__);
/*	
 *	lose quorum
 */
	kill_server_pid(server_pids[0]);
	kill_server_pid(server_pids[1]);
/*
 *	make sure client dies
 */
	red_sleep(35);
	for(i=0;i<20;i++)
	{
		if (!is_client()) break;
		red_sleep(5);
	}
	if (i==20)
	{
		printf("lost quorum, client didn't die, line %d\n", __LINE__);
	}

/*
 *	restart server1
 */
	setenv("DEBUG_HOSTNAME", "flexlm_one");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock1");
	server_pids[0] = run_server(RCF1, "6 server1 part 2", RLOG1);
	red_sleep(45);
/*
 *	start client
 */
	good_client_line(FEATURE, VER, 5, LM_DUP_NONE, argv_0, 
						"ckout failed", __LINE__);

	kill_clients();
	kill_server();
	
    }

    if (dotest < 0 || dotest == 7)
    {
/*
 * -----------------------------------------------------------------------------
 *	7	first server in list is not up, make sure you can checkout
 */
	printf(delim);
	printf("7 - first server in list is not up, make sure you can checkout\n");
	printf(delim);
	setenv("DEBUG_HOSTNAME", "flexlm_nowhere");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock1");
	server_pids[0] = run_server_nowait(RCF2, "7 bad server 1", RLOG1);

	setenv("DEBUG_HOSTNAME", "flexlm_two");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock2");
	server_pids[1] = run_server_nowait(RCF2, "7 server2", RLOG2);

	setenv("DEBUG_HOSTNAME", "flexlm_three");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock3");
	server_pids[2] = run_server(RCF2, "7 server3", RLOG3);
	setenv("LM_LICENSE_FILE", "rlicense.dat2");
	l_flush_config(lm_job);
	red_sleep(30); /*Give it time to respond*/
	if (lm_checkout(FEATURE, "1.0", 1, 0, &code, LM_DUP_NONE)) 
	{
		printf("checkout failed, _lm_errno = %d %s, line %d\n", 
			_lm_errno, lc_errstring(lm_job), __LINE__);
	}
	kill_server();
    }
    if (dotest < 0 || dotest == 8)
    {
/*
 * -----------------------------------------------------------------------------
 *	8	TCP/UDP with second server as master
 */
	printf(delim);
	printf("8 - TCP/UDP, with second server as master\n");
	printf(delim);
	setenv("DEBUG_HOSTNAME", "flexlm_one");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock1");
	server_pids[0] = run_server_nowait(RCF1, "8 server1", RLOG1);

	setenv("DEBUG_HOSTNAME", "flexlm_two");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock2");
	server_pids[1] = run_server_nowait(RCF2, "8 server2", RLOG2);

	setenv("DEBUG_HOSTNAME", "flexlm_three");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock3");
	server_pids[2] = run_server(RCF3, "8 server3", RLOG3);

	lm_set_attr(LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lm_set_attr(LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)RCF1);
	lm_set_attr(LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)LM_TCP);
	lm_sleep(10);
	if (lm_checkout(FEATURE, "1.0", 1, 0, &code, LM_DUP_NONE)) 
	{
		printf("checkout failed, _lm_errno = %d %s, line %d\n", 
			_lm_errno, lc_errstring(lm_job), __LINE__);
	}
	lm_init("demo", &code, &lm_job);
	lc_set_registry(lm_job, "DEMO_LICENSE_FILE", 0);
	lc_set_registry(lm_job, "LM_LICENSE_FILE", 0);
	lm_set_attr(LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lm_set_attr(LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)RCF1);
	lm_set_attr(LM_A_COMM_TRANSPORT, (LM_A_VAL_TYPE)LM_UDP);
	if (lm_checkout(FEATURE, "1.0", 3, 0, &code, LM_DUP_NONE)) 
	{
		printf("checkout failed, _lm_errno = %d %s, line %d\n", 
			_lm_errno, lc_errstring(lm_job), __LINE__);
	}
	kill_server();
    }
    if (dotest < 0 || dotest > 8)
    {
/*
 * -----------------------------------------------------------------------------
 *	Put things back to "normal"
 */
	(void) printf(delim);
	(void) printf("Cleaning up -- all errors reported above\n");
	(void) printf(delim);
	kill_server();
	rm(RCF1);    
	rm(RCF2);    
	rm(RCF3);    
	cp(RLOG1, "rlog1.sav"); rm(RLOG1);
	cp(RLOG2, "rlog2.sav"); rm(RLOG2);
	cp(RLOG3, "rlog3.sav"); rm(RLOG3);
    }
#endif /* 0 */
    if (dotest <= 7)
	exit(0);
    else
	exit(-1);
}

#if 0
test3(which)
char * which;
{


/*
 *	start 3 servers
 */
	setenv("DEBUG_HOSTNAME", "flexlm_one");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock1");
	printf("1."); fflush(stdout);
	server_pids[0] = run_server_nowait(RCF1, which, RLOG1);

	setenv("DEBUG_HOSTNAME", "flexlm_two");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock2");
	printf("2."); fflush(stdout);
	server_pids[1] = run_server_nowait(RCF2, which, RLOG2);

	setenv("DEBUG_HOSTNAME", "flexlm_three");
	setenv("DEBUG_LOCKFILE", "/usr/tmp/lock3");
	printf("3."); fflush(stdout);
	server_pids[2] = run_server(RCF3, which, RLOG3);

	printf("4,"); fflush(stdout);
	red_sleep(30); /*Give it time to respond*/


/*
 *	start client -- checkout 5
 */
	lc_new_job(0, 0, &ckout_code, &job); /*new job*/
	setenv("LM_LICENSE_FILE", "rlicense.dat1");
	lc_set_registry(job, "DEMO_LICENSE_FILE", 0);
	lc_set_registry(job, "LM_LICENSE_FILE", 0);
	lc_set_attr(job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5);
	lc_set_attr(job, LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)5);
	lc_set_attr(job, LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)10);

	printf("5."); fflush(stdout);
	if (lc_checkout(job, FEATURE, VER, 1, LM_CO_NOWAIT, &code, LM_DUP_NONE))
	{
		printf("Checkout failed line %d %s\n", __LINE__, lc_errstring(job));
	}
	
	printf("6,"); fflush(stdout);
	red_sleep(1);

/*
 *	kill 1st/master server
 */
	printf("7."); fflush(stdout);
	kill_server_pid(server_pids[0]);
	printf("8,"); fflush(stdout);
	red_sleep(10); /*Give it time to respond*/
	printf("9."); fflush(stdout);
	if (!r_get_feat_info(FEATURE, __LINE__, &fi))
	{
		printf("Server not supporting %s?\n", FEATURE);
		return;
	}
	printf("10."); fflush(stdout);
	if (fi.tot_lic_in_use != 1)
	{
		printf("Error, Expected %d, got %d line %d\n",
			1, fi.tot_lic_in_use, __LINE__);
	}
	puts("");
	lc_free_job(job);
}
#endif /* 0 */
r_get_feat_info(feat, line, fi)
char * feat;
int line;
LM_VD_FEATURE_INFO *fi;
{
  CONFIG *conf;
  CONFIG *pos = (CONFIG *)0;
  int found = 0;
  LM_HANDLE *job;
  int ret = 0;

	lc_init((LM_HANDLE *)0, "demo", &code, &job);
	while (conf = lc_next_conf(job, feat, &pos))
	{
		fi->type = LM_VD_FEATINFO_HANDLE_TYPE; fi->feat = conf;
		if (!lc_get_attr(job, LM_A_VD_FEATURE_INFO, (short *)fi))
		{
			ret = 1; /* return the first pool */
			break;
		}
		if (lc_get_errno(job) != LM_NOSERVSUPP) /* real error */
			break;
	}
	if (!ret)
	{
		printf("%s (%s) not in server, line %d: %s\n", feat, 
			conf ? conf->code : "" , line,
			lc_errstring(job));
	}
	lc_free_job(job);
	return ret;
}
char *
replace(str, opatt, npatt)
char *str, *opatt, *npatt;
{
  char *cp;
  static char ret[2000];
  int i, len;
	
	strcpy(ret, str);

	cp = strstr(ret, opatt);
	len = strlen(npatt);
	for (i = 0; i < len; i++)
		cp[i] = npatt[i];
	return(ret);
}
