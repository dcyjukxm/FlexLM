/******************************************************************************

            COPYRIGHT (c) 1998, 2003  by Macrovision Corporation.
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
 *	Module: $Id: borrtest.c,v 1.16 2003/01/13 22:55:14 kmaclean Exp $
 *
 *      D. Birns
 *      10/98
 *
 *	Last changed:  12/29/98
 *
 */

#define BORRMETER "fv:borrtest.met"
#define BORRMETER2 "fv:bor2test.met"
#define BORRMETERF "borrtest.met"
#define BORRMETER2F "bor2test.met"
#define BORRLIC "borrow.dat"
#define TBORRLIC "tmp.dat"
#define CLIENT_METER "fv:cbortest.met"
#define CLIENT_METERF "cbortest.met"
#define OPTIONS_FILE "demo.opt"
char *feature  = "f1";
char *device = BORRMETER;
char *client_device = CLIENT_METER;
char *borrmeter2 = BORRMETER2;
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_attr.h"
#include <string.h>
#include <errno.h>
#ifdef SUPPORT_METER_BORROW
VENDORCODE code;


LM_HANDLE *job = 0, *job2 = 0, *job3 = 0, *job4 = 0;
char license[MAX_CONFIG_LINE + 1];
char tlicense[MAX_CONFIG_LINE + 1];

LM_HANDLE * new_job(int line, char *path);
void borrow(int line, int exp , char *feat, char *comp, LM_HANDLE *job);
static  void flush_server(LM_HANDLE *job);

FILE *ofp = stdout;
int whichtest;
int testnum = 0;
char *snval;
int counter, val;
unsigned int lock;
char *update_keyval;
CONFIG *conf;
LM_VD_FEATURE_INFO fi; 
#define TESTCOUNTER 4
#define PKGCOUNTER 5
#define EXCLUDECOUNTER 6
#define INCLUDECOUNTER 7
#define LOWWATERCOUNTER 8
#define INCR 8
LM_HANDLE * new_job();
LM_HANDLE *sjob = 0;
int reset = 1;

void
main(int argc, char *argv[])
{
  int i;

	borrsetup();
  	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-o"))
		{
			i++;
			if (!(ofp = fopen(argv[i], "w")))
			{
				fprintf(stderr, "Can't open %s");
				perror("");
				exit(1);
			}
		}
		else if (!strcmp(argv[i], "-servermeter"))
		{
			i++;
			device = argv[i];
		}
		else if (!strcmp(argv[i], "-meter2"))
		{
			i++;
			borrmeter2 = argv[i];
		}
		else if (!strcmp(argv[i], "-clientmeter"))
		{
			i++;
			client_device = argv[i];
		}
		else if (!strcmp(argv[i], "-noreset"))
			reset = 0;
		else if (isdigit(argv[i][0]))
		{
			if (argv[i][0] == '0')
			{
				doinit();
				exit(0);
			}
			whichtest = atoi(argv[i]);
		}
		else usage();
		
	}
#if !defined (_MSC_VER) 
	setvbuf(ofp, 0, _IOLBF, BUFSIZ);
	setvbuf(stderr, 0, _IOLBF, BUFSIZ);
#endif
	/*if (!whichtest) doinit();*/
	fprintf(ofp, "+++++++ API\n");
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
		setup_test(testnum, "serial number");
		sn(__LINE__);
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  long pos = 0;
	  int stat;
	  int i;

	  	

		setup_test(testnum, "dump");
		sn(__LINE__);
		i = 0;
		fputs("lc_borrow_dump test disabled for now!!!!\n", stdout);
#if 0
	  	while (!(stat = lc_borrow_dump(job, LM_BORROW_METER, &device, 
				&counter, &val, &lock, &pos)))
		{
			if (i == 0) 
			{
				if ((counter != TESTCOUNTER) || (val != 1))
				{
					fprintf(ofp, 
				"dump failed expected %d/%d, got %d/%d\n", 
						TESTCOUNTER, 1, counter, val);
				}

			}
			i++;
		}
		if (stat < 0) 
		{
			fprintf(ofp, "borrow _sn failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		}
#endif
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
		setup_test(testnum, "get-counter");

		get_counter(__LINE__);
		if ((val != 1) || !lock)
			fprintf(ofp, "expected 1, nonzero, got %d %u %s/%d\n",
				val, lock, __FILE__, __LINE__);
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
		setup_test(testnum, "update_key");

		sn(__LINE__);
		update_key(__LINE__);
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
		setup_test(testnum, "increment");

		sn(__LINE__);
		update_key(__LINE__);
		increment(__LINE__);
		get_counter(__LINE__);
		if ((val != INCR + 1) || !lock)
			fprintf(ofp, "expected %d, nonzero, got %d %u %s/%d\n",
				INCR + 1, val, lock, __FILE__, __LINE__);
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
		setup_test(testnum, "free_counter");
		free_counter(__LINE__);
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
		setup_test(testnum, "increment");

		sn(__LINE__);
		update_key(__LINE__);
		increment(__LINE__);
		get_counter(__LINE__);
		if ((val != INCR + 1) || !lock)
			fprintf(ofp, "expected %d, nonzero, got %d %u %s/%d\n",
				INCR + 1, val, lock, __FILE__, __LINE__);
		doinit(); /* reset */
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  int stat;
		setup_test(testnum, "checkout");

		checkout(__LINE__, job, 0, 0);
		job2 = new_job(__LINE__, "borrow.dat");
		checkout(__LINE__, job2, LM_MAXUSERS, 0);
		lc_checkin(job, feature, 1);
		lc_checkin(job2, feature, 0);
		lc_free_job(job2);
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  int stat;

		setup_test(testnum, "borrow");

	  	*tlicense = 0;

		
		borrow(__LINE__, 0, 0,0, 0);
		checkout(__LINE__, job, LM_BORROW_METEREMPTY, 0); /* should fail */
		unborrow(__LINE__, 0, feature);
		checkout(__LINE__, job, 0, 0); /* should succeed */
		unlink(TBORRLIC);
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  int stat;

		setup_test(testnum, "borrow package");
		borrow(__LINE__, 0, "pkg", "comp2", 0);
		job2 = new_job(__LINE__, TBORRLIC);
		checkout(__LINE__, job2, 0, "comp1");
		unborrow(__LINE__, 0, "pkg");
		checkout(__LINE__, job2, LM_BORROW_ERROR, "comp1");
		lc_free_job(job2);

	}

	lc_free_job(job); /* reinit everything */
	borrsetup();

	fprintf(ofp, "+++++++ lmutil funcs\n");
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  char lockstr[100];
	  char sn[100];
	  unsigned int lock;
	  char *update_key;
		setup_test(testnum, "update server");
		if (l_borrow_startupdate_server(job, LM_BORROW_METER, device, 
			"demo", TESTCOUNTER, lockstr, sn))
			fprintf(ofp, "startupdate_server failed: %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		sscanf(lockstr, "%u", &lock);
		if (lc_borrow_update_key(job, LM_BORROW_METER,
			sn, TESTCOUNTER, 2, lock, &update_key) || 
				!update_key)
			fprintf(ofp, "update_key failed: %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (l_borrow_update_server(job, LM_BORROW_METER, device, 
			"demo", TESTCOUNTER, 2, update_key))
			fprintf(ofp, "update_server failed: %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		free(update_key);
/*
 *		Server now has 3 licenses, make sure
 */
		if (!(conf = lc_get_config(job, feature)))
			fprintf(ofp, "get_config failed %d %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
 		if (!(conf = lc_get_config(job, feature)))
			fprintf(ofp, "getconfig failed: %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		fi.type = LM_VD_FEATINFO_HANDLE_TYPE; fi.feat = conf;
		if (lc_get_attr(job, LM_A_VD_FEATURE_INFO, (short *)&fi) )
			fprintf(ofp, "FEATURE_INFO failed: %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if( fi.num_lic != 3)
			fprintf(ofp, "serv wrong exp %d got %d failed:%s/%d\n", 
				3, fi.num_lic, __FILE__, __LINE__);
	}
	fprintf(ofp, "+++++++ error conditions\n");
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  int stat;
		setup_test(testnum, "NONETOBORROW");
		job2 = new_job(__LINE__, "borrow.dat");
		job3 = new_job(__LINE__, "borrow.dat");
		job4 = new_job(__LINE__, "borrow.dat");
		checkout(__LINE__, job, 0, 0); /* should succeed */
		checkout(__LINE__, job2, 0, 0); /* should succeed */
		checkout(__LINE__, job3, 0, 0); /* should succeed */
		checkout(__LINE__, job4, LM_MAXUSERS, 0); /* should fail */
		borrow(__LINE__, LM_NONETOBORROW, feature, 0, job4);
		lc_free_job(job2);
		lc_free_job(job3);
		lc_free_job(job4);
		job2 = job3 = job4 = 0;

	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  int stat;
		setup_test(testnum, "BORROWED_ALREADY");
		borrow(__LINE__, 0, feature, 0, 0);
		borrow(__LINE__, LM_BORROWED_ALREADY, feature, 0, 0);
		unborrow(__LINE__, 0, feature);
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  int stat;
		setup_test(testnum, "unborrow twice");
		borrow(__LINE__, 0, feature, 0, 0);
		unborrow(__LINE__, 0, feature);
/*
 *		NOTE:  We would like this error to be UNBORROW_ALREADY, but
 *		I'm currently removing the counter when you do an unborrow --
 *		so that you won't needlessly run out of counters.
 */
		unborrow(__LINE__, LM_UNBORROWED_ALREADY, feature);
	}
	fprintf(ofp, "+++++++ Advanced tests\n");
	testnum++;
	if (!whichtest || (whichtest == testnum) || whichtest == (testnum + 1))
	{
	  int stat;
		setup_test(testnum, "Add to meter, reread");
		job2 = new_job(__LINE__, "borrow.dat");
		checkout(__LINE__, job2, LM_BORROW_METEREMPTY, "b100");
		if (lc_borrow_init(job, LM_BORROW_METER, &borrmeter2))
			fprintf(ofp, "borrow init failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			100, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			101, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			102, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			103, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			104, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			105, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			106, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			107, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			108, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if (lc_borrow_make_counter( job, LM_BORROW_METER, &borrmeter2,
			109, 1, &lock))
			fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
				lc_errstring(job), __FILE__, __LINE__);
		if ((stat  = lc_borrow_make_counter( job, LM_BORROW_METER, 
				&borrmeter2, 110, 1, &lock) )
					!= LM_BORROW_NOCOUNTER)
			fprintf(ofp, "exp %d got %d  %s/%d\n", 
				LM_BORROW_NOCOUNTER, stat, __FILE__, __LINE__);
		reread(__LINE__);
		checkout(__LINE__, job2, 0, "b100");
		lc_checkin(job2, "b100", 0);
		lc_free_job(job2);
	}
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  int stat;
		setup_test(testnum, "Use up client meter");
		borrow(__LINE__, 0, "b100", 0, 0);	
		borrow(__LINE__, 0, "b101", 0, 0);	
		borrow(__LINE__, 0, "b102", 0, 0);	
		borrow(__LINE__, 0, "b103", 0, 0);	
		borrow(__LINE__, 0, "b104", 0, 0);	
		borrow(__LINE__, 0, "b105", 0, 0);	
		borrow(__LINE__, 0, "b106", 0, 0);	
		borrow(__LINE__, 0, "b107", 0, 0);	
		borrow(__LINE__, 0, "b108", 0, 0);	
		borrow(__LINE__, 0, "b109", 0, 0);	
		borrow(__LINE__, LM_BORROW_NOCOUNTER, "f1", 0, 0); 
		unborrow(__LINE__, 0, "b100");
		borrow(__LINE__, 0, "f1", 0, 0);	
/*
 *		test done, now cleanup
 */
		unborrow(__LINE__, 0, "f1", 0, 0);	
		unborrow(__LINE__, 0, "b101", 0, 0);	
		unborrow(__LINE__, 0, "b102", 0, 0);	
		unborrow(__LINE__, 0, "b103", 0, 0);	
		unborrow(__LINE__, 0, "b104", 0, 0);	
		unborrow(__LINE__, 0, "b105", 0, 0);	
		unborrow(__LINE__, 0, "b106", 0, 0);	
		unborrow(__LINE__, 0, "b107", 0, 0);	
		unborrow(__LINE__, 0, "b108", 0, 0);	
		unborrow(__LINE__, 0, "b109", 0, 0);	
	}
	fprintf(ofp, "+++++++ Options file\n");
	testnum++;
	if (!whichtest || (whichtest == testnum))
	{
	  LM_HANDLE *notme;

		setup_test(testnum, "Options file");
		notme = new_job(__LINE__, "borrow.dat");
		lc_set_attr(notme, LM_A_USER_OVERRIDE, 
						(LM_A_VAL_TYPE)"notme");
/*
 *		First, make sure checkout is unaffect by EXCLUDE_BORROW and
 *		INCLUDE_BORROW:
 */
		checkout(__LINE__, job, 0, "exclude_borrow");
		lc_checkin(job, "exclude_borrow", 0);
		checkout(__LINE__, job, 0, "include_borrow");
		lc_checkin(job, "include_borrow", 0);
		checkout(__LINE__, notme, 0, "exclude_borrow");
		lc_checkin(job, "exclude_borrow", 0);
		checkout(__LINE__, notme, 0, "include_borrow");
		lc_checkin(job, "include_borrow", 0);
/*
 *		Now test exclude/include
 */
		borrow(__LINE__, 0, "exclude_borrow", 0, 0); 
		unborrow(__LINE__, 0, "exclude_borrow"); 
		borrow(__LINE__, LM_FEATEXCLUDE, "exclude_borrow", 0, notme); 
		borrow(__LINE__, 0, "include_borrow", 0, 0); 
		unborrow(__LINE__, 0, "include_borrow"); 
		borrow(__LINE__, LM_FEATNOTINCLUDE, "include_borrow", 0, notme);
/*
 *		borrow_lowwater, 2 checkouts should succeed, but 2 borrows 
 *		should fail
 */
		checkout(__LINE__, job, 0, "borrow_lowwater");
		checkout(__LINE__, notme, 0, "borrow_lowwater");
		lc_checkin(job, "borrow_lowwater", 0);
		borrow(__LINE__, 0, "borrow_lowwater", 0, 0);
		if (lc_borrow_free_counter(job, LM_BORROW_METER,
					&client_device, LOWWATERCOUNTER))
			fprintf(ofp, "free_counter failed %s %s/%d\n",
				lc_errstring(job), __FILE__, __LINE__);
		borrow(__LINE__, LM_NONETOBORROW, "borrow_lowwater", 0, 0);
		lc_checkin(notme, "borrow_lowwater", 0);
		borrow(__LINE__, LM_NONETOBORROW, "borrow_lowwater", 0, 0);
		lc_free_job(notme);
	}

	if (reset)
	{
		fprintf(ofp, "Reset\n");
		/* reset */
		if (sjob) lc_free_job(sjob);
		if (job2) lc_free_job(job2);
		if (job3) lc_free_job(job3);
		if (job) lc_free_job(job);
		borrsetup();	
		doinit();
		reread();
	}

	if (ofp != stdout) fclose(ofp);

	exit(0);
}
reread(int line)
{
  LM_HANDLE *rjob;
  char buf[50];
  int stat;

	rjob = new_job(__LINE__, BORRLIC);
        if (la_reread(rjob, 0, 0, 0) <= 0) 
                printf("la_reread failed", lc_errstring(rjob));
        lc_free_job(rjob);
}
doinit()
{
  unsigned int lock;
  FILE *fp;
  char *me;
  LM_METERS *d = 0;
  char devices[50][50];
  int i;

	unlink(BORRMETERF); 
	unlink(BORRMETER2F); 
	unlink(CLIENT_METERF);
	unlink(TBORRLIC);
	unlink(OPTIONS_FILE);
        memset(devices, 0, sizeof(devices));
        for (lc_borrow_find_meter(job, &d ), i = 0; d; 
                        d = d->next, i++)
        {
                strcpy(devices[i], d->device);
        }
        for (i = 0; *devices[i]; i++)
        {
                
           char *cp = devices[i];
		if (lc_borrow_init(job, LM_BORROW_METER, &cp))
		{
		        printf("Can't init %s: %s\n", cp, lc_errstring(job));
		}
        }
/*
 *	Initialize client-device
 */
	if (lc_borrow_init(job, LM_BORROW_METER, &client_device))
		fprintf(ofp, "lc_borrow_init %s failed: %s %s/%d", 
			client_device, lc_errstring(job), __FILE__, __LINE__);
	if (lc_borrow_init(job, LM_BORROW_METER, &device))
		fprintf(ofp, "borrow init failed %s %s/%d\n", lc_errstring(job),
			__FILE__, __LINE__);
	if (lc_borrow_make_counter( job, LM_BORROW_METER, &device,
		TESTCOUNTER, 1, &lock))
		fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, __LINE__);
	if (lc_borrow_make_counter( job, LM_BORROW_METER, &device,
		PKGCOUNTER, 1, &lock))
		fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, __LINE__);
	if (lc_borrow_make_counter( job, LM_BORROW_METER, &device,
		EXCLUDECOUNTER, 4, &lock))
		fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, __LINE__);
	if (lc_borrow_make_counter( job, LM_BORROW_METER, &device,
		INCLUDECOUNTER, 4, &lock))
		fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, __LINE__);
	if (lc_borrow_make_counter( job, LM_BORROW_METER, &device,
		LOWWATERCOUNTER, 4, &lock))
		fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, __LINE__);
/*
 *	Make options file
 */
	me = lc_username(job, 0);
	if (!(fp = fopen(OPTIONS_FILE, "w")))
	{
		perror("Can't open demo.opt, exiting...");
		exit(1);
	}
	fprintf(fp, "EXCLUDE_BORROW exclude_borrow USER notme\n");
	fprintf(fp, "INCLUDE_BORROW include_borrow USER %s\n", me);
	fprintf(fp, "BORROW_LOWWATER borrow_lowwater 3\n");
	fprintf(fp, "REPORTLOG borrow.rl\n");
	fclose(fp);
}
borrsetup()
{
  int stat;
	if (stat = lc_new_job(0, 0, &code, &job))
		fprintf(ofp, "new_job failed %d %s/%d\n", stat, 
			__FILE__, __LINE__);

	if (lc_set_attr(job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)"borrow.dat"))
		fprintf(ofp, "lic path failed %d %s/%d\n", lc_errstring(job),
			__FILE__, __LINE__);
	if (lc_set_attr(job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)0))
		fprintf(ofp, "setattr failed %d %s/%d\n", lc_errstring(job),
			__FILE__, __LINE__);
		
}
sn(line)
{
	if (snval) free(snval);
	if (lc_borrow_sn( job,  LM_BORROW_METER, &device, &snval))
		fprintf(ofp, "borrow _sn failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, line);

}
get_counter(line)
{
	if (lc_borrow_get_counter( job,  LM_BORROW_METER, &device,
		TESTCOUNTER, &val, &lock) )
		fprintf(ofp, "get_counter failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, line);
}
update_key(line)
{
  int a, b, c, d;
	if (update_keyval) free(update_keyval);
	update_keyval = 0;
	if (lc_borrow_update_key(job, LM_BORROW_METER, snval, TESTCOUNTER, 
			INCR, lock, &update_keyval) || !update_keyval)
		fprintf(ofp, "get_counter failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, line);
	
	sscanf(update_keyval, "%d %d %d %d", &a, &b, &c, &d);
	if ((!a && !b) || (!c && !d))
		fprintf(ofp, "get_counter invalid update-keyval %s %s/%d\n", 
			update_keyval, __FILE__, line);
}
increment(line)
{
	if (lc_borrow_increment(job,
	        LM_BORROW_METER, &device, TESTCOUNTER, INCR, update_keyval))
		fprintf(ofp, "increment failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, line);

}
free_counter(line)
{
	if (lc_borrow_free_counter(job, LM_BORROW_METER, &device, TESTCOUNTER))
		fprintf(ofp, "increment failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, line);
	if (!lc_borrow_get_counter( job,  LM_BORROW_METER, &device,
		TESTCOUNTER, &val, &lock) )
		fprintf(ofp, 
		"get_counter succeeded after free_counter %s %s/%d\n", 
			__FILE__, line);
	if (lc_borrow_make_counter( job, LM_BORROW_METER, &device,
		TESTCOUNTER, 1, &lock))
		fprintf(ofp, "borrow make counter failed %s %s/%d\n", 
			lc_errstring(job), __FILE__, __LINE__);
	get_counter(line);
}
checkout(line, job, exp, feat)
LM_HANDLE *job;
char *feat; /* defaults to feature */
{
  int stat;
	stat = lc_checkout(job, feat ? feat : feature, "1.0", 1, LM_CO_NOWAIT, 
		&code, LM_DUP_NONE);
	if (stat != exp)
		fprintf(ofp, "checkout failed: exp %d, got %d %s %s/%d\n", 
			exp, stat, lc_errstring(job), __FILE__, line);
}
LM_HANDLE *
new_job(line, path)
char *path;
{
  int stat;
  LM_HANDLE *ret;
	if (stat = lc_new_job(0, 0, &code, &ret))
		fprintf(ofp, "new_job failed %d %s/%d\n", stat, 
			__FILE__, line);
	if (lc_set_attr(ret, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1))
		fprintf(ofp, "setattr failed %d %s/%d\n", 
		lc_errstring(job2), __FILE__, __LINE__);
	if (path)
	{
		if (lc_set_attr(ret, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1))
			fprintf(ofp, "setattr failed %d %s/%d\n", 
			lc_errstring(ret), __FILE__, line);
		if (lc_set_attr(ret, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)path))
			fprintf(ofp, "setattr %s failed %d %s/%d\n",  path,
			lc_errstring(ret), __FILE__, line);
	}
	return ret;
}
void
borrow(int line, int exp , char *feat, char *comp, LM_HANDLE *bjob)
{
  FILE *fp;
  int got;
  	if (!bjob) bjob = job;
  	if (!feat) feat = feature;
  	if (!comp) comp = feat;
	got = lc_borrow(bjob, LM_BORROW_METER, &client_device, feat, 
		"1.0", &code, tlicense, 0, 0, 0, 0);
	if(exp)
	{
		if (exp != got)
			fprintf(ofp, "failed but status exp %d got %d %s/%d\n",
				exp, got, __FILE__, line);
		return;
	}
	else if (got)
	{
		fprintf(ofp, "borrow failed %s %s/%d\n",
			lc_errstring(bjob) , __FILE__, line);
	        return;
        }
        flush_server(bjob);
	if (!(fp = fopen(TBORRLIC, "a")))
	{
		fprintf(ofp, "Can't open %s: %s %s/%d", 
			TBORRLIC, strerror(errno), __FILE__, __LINE__);
		
	}
	fprintf(fp, "%s\n", tlicense);
	fclose(fp);
	job2 = new_job(__LINE__, TBORRLIC);
	checkout(__LINE__, job2, 0, comp );
	conf = lc_auth_data(job2, comp );
	if (!conf || !conf->idptr || 
		(conf->idptr->type != HOSTID_METER_BORROW))
	{
		fprintf(ofp, "checkout borrowed license wrong: %s/%d\n",
			__FILE__, __LINE__);
	}
	lc_checkin(job2, comp , 1);
	lc_free_job(job2);
}
unborrow(int line, int exp, char *feat)
{
  int stat;
  LM_HANDLE *ujob;
	ujob = new_job(line, TBORRLIC);
	if ((stat = lc_unborrow(ujob, LM_BORROW_METER, &client_device, feat, 
			0, 0, 0, 0)) != exp)
		fprintf(ofp, "unborrow exp %d got %d: %s %s/%d\n", 
			exp, stat, lc_errstring(ujob), __FILE__, line);
        flush_server(job);
	lc_free_job(ujob);
}


usage()
{
	fprintf(stderr, "borrtest [-o outfile] [testnum] [-clientmeter device] [-servermeter device]\n");
	exit(1);
}
setup_test(int testnum, char *str)
{

  char msg[200];
	fprintf(ofp, "---------------Test %d. %s\n", testnum, str);
	if (!sjob) 
	{
		sjob = new_job(__LINE__, "borrow.dat");
		l_connect_by_conf(sjob, 0);
	}
	sprintf(msg, "--------------%s--------------\n", str);
	lc_log(sjob, msg);
}

static 
void
flush_server(LM_HANDLE *job)
{
  LM_HANDLE *lm_job;
  char *ret;
	if (!(ret = lc_vsend(job, "flush")) || strcmp(ret, "flushed"))
		printf("flush 1 error: %s", lc_errstring(job));
	else if (!(ret = lc_vsend(job, "test")) || strcmp(ret, "ok"))
		printf("flush 2 error: %s", lc_errstring(job));
}
#else /* SUPPORT_METER_BORROW */
main()
{
	fputs("meter_borrow unsupported", stdout);
}
#endif /* SUPPORT_METER_BORROW */
