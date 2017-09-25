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
 *	Module: $Id: st_bugs.c,v 1.63 2003/03/07 23:20:51 kmaclean Exp $
 *	Function: servtest
 *
 *	Description: More Test suites for configurations with a single-host 
 *			no child processes
 *
 *
 *	Tests performed:
 *
 *	D. Birns
 *	6/19/96
 *
 *	Last changed:  10/22/98
 *
 */

#ifndef LM_INTERNAL
#define LM_INTERNAL
#endif /* LM_INTERNAL */
#include "lmachdep.h"
#include "lmclient.h"
#include "lmpolicy.h"
#include "lm_attr.h"
#include "lsmaster.h"
#include "lmprikey.h"
#ifdef FREE_VERSION
#include "free.h"
LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
#else
#define SIG VENDOR_KEY5
#include "code.h"
#define TEST_DEMOF
#endif
#include "l_prot.h"
#ifdef PC		 
#include <io.h>		    
#include <direct.h>
#include <time.h>
#ifndef WINNT
#include <stdlib.h>
#ifndef OS2
#include <lzexpand.h>
#endif /* OS2 */
#endif /* WINNT */	    
#endif /* PC */
#ifndef WINNT
#include <string.h>
#endif


#ifdef USE_WINSOCK	    
#include <winsock.h>	    
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
extern LM_HANDLE *job[];
extern LM_VD_FEATURE_INFO fi;
extern LM_VD_GENERIC_INFO gi;
extern int bugnumber;
#ifdef VMS
#define PORT 200
#define RPORT -1
#else
#define PORT 2837
#define RPORT PORT
#endif /* VMS */

static void bug1414 lm_args((lm_noargs));
static do_bug3188 lm_args((char *));
extern FILE *ofp;

#define LC_REMOVE_MIN (15 * 1000)


static
void
bug973(close_order)
int close_order;
{
 int i;
 char *feat = "f3";
 char vd[2];
 long t;
 char *ret;
#define LINGER_TIME 5

	serv_log("  ==>bug 973\n");
	ts_lc_new_job(&code, &job[3], __LINE__);  /* for non-checkouts */
/*
 *	Make 2 clients
 */
	for (i=0;i<2;i++)
	{
		sprintf(vd, "%d", i);
		ts_lc_new_job(&code, &job[i], __LINE__); 

/*
 *		Each client checks out the same feature with a linger
 *		And a different CHECKOUT-DATA
 */
		lc_set_attr(job[i], LM_A_LINGER, (LM_A_VAL_TYPE)LINGER_TIME);
		lc_set_attr(job[i], LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)vd);
		if (lc_checkout(job[i], feat, "1.0", 1, 0, &code, 
							LM_DUP_VENDOR))
			fprintf(ofp, "%s failed job[%d], line %d:%s\n", feat, 
				i, __LINE__, lc_errstring(job[i]));
	}
/*
 *	Verify status
 */
	if (get_feat_info(feat, __LINE__, &fi, job[3]) && 
		(fi.tot_lic_in_use != 2 || fi.user_cnt != 2))
			fprintf(ofp, "Error line %d, exp 2/2 got %d/%d\n", __LINE__, 
					fi.tot_lic_in_use, fi.user_cnt);
/*
 *	Both clients exit
 */
	t = time(0);
	if (close_order == 0)
	{
		lc_free_job(job[0]);
		lc_free_job(job[1]);
	}
	else
	{
		lc_free_job(job[1]);
		lc_free_job(job[0]);
	}
	get_feat_info(feat, __LINE__, &fi, job[3]) ;
	if (fi.user_cnt != 2 || fi.tot_lic_in_use != 2)
		fprintf(ofp, "Error line %d, exp 2/2 got %d/%d\n", __LINE__, 
						fi.tot_lic_in_use, fi.user_cnt);

/* 
 * 	Wait until the linger is over
 */
	l_select_one(0, -1, LC_REMOVE_MIN ); 
/* 
 * 	flush server
 */
	ts_flush_server(job[3], __LINE__);
	get_feat_info(feat, __LINE__, &fi, job[3]) ;
	if (fi.user_cnt)
	{
		fprintf(ofp, "Error line %d, exp 0/0 got %d/%d\n", __LINE__, 
						fi.tot_lic_in_use, fi.user_cnt);
		fprintf(ofp, "Waiting to free license...\n");
		if (lc_remove(job[3], feat, lc_username(job[3], 1), 
					lc_hostname(job[3],1),
					lc_display(job[3],1)))
			fprintf(ofp, "lc_remove failed line%d:%s\n", __LINE__, 
						lc_errstring(job[3]));
		while(fi.tot_lic_in_use != 0)
		{
			ts_flush_server(job[3], __LINE__);
			l_select_one(0, -1, 5000); /* sleep 5 seconds */
			get_feat_info(feat, __LINE__, &fi, job[3]);
		}
	}
	lc_free_job(job[3]);
}
static
void
bug1304()
{
 char *feat1 = "bug1304_1";
 char *feat2 = "bug1304_2";
	serv_log("  ==>bug 1304\tTIMEOUT 2 features from the same job\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  /* for non-checkouts */
	if (lc_checkout(job[0], feat1, "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "%s failed, line %d:%s\n", feat1, __LINE__, 
							lc_errstring(job[0]));
	if (lc_checkout(job[0], feat2, "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "%s failed, line %d:%s\n", feat2, __LINE__, 
						lc_errstring(job[0]));
	if (lc_checkout(job[0], "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "%s failed, line %d:%s\n", "f1", __LINE__, 
						lc_errstring(job[0]));
	l_select_one(0, -1, 11000);  /* wait for timeout (11 seconds) */
	ts_lc_new_job(&code, &job[3], __LINE__);  /* for non-checkouts */
	ts_flush_server(job[3], __LINE__);
	ts_lc_new_job(&code, &job[1], __LINE__);  /* for non-checkouts */
	if (lc_checkout(job[1], feat1, "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "%s failed, line %d:%s\n", feat1, 
				__LINE__, lc_errstring(job[1]));
	if (lc_checkout(job[1], feat2, "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "%s failed, line %d:%s\n", feat2, 
				 __LINE__, lc_errstring(job[1]));
	lc_free_job(job[1]);
	/* make sure job[0] can reconnect! */
	if (lc_timer(job[0])) lc_perror(job[0], "timer");
	if (lc_timer(job[0])) lc_perror(job[0], "timer");
	if (lc_timer(job[0])) lc_perror(job[0], "timer");
	if (lc_checkout(job[0], "f2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "%s failed, line %d:%s\n", "f1", __LINE__, 
						lc_errstring(job[0]));
	lc_free_job(job[0]);
	lc_free_job(job[3]);
}
static
void
bug1459()
{
 int i;
 char *feat = "dup_19_1";
	serv_log("  ==>bug 1459\tlmremove can orphan brothers\n");
	ts_lc_new_job(&code, &job[3], __LINE__);  /* for non-checkouts */
/*
 *	3 clients checkout feature -- all 3 are dup'd 
 */
	for (i=0;i<3;i++)
	{
		ts_lc_new_job(&code, &job[i], __LINE__);  /* for non-checkouts */
		lc_set_attr(job[i], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5);
		lc_set_attr(job[i], LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)5);
		if (lc_checkout(job[i], feat, "1.0", 1, 0, &code, LM_DUP_USER))
			fprintf(ofp, "%s failed job[%d], line %d:%s\n", feat, 
							i, __LINE__, 
						lc_errstring(job[i]));
	}
/*
 *	Make sure that there's one license used, and 3 users in the server
 */
	if (get_feat_info(feat, __LINE__, &fi, job[0]) && 
		(fi.tot_lic_in_use != 1 || fi.user_cnt != 3))
			fprintf(ofp, "Error line %d, exp 1/3 got %d/%d\n", __LINE__, 
					fi.tot_lic_in_use, fi.user_cnt);
/*
 *	wait 15 seconds so we can use lc_remove 
 */
	l_select_one(0, -1, LC_REMOVE_MIN); 
	if (lc_remove(job[3], feat, lc_username(job[3], 1), 
				lc_hostname(job[3],1),
				lc_display(job[3],1)))
		fprintf(ofp, "lc_remove failed line%d:%s\n", __LINE__, 
							lc_errstring(job[3]));
/*
 *	Make sure there's 1 lic-in-use and 2 users now.
 */
	if (get_feat_info(feat, __LINE__, &fi, job[3]) && 
		(fi.tot_lic_in_use != 1 || fi.user_cnt != 2))
			fprintf(ofp, "Error line %d, exp 1/2 got %d/%d\n", __LINE__, 
					fi.tot_lic_in_use, fi.user_cnt);
/*
 *	Kill the user whose license was removed 
 */
	lc_free_job(job[0]);

	ts_flush_server(job[3], __LINE__);
/* 
 *	Kill all clients
 */
	lc_free_job(job[1]);
	lc_free_job(job[2]);
/*
 *	Make sure server is back to 0
 */
	l_select_one(0, -1, 1000); /* 1 second */
	if (get_feat_info(feat, __LINE__, &fi, job[3]) && 
		(fi.tot_lic_in_use !=0 || fi.user_cnt != 0))
	{
			fprintf(ofp, "Error line %d, exp 0/0 got %d/%d\n", __LINE__, 
					fi.tot_lic_in_use, fi.user_cnt);
/*
 *		Try to set the server back so the other tests will
 *		be ok
 */
		for (i=0;i<fi.user_cnt;i++)
			lc_remove(job[3], feat, lc_username(job[3], 1), 
				lc_hostname(job[3],1),
				lc_display(job[3],1));
	}
	lc_free_job(job[3]);

}

static
void
bug1460()
{
  int i;
  char vd_str[2];
  char *feat = "f3";

	serv_log("  ==>bug 1460\tDUP_VENDOR w/ no checkin can be orphaned\n");
	ts_lc_new_job(&code, &job[3], __LINE__);  /* for non-checkouts */
	lc_set_attr(job[3], LM_A_MT_HEARTBEAT, (LM_A_VAL_TYPE)0);
/*
 *	2 clients 
 */
	for (i=0;i<2;i++)
	{
		ts_lc_new_job(&code, &job[i], __LINE__);  /* for non-checkouts */
		lc_set_attr(job[i], LM_A_MT_HEARTBEAT, (LM_A_VAL_TYPE)0);
/*
 *		Each client first checks out feat with vd-data = "a"
 */
		lc_set_attr(job[i], LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"a");
		if (lc_checkout(job[i], feat, "1.0", 1, 0, &code,LM_DUP_VENDOR))
			fprintf(ofp, "%s failed job[%d], line %d:%s\n", feat, 
				i, __LINE__, lc_errstring(job[i]));
/*
 *		... then "b"
 */
		lc_set_attr(job[i], LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"b");
		if (lc_checkout(job[i], feat, "1.0", 1, 0, &code,LM_DUP_VENDOR))
			fprintf(ofp, "%s failed job[%d], line %d:%s\n", feat, 
				i, __LINE__, lc_errstring(job[i]));
	}
	if (get_feat_info(feat, __LINE__, &fi, job[3]) && 
		(fi.tot_lic_in_use !=2 || fi.user_cnt != 4))
			fprintf(ofp, "Error line %d, exp 2/4 got %d/%d\n", __LINE__, 
					fi.tot_lic_in_use, fi.user_cnt);
/*
 *	Both clients exit
 */
	for (i=0;i<2;i++) lc_free_job(job[i]);
/*
 *	Make sure all licenses are freed
 */
	l_select_one(0, -1, 100); 
	
	if (get_feat_info(feat, __LINE__, &fi, job[3]) && 
		(fi.tot_lic_in_use !=0 || fi.user_cnt != 0))
	{
			fprintf(ofp, "Error line %d, exp 0/0 got %d/%d\n", __LINE__, 
					fi.tot_lic_in_use, fi.user_cnt);
#if 0
		l_select_one(0, -1, LC_REMOVE_MIN); 
		for (i=0;i<fi.user_cnt;i++)
			lc_remove(job[3], feat, lc_username(job[3], 1), 
				lc_hostname(job[3],1),
				lc_display(job[3],1));
#endif
	}

	lc_free_job(job[3]);
}
static
void
bug1621()
{
  int i;
  char vd_str[2];
  char *feat = "f3";

	serv_log("  ==>bug 1621\tSUITE checkin releases token when still in use\n");
/*
 *	2 clients 
 */
	ts_lc_new_job(&code, &job[0], __LINE__);  /* for non-checkouts */
	ts_lc_new_job(&code, &job[1], __LINE__);  /* for non-checkouts */
	if (lc_checkout(job[0], "comp1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	if (lc_checkout(job[0], "comp2", "1.0", 1, LM_CO_NOWAIT, &code, 
								LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	lc_checkin(job[0], "comp1", 1);
	if (!lc_checkout(job[1], "comp1", "1.0", 1, LM_CO_NOWAIT, &code, 
								LM_DUP_NONE))
		fprintf(ofp, "ckout succeeded, line %d\n", __LINE__);
	if (!lc_checkout(job[1], "comp2", "1.0", 1, LM_CO_NOWAIT, &code, 
								LM_DUP_NONE))
		fprintf(ofp, "ckout succeeded, line %d\n", __LINE__);
	lc_checkin(job[0], "comp2", 1);
	if (lc_checkout(job[1], "comp1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[1]));
	if (lc_checkout(job[1], "comp2", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[1]));
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}
static
void
bug1545()
{
  CONFIG *conf;
  extern char hostname[];
  char addr[100];
  char *savfilep;
  char savfile[1024];

        serv_log("  ==>bug 1545\tport@host auth_data fails with 100 len for feat\n");
	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	setenv ("LM_LICENSE_FILE", "@localhost");
	ts_lc_new_job(&code, &job[0], __LINE__);  /* for non-checkouts */
	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	if (!(conf = lc_auth_data(job[0], "f1")) || 
					strcmp(conf->feature, "f1"))
		fprintf(ofp, "auth_data failed , line %d: got %s shd be %s\n", 
			__LINE__, conf->feature, "f1");
	lc_free_job(job[0]);
	setenv("LM_LICENSE_FILE", savfile);
}
static
void
bug1652()
{
  CONFIG *conf;
  extern char hostname[];
  char addr[100];
  int i;

	serv_log("  ==>bug 1652\tRESERVE always returned to first license pool\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  /* for non-checkouts */
	ts_lc_new_job(&code, &job[1], __LINE__);  /* for non-checkouts */
/* 
 *	NOTE:  There's supposed to be 2 pools.  get_feat_info
 *	(used only by servtest) returns only the first pool.  Fine.
 *	We'll just do a sanity check on that pool
 */
	for (i=0; i < 2; i++) /* do it twice */
	{
		if (get_feat_info("bug1652", __LINE__, &fi, job[1]) && 
			(fi.tot_lic_in_use != 0 || fi.user_cnt != 0 || 
								fi.res != 1))
			fprintf(ofp, "Error line %d, exp 0/0/1 got %d/%d/%d\n", 
				__LINE__, fi.tot_lic_in_use, fi.user_cnt,
								fi.res);
/* 
 *	checkout 2nd INCREMENT line, the one that's reserved to "us" 
 */
		if (lc_checkout(job[0], "bug1652", "1.0", 1, 0, &code,
							LM_DUP_VENDOR))
			fprintf(ofp, "%s failed line %d:%s\n", "bug1652", __LINE__, 
							lc_errstring(job[0]));
		if (get_feat_info("bug1652", __LINE__, &fi, job[1]) && 
			(fi.tot_lic_in_use != 0 || fi.user_cnt != 0 || 
							fi.res != 1))
			fprintf(ofp, "Error line %d, exp 0/0/1 got %d/%d/%d\n", 
				__LINE__, fi.tot_lic_in_use, fi.user_cnt,
								fi.res);
		lc_checkin(job[0], "bug1652", 0);
		if (get_feat_info("bug1652", __LINE__, &fi, job[1]) && 
			(fi.tot_lic_in_use != 0 || fi.user_cnt != 0 || 
				fi.res != 1))
			fprintf(ofp, "Error line %d, exp 0/0/1 got %d/%d/%d\n", 
				__LINE__, fi.tot_lic_in_use, fi.user_cnt, 
								fi.res);
/* 
 *		Do it again, to make sure...
 */
	}
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}
static
void
bug1414()
{
  FILE *fp;
  char line[100];
  int num ;

	serv_log("  ==>bug 1414\tMake sure license files are < 80 chars per line\n");
	if (!(fp = fopen("servtest.lic", "r")))
	{
		perror("Can't open servtest.lic");
		return;
	}
	for (num = 1; fgets(line, 100, fp); num++)
		if (strlen(line) > 80)
			fprintf(ofp, "Error line %d has length %d\n", strlen(line));
	fclose(fp);
}

static
void
bug1676()
{

	serv_log( "  ==>bug 1676\tCHECKIN 2ce core dumped\n");
#ifdef PC
        serv_log("Test not done on PC -- core dumps...\n");
#else
	if (CHECKOUT(LM_RESTRICTIVE, "f2", "1.0", "."))
	{
		fprintf(ofp, "err LINE %d %s\n", __LINE__, ERRSTRING());
	}
	CHECKIN();
	CHECKIN();
#endif /* PC */
}
static
void
bug1511()
{

	serv_log( "  ==>bug 1511\tlc_checkout fails after TIMEOUT but before heartbeat\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  /* for non-checkouts */
	ts_lc_new_job(&code, &job[1], __LINE__);  /* for non-checkouts */
	lc_set_attr(job[1], LM_A_TCP_TIMEOUT, (LM_A_VAL_TYPE) 1);
	lc_set_attr(job[1], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE) -1);
	if (lc_checkout(job[1], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	if (get_feat_info("f1", __LINE__, &fi, job[0]) &&
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
 *	this is where the bug was -- should reconnect...
 */
	if (lc_checkout(job[1], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[1]));
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}
static
void
bug2144()
{

  char *u, *h, *d;
  int rc;
  int i;
  extern char hostname[];
  LM_USERS *users;

	serv_log("  ==>bug 2144\tlmremove'd license is never recovered\n");
	for (i=0;i<2;i++)
	{
		fprintf(ofp, "	%s\n", i ? "lc_removeh()" : "lc_remove()");
		ts_lc_new_job(&code, &job[0], __LINE__);  /* for non-checkouts */
		ts_lc_new_job(&code, &job[1], __LINE__);  /* for non-checkouts */
		lc_set_attr(job[1], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE) -1);
		if (lc_checkout(job[1], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
								LM_DUP_NONE))
			fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
						lc_errstring(job[0]));
		if (get_feat_info("f1", __LINE__, &fi, job[0]) &&
						(fi.tot_lic_in_use != 1))
			fprintf(ofp, 
			"Error line %d, exp %d got %d\n",
						__LINE__, 1, fi.tot_lic_in_use);
		l_select_one(0, -1, LC_REMOVE_MIN); 
		ts_flush_server(job[0], __LINE__);
		if (i == 0)
		{
			/* use regular lc_remove */
			u = lc_username(job[1], 1);
			h = lc_hostname(job[1], 1); 
			d = lc_display(job[1], 1);
			if (lc_remove(job[0], "f1", u, h, d))
				fprintf(ofp, "error line %d %s\n", __LINE__, 
					lc_errstring(job[0]));
		}
		else
		{
		  char handle[10];
			users = lc_userlist(job[0], "f1");
			/* use lc_removeh */

			sprintf(handle, "%d", users->next->ul_license_handle);
			if (lc_removeh(job[0], "f1", hostname,  -1, handle))
				fprintf(ofp, "error line %d %s\n", __LINE__, 
					lc_errstring(job[0]));
		}
		if (get_feat_info_err("f1", __LINE__, &fi, job[0], 0, 0) &&
						(fi.tot_lic_in_use != 0))
			fprintf(ofp, 
			"Error line %d, exp %d got %d\n",
					__LINE__, 0, fi.tot_lic_in_use);
/*
 *	this is where the bug was -- should reconnect...
 */
		lc_heartbeat(job[1], 0, 0);
		lc_heartbeat(job[1], 0, 0);
		if (get_feat_info("f1", __LINE__, &fi, job[0]) &&
						(fi.tot_lic_in_use != 1))
		{
			fprintf(ofp, 
			"Error line %d, exp %d got %d ",
					__LINE__, 1, fi.tot_lic_in_use);
			lc_heartbeat(job[1], 0, 0);
			fprintf(ofp, "heartbeat returns %s\n", lc_errstring(job[1]));
		}
		lc_free_job(job[0]);
		lc_free_job(job[1]);
	}
}
int
LM_CALLBACK_TYPE
p2206_filter(c)
CONFIG *c;
{
	return(0); 
}

static
void
bug2206()
{
  CONFIG *conf;
  extern char hostname[];
  char addr[100];
  char *savfilep;
  char savfile[1024];

	serv_log("  ==>bug 2206\tport@host w/ CHECKOUT FILTER and no such feature succeeds\n");
	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	setenv ("LM_LICENSE_FILE", "@localhost");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, 
					(LM_A_VAL_TYPE) p2206_filter);
	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	if (!lc_checkout(job[0], "nonexist", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		
		fprintf(ofp, "checkout filter, line %d\n", __LINE__);
	lc_free_job(job[0]);
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, 
					(LM_A_VAL_TYPE) p2206_filter);
	if (!lc_checkout(job[0], "nonexist", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		
		fprintf(ofp, "checkout filter failure, line %d\n", __LINE__);
	lc_free_job(job[0]);
	setenv("LM_LICENSE_FILE", savfile);
}
LM_CODE(code2477, 0x12345eee, 0x54321fea, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
static
void
bug2477()
{
  CONFIG *conf;
  extern char hostname[];
  char addr[100];
  char *savfilep;
  char savfile[1024];

	serv_log( "  ==>bug 2477\n");
	ts_lm_init("demo", &code2477, &job[0], __LINE__); 
	/*lc_set_attr(job[0], LM_A_INTERNAL2, (LM_A_VAL_TYPE)1);*/
	lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	ts_lc_new_job(&code, &job[1], __LINE__);  /* for non-checkouts */
	/*lc_set_attr(job[1], LM_A_INTERNAL2, (LM_A_VAL_TYPE)1);*/
	lc_set_attr(job[1], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	ts_lm_init("demo", &code2477, &job[2], __LINE__); 
	/*lc_set_attr(job[2], LM_A_INTERNAL2, (LM_A_VAL_TYPE)1);*/
	lc_set_attr(job[2], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (lc_checkout(job[1], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		
		fprintf(ofp, "ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[1]));

	/* try the wrong seeds with a file */
	lc_set_attr(job[0], LM_A_RETRY_CHECKOUT, (LM_A_VAL_TYPE)1);
	if (!lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code2477, 
							LM_DUP_NONE) ||
					job[0]->lm_errno != LM_BADCODE)
	{
		
		fprintf(ofp, "ckout succeeded, line %d\n", __LINE__);
		if (job[0]->lm_errno)
			fprintf(ofp, "no it didn't: %s\n", lc_errstring(job[0]));
	}
	/* try the wrong seeds with port @host*/
	lc_set_attr(job[2], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[2], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@localhost");
	lc_set_attr(job[2], LM_A_RETRY_CHECKOUT, (LM_A_VAL_TYPE)1);
	if (!lc_checkout(job[2], "f1", "1.0", 1, LM_CO_NOWAIT, &code2477, 
							LM_DUP_NONE) ||
					job[2]->lm_errno != LM_BADHANDSHAKE)
	{
		
		fprintf(ofp, "ckout succeeded, line %d\n", __LINE__);
		if (job[2]->lm_errno)
			fprintf(ofp, "no it didn't: %s\n", lc_errstring(job[2]));
	}
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
}

static
void
bug2548()
{
  CONFIG *conf;
  extern char hostname[];
  char addr[100];
  char *savfilep;
  char savfile[1024];

	serv_log( "  ==>bug 2548\n");
#ifdef PC
	sprintf(addr, "2222@%s;%d@%s", hostname, LMGRD_PORT_START + 1, hostname);
#else
        sprintf(addr, "2222@%s:%d@%s", hostname, LMGRD_PORT_START + 1, hostname);

#endif
	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	setenv ("LM_LICENSE_FILE", addr);
	ts_lc_new_job(&code, &job[0], __LINE__);  
	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (!(conf = lc_auth_data(job[0], "f1")))
	{
		fprintf(ofp, "authdata failed  %d\n", __LINE__);
		setenv ("LM_LICENSE_FILE", savfile);
		return;
	}

	if (!L_STREQ(conf->feature, "f1"))
		fprintf(ofp, "authdata failed , line %d exp f2 got %s\n", 
		__LINE__, conf->feature);
	fi.feat = conf;
	if (!get_feat_info("f1", __LINE__, &fi, job[0]))
		fprintf(ofp, "featinfo failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (lc_checkout(job[0], "f2", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	conf = lc_auth_data(job[0], "f2");
	if (!L_STREQ(conf->feature, "f2"))
		fprintf(ofp, "authdata failed , line %d exp f2 got %s\n", 
		__LINE__, conf->feature);
	fi.feat = conf;
	if (!get_feat_info("f2", __LINE__, &fi, job[0]))
		fprintf(ofp, "featinfo failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	lc_free_job(job[0]);
	setenv ("LM_LICENSE_FILE", savfile);

}

static
void
bug2269()
{
  int rc;

	serv_log( "  ==>bug 2269\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	ts_lc_new_job(&code, &job[2], __LINE__);  
	if (lc_checkout(job[0], "comp1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (!lc_checkout(job[1], "comp1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout succeeded , line %d %s\n", __LINE__);
	rc = lc_checkout(job[2], "comp1", "1.0", 1, LM_CO_QUEUE, &code, 
							LM_DUP_NONE);
	if (rc != LM_FEATQUEUE && rc != LM_USERSQUEUED)
		fprintf(ofp, "wrong status, expected %d or %d, got %d, line %d\n", 
			LM_FEATQUEUE, LM_USERSQUEUED, rc, __LINE__);
	lc_checkin(job[0], "comp1", 0);
	l_select_one(0, -1, 1000 );  /* sleep 1 second */

	if (lc_status(job[2], "comp1"))
		fprintf(ofp, "queueing failed line %d %s\n", 
			__LINE__, lc_errstring(job[2]));
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
}
static
void
bug2493()
{
  int rc;

	serv_log("  ==>bug 2493\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	ts_lc_new_job(&code, &job[2], __LINE__);  
	ts_lc_new_job(&code, &job[3], __LINE__);  
	if (lc_checkout(job[0], "p2493", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (!lc_checkout(job[1], "p2493", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout succeeded , line %d %s\n", __LINE__);
	rc = lc_checkout(job[2], "p2493", "1.0", 1, LM_CO_QUEUE, &code, 
							LM_DUP_NONE);
	if (rc != LM_FEATQUEUE && rc != LM_USERSQUEUED)
		fprintf(ofp, "wrong status, expected %d or %d, got %d, line %d\n", 
			LM_FEATQUEUE, LM_USERSQUEUED, rc, __LINE__);
	lc_checkin(job[0], "p2493", 0);
	l_select_one(0, -1, 1000 );  /* sleep 1 second */

	if (lc_status(job[2], "p2493"))
		fprintf(ofp, "queueing failed line %d %s\n", 
			__LINE__, lc_errstring(job[2]));
	lc_set_attr(job[3], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"nobody");
	if ((rc = lc_checkout(job[3], "p2493", "1.0", 1, LM_CO_QUEUE, &code, 
					LM_DUP_NONE)) != LM_RESVFOROTHERS)
		fprintf(ofp, "p2493 bug, line %d exp %d, got %d\n", __LINE__,
			LM_RESVFOROTHERS, rc);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
	lc_free_job(job[3]);
}
static char *good_vlines[] = { 
	"VENDOR demo \"/path/to the/demo\" /path/to/options",
	"VENDOR demo \"/path/to the/demo\" /path/to/options 1234",
	"VENDOR demo \"/path/to the/demo\" options=/path/to/options",
	"VENDOR demo options=\"/path/to the/options\"",
	"VENDOR demo options=\"/path/to the/options\" 1234",
	"VENDOR demo port=1234 options=\"/path/to the/options\"" ,
	"VENDOR demo port=1234 ",
	"VENDOR demo dpath port=1234",
	0 };
static char *bad_vlines[] = { 
	"VENDOR demo \"/path/to the/demo\" /path/to/options 1234 1234",
	"VENDOR demo path port=foo",
	"VENDOR demo port=foo options=\"opath\"",
	"VENDOR demo options=\"opath\" options=bado",
	0 };

static
void
bug2484()
{
  struct _daemon *dp;
  char **p;
  char buf[300];
  LM_HANDLE *lm_job;


	for (p = good_vlines; *p; p++)
	{
		sprintf(buf, "%s\n%s\n%s", LM_LICENSE_START, *p, 
			LM_LICENSE_END);
		ts_lc_new_job(&code, &lm_job, __LINE__);  
		lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
		lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
		l_init_file(lm_job);
		if (!(dp = l_get_dlist(lm_job)))
			fprintf(ofp, "Error line %d, %s failed\n", __LINE__, *p);
		l_free_daemon_list(lm_job, dp);
		lc_free_job(lm_job);
	}
	for (p = bad_vlines; *p; p++)
	{
		sprintf(buf, "%s\n%s\n%s", LM_LICENSE_START, *p, 
			LM_LICENSE_END);
		ts_lc_new_job(&code, &lm_job, __LINE__);  
		lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
		lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
		l_init_file(lm_job);
		if (l_get_dlist(lm_job))
			fprintf(ofp, "Error line %d, %s succeeded\n", __LINE__, *p);
		lc_free_job(lm_job);
	}
			


	
}

static
void
bug2781()
{
  char addr[100];
  char *savfilep;
  char savfile[1024];
  HOSTID *h;


	serv_log( "  ==>bug 2781\tport@host checkout succeeds for badhost\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	h = lc_gethostid(job[0]);
	if (h->type == HOSTID_STRING)
	{
		fprintf(ofp, "Skipping this testing because the hostid type id ID_STRING\n");
		return;
	}
	if (!lc_checkout(job[0], "p2781", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "error: ckout succeeded , line %d\n", __LINE__);
	lc_free_job(job[0]);
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@localhost");
	if (!lc_checkout(job[0], "p2781", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "error: ckout succeeded , line %d\n", __LINE__);
	lc_free_job(job[0]);
}

static
void
bug2894()
{
  char addr[100];
  char *savfilep;
  char savfile[1024];
  HOSTID *h;
#ifdef PC
  char *path = "@flexlm_nowhere;@localhost";
#else
  char *path = "@flexlm_nowhere:@localhost";
#endif /* PC */


	serv_log( "  ==>bug 2894 (about a minute)\t@flexlm_nowhere:@localhost -- won't use 2nd file in list\n");
/*
 * 	checkout 
 */
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)path);
	lc_set_attr(job[0], LM_A_CONN_TIMEOUT, (LM_A_VAL_TYPE)3);
	lc_set_attr(job[1], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[1], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)path);
	lc_set_attr(job[1], LM_A_CONN_TIMEOUT, (LM_A_VAL_TYPE)3);
	if (lc_checkout(job[0], "pathtest1", "1.0", 2, 0, &code, LM_DUP_VENDOR))
		fprintf(ofp, "failed line %d:%s\n", __LINE__, lc_errstring(job[0]));
	if (lc_checkout(job[1], "pathtest1", "1.0", 2, 0, &code, LM_DUP_VENDOR))
		fprintf(ofp, "failed line %d:%s\n", __LINE__, lc_errstring(job[1]));
	lc_free_job(job[0]);
	lc_free_job(job[1]);
/*
 * 	get_config
 */
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)path);
	lc_set_attr(job[0], LM_A_CONN_TIMEOUT, (LM_A_VAL_TYPE)3);
	if (!lc_get_config(job[0], "pathtest1"))
		fprintf(ofp, "failed line %d:%s\n", __LINE__, lc_errstring(job[0]));
	lc_free_job(job[0]);
/*
 * 	lc_feat_list
 */
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)path);
	lc_set_attr(job[0], LM_A_CONN_TIMEOUT, (LM_A_VAL_TYPE)3);
	if (!lc_feat_list(job[0], LM_FLIST_ALL_FILES, 0))
		fprintf(ofp, "failed line %d:%s\n", __LINE__, lc_errstring(job[0]));
	lc_free_job(job[0]);
/*
 * 	lc_userlist
 */
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)path);
	lc_set_attr(job[0], LM_A_CONN_TIMEOUT, (LM_A_VAL_TYPE)3);
	if (!lc_userlist(job[0], "pathtest1"))
		fprintf(ofp, "failed line %d:%s\n", __LINE__, lc_errstring(job[0]));
	lc_free_job(job[0]);
}
int co_filter_cnt;
int
LM_CALLBACK_TYPE
co_filter(c)
CONFIG *c;
{
	co_filter_cnt++;
	return(0); 
}
static
void
bug3057()
{
  char addr[100];
  char *savfilep;
  char savfile[1024];
  HOSTID *h;


	serv_log( "  ==>bug 3057\tport@host LM_CO_LOCALTEST incorrectly succeeds when preceded with lc_get_config\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)co_filter);
	if (!lc_get_config(job[0], "badcodefeat"))
		fprintf(ofp, "lc_get_config badcodefeat line %d:%s\n", __LINE__, 
				lc_errstring(job[0]));
	if (!lc_get_config(job[0], "f1"))
		fprintf(ofp, "lc_get_config f1 line %d:%s\n", __LINE__, 
				lc_errstring(job[0]));
	if (!lc_checkout(job[0], "badcodefeat", "1.0", 1, LM_CO_LOCALTEST, 
			&code, LM_DUP_NONE))
		fprintf(ofp, "error: ckout succeeded , line %d\n", __LINE__);
	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_LOCALTEST, 
			&code, LM_DUP_NONE))
		fprintf(ofp, "error: ckout failed , line %d %s\n", __LINE__, 
						lc_errstring(job[0]));
	lc_free_job(job[0]);
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@localhost");
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)co_filter);
	if (!lc_get_config(job[0], "f1"))
		fprintf(ofp, "lc_get_config f1 line %d:%s\n", __LINE__, 
				lc_errstring(job[0]));
	if (!lc_get_config(job[0], "badcodefeat"))
		fprintf(ofp, "lc_get_config badcodefeat line %d:%s\n", __LINE__, 
				lc_errstring(job[0]));
	if (!lc_checkout(job[0], "badcodefeat", "1.0", 1, LM_CO_LOCALTEST, 
						&code, LM_DUP_NONE))
		fprintf(ofp, "error: ckout succeeded , line %d\n", __LINE__);
	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_LOCALTEST, 
			&code, LM_DUP_NONE))
		fprintf(ofp, "error: ckout failed , line %d %s\n", __LINE__, 
						lc_errstring(job[0]));
	if (co_filter_cnt < 4) fprintf(ofp, "error line %d outfilter got %d exp %d\n",
		__LINE__, co_filter_cnt, 4);
	lc_free_job(job[0]);
}
static
void
bug3119()
{
  char addr[100];
  char *savfilep;
  char savfile[1024];
  HOSTID *h;
  time_t startt;


	serv_log( "  ==>bug 3119\tlc_idle(1) can fail\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)-1);
	lc_set_attr(job[0], LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)-1);
	if (lc_checkout(job[0], "f29", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "error: ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (get_feat_info("f29", __LINE__, &fi, job[0]) && 
		(fi.tot_lic_in_use != 1 || fi.user_cnt != 1))
			fprintf(ofp, "Error line %d, exp 1/1 got %d/%d\n", __LINE__, 
				fi.tot_lic_in_use, fi.user_cnt);
	startt = time(0);
	while ((time(0) - startt) <  12)
	{
		lc_idle(job[0], 1);
		lc_heartbeat(job[0], 0, 0);
		l_select_one(0, -1, 1000 );  /* sleep 1 second */
	}
	ts_lc_new_job(&code, &job[1], __LINE__);  
	ts_flush_server(job[1], __LINE__);
	if (get_feat_info("f29", __LINE__, &fi, job[1]) && 
		(fi.tot_lic_in_use != 0 || fi.user_cnt != 0))
			fprintf(ofp, "Error line %d, exp 0/0 got %d/%d\n", __LINE__, 
				fi.tot_lic_in_use, fi.user_cnt);
	lc_idle(job[0], 0);
	lc_heartbeat(job[0], 0, 0);
	lc_heartbeat(job[0], 0, 0);
	lc_heartbeat(job[0], 0, 0);
	if (get_feat_info("f29", __LINE__, &fi, job[1]) && 
		(fi.tot_lic_in_use != 1 || fi.user_cnt != 1))
			fprintf(ofp, "Error line %d, exp 1/1 got %d/%d\n", __LINE__, 
				fi.tot_lic_in_use, fi.user_cnt);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
				
}
static
void
bug3125()
{
  char addr[100];
  char *savfilep;
  char savfile[1024];
  HOSTID *h;
  time_t startt;


	serv_log( "  ==>bug 3125\tUSE_SERVER, but server down, -5 err is wrong\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)
	"START_LICENSE\nSERVER localhost 4321 any\nUSE_SERVER\nEND_LICENSE");
	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE) 
			== LM_NOFEATURE)
		fprintf(ofp, "error: got -5, line %d\n", __LINE__);
	lc_free_job(job[0]);
}
static
void
bug3155()
{
  char addr[100];
  char *savfilep;
  char savfile[1024];
  HOSTID *h;
  time_t startt;
  char *u0 = "u0";
  char *u1 = "u1";
  char *u2 = "u2";
  int rc;


	serv_log( "  ==>bug 3155\tTIMEOUT/QUEUE bugs\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	ts_lc_new_job(&code, &job[2], __LINE__);  
	ts_lc_new_job(&code, &job[3], __LINE__);  
	ts_lc_new_job(&code, &job[4], __LINE__);  
	lc_set_attr(job[0], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)u0);
	lc_set_attr(job[1], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)u1);
	lc_set_attr(job[2], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)u2);
	if (lc_checkout(job[0], "bug1304_1", "1.0", 1, LM_CO_NOWAIT, &code, 
								LM_DUP_NONE) )
		fprintf(ofp, "error: ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if ((rc = lc_checkout(job[1], "bug1304_1", "1.0", 1, LM_CO_QUEUE, 
					&code, LM_DUP_NONE)) != LM_FEATQUEUE)
		fprintf(ofp, 
		"error: ckout result is %d, and should be %d , line %d\n", 
			rc, LM_FEATQUEUE, __LINE__);
	l_select_one(0, -1, 11000 );  /* sleep 10 seconds */
	ts_flush_server(job[3], __LINE__);
	ts_flush_server(job[3], __LINE__);
	if (get_feat_info("bug1304_1", __LINE__, &fi, job[4]) && 
		(fi.tot_lic_in_use != 1 || fi.user_cnt != 1))
			fprintf(ofp, "Error line %d, exp 1/1 got %d/%d\n", __LINE__, 
				fi.tot_lic_in_use, fi.user_cnt);
	lc_heartbeat(job[0], 0,0);
	lc_heartbeat(job[1], 0,0);
	if (!lc_heartbeat(job[0], 0,0) && !lc_heartbeat(job[1],0,0))
		fprintf(ofp, "error: too many licenses out, line %d\n", __LINE__);
	if (lc_heartbeat(job[1], 0,0))
		fprintf(ofp, "error: license not dequeued, line %d: %s\n", __LINE__,
			lc_errstring(job[1]));
	if (get_feat_info("bug1304_1", __LINE__, &fi, job[4]) && 
		(fi.tot_lic_in_use != 1 || fi.user_cnt != 1))
			fprintf(ofp, "Error line %d, exp 1/1 got %d/%d\n", __LINE__, 
				fi.tot_lic_in_use, fi.user_cnt);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
	ts_flush_server(job[3], __LINE__);
	ts_flush_server(job[3], __LINE__);
	lc_free_job(job[3]);
/*
 *	2nd test -- this time with a license for 2
 *			checkout 2, then queue 1, then timeout
 *			first 2
 */
	serv_log( "----------bug 3155 part 2-------------\n");
 
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	ts_lc_new_job(&code, &job[2], __LINE__);  
	ts_lc_new_job(&code, &job[3], __LINE__);  
	lc_set_attr(job[0], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)u0);
	lc_set_attr(job[1], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)u1);
	lc_set_attr(job[2], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)u2);
	if (lc_checkout(job[0], "bug9999", "1.0", 1, LM_CO_NOWAIT, &code, 
					LM_DUP_NONE))
		fprintf(ofp, "error: ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (lc_checkout(job[1], "bug9999", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE) )
		fprintf(ofp, "error: ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (lc_checkout(job[2], "bug9999", "1.0", 1, LM_CO_QUEUE, &code, 
					LM_DUP_NONE) != LM_FEATQUEUE)
		fprintf(ofp, "error: ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	l_select_one(0, -1, 11000 );  /* sleep 10 seconds */
	ts_flush_server(job[3], __LINE__);
	ts_flush_server(job[3], __LINE__);
	if (get_feat_info("bug9999", __LINE__, &fi, job[4]) && 
		(fi.tot_lic_in_use != 1 || fi.user_cnt != 1))
			fprintf(ofp, "Error line %d, exp 2/2 got %d/%d\n", __LINE__, 
				fi.tot_lic_in_use, fi.user_cnt);
	lc_heartbeat(job[0], 0,0);
	lc_heartbeat(job[1], 0,0);
	lc_heartbeat(job[2], 0,0);
	lc_heartbeat(job[0], 0,0);
	lc_heartbeat(job[1], 0,0);
	lc_heartbeat(job[2], 0,0);
	if (!lc_heartbeat(job[0], 0,0) && !lc_heartbeat(job[1],0,0)
	 && !lc_heartbeat(job[2], 0,0))
		fprintf(ofp, "error: too many licenses out, line %d\n", __LINE__);
	if (get_feat_info("bug9999", __LINE__, &fi, job[4]) && 
		(fi.tot_lic_in_use != 2 || fi.user_cnt != 2))
			fprintf(ofp, "Error line %d, exp 1/1 got %d/%d\n", __LINE__, 
				fi.tot_lic_in_use, fi.user_cnt);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
	lc_free_job(job[3]);
	lc_free_job(job[4]);
}
static
void
bug3167()
{
  char addr[100];
  char *savfilep;
  char savfile[1024];
  HOSTID *h;
  time_t startt;
  FILE *fp;


	serv_log( "  ==>bug 3167\n");
	unlink("buf3167");
	fclose(fopen("bug3167", "w"));
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"bug3167");
	unlink("bug3167");
	fclose(fopen("bug3167", "w"));
	if (!lc_checkout(job[0], "bug3167", "1.0", 1, LM_CO_NOWAIT, &code, 
					LM_DUP_NONE))
		fprintf(ofp, "error: ckout succeeded, line %d\n", __LINE__);
	lc_free_job(job[0]);
	unlink("bug3167");
}
static
void
bug3175()
{
#ifndef sony_news
  char addr[100];
  char *savfilep;
  char savfile[1024];
  HOSTID *h;
  time_t startt;
  FILE *fp;
  int pid;


	serv_log( "  ==>bug 3175\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	lc_set_attr(job[0], LM_A_LINGER, (LM_A_VAL_TYPE)5);
	lc_set_attr(job[0], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"first");
	lc_checkout(job[0], "p1981_2", "1.0", 1, LM_CO_WAIT, &code, LM_DUP_NONE);
	lc_checkin(job[0], "p1981_2", 0);
#ifdef UNIX
	if (!(pid = fork()))
	{
		l_select_one(0, -1, 6000 );  /* sleep 6 seconds */
		ts_lc_new_job(&code, &job[2], __LINE__);  
		ts_flush_server(job[2], __LINE__);
		lc_free_job(job[2]);
		exit(0);
	}
#endif
	lc_set_attr(job[1], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"second");
	if (lc_checkout(job[1], "p1981_2", "1.0", 1, LM_CO_WAIT, &code, 
								LM_DUP_NONE))
		fprintf(ofp, "queue failed, line %d %s\n", __LINE__,
							lc_errstring(job[1]));
	lc_free_job(job[0]);
	lc_free_job(job[1]);
#ifdef UNIX
	waitpid(pid, 0, 0);
#endif
#endif /* sony */
}
static
void
bug3188()
{

  char *savfilep;
  char savfile[300];

	serv_log( "  ==>bug 3188\n");
	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	unsetenv ("LM_LICENSE_FILE");
	do_bug3188(".");
	do_bug3188("@localhost");
	setenv ("LM_LICENSE_FILE", savfile);
}
static
do_bug3188(path)
char *path;
{
  int rc;
  CONFIG *c;
	fprintf(ofp, "     path:%s \n", path);
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	lc_set_attr(job[0], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)path);
	lc_set_attr(job[1], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)path);

	if (lc_checkout(job[0], "bug3188", "1.0", 2, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "ckout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	}
	else
	{
		c = lc_auth_data(job[0], "bug3188");
		if (!c || !c->lc_serial || strcmp(c->lc_serial, "1234"))
			fprintf(ofp, "can't get lc_serial line %d\n", __LINE__);
		if (!c->lc_vendor_def || strcmp(c->lc_vendor_def, "ABC"))
			fprintf(ofp, "can't get lc_vendor_def line %d\n", __LINE__);
	}
	rc = lc_checkout(job[1], "bug3188", "1.0", 1, LM_CO_QUEUE, &code, 

							LM_DUP_NONE);
	if (rc != LM_FEATQUEUE && rc != LM_USERSQUEUED)
		fprintf(ofp, "wrong status, expected %d or %d, got %d, line %d\n", 
			LM_FEATQUEUE, LM_USERSQUEUED, rc, __LINE__);
	lc_checkin(job[0], "bug3188", 0);
	l_select_one(0, -1, 1000 );  /* sleep 1 second */

	if (lc_status(job[1], "bug3188"))
		fprintf(ofp, "queueing failed line %d %s\n", 
			__LINE__, lc_errstring(job[1]));
	else
	{
		c = lc_auth_data(job[1], "bug3188");
		if (!c->lc_serial || strcmp(c->lc_serial, "1234"))
			fprintf(ofp, "can't get lc_serial line %d\n", __LINE__);
		if (!c->lc_vendor_def || strcmp(c->lc_vendor_def, "ABC"))
			fprintf(ofp, "can't get lc_vendor_def line %d\n", __LINE__);
	}
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}
static
void
bug3280()
{

  char *savfilep;
  char savfile[300];

	serv_log( "  ==>bug 3280\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	ts_lc_new_job(&code, &job[2], __LINE__);  
	lc_set_attr(job[1], LM_A_TCP_TIMEOUT, (LM_A_VAL_TYPE) 1);
	lc_set_attr(job[1], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE) -1);
#if 0
	if (lc_checkout(job[1], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	if (get_feat_info("f1", __LINE__, &fi, job[0]) &&
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
#endif
	if (lc_checkout(job[0], "f1", "1.0", 9, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	if (!lc_checkout(job[1], "f1", "1.0", 1, LM_CO_QUEUE, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "queue failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	if (get_feat_info("f1", __LINE__, &fi, job[2]) &&
						(fi.queue_cnt != 1))
		fprintf(ofp, 
		"TCP_TIMEOUT not working, line %d, exp %d got %d\n",
						__LINE__, 1, fi.queue_cnt);
	
	l_select_one(0, -1, 2000 );  /* 2 sec sleep */
	ts_flush_server(job[2], __LINE__);
	ts_flush_server(job[2], __LINE__);
	ts_flush_server(job[2], __LINE__);
	if (get_feat_info("f1", __LINE__, &fi, job[2]) &&
						(fi.queue_cnt != 1))
		fprintf(ofp, 
		"TCP_TIMEOUT removed queued license , line %d, exp %d got %d\n",
						__LINE__, 1, fi.queue_cnt);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);

}
LM_CODE(code_4449, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, 
			VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
static void
bug4449()
{
  VENDORCODE vc;
  int rc;
  char *decstr, *err;
  char buf[MAX_CONFIG_LINE];
	serv_log( "  ==>bug 4449\n");
	memcpy(&vc, &code_4449, sizeof(VENDORCODE));
	LM_CODE_GEN_INIT(&vc);
	lc_init((LM_HANDLE *)0, VENDOR_NAME, &vc, &job[0]);
	if (lc_cryptstr(job[0], "FEATURE lic_string demo 1.0 permanent \
			uncounted 0 HOSTID=ANY START=1-dec-2020", &decstr, 
				&code_4449, 
				LM_CRYPT_FORCE| LM_CRYPT_DECIMAL_FMT, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(job[0]), SIG);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", decstr ? decstr : "");
        if ((rc = lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "lic_string", "1.0", 1, 
					buf, &lp) ) !=  LM_TOOEARLY)
		fprintf(ofp, 
		"lic_string failed, error line %d expected %d, got %d\n", 
					__LINE__, LM_TOOEARLY, rc);
	if (err) free(err);
	if (decstr) free(decstr);
	lp_checkin(lp);
	lc_free_job(job[0]);

	memcpy(&vc, &code_4449, sizeof(VENDORCODE));
	LM_CODE_GEN_INIT(&vc);
	lc_init((LM_HANDLE *)0, VENDOR_NAME, &vc, &job[0]);
	if (lc_cryptstr(job[0], "FEATURE lic_string demo 1.0 permanent \
			uncounted 0 HOSTID=ANY START=1-dec-1998", &decstr, &vc, 
				LM_CRYPT_FORCE| LM_CRYPT_DECIMAL_FMT, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(job[0]), SIG);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", decstr ? decstr : "");
        if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "lic_string", "1.0", 1, 
					buf, &lp) )
		fprintf(ofp, 
		"lic_string failed, error line %d %s\n", 
					__LINE__, lp_errstring(lp));
	if (err) free(err);
	if (decstr) free(decstr);
	lp_checkin(lp);
	lc_free_job(job[0]);
}
static void
bug4612()
{
  char *ret;
	serv_log( "  ==>bug 4612\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	if (lc_checkout(job[0], "f1", "1.0", 9, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "ckout failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	if (!lc_checkout(job[1], "f1", "1.0", 1, LM_CO_QUEUE, &code, 
							LM_DUP_NONE))
		fprintf(ofp, "queue failed , line %d:%s\n", __LINE__, 
					lc_errstring(job[0]));
	if (!(ret = lc_vsend(job[0], "dump")) || strcmp(ret, "ok"))
		lc_perror(job[0], "dump error");
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}

static void
bug4622()
{
	serv_log( "  ==>bug 4622\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	fprintf(ofp, "Expect a short and long error message here:\n");
	lc_set_attr(job[0], LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)0);
	fprintf(ofp, "%s\n", lc_errtext(job[0], -1));
	lc_set_attr(job[0], LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	fprintf(ofp, "------------\n%s\n", lc_errtext(job[0], -1));
	lc_free_job(job[0]);
}

static void
bug4631()
{
  char *ret;
  unsigned char buf[1000];
  unsigned char *cp = buf;
  char *savfilep;
  char savfile[1024];
	serv_log( "  ==>bug 4631\n");
	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	while (cp < &buf[990])
	{
#ifdef PC
		strcpy((char *)cp, "foo.dat;");
#else
		strcpy((char *)cp, "foo.dat:");
#endif
		cp += strlen((char *)cp);
	}
	setenv ("LM_LICENSE_FILE", buf);
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
							LM_DUP_NONE) != LM_NOCONFFILE)
		fprintf(ofp, "checkout succeeded , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	lc_errstring(job[0]);
	lc_free_job(job[0]);
	setenv("LM_LICENSE_FILE", savfile);
}

#if 0
bug4449()
{
  VENDORCODE vc;
  int rc;
  char *decstr, *err;
  char buf[MAX_CONFIG_LINE];
	serv_log( "  ==>bug 4449\n");
	ts_lm_init("demo", &code, &job[0], __LINE__);  
	memset(&vc, 0, sizeof(vc));
	memcpy(&vc, (char *)&code, sizeof(code));
	vc.data[0] = code.data[0]^SIG;
	vc.data[1] = code.data[1]^SIG;
	if (lc_cryptstr(job[0], "FEATURE lic_string demo 1.0 permanent \
			uncounted 0 HOSTID=ANY START=1-dec-2020", &decstr, &vc, 
				LM_CRYPT_FORCE| LM_CRYPT_DECIMAL_FMT, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(job[0]), SIG);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", decstr ? decstr : "");
        if ((rc = lp_checkout(LPCODE, LM_RESTRICTIVE, "lic_string", "1.0", 1, 
					buf, &lp) ) !=  LM_TOOEARLY)
		fprintf(ofp, 
		"lic_string failed, error line %d expected %d, got %d\n", 
					__LINE__, LM_TOOEARLY, rc);
	if (err) free(err);
	if (decstr) free(decstr);
	lp_checkin(lp);
	lc_free_job(job[0]);

	ts_lc_new_job(&code, &job[0], __LINE__);  
	memset(&vc, 0, sizeof(vc));
	memcpy(&vc, (char *)&code, sizeof(code));
	vc.data[0] = code.data[0]^SIG;
	vc.data[1] = code.data[1]^SIG;
	if (lc_cryptstr(job[0], "FEATURE lic_string demo 1.0 permanent \
			uncounted 0 HOSTID=ANY START=1-dec-1998", &decstr, &vc, 
				LM_CRYPT_FORCE| LM_CRYPT_DECIMAL_FMT, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(job[0]), SIG);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", decstr ? decstr : "");
        if (lp_checkout(LPCODE, LM_RESTRICTIVE, "lic_string", "1.0", 1, 
					buf, &lp) )
		fprintf(ofp, 
		"lic_string failed, error line %d %s\n", 
					__LINE__, lp_errstring(lp));
	if (err) free(err);
	if (decstr) free(decstr);
	lp_checkin(lp);
	lc_free_job(job[0]);
}
#endif


static void
bug4807()
{
	serv_log( "  ==>bug 4807\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	if (lc_checkout(job[0], "p4807", "2.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	lc_errstring(job[0]);
	lc_free_job(job[0]);
}

static void
bug4725()
{
	serv_log( "  ==>bug 4725\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	if (lc_checkout(job[0], "max", "1.0", 4, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE) != LM_MAXLIMIT)
		fprintf(ofp, "checkout succeeded , line %d %s\n", __LINE__);
			
	lc_free_job(job[0]);
}

static
void
bug4775()
{
	serv_log( "  ==>bug 4775\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  

	lc_set_attr(job[0], LM_A_PLATFORM_OVERRIDE, (LM_A_VAL_TYPE)"i87_w");
	if (lc_checkout(job[0], "platform", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE) )
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
			
	lc_free_job(job[0]);
}

static
void
bug4219()
{
	serv_log( "  ==>bug 4219\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  

	if (lc_checkout(job[0], "max", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE) )
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	if (lc_checkout(job[1], "max", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE) != LM_MAXLIMIT )
		fprintf(ofp, "checkout didnt' return LM_MAXLIMIT %d, line %d\n", 
			job[1]->lm_errno, __LINE__);
	if (lc_checkout(job[0], "max", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE) )
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	lc_free_job(job[0]);
}
filter()
{
	return 0;
}


static
void
bug4257()
{
	serv_log( "  ==>bug 4257\n");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	ts_lc_new_job(&code, &job[1], __LINE__);  
	ts_lc_new_job(&code, &job[2], __LINE__);  
	ts_lc_new_job(&code, &job[3], __LINE__);  
	lc_set_attr(job[0], LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"0");
	lc_set_attr(job[1], LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"1");
	lc_set_attr(job[2], LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"2");
	lc_set_attr(job[3], LM_A_CHECKOUT_DATA, (LM_A_VAL_TYPE)"3");
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter);
	lc_set_attr(job[1], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter);
	lc_set_attr(job[2], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter);
	lc_set_attr(job[3], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter);

	if (lc_checkout(job[0], "p4257", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_VENDOR) )
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	if (lc_checkout(job[1], "p4257", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_VENDOR) )
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__, 
			lc_errstring(job[1]));
	if (lc_checkout(job[2], "p4257", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_VENDOR) )
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__, 
			lc_errstring(job[2]));
	if (!lc_checkout(job[3], "p4257", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_VENDOR) )
		fprintf(ofp, "checkout succeeded , line %d %s\n", __LINE__, 
			lc_errstring(job[3]));
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
	lc_free_job(job[3]);
}

static
void
bug4859()
{
  char *savfilep;
  char savfile[1024];

	serv_log( "  ==>bug 4859\n");
	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	unsetenv ("LM_LICENSE_FILE");
	setenv ("DEMO_LICENSE_FILE", "START_LICENSE\n\
FEATURE f1 demo 1.0 permanent uncounted 123456789012 HOSTID=ANY\n\
END_LICENSE");
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(job[0], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)"@localhost");

	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_VENDOR) )
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	lc_free_job(job[0]);
	ts_lc_new_job(&code, &job[0], __LINE__);  
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter);
	lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(job[0], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)"@localhost");
	if (lc_checkout(job[0], "f1", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_VENDOR) )
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	lc_free_job(job[0]);
	setenv("LM_LICENSE_FILE", savfile);
	unsetenv ("DEMO_LICENSE_FILE");
}
static
void
bug4920()
{
 VENDORCODE v;
 int rc;
  char *savfilep;
  char savfile[1024];
	serv_log( "  ==>bug 4920\n");

	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	setenv ("LM_LICENSE_FILE", "@localhost");
	v7_new_job(&v, &job[0]);

	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"i87_w");
	if (((rc = lc_checkout(job[0], "", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE)) != LM_NOFEATURE) 
				&& rc != LM_BADPARAM)
		fprintf(ofp, "checkout succeeded , line %d expected %d, got %d %s\n", __LINE__, LM_NOSERVSUPP, rc, lc_errstring(job[0]));
			
	lc_free_job(job[0]);
	setenv("LM_LICENSE_FILE", savfile);
}

static
void
bug4976()
{
 VENDORCODE v;
 int rc;
  char *savfilep;
  char savfile[1024];
	serv_log( "  ==>bug 4976\n");

	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	unsetenv ("LM_LICENSE_FILE");
	v7_new_job(&v, &job[0]);
	lc_set_attr(job[0], LM_A_CKOUT_INSTALL_LIC, (LM_A_VAL_TYPE)0);
	lc_set_registry(job[0], "DEMO_LICENSE_FILE", 0);
	lc_set_registry(job[0], "DEMOF_LICENSE_FILE", 0);
	lc_set_registry(job[0], "DEMOF2_LICENSE_FILE", 0);
	lc_set_registry(job[0], "LM_LICENSE_FILE", 0);
	setenv ("DEMO_LICENSE_FILE", "@localhost");
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)p2206_filter);

	if (!lc_checkout(job[0], "p4976", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout succeeded , line %d \n", __LINE__);
			
	lc_free_job(job[0]);
	setenv("LM_LICENSE_FILE", savfile);
	unsetenv("DEMO_LICENSE_FILE");
}

static
void
bug4888()
{
 VENDORCODE v;
 int rc;
  char *savfilep;
  char savfile[1024];
  char **files;
	serv_log( "  ==>bug 4888\n");

	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	unsetenv ("LM_LICENSE_FILE");
	v7_new_job(&v, &job[0]);
	lc_set_attr(job[0], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)"@localhost");
	setenv ("DEMO_LICENSE_FILE", "@foo");
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)p2206_filter);

	if (lc_checkout(job[0], "p4888", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	lc_free_job(job[0]);
	lc_new_job(0, 0, &v, &job[0]);
	lc_get_config(job[0], "f1");
	lc_get_attr(job[0], LM_A_LF_LIST, (short*)&files);
	while (*files)			
	{
		if (!strcmp(*files, "foo"))
			printf("failed line %d\n", __LINE__);
		files++;
	}
	lc_free_job(job[0]);

		
			
	setenv("LM_LICENSE_FILE", savfile);
	unsetenv("DEMO_LICENSE_FILE");
}
static
void
bug5002()
{
 VENDORCODE v;
 int rc;
  char *savfilep;
  char savfile[1024];
  char **files;
	serv_log( "  ==>bug 5002\n");

	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	unsetenv ("LM_LICENSE_FILE");
	setenv ("DEMO_LICENSE_FILE", "@localhost");
	v7_new_job(&v, &job[0]);
	lc_set_registry(job[0], "DEMO_LICENSE_FILE", "2500@localhost");
	lc_set_attr(job[0], LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	if (lc_checkout(job[0], "f2", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));
	lc_free_job(job[0]);
/* 
 *	same thing, but this time LM_A_LICENSE_DEFAULT is set to a bad
 *	location
 */
	v7_new_job(&v, &job[0]);
	lc_set_registry(job[0], "DEMO_LICENSE_FILE", "2500@localhost");
	lc_set_attr(job[0], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)"junk.dat");
	if (lc_checkout(job[0], "f2", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
			lc_errstring(job[0]));

	/* cleanup */
	lc_set_registry(job[0], "DEMO_LICENSE_FILE", 0);
	lc_free_job(job[0]);
	unsetenv("DEMO_LICENSE_FILE");
	setenv("LM_LICENSE_FILE", savfile);
}
static
void
bug5203()
{
  VENDORCODE v;
  int rc;
  int cnt;
	
	serv_log( "  ==>bug 5203\n");
	v7_new_job(&v, &job[0]);	
	v7_new_job(&v, &job[1]);
	get_feat_info("p5203c1", __LINE__, &fi, job[1]);
	cnt = fi.tot_lic_in_use;
	if (lc_checkout(job[0], "p5203c1", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
						lc_errstring(job[0]));
	if (lc_checkout(job[0], "p5203c2", "1.0", 1, LM_CO_NOWAIT, &code, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
						lc_errstring(job[0]));
	get_feat_info("p5203c1", __LINE__, &fi, job[1]);
	if (cnt != fi.tot_lic_in_use - 1)
		fprintf(ofp, "Error ??? line %d\n", __LINE__);
	
	lc_checkin(job[0], "p5203c1", 0);
	get_feat_info("p5203c1", __LINE__, &fi, job[1]);
	if (cnt != fi.tot_lic_in_use)
		fprintf(ofp, "Checkin failed line %d\n", __LINE__);
	lc_free_job(job[0]);
	get_feat_info("p5203c1", __LINE__, &fi, job[1]);
	if (cnt != fi.tot_lic_in_use)
		fprintf(ofp, "Error ??? line %d\n", __LINE__);
	lc_free_job(job[1]);
}
static
void
bug5138()
{
  VENDORCODE v;
  int rc;
  int cnt;
	
	serv_log( "  ==>bug 5138\n");
	v7_new_job(&v, &job[0]);	
	v7_new_job(&v, &job[1]);
	v7_new_job(&v, &job[2]);
	v7_new_job(&v, &job[3]);
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[1], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[2], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[2], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@foo" PATHSEPARATORSTR "@localhost");
	lc_set_attr(job[1], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@localhost");
	lc_set_attr(job[2], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@foo" PATHSEPARATORSTR "@localhost");
	lc_set_attr(job[2], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@foo" PATHSEPARATORSTR "@localhost");
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter);
	lc_set_attr(job[1], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter);
	lc_set_attr(job[2], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter);
	if (lc_checkout(job[0], "f1", "1.0", 9, LM_CO_QUEUE, &code, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
						lc_errstring(job[0]));
	if (lc_checkout(job[1], "f1", "1.0", 1, LM_CO_QUEUE, &code, 
						LM_DUP_NONE) != LM_FEATQUEUE)
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
						lc_errstring(job[1]));
	lc_free_job(job[1]);
/* 
 *	Job 3 doesn't have a checkout filter
 */
	if (lc_checkout(job[3], "f1", "1.0", 1, LM_CO_QUEUE, &code, 
						LM_DUP_NONE) != LM_FEATQUEUE)
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
						lc_errstring(job[3]));
	lc_free_job(job[3]);
	if (lc_checkout(job[2], "f1", "1.0", 1, LM_CO_QUEUE, &code, 
						LM_DUP_NONE) != LM_FEATQUEUE)
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
						lc_errstring(job[2]));
	lc_free_job(job[0]);
	lc_free_job(job[2]);
}
static
void
bug5423()
{
  VENDORCODE v;
  int rc;
  int cnt;
	
	serv_log( "  ==>bug 5423\n");
	v7_new_job(&v, &job[0]);	
	v7_new_job(&v, &job[1]);

	lc_set_attr(job[0], LM_A_STRENGTH_OVERRIDE, (LM_A_VAL_TYPE)LM_STRENGTH_DEFAULT);
	lc_set_attr(job[1], LM_A_STRENGTH_OVERRIDE, (LM_A_VAL_TYPE)LM_STRENGTH_DEFAULT);
	lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)LM_STRENGTH_DEFAULT);
	lc_set_attr(job[1], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)LM_STRENGTH_DEFAULT);
	if (lc_checkout(job[0], "p5423", "1.0", 1, LM_CO_NOWAIT, &v, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
						lc_errstring(job[0]));
	if (!lc_checkout(job[1], "p5423", "1.0", 1, LM_CO_NOWAIT, &v, 
						LM_DUP_NONE))
		fprintf(ofp, "checkout should have failed , line %d %s\n", __LINE__,
						lc_errstring(job[1]));
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}
static
void
bug5659()
{
  VENDORCODE v;
  int rc;
  int cnt;
  char *ofile_str, *err;
  VENDORCODE vc;
	
	serv_log( "  ==>bug 5659\n");
	memcpy(&vc, &code_4449, sizeof(VENDORCODE));
	LM_CODE_GEN_INIT(&vc);
	lc_init((LM_HANDLE *)0, VENDOR_NAME, &vc, &job[0]);
	if (!lc_cryptstr(job[0], "FEATURE lic_string demo 1.0 permanent \
			uncounted 0 HOSTID=ANY foo=bar", &ofile_str, 
				&code_4449, 
				LM_CRYPT_FORCE| LM_CRYPT_DECIMAL_FMT, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr did not fail line %d \n", __LINE__);
	}
	lc_free_job(job[0]);
}
static
void
bug5505()
{
  VENDORCODE v;
      
	serv_log( "  ==>bug 5505\n");
	v7_new_job(&v, &job[0]);        
	if (get_feat_info("f22", __LINE__, &fi, job[0]) && 
		(fi.num_lic != 2 ))
			fprintf(ofp, "Error line %d, exp 2 got %d\n", __LINE__, 
							fi.num_lic);
	lc_free_job(job[0]);
}
static
void
bug5449()
{
  VENDORCODE v;
  char **lic_module_list;
  int index;
  CONFIG *conf, *pos = 0;

	serv_log( "  ==>bug 5449\n");
	v7_new_job(&v, &job[0]);        
	lc_set_attr(job[0], LM_A_DISABLE_ENV,(LM_A_VAL_TYPE) 1);
	lc_set_attr(job[0], LM_A_LICENSE_DEFAULT, 
					(LM_A_VAL_TYPE)"servtest.lic");
	lic_module_list = lc_feat_list( job[0], 0, NULL );
	if ( !lic_module_list ) return;
	for (index = 0; lic_module_list[index]; index++)
	{
		while (conf = lc_next_conf(job[0], lic_module_list[index], 
								&pos))
		{
			;
		}
	}
	if (index < 30) fprintf(ofp, "bug line %d\n", __LINE__);
	lc_free_job(job[0]);
}
static
void
bug5688()
{
  VENDORCODE v;
  char **lic_module_list;
  int index;
  CONFIG *conf, *pos = 0;
      
	serv_log( "  ==>bug 5688\n");
	v7_new_job(&v, &job[0]);        
        lc_set_attr(job[0], LM_A_DISABLE_ENV,(LM_A_VAL_TYPE) 1);
        lc_set_attr(job[0], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)"@localhost");
        lc_set_attr(job[0], LM_A_STRENGTH_OVERRIDE, 
				(LM_A_VAL_TYPE)LM_STRENGTH_LICENSE_KEY);
        lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (lc_checkout(job[0], "RationalSuiteEnterprise12345", "1.0", 1, 
						LM_CO_NOWAIT, &v, LM_DUP_NONE))
              fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
                                              lc_errstring(job[0]));
	if (lc_checkout(job[0], "ZYBIS12345678", "1.0", 1, LM_CO_NOWAIT, &v, 
								LM_DUP_NONE))
              fprintf(ofp, "checkout failed , line %d %s\n", __LINE__,
                                              lc_errstring(job[0]));
	lc_free_job(job[0]);
}

static
void
bug5552()
{
  VENDORCODE v;
  char **lic_module_list;
  int index;
  CONFIG *conf, *pos = 0;
      
	serv_log( "  ==>bug 5552\n");
	v7_new_job(&v, &job[0]);        
        lc_set_attr(job[0], LM_A_STRENGTH_OVERRIDE, 
				(LM_A_VAL_TYPE)LM_STRENGTH_LICENSE_KEY);
        lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (!lc_checkout(job[0], "p5552", "1.0", 1, LM_CO_NOWAIT, &v, 
                                              LM_DUP_NONE))
              fprintf(ofp, "checkout should have failed , line %d %s\n", 
						__LINE__, lc_errstring(job[0]));
	lc_free_job(job[0]);
	v7_new_job(&v, &job[0]);        
	job[0]->key_filters = 0;
        lc_set_attr(job[0], LM_A_STRENGTH_OVERRIDE, 
					(LM_A_VAL_TYPE)LM_STRENGTH_DEFAULT);
        lc_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)1);
	if (!lc_checkout(job[0], "P5552_sign", "1.0", 1, LM_CO_NOWAIT, &v, 
                                              LM_DUP_NONE))
              fprintf(ofp, "checkout should have failed , line %d %s\n", 
				__LINE__, lc_errstring(job[0]));
	lc_free_job(job[0]);
}
static
void
bug5866()
{
  VENDORCODE v;
  char **lic_module_list;
  int index;
  CONFIG *conf, *pos = 0;
      
	serv_log( "  ==>bug 5866\n");
	v7_new_job(&v, &job[0]);        
	if (lc_checkout(job[0], "p5866", "1.0", 1, LM_CO_NOWAIT, &v, 
                                              LM_DUP_NONE))
              fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
	lc_free_job(job[0]);
}
int gotexit;
void
p5411_exit(void)
{
	gotexit = 1;
}
static
void
bug5411()
{
  VENDORCODE v;
  char **lic_module_list;
  int index;
  CONFIG *conf, *pos = 0;
  time_t t;
  FILE *fp;
  extern LM_HANDLE *lm_job;
      
	fp = fopen("hostid.txt", "w");
	fprintf(fp, "FILE_HOSTID_VAL\n");
	fclose(fp);
	serv_log( "  ==>bug 5411\n");
	v7_new_job(&v, &job[0]);        
	lm_job = job[0];
	x_flexlm_newid();
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)
	"START_LICENSE\n\
		FEATURE p5411 demo 1.0 permanent uncounted \
		HOSTID=FILE_HOSTID=FILE_HOSTID_VAL ck=224 \
		SIGN=\"009C D323 15AE \
		FD3B A74E 6B08 F881 4F00 5B17 FCDA 3E0F 36F7 8ADD 1AE7 7F1D\"\n\
		END_LICENSE");
	lc_set_attr(job[0], LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)40);
	lc_set_attr(job[0], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_USER_EXITCALL, (LM_A_VAL_TYPE)p5411_exit);
	if (lc_checkout(job[0], "p5411", "1.0", 1, LM_CO_NOWAIT, &v, 
							      LM_DUP_NONE))
              fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
	unlink("hostid.txt");	
	t = time(0);
	while (!gotexit && ((time(0) - t) < 4)) l_select_one(0, -1, 500); 
				/* check every 1/2 seconds */
	if (!gotexit)
		fprintf(ofp, "Took too long exp < 4 secnds, took %d\n",
		time(0) - t);
	lc_free_job(job[0]);
}
void
bug5923()
{
  VENDORCODE v;
  CONFIG *conf;

        serv_log( "  ==>bug 5923\n");
        v7_new_job(&v, &job[0]);
        /* set different user names so we can track easier for debugging */
        lc_set_attr(job[0], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"firstuser");
        v7_new_job(&v, &job[1]);
        lc_set_attr(job[1], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"seconduser");
        if (lc_checkout(job[0], "VSCRIPT", "1.0", 1, LM_CO_WAIT, &v,
                                              LM_DUP_NONE))
              fprintf(ofp, "checkout failed , line %d %s\n",
                                        __LINE__, lc_errstring(job[0]));
        if (!lc_checkout(job[1], "VSCRIPT", "1.0", 1, LM_CO_QUEUE, &v,
                                              LM_DUP_NONE) || job[1]->lm_errno != LM_FEATQUEUE)
              fprintf(ofp, "checkout should have failed with %d, got %d, line %d\n", LM_FEATQUEUE, job[1]->lm_errno, __LINE__);
        if (!lc_status(job[1], "VSCRIPT"))
                fprintf(ofp, "queued should have failed line %d\n",
                        __LINE__);
        lc_checkin(job[0], "VSCRIPT", 0);
        lc_free_job(job[0]);
        l_select_one(0, -1, 1000); /* sleep 1 second */

        if (lc_status(job[1], "VSCRIPT"))
                fprintf(ofp, "queued failed line %d %s\n", __LINE__,
                                lc_errstring(job[1]));
        lc_free_job(job[1]);
}
static
void
bug5910()
{
  VENDORCODE v;
  CONFIG *conf;
      
	serv_log( "  ==>bug 5910 (~20 seconds)\n");
	v7_new_job(&v, &job[0]);        
	v7_new_job(&v, &job[1]);        
	if (lc_checkout(job[0], "p5910", "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
              fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
	if (!lc_checkout(job[1], "p5910", "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
              fprintf(ofp, "checkout should have failed , line %d\n", __LINE__);
	
	l_select_one(0, -1, LC_REMOVE_MIN); /* until lmremove will work */
	if (lc_remove(job[1], "p5910", lc_username(job[1], 1), 
				lc_hostname(job[1],1),
				lc_display(job[1],1)))
		fprintf(ofp, "lc_remove failed line%d:%s\n", __LINE__, 
					lc_errstring(job[1]));
	if (get_feat_info("p5910", __LINE__, &fi, job[1]) &&
		(fi.tot_lic_in_use != 1 ))
			fprintf(ofp, "Error line %d, exp 1 got %d\n", __LINE__, 
					fi.tot_lic_in_use);
	/* this checkout should fail because the license is lingering */
	if (lc_heartbeat(job[0], 0, 0))
		fprintf(ofp, "heartbeat failed line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	l_select_one(0, -1, 6000); /* wait for linger */
	ts_flush_server(job[1], __LINE__);
	if (get_feat_info("p5910", __LINE__, &fi, job[1]) &&
		(fi.tot_lic_in_use != 0 ))
			fprintf(ofp, "Error line %d, exp 0 got %d\n", __LINE__, 
					fi.tot_lic_in_use);

	lc_heartbeat(job[0], 0, 0);
	lc_heartbeat(job[0], 0, 0);
	if (get_feat_info("p5910", __LINE__, &fi, job[1]) &&
		(fi.tot_lic_in_use != 1 ))
			fprintf(ofp, "Error line %d, exp 1 got %d\n", __LINE__, 
					fi.tot_lic_in_use);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}
static
int
filter6047(CONFIG *conf)
{

	int ret_val = 29;


	if ( (strstr (conf->lc_vendor_def,  "STARTUP" )) != NULL )
	{
		/*printf ( " Disallow checkout for %s\n", conf->lc_vendor_def);*/
		return ( ret_val );
	}
	else
	{
		/*printf ("OK !!  Proceed with checkout  for %s  \n", conf->lc_vendor_def);*/
		return ( 0 );
	}
}
static
void
bug6047()
{
  VENDORCODE v;
  CONFIG *conf;
      
	serv_log( "  ==>bug 6047\n");
	v7_new_job(&v, &job[0]);        
	v7_new_job(&v, &job[1]);        
	lc_set_attr(job[0], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter6047);
	lc_set_attr(job[1], LM_A_CHECKOUTFILTER, (LM_A_VAL_TYPE)filter6047);
	lc_set_attr(job[0], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)"27002@localhost");
	lc_set_attr(job[1], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)"27002@localhost");
	if (lc_checkout(job[0], "p6047", "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
		return;
	}
	conf = lc_auth_data(job[0], "p6047");
	if (strstr (conf->lc_vendor_def,  "STARTUP" ))
		fprintf(ofp, 
		"Error line %d, lc_auth_data returned the wrong conf \"%s\"\n", 
				__LINE__, conf->lc_vendor_def);
	if (!lc_checkout(job[1], "p6047", "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
              fprintf(ofp, "checkout should have failed, line %d %s\n", 
					__LINE__);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}
static
void
bug6241()
{
#if 0
  VENDORCODE v;
  CONFIG *conf;
  int rc;
      
	serv_log( "  ==>bug 6241 (about 30 seconds)\n");
	v7_new_job(&v, &job[0]);        
	v7_new_job(&v, &job[1]);        
	lc_set_attr(job[0], LM_A_TCP_TIMEOUT, (LM_A_VAL_TYPE)1);
	/* no heartbeats */
	lc_set_attr(job[0], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)-1);
	lc_set_attr(job[0], LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)-1);
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[1], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@localhost");
	lc_set_attr(job[1], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@localhost");
	if (lc_checkout(job[0], "f1", "1.0", 9, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
	}
	conf = lc_auth_data(job[0], "f1");
	if (strncmp (conf->lc_vendor_def,  "Vendor" , 6))
		fprintf(ofp, 
		"Error line %d, lc_auth_data returned the wrong conf \"%s\"\n", 
				__LINE__, conf->lc_vendor_def);
	if (lc_checkout(job[1], "f1", "1.0", 1, LM_CO_WAIT, &v, 
						LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[1]));
	}
	conf = lc_auth_data(job[1], "f1");
	if (strncmp (conf->lc_vendor_def,  "Vendor" , 6))
		fprintf(ofp, 
		"Error line %d, lc_auth_data returned the wrong conf \"%s\"\n", 
				__LINE__, conf->lc_vendor_def);
	lc_free_job(job[0]); 
	lc_free_job(job[1]);
#endif
}
static
void
bug5881()
{
  VENDORCODE v;
  CONFIG *conf;
  int rc;
      
	serv_log( "  ==>bug 5881 \n");
	v7_new_job(&v, &job[0]);        
	if (lc_checkout(job[0], "p5881", "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
	}
	conf = lc_auth_data(job[0], "p5881");
	if (!conf->idptr)
		fprintf(ofp, "Checked out wrong feature %s line %d\n", conf->code, __LINE__);
	lc_free_job(job[0]);
}

static
void
bug6288()
{
	VENDORCODE	v;
	CONFIG *	conf = NULL;
	int		err = 0;
	int		flag = LM_CRYPT_FORCE;
	/*
	 *	The following text is taken from bug P6288.
	 */
	char		szIn[] = "SERVER this_host any\n"
				 "VENDOR demo\n"
				 "INCREMENT f1 demo 1.00 01-jan-2005 1 0 \\\n"
				 "\tSUPERSEDE \\\n"
				 "\tISSUER=\" \\\n"
				 "\tgroup \" \\\n"
				 "\tSTART=01-jan-2002\n";
	char *		pszOut = NULL;
	char *		pszErrMsg = NULL;

	memset(&v, 0, sizeof(VENDORCODE));
	
	serv_log( " ==>bug 6288 \n");
	v7_new_job(&v, &job[0]);

	err = lc_cryptstr(job[0], szIn, &pszOut, &v, flag, "BUG P6288", &pszErrMsg);
	if(err)
	{
		if(pszErrMsg)
		{
			fprintf(ofp, "Error: lc_cryptstr failed.\n%s\n", pszErrMsg);
			lc_free_mem(job[0], pszErrMsg);
		}
	}
	else
	{
		if(pszOut)
		{
			lc_free_mem(job[0], pszOut);
		}
	}
	/*	Free up resources.	*/
	lc_free_job(job[0]);


}

static
void
bug6215()
{
	VENDORCODE	v;
	memset(&v, 0, sizeof(VENDORCODE));

	serv_log( " ==>bug 6215 \n");
	v7_new_job(&v, &job[0]);
	v7_new_job(&v, &job[1]);

	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[1], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"27002@localhost");
	lc_set_attr(job[1], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"27002@localhost");

	if (lc_checkout(job[0], "p6215", "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
	}
	else
	{
	    if (lc_checkout(job[0], "p6215", "2.0", 1, LM_CO_NOWAIT, &v, 
							    LM_DUP_NONE))
	    {
		    fprintf(ofp, "checkout failed , line %d %s\n", 
					    __LINE__, lc_errstring(job[0]));
	    }
	}

	/*	Free up resources.	*/
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}

static
void
bug6376()
{
	VENDORCODE	v;
	CONFIG *	feat_config;
	memset(&v, 0, sizeof(VENDORCODE));

	serv_log( " ==>bug 6376 \n");
	v7_new_job(&v, &job[0]);

	if (lc_checkout(job[0], "p6376_AAA", "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
	}
	else
	{
		/*	Now get vendor string from this PACKAGE/INCREMENT line	*/
		feat_config = lc_auth_data(job[0], "p6376_AAA");
		if (!feat_config)
		{
			fprintf(ofp, "getting auth data failed, line %d %s\n",
					__LINE__, lc_errstring(job[0]));
		}
		else if(feat_config->lc_vendor_def == NULL)
		{
			fprintf(ofp, "Error line %d, vendor string is NULL\n",
				__LINE__);
		}
		else
		{
			/*	SUCCESS		*/
		}
	}

	/*	Free up resources.	*/
	lc_free_job(job[0]);

}

static
void
bug6333()
{
#if 0
    /*
     *  having problems getting to package/borrow to work with testsuite,
     *  add to testsuite later.
     */

    VENDORCODE	v;
    char        szDate[256] = {'\0'};
    char        szBorrow[256] = {'\0'};
    char        szMonths[12][4] = { "jan", "feb", "mar", "apr", "may",
                                    "jun", "jul", "aug", "sep", "oct",
                                    "nov", "dec" };
    time_t      ltime;
    struct tm * pLocalTime = NULL;
    struct tm   LocalTime;
#ifdef THREAD_SAFE_TIME
	struct tm	tst;
#endif

    serv_log( " ==>bug 6333 \n");

	memset(&v, 0, sizeof(VENDORCODE));
    time(&ltime);

#ifdef THREAD_SAFE_TIME
	localtime_r(&ltime, &tst);
	pLocalTime = &tst;
#else /* !THREAD_SAFE_TIME */
    pLocalTime = localtime(&ltime);
#endif

    if(pLocalTime)
    {
        memcpy(&LocalTime, pLocalTime, sizeof(struct tm));
    }
    else
    {
        fprintf(ofp, "Error getting date for borrow, line %d\n", __LINE__);
        return;
    }

    sprintf(szDate, "%02d-%s-%04d", LocalTime.tm_mday, szMonths[LocalTime.tm_mon], LocalTime.tm_year + 1900);
    sprintf(szBorrow, "%s:%s:%s", szDate, "demo", szDate);

    lc_set_registry(job[0], "LM_BORROW", szBorrow);

	v7_new_job(&v, &job[0]);

    lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
    lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"27002@localhost");

	if (lc_checkout(job[0], "p6333", "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
	}
    else
    {
        /*  Check it in */
        lc_free_job(job[0]);
       
        memset(&v, 0, sizeof(VENDORCODE));
        v7_new_job(&v, &job[0]);

        lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
        lc_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"27002@localhost");

	    if (lc_checkout(job[0], "p6333", "1.0", 1, LM_CO_NOWAIT, &v, 
							    LM_DUP_NONE))
	    {
		    fprintf(ofp, "checkout failed , line %d %s\n", 
					    __LINE__, lc_errstring(job[0]));
	    }
    }

	/*	Free up resources.	*/
	lc_free_job(job[0]);
#endif
}

static
void
bug6414()
{
#if 0
 	VENDORCODE	v;
	char		szDate[256] = {'\0'};
	char		szBorrow[256] = {'\0'};
	char		szMonths[12][4] = { "jan", "feb", "mar", "apr",
					"may", "jun", "jul", "aug",
					"sep", "oct", "nov", "dec" };
	int		count = 0;
	time_t		ltime;
	struct tm *	pLocalTime = NULL;
	struct tm	LocalTime;
#ifdef THREAD_SAFE_TIME
	struct tm	tst;
#endif
	LM_BORROW_STAT *pStat = 0;

#define P6414A	"p6414a"
#define P6414B	"p6414b"

	serv_log( " ==>bug 6414 \n");

	memset(&v, 0, sizeof(VENDORCODE));
	v7_new_job(&v, &job[0]);
	time(&ltime);

#ifdef THREAD_SAFE_TIME
	localtime_r(&ltime, &tst);
	pLocalTime = &tst;
#else /* !THREAD_SAFE_TIME */
	pLocalTime = localtime(&ltime);
#endif

	if(pLocalTime)
	{
		memcpy(&LocalTime, pLocalTime, sizeof(struct tm));
	}
	else
	{
		fprintf(ofp, "Error getting date for borrow, line %d\n", __LINE__);
		return;
	}

	sprintf(szDate, "%02d-%s-%04d", LocalTime.tm_mday,
		szMonths[LocalTime.tm_mon], LocalTime.tm_year + 1900);
	sprintf(szBorrow, "%s:%s:%s", szDate, "demo", szDate);

	lc_set_registry(job[0], "LM_BORROW", szBorrow);

#if 0
	lc_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
    	lc_set_attr(job[0], LM_A_LICENSE_FILE, 
		(LM_A_VAL_TYPE)LIC_PATH);
#endif

	if (lc_checkout(job[0], P6414A, "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
		goto done;
	}

	if (lc_checkout(job[0], P6414B, "1.0", 1, LM_CO_NOWAIT, &v,
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
		goto done;
	}

	/*
	 *	Now checkin and check it out and then get borrow info
	 */

	lc_checkin(job[0], P6414A, 0);
	lc_checkin(job[0], P6414B, 0);

	/*
	 *	Now checkout again and see if we can get info
	 */

	memset(&v, 0, sizeof(VENDORCODE));
	v7_new_job(&v, &job[0]);
	if (lc_checkout(job[0], P6414A, "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
		goto done;
	}

	if (lc_checkout(job[0], P6414B, "1.0", 1, LM_CO_NOWAIT, &v,
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
		goto done;
	}

	if (l_borrow_stat(job[0], &pStat, 1))
	{
		fprintf(ofp, "Error: %s, line %d\n", lc_errstring(job[0]),
				__LINE__);
		goto done;

	}

	for (; pStat; pStat = pStat->next)
	{
		if(strcmp(P6414A, pStat->feature) == 0)
			count++;
		if(strcmp(P6414B, pStat->feature) == 0)
			count++;
	}

	if(count < 2)
	{
		/*	Should have at least 2 entries in borrow stat	*/
		fprintf(ofp, "Error line %d, borrow stat incorrect, found %d entries\n", __LINE__, count);

	}

	/*	Free up resources.	*/
done:
	lc_free_job(job[0]);
#endif
}

static
void
bug6309()
{
 	VENDORCODE	v;
#define P6309		"P6309"

	serv_log( " ==>bug 6309 \n");

	memset(&v, 0, sizeof(VENDORCODE));
	v7_new_job(&v, &job[0]);

	if (lc_checkout(job[0], P6309, "1.0", 1, LM_CO_NOWAIT, &v, 
							LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed , line %d %s\n", 
					__LINE__, lc_errstring(job[0]));
		goto done;
	}

	/*
	 *	Now checkin and check it out and then get borrow info
	 */

	lc_checkin(job[0], P6309, 0);
	/*	Free up resources.	*/
done:
	lc_free_job(job[0]);
}

static
void
bug6567()
{
#if 0
    /*
     *  No way to really test this with current testsuite.
     */
#endif
}

static
void
bug6660()
{
#if 0
	/*
	 *	No way to really test this with current testsuite
	 */
#endif
}



bugtests()
{
	if (bugnumber == 0 || bugnumber == 973) 
	{
		bug973(0);
		bug973(1);
	}
	if (bugnumber == 0 || bugnumber == 1304) bug1304();
	if (bugnumber == 0 || bugnumber == 1414) bug1414();
	if (bugnumber == 0 || bugnumber == 1459) bug1459();
	if (bugnumber == 0 || bugnumber == 1460) bug1460();
	if (bugnumber == 0 || bugnumber == 1511) bug1511();
	if (bugnumber == 0 || bugnumber == 1545) bug1545();
	if (bugnumber == 0 || bugnumber == 1621) bug1621();
	if (bugnumber == 0 || bugnumber == 1652) bug1652();
#if 0
	if (bugnumber == 0 || bugnumber == 1676) bug1676();
#endif
	if (bugnumber == 0 || bugnumber == 2269) bug2269();
	if (bugnumber == 0 || bugnumber == 2144) bug2144();
	if (bugnumber == 0 || bugnumber == 2206) bug2206();
	if (bugnumber == 0 || bugnumber == 2477) bug2477();
	if (bugnumber == 0 || bugnumber == 2484) bug2484();
	if (bugnumber == 0 || bugnumber == 2493) bug2493();
	if (bugnumber == 0 || bugnumber == 2548) bug2548();
	if (bugnumber == 0 || bugnumber == 2781) bug2781();
	if (bugnumber == 0 || bugnumber == 2894) bug2894();
	if (bugnumber == 0 || bugnumber == 3057) bug3057();
	if (bugnumber == 0 || bugnumber == 3119) bug3119();
	if (bugnumber == 0 || bugnumber == 3125) bug3125();
	if (bugnumber == 0 || bugnumber == 3155) bug3155();
	if (bugnumber == 0 || bugnumber == 3167) bug3167();
	if (bugnumber == 0 || bugnumber == 3175) bug3175();
	if (bugnumber == 0 || bugnumber == 3188) bug3188();
	if (bugnumber == 0 || bugnumber == 3280) bug3280();
	if (bugnumber == 0 || bugnumber == 4449) bug4449();
	if (bugnumber == 0 || bugnumber == 4612) bug4612();
	if (bugnumber == 0 || bugnumber == 4622) bug4622();
	if (bugnumber == 0 || bugnumber == 4631) bug4631();
	if (bugnumber == 0 || bugnumber == 4725) bug4725();
	if (bugnumber == 0 || bugnumber == 4775) bug4775();
	if (bugnumber == 0 || bugnumber == 4807) bug4807();
	if (bugnumber == 0 || bugnumber == 4219) bug4219();
	if (bugnumber == 0 || bugnumber == 4920) bug4920();
#if 0
	This bug has never been reproduced, so I dont think we 
	need to test for it.

	if (bugnumber == 0 || bugnumber == 4257) bug4257();
#endif
	if (bugnumber == 0 || bugnumber == 4859) bug4859();
	if (bugnumber == 0 || bugnumber == 4888) bug4888();
	if (bugnumber == 0 || bugnumber == 5002) bug5002();
	if (bugnumber == 0 || bugnumber == 5138) bug5138();
	if (bugnumber == 0 || bugnumber == 5203) bug5203();
	if (bugnumber == 0 || bugnumber == 5423) bug5423();
	if (bugnumber == 0 || bugnumber == 5659) bug5659();
	if (bugnumber == 0 || bugnumber == 5505) bug5505();
	if (bugnumber == 0 || bugnumber == 5688) bug5688(); 
	if (bugnumber == 0 || bugnumber == 5552) bug5552();
	if (bugnumber == 0 || bugnumber == 5449) bug5449();
	if (bugnumber == 0 || bugnumber == 5866) bug5866();
	if (bugnumber == 0 || bugnumber == 5923) bug5923();
	if (bugnumber == 0 || bugnumber == 5910) bug5910();
	if (bugnumber == 0 || bugnumber == 6047) bug6047();
	if (bugnumber == 0 || bugnumber == 5881) bug5881();
	if (bugnumber == 0 || bugnumber == 6288) bug6288();
	if (bugnumber == 0 || bugnumber == 6215) bug6215();
	if (bugnumber == 0 || bugnumber == 6376) bug6376();
	if (bugnumber == 0 || bugnumber == 6309) bug6309();
	if (bugnumber == 0 || bugnumber == 6660) bug6660();
}
