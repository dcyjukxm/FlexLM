/******************************************************************************

	    COPYRIGHT (c) 1997, 2003 by Macrovision Corporation.
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
 *	Module: $Id: st_vers.c,v 1.137 2003/04/29 23:54:11 sluu Exp $
 *
 *	D. Birns
 *	8/97
 *
 *	Last changed:  11/18/98
 *
 */
#include "lmachdep.h"
#include "code.h"
#define TEST_DEMOF
#include "lmclient.h"
#include "l_prot.h"
#include "lmpolicy.h"
#include "lmpubkey.h"
#include "lmprikey.h"
#include "lm_attr.h"
#include "l_prot.h"
#include "l_borrow.h"
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
extern char *delim;
extern LM_HANDLE *job[];
extern LM_VD_FEATURE_INFO fi;
extern LM_VD_GENERIC_INFO gi;
extern int bugnumber;
extern LM_HANDLE_PTR lm_job;
extern CONFIG *conf, *pos;
extern VENDORCODE vc;
extern VENDORCODE lic_vc;
extern char hostid [];
extern char *replace();
static void date_test lm_args((char *));

extern FILE *ofp;

#ifdef VMS
#define PORT 200
#define RPORT -1
#else
#define PORT 2837
#define RPORT PORT
#endif /* VMS */
static void test_registry(char *key, char *exp, int len, int line);
#ifdef LM_UNIX_MT
static void st_unixmt(void);
#endif /* LM_UNIX_MT */
static void st_borrow_test(int flag);


v7_new_job(v, job)
VENDORCODE *v;
LM_HANDLE **job;
{
  extern LM_HANDLE *main_job;

	if (lc_new_job(main_job, 0, v, job))
		fprintf(ofp, "Error lc_new_job line %d %s\n", __LINE__,
			lc_errstring(*job));
	st_set_attr(*job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0, __LINE__);
	st_set_attr(*job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE) 1, __LINE__);
	lc_set_registry(*job, "LM_LICENSE_FILE", 0);
	lc_set_registry(*job, "DEMO_LICENSE_FILE", 0);
}
test45()
{
  int i, rc;
  FILE *err_fp;
  char buf[MAX_CONFIG_LINE + 1];
  LP_HANDLE *lp;
  char *err, *decstr, *licstring;
  CONFIG *conf;
  extern (*L_UNIQ_KEY5_FUNC)();
  char *sav_env = 0;
  char *sav_demoenv = 0;

	serv_log( "45 v6 Attributes\n");
	{
	  char *cp = getenv("LM_LICENSE_FILE");
		if (cp) 
		{
			sav_env = malloc(strlen(cp) + 1);
			strcpy(sav_env, cp);
		}
		cp = getenv("DEMO_LICENSE_FILE");
		if (cp) 
		{
			sav_demoenv = malloc(strlen(cp) + 1);
			strcpy(sav_demoenv, cp);
		}
	}


	ts_lc_new_job(&code, &lm_job, __LINE__); 
	x_flexlm_newid();
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)".");
	pos = 0;
	while (conf = lc_next_conf(lm_job, "f22", &pos))
	{
		if (lc_check_key(lm_job, conf, &code))
		{
			fprintf(ofp, "lc_check_key error line %d %s %s %s\n", 
						__LINE__, 
					conf->feature, conf->code,
					lc_errstring(lm_job));
		}
				
	}
	pos = 0;
	conf = lc_next_conf(lm_job, "e1", &pos);
	if (!lc_check_key(lm_job, conf, &code))
	{
			fprintf(ofp, "lc_check_key error line %d %s %s\n", 
						__LINE__, 
					conf->feature, conf->code);
	}
	lc_free_job(lm_job);
	ts_lc_new_job( &code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)".");
	st_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0, __LINE__);
        if (lc_checkout(lm_job, "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "f1 checkout failed line %d \n", __LINE__,
			lc_errstring(lm_job));
	if (!(err_fp = fopen("errmsgs.out", "w")))
	{
		fputs("Can't open errmsgs.out, exiting", ofp);
		exit(1);
	}
		
	for (i = 0; i >= LM_LAST_ERRNO; i--)
	{
	  int j = i * -1;

		LM_SET_ERROR(lm_job, i, j, j, "teststring", LM_ERRMASK_ALL);
		st_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0, __LINE__);
		fprintf(err_fp, "%s\n", lc_errstring(lm_job));
		st_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 1, __LINE__);
		fprintf(err_fp, "%s\n---------------------------------------------------------------\n", lc_errstring(lm_job));
	}
	fclose(err_fp);
	lc_free_job(lm_job);

/*
 *	Decimal format tests
 */
	strcpy(buf, 
	"START_LICENSE\nFEATURE lic_string demo 1.0 permanent uncounted E5DEABDD3A2F HOSTID=ANY\nEND_LICENSE");
        if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "lic_string", "1.0", 1, buf, &lp))
		fprintf(ofp, "lic_string failed, error line %d %s\n", __LINE__,
			lp_errstring(lp));
	lp_checkin(lp);
	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	memset(&vc, 0, sizeof(vc));
	memcpy(&vc, (char *)&lic_vc, sizeof(vc));
	vc.data[0] ^= VENDOR_KEY5;
	vc.data[1] ^= VENDOR_KEY5;
	if (lc_cryptstr(lm_job, "FEATURE lic_string demo 1.0 permanent \
				uncounted 0 HOSTID=ANY ", &decstr, &vc, 
				LM_CRYPT_FORCE| LM_CRYPT_DECIMAL_FMT, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(lm_job), VENDOR_KEY5);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", decstr ? decstr : "");
        if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "lic_string", "1.0", 1, buf, &lp))
		fprintf(ofp, "lic_string failed, error line %d %s decstr %s\n", 
			__LINE__, lp_errstring(lp), decstr);
	lp_checkin(lp);
	lc_free_mem(lm_job,decstr);

	sprintf(buf,  "FEATURE lic_string demo 1.0 1-jan-2020 uncounted 0 \
			HOSTID=%s VENDOR_STRING=12345678", hostid);
	if (lc_cryptstr(lm_job, buf, &decstr, &vc, 
			LM_CRYPT_FORCE| LM_CRYPT_DECIMAL_FMT, "string", &err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(lm_job), VENDOR_KEY5);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", decstr ? decstr : "");
        if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "lic_string", "1.0", 1, buf, &lp))
		fprintf(ofp, "lic_string failed, error line %d %s decstr %s\n", 
			__LINE__, lp_errstring(lp), decstr);
	lp_checkin(lp);
	lc_free_mem(lm_job,decstr);
	lc_free_job(lm_job);

/*
 *	Check compatibility rules with new license format
 */
/*
 *	1. If you set long key, short keys will fail
 */
	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_get_attr(lm_job, LM_A_LKEY_LONG, (short *)&i);
	if (i) fprintf(ofp, "Error line %d -- default wrong for lkey length\n",
							__LINE__);
	if (lc_cryptstr(lm_job, "FEATURE lic_string demo 1.0 permanent \
				uncounted 0 HOSTID=ANY", &licstring, &vc, 
				LM_CRYPT_FORCE, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(lm_job), VENDOR_KEY5);
	}
	lc_free_job(lm_job);
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", licstring ? licstring 
							: "");
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "error line %d %s\n", __LINE__, 
			lc_errstring(lm_job));
	}
	lc_free_job(lm_job);
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1);
	if (!lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE)  || lm_job->lm_errno != LM_BADCODE)
	{
		fprintf(ofp, "checkout should have failed %d with lic %s with err\n",
			__LINE__, licstring, lm_job->lm_errno);
	}
	lc_free_mem(lm_job,licstring);
	lc_free_job(lm_job);

	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "error line %d %s\n", __LINE__, 
			lc_errstring(lm_job));
	}
	lc_free_job(lm_job);

/*
 *	1. If you set long  and start date, long keys w/o start will fail
 */

	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1);
	if (lc_cryptstr(lm_job, "FEATURE lic_string demo 1.0 permanent \
				uncounted 0 HOSTID=ANY", &licstring, &vc, 
				LM_CRYPT_FORCE, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(lm_job), VENDOR_KEY5);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", licstring ? licstring 
							: "");
	lc_free_mem(lm_job,licstring);
	lc_free_job(lm_job);
	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_LKEY_START_DATE, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_USE_START_DATE, (LM_A_VAL_TYPE)1);
	if (!lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE)  || lm_job->lm_errno != LM_BADCODE)
	{
		fprintf(ofp, "checkout should have failed %d with lic %s with err\n",
			__LINE__, licstring, lm_job->lm_errno);
	}
	lc_free_job(lm_job);
/*
 *	Sanity check -- try it again where it should succeed 
 */
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n",
			__LINE__, lc_errstring(lm_job));
	}
	lc_free_job(lm_job);
/*
 *	Now try both long key and start date
 */
	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_LKEY_START_DATE, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_USE_START_DATE, (LM_A_VAL_TYPE)1);
	if (lc_cryptstr(lm_job, "FEATURE lic_string demo 1.0 permanent \
				uncounted 0 HOSTID=ANY", &licstring, &vc, 
				LM_CRYPT_FORCE, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(lm_job), VENDOR_KEY5);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", licstring ? licstring 
							: "");
	lc_free_mem(lm_job,licstring);
	lc_free_job(lm_job);
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_LKEY_LONG, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_LKEY_START_DATE, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_USE_START_DATE, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n",
			__LINE__, lc_errstring(lm_job));
	}
	lc_free_job(lm_job);
/*
 *	Now try key long with start-date where job is short, should succeed
 */
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n",
			__LINE__, lc_errstring(lm_job));
	}
	lc_free_job(lm_job);
/*
 *	VENDOR_LICENSE_FILE
 */
/*
 *	First, sanity check
 */
	ts_lc_new_job(&code, &lm_job, __LINE__);  
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
 	lc_set_registry(lm_job, "DEMO_LICENSE_FILE", 0);
	if (!lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout succeeded %d\n", __LINE__);
	}
	lc_free_job(lm_job);
        setenv("DEMO_LICENSE_FILE", buf);
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n",
			__LINE__, lc_errstring(lm_job));
	}
	lc_free_job(lm_job);
        unsetenv("DEMO_LICENSE_FILE");
        unsetenv("LM_LICENSE_FILE");
/*
 *	Test @localhost
 */
	
/*
 *	First, sanity check
 */
	ts_lc_new_job(&code, &lm_job, __LINE__);  
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (!lc_checkout(lm_job, "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout succeeded %d\n", __LINE__);
	}
	lc_free_job(lm_job);

        setenv("DEMO_LICENSE_FILE", "@localhost");
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (lc_checkout(lm_job, "f1", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n",
			__LINE__, lc_errstring(lm_job));
	}
	lc_free_job(lm_job);
/*
 *	License Files are case insensitive
 */
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n",
			__LINE__, lc_errstring(lm_job));
	}
	lc_free_job(lm_job);

	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	if (lc_checkout(lm_job, "Lic_String", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n", __LINE__, 
						lc_errstring(lm_job));
	}
	lc_free_job(lm_job);
	{
	 char *cp = replace(buf, "demo", "DemO");
		strcpy(buf, cp);
		cp = replace( buf, "HOSTID", "HostID");
		strcpy(buf, cp);
	}

	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n", __LINE__, 
						lc_errstring(lm_job));
	}
	lc_free_job(lm_job);

/*
 *	LM_A_LICENSE_CASE_SENSITIVE
 */
	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (lc_cryptstr(lm_job, "FEATURE lic_string demo 1.0 permanent \
				uncounted 0 HOSTID=ANY", &licstring, &vc, 
				LM_CRYPT_FORCE, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(lm_job), VENDOR_KEY5);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", licstring ? licstring 
							: "");
	lc_free_mem(lm_job,licstring);
	lc_free_job(lm_job);

	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_CASE_SENSITIVE, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	if (!lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout succeeded %d %s\n", __LINE__, 
						lc_errstring(lm_job));
	}
	lc_free_job(lm_job);

	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_CASE_SENSITIVE, (LM_A_VAL_TYPE)1);
	if (lc_cryptstr(lm_job, "FEATURE lic_string demo 1.0 permanent \
				uncounted 0 HOSTID=ANY", &licstring, &vc, 
				LM_CRYPT_FORCE, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(lm_job), VENDOR_KEY5);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", licstring ? licstring 
							: "");
	lc_free_mem(lm_job,licstring);
	lc_free_job(lm_job);

	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_CASE_SENSITIVE, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n", __LINE__, 
						lc_errstring(lm_job));
	}
	lc_free_job(lm_job);

	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
	{
		fprintf(ofp, "checkout failed %d %s\n", __LINE__, 
						lc_errstring(lm_job));
	}
	lc_free_job(lm_job);
/*
 *	V6 attributes -- START=
 */
	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (lc_cryptstr(lm_job, 
			"FEATURE v6_1 demo 1.0 permanent \
			uncounted 0 HOSTID=ANY START=1-jan-1990\n\
			FEATURE v6_2 demo 1.0 permanent \
			uncounted 0 HOSTID=ANY START=1-jan-2020", 
			&licstring, &vc, LM_CRYPT_FORCE, "string", &err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s sig 0x%x\n", __LINE__, 
			err ? err : "", lc_errstring(lm_job), VENDOR_KEY5);
	}
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", licstring ? licstring 
							: "");
	lc_free_mem(lm_job,licstring);
	lc_free_job(lm_job);

        if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "v6_1", "1.0", 1, buf, &lp))
		fprintf(ofp, "v6_1 failed, error line %d %s\n" ,
			__LINE__, lp_errstring(lp));
	lp_checkin(lp);

        if (!lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "v6_2", "1.0", 1, buf, &lp))
		fprintf(ofp, "v6_2 succeeded, error line %d\n" , __LINE__);
	lp_checkin(lp);
/*
 *	lc_check_key();
 */
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)buf);
	if (!(conf = lc_get_config(lm_job, "v6_1")))
		fprintf(ofp, "Can't get config for v6_1 line %d\n", __LINE__);
	if (lc_check_key(lm_job, conf, &code))
		fprintf(ofp, "lc_check_key failed l:%d %s\n", __LINE__,
			lc_errstring(lm_job));
	if (!(conf = lc_get_config(lm_job, "v6_2")))
		fprintf(ofp, "Can't get config for v6_2 line %d\n", __LINE__);
	if (lc_check_key(lm_job, conf, &code))
		fprintf(ofp, "lc_check_key failed l:%d %s\n", __LINE__,
			lc_errstring(lm_job));
	lc_free_job(lm_job);

	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)".");
	if (!(conf = lc_get_config(lm_job, "e1")))
		fprintf(ofp, "Can't get config for e1 line %d\n", __LINE__);
	if (!lc_check_key(lm_job, conf, &code))
		fprintf(ofp, "lc_check_key succeeded l:%d\n", __LINE__);
	lc_free_job(lm_job);

/*
 *	ls_host tests
 */	
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	x_flexlm_newid();
	rc = lc_checkout(lm_job, "g1", "1.0", 1, 0, &code, LM_DUP_NONE);
	if (rc != LM_SERVER_REMOVED)
	{
		fprintf(ofp, "ls_host checkout succeeded line %d, exp %d got %d\n", 
			__LINE__, LM_SERVER_REMOVED, rc);
	}
	ts_flush_server(lm_job, __LINE__);
	rc = lc_checkout(lm_job, "g1", "1.0", 1, 0, &code, LM_DUP_NONE);
	if (rc != LM_SERVER_REMOVED)
	{
		fprintf(ofp, "ls_host checkout succeeded line %d, exp %d got %d\n", 
			__LINE__, LM_SERVER_REMOVED, rc);
	}
	lc_free_job(lm_job);
/*	
 *	test l_upgrade()
 */


	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (!lc_checkout(lm_job, "noprevfeat", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "checkout noprevfeat succeeded line %d\n", __LINE__);
	lc_free_job(lm_job);
/*
 *	test lc_convert
 */
	ts_lc_new_job(&code, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
#define DECIMAL_FORMAT_TEST \
"demo-f0-26945-46154-29142-49269-576\n\
demo-f1-24897-52290-19320-16526-3\n"


	if (lc_convert(lm_job, DECIMAL_FORMAT_TEST, &licstring, &err, 
				LM_CONVERT_TO_READABLE))
		fprintf(ofp, "Error lc_convert line %d %s %s\n", __LINE__,
			lc_errstring(lm_job), err ? err : "");
	if (err) lc_free_mem (lm_job,err);
	if (lc_convert(lm_job, licstring, &decstr, &err, LM_CONVERT_TO_DECIMAL))
		fprintf(ofp, "Error lc_convert line %d %s %s\n", __LINE__,
			lc_errstring(lm_job), err ? err : "");
	if (err) lc_free_mem (lm_job,err);
	if (!L_STREQ(decstr, DECIMAL_FORMAT_TEST))
		fprintf(ofp, "Error lc_convert line %d \n\texp %s\n\tgot %s %d %d\n", 
			__LINE__, DECIMAL_FORMAT_TEST, decstr ? decstr :
				"", strlen(DECIMAL_FORMAT_TEST), 
					decstr ? strlen(decstr) : 0);
	if (decstr) lc_free_mem (lm_job,decstr);
	if (licstring) lc_free_mem (lm_job,licstring);
	lc_free_job(lm_job);
/*
 *	HOSTID=DOMAIN
 */
#if 0
	if (CHECKOUT(LM_RESTRICTIVE, "domain", "1.0", "."))
		fprintf(ofp, "domain hostid failed line %d %s\n", __LINE__, 
			ERRSTRING());

	CHECKIN();
	if (CHECKOUT(LM_RESTRICTIVE, "domain", "1.0", "@localhost"))
		fprintf(ofp, "domain hostid failed line %d %s\n", __LINE__, 
			ERRSTRING());
	CHECKIN();
	if (!CHECKOUT(LM_RESTRICTIVE, "bad_domain", "1.0", "."))
		fprintf(ofp, "domain hostid worked with bad_domain line %d\n", 
			__LINE__);
	CHECKIN();
	if (!CHECKOUT(LM_RESTRICTIVE, "bad_domain", "1.0", "@localhost"))
		fprintf(ofp, "domain hostid worked with bad_domain line %d\n", 
			__LINE__);
	CHECKIN();
#endif
	if (sav_demoenv)
	{
		setenv("DEMO_LICENSE_FILE", sav_demoenv);
		free(sav_demoenv);
	}
	if (sav_env)
	{
		setenv("LM_LICENSE_FILE", sav_env);
		free(sav_env);
	}
	  
}
test46()
{
  VENDORCODE v;
  char *lic, *err, licbuf[200];
  extern user_crypt_filter_gen();
  extern user_crypt_filter();
  LM_HANDLE *lm_job;

	serv_log( "46 -- v6.1 attributes\n");

	v7_new_job(&v,&lm_job);
	if (lc_checkout(lm_job, "f1", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error lc_new_job/checkout line %d %s\n", __LINE__,
			lc_errstring(lm_job));
	lc_free_job(lm_job);
/*
 *	Now make sure lc_init works
 */
	ts_lc_new_job( &v, &lm_job, __LINE__); 
	if (lc_checkout(lm_job, "f1", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error lc_new_job/checkout line %d %s\n", __LINE__,
			lc_errstring(lm_job));
	lc_free_job(lm_job);
#if 1
/*
 *	Generate license using filter
 */

	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	memcpy((char *)&v, (char *)&lic_vc, sizeof(v));
	v.data[0] ^= VENDOR_KEY5;
	v.data[1] ^= VENDOR_KEY5;
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_USER_CRYPT_FILTER_GEN, 
				(LM_A_VAL_TYPE)user_crypt_filter_gen);

	if (lc_cryptstr(lm_job, 
		"FEATURE testfilter demo 1.0 permanent uncounted 0 HOSTID=demo",
		&lic, &v, LM_CRYPT_FORCE, "test", &err))
	{
		fprintf(ofp, "cryptstr failed line %d: %s", __LINE__, err?err:"");
		if (err) free(err);
		*licbuf = 0;
	}
	else
	{
		sprintf(licbuf, "START_LICENSE\n%s\nEND_LICENSE",
			lic);
	}
	if (lic) free(lic);
	lc_free_job(lm_job);
/*
 *	Use that license
 */
	v7_new_job(&v,&lm_job);

	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_USER_CRYPT_FILTER, 
					(LM_A_VAL_TYPE)user_crypt_filter);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)licbuf);
	if (lc_checkout(lm_job, "testfilter", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error user_crypt_filter/checkout line %d %s\n", 
			__LINE__,
			lc_errstring(lm_job));
	lc_free_job(lm_job);
/*
 *	Make sure a job with this new filter won't checkout from a 
 *	non-filtered feature
 */
/*
 *	Generate license without filter
 */
	ts_lm_init("demo", &code, &lm_job, __LINE__); 

	memcpy((char *)&v, (char *)&code, sizeof(v));
#if 0
	v.data[0] ^= VENDOR_KEY5;
	v.data[1] ^= VENDOR_KEY5;
	memcpy(&v.pubkeysize, &lm_prisize, sizeof(lm_prisize));
	memcpy(&v.pubkey, &lm_prikey, sizeof(lm_prikey));
        v.pubkey_fptr = l_prikey_sign;

#endif
	LM_CODE_GEN_INIT(&v)
	if (lc_cryptstr(lm_job, 
		"FEATURE testfilter demo 1.0 permanent uncounted 0 HOSTID=demo",
		&lic, &v, LM_CRYPT_FORCE, "test", &err))
	{
		fprintf(ofp, "cryptstr failed line %d: %s", __LINE__, err?err:"");
		if (err) free(err);
		*licbuf = 0;
	}
	else
	{
		sprintf(licbuf, "START_LICENSE\n%s\nEND_LICENSE",
			lic);
	}
	if (lic) free(lic);
	lc_free_job(lm_job);
	v7_new_job(&v,&lm_job);
	lc_set_attr(lm_job, LM_A_USER_CRYPT_FILTER, 
					(LM_A_VAL_TYPE)user_crypt_filter);
	lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)licbuf);
	if (!lc_checkout(lm_job, "testfilter", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error user_crypt_filter line %d shd have failed\n", 
			__LINE__);
	lc_free_job(lm_job);
#endif
}
test47()
{
 VENDORCODE v;
 LP_HANDLE *lp_handle;
 CONFIG *conf;
 char path[1000];

	serv_log( "47 -- v7 attributes \n");

/******************************************************************
 *	package optimize problems:
 *		First:  feature both in FEATURE and in SUITE
 *
 *	1. feature standalone and in suite:
 *		SUITE .. components="opt_c1 opt_c2"
 *		FEATURE opt_c1
 *		user1 checks out opt_c1 
 *		user2 checks out opt_c2 and fails
 *
 */
	v7_new_job(&v, &job[0]);
	if (lc_checkout(job[0], "opt_c1", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[0]));
	v7_new_job(&v, &job[1]);
	if (lc_checkout(job[1], "opt_c2", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[1]));
	lc_free_job(job[0]);
	lc_free_job(job[1]);
/*
 *	2. feature standalone and in suite: reverse order
 *		FEATURE opt_c1_1 
 *		SUITE opt_s1_1 .. components="opt_c1_1 opt_c1_2"
 *		FEATURE opt_s1_1 ... DUP_GROUP=U
 *		user1 checks out opt_c1_1 and opt_c1_2
 *		user2 checks out opt_c1_c1 fails
 */
	v7_new_job(&v, &job[0]);
	v7_new_job(&v, &job[1]);
	lc_set_attr(job[0], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"u1");
	lc_set_attr(job[1], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"u2");
	if (lc_checkout(job[0], "opt_c1_1", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (lc_checkout(job[0], "opt_c1_2", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[0]));
	/*fprintf(ofp, "Expect error line %d (not fixed yet)\n", __LINE__ + 2);*/
	if (lc_checkout(job[1], "opt_c1_1", "1.0", 1, 0, &v, LM_DUP_NONE))
		/*fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[1]))*/
			;
	lc_free_job(job[0]);
	lc_free_job(job[1]);

/*
 *		Second:  feature in 2 different SUITEs
 *	3. 2 suites, shared feature:
 *		PACKAGE opt_s2 components="opt_c2_1 opt_c2_2" OPTIONS=SUITE
 *		PACKAGE opt_s3 components="opt_c2_1 opt_c2_3" OPTIONS=SUITE
 *		INCREMENT opt_s2 ... 2 ... DUP_GROUP=U
 *		INCREMENT opt_s3 ... 2 ... DUP_GROUP=U
 *	3 users:
 *	user1 checkouts 1 and 3 (consumes s2 and s3 tokens)
 *	user2 checkouts 1 and 2 (consumes last s2 token)
 *	user3 checkouts 1 and 2 (consumes last s3 token, and tries 
 *				 to consumeopt_s2, but fails)
 *	If the first checkout (user1, 1) were moved to package opt_s3, 
 *	all 3 users would get their requests.
 */

	v7_new_job(&v, &job[0]);
	v7_new_job(&v, &job[1]);
	v7_new_job(&v, &job[2]);
	lc_set_attr(job[0], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"u1");
	lc_set_attr(job[1], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"u2");
	lc_set_attr(job[2], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"u3");
	if (lc_checkout(job[0], "opt_c2_1", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (lc_checkout(job[0], "opt_c2_3", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (lc_checkout(job[1], "opt_c2_1", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[1]));
	if (lc_checkout(job[1], "opt_c2_2", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[1]));
	if (lc_checkout(job[2], "opt_c2_1", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[2]));
	/*fprintf(ofp, "Expect error line %d (not fixed yet)\n", __LINE__ + 2);*/
	if (lc_checkout(job[2], "opt_c2_2", "1.0", 1, 0, &v, LM_DUP_NONE))
		/*fprintf(ofp, "Error checkout component %d %s\n", __LINE__,
			lc_errstring(job[2]))*/;
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
/******************************************************************
 *	Licenses safe for emailers that ignore newlines
 */
 	/* first test pre-v7 behavior */
	if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "nl_test", "1.0", 1, 
	"START_LICENSE\n\
	\nFEATURE nl_test demo 1.0 permanent uncounted DF542762620D HOSTID=DEMO\
	\nEND_LICENSE", 
		&lp_handle))
		fprintf(ofp, "Error %s %s:%d\n", lp_errstring(lp_handle),
			__FILE__, __LINE__);
	else lp_checkin(lp_handle);

	/* sanity check, this one should fail: */
	if (!lp_checkout(LPCODE, LM_RESTRICTIVE, "nl_test", "1.0", 1, 
	"START_LICENSE\n\
	\nFEATURE nl_test demo 1.0 permanent uncounted DF542762620D HOSTID=1234\
	\nEND_LICENSE", &lp_handle))
	{
		fprintf(ofp, "Error %s:%d\n", __FILE__, __LINE__);
		lp_checkin(lp_handle);
	}
	else
		lp_checkin(lp_handle);


	if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "nl_test", "1.0", 1, 
	"START_LICENSE\n\
	\nFEATURE nl_test demo 1.0 permanent uncounted DF542762620D HOSTID=DEMO\
	\n#	comment here\
	\nEND_LICENSE", &lp_handle))
		fprintf(ofp, "Error %s %s:%d\n", lp_errstring(lp_handle),
			__FILE__, __LINE__);
	else lp_checkin(lp_handle);

	if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "nl_test", "1.0", 1, 
	"START_LICENSE\n\
	\nFEATURE nl_test demo 1.0 permanent uncounted DF542762620D \
	HOSTID=DEMO\
	\n#	comment here\
	\nEND_LICENSE", &lp_handle))
		fprintf(ofp, "Error %s %s:%d\n", lp_errstring(lp_handle),
			__FILE__, __LINE__);
	else lp_checkin(lp_handle);

	if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "nl_test", "1.0", 1, 
	"START_LICENSE\n\
	\nFEATURE nl_test demo 1.0 \
	permanent uncounted DF542762620D \
	HOSTID=DEMO\
	\n#	comment here\
	\nEND_LICENSE", &lp_handle))
		fprintf(ofp, "Error %s %s:%d\n", lp_errstring(lp_handle),
			__FILE__, __LINE__);
	else lp_checkin(lp_handle);
					
	/* TEST V7 NEWLINES */ 
	if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "nl_test", "1.0", 1, 
	"START_LICENSE\n\
	\nFEATURE nl_test demo 1.0 permanent uncounted DF542762620D \n\
	HOSTID=DEMO\n\
	\n#	comment here\n\
	\nEND_LICENSE", &lp_handle))
		fprintf(ofp, "Error %s %s:%d\n", lp_errstring(lp_handle),
			__FILE__, __LINE__);
	else lp_checkin(lp_handle);

	if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "nl_test", "1.0", 1, 
					"START_LICENSE\n\
	\nFEATURE \
	\nnl_test \
	\ndemo \
	\n1.0 \
	\npermanent \
	\nuncounted \
	\nDF542762620D \
	\nHOSTID=DEMO\
	\n#	comment here\n\
	\nEND_LICENSE", &lp_handle))
		fprintf(ofp, "Error %s %s:%d\n", lp_errstring(lp_handle),
			__FILE__, __LINE__);
	else lp_checkin(lp_handle);

	if (lp_checkout(LPCODE, LM_RESTRICTIVE|LM_USE_LICENSE_KEY, "nl_test", "1.0", 1, 
					"START_LICENSE\n\
	\nFEATURE \
	\nnl_test \
	\ndemo \
	\n1.0 \
	\npermanent \
	\nuncounted \
	\nDF542762620D \
	\nHOSTID=DEMO\
	\ncomment here\
	\nEND_LICENSE", &lp_handle))
		fprintf(ofp, "Error %s %s:%d\n", lp_errstring(lp_handle),
			__FILE__, __LINE__);
	else lp_checkin(lp_handle);

/******************************************************************
 *	Sort tests
 */
/*
 * 	1. node-locked uncounted before counted for the same feature
 */
	v7_new_job(&v, &job[0]);
	if (lc_checkout(job[0], "sort1", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout sort1 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	
	if (!(conf = lc_auth_data(job[0], "sort1")))
		fprintf(ofp, "Error checkout sort1 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (conf && !conf->idptr)
		fprintf(ofp, "checked out wrong feat %s/%d\n", 
			__FILE__, __LINE__);
	lc_free_job(job[0]);
/*
 * 	1. node-locked uncounted before counted for the same feature
 */
	v7_new_job(&v, &job[0]);
	if (lc_checkout(job[0], "sort2", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout sort2 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (!(conf = lc_auth_data(job[0], "sort2")) || !conf->lc_issued)
		fprintf(ofp, "Error checkout sort2 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (conf && conf->lc_issued && strcmp(conf->lc_issued, "1-jan-1998"))
		fprintf(ofp, "checked out wrong feat exp %s, got %s %s/%d\n", 
			"1-jan-1998", conf->lc_issued, __FILE__, __LINE__);
	lc_free_job(job[0]);
/*
 * 	In the client, do not break up license -- keep $LM_LICENSE_FILE
 *                 list order 
 */
 	
	v7_new_job(&v, &job[0]);
	st_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0, __LINE__);
	sprintf(path, "%s%c%s", 
	"START_LICENSE\n\
	FEATURE sort3 demo 1.0 permanent uncounted 640DCA1F6CED \
	HOSTID=DEMO ISSUED=1-jan-1997\n\
	END_LICENSE", 
	PATHSEPARATOR,
	"FEATURE sort3 demo 1.0 permanent uncounted 630ECB206CED \
	HOSTID=DEMO ISSUED=1-jan-1998\n\
	END_LICENSE");
	st_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)path, __LINE__);
	if (lc_checkout(job[0], "sort3", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout sort3 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (!(conf = lc_auth_data(job[0], "sort3")) || !conf->lc_issued)
		fprintf(ofp, "Error checkout sort3 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (conf && conf->lc_issued && strcmp(conf->lc_issued, "1-jan-1997"))
		fprintf(ofp, "checked out wrong feat exp %s, got %s %s/%d\n", 
			"1-jan-1997", conf->lc_issued, __FILE__, __LINE__);
	lc_free_job(job[0]);
/*
 * 	In the client, do not break up license -- keep $LM_LICENSE_FILE
 */


/*
 * 	3. sort=
 */
	v7_new_job(&v, &job[0]);
	if (lc_checkout(job[0], "sort4", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout sort4 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	
	if (!(conf = lc_auth_data(job[0], "sort4")))
		fprintf(ofp, "Error checkout sort4 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (conf && conf->idptr)
		fprintf(ofp, "checked out wrong feat %s/%d\n", 
			__FILE__, __LINE__);
	lc_free_job(job[0]);

	v7_new_job(&v, &job[0]);
	if (lc_checkout(job[0], "sort5", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout sort5 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	
	if (!(conf = lc_auth_data(job[0], "sort5")))
		fprintf(ofp, "Error checkout sort5 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (conf && conf->lc_issued && strcmp(conf->lc_issued, "1-jan-1998"))
		fprintf(ofp, "checked out wrong feat exp %s, got %s %s/%d\n", 
			"1-jan-1998", conf->lc_issued, __FILE__, __LINE__);
	lc_free_job(job[0]);


	v7_new_job(&v, &job[0]);
	st_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0, __LINE__);
	st_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)
		"START_LICENSE\n\
		\nFEATURE sort6 demo 1.0 permanent uncounted 776F3C60189F \
			HOSTID=DEMO NOTICE=first\
		\nFEATURE sort6 demo 1.0 permanent uncounted FC67F7B5D5CC \
			HOSTID=DEMO sort=first NOTICE=last\
		\nEND_LICENSE"
		, __LINE__);
	if (lc_checkout(job[0], "sort6", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout sort6 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (!(conf = lc_auth_data(job[0], "sort6")) || !conf->lc_notice)
		fprintf(ofp, "Error checkout sort6 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (conf && conf->lc_notice && strcmp(conf->lc_notice, "last"))
		fprintf(ofp, "checked out wrong feat exp %s, got %s %s/%d\n", 
			"last", conf->lc_notice, __FILE__, __LINE__);
	lc_free_job(job[0]);

	v7_new_job(&v, &job[0]);
	st_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0, __LINE__);
	st_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)
		"START_LICENSE\n\
		\nFEATURE sort7 demo 1.0 permanent uncounted 776F3C60189F \
			HOSTID=DEMO NOTICE=first sort=last\
		\nFEATURE sort6 demo 1.0 permanent uncounted FC67F7B5D5CC \
			HOSTID=DEMO NOTICE=last\
		\nEND_LICENSE"
		, __LINE__);
	if (lc_checkout(job[0], "sort6", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout sort6 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (!(conf = lc_auth_data(job[0], "sort6")) || !conf->lc_notice)
		fprintf(ofp, "Error checkout sort6 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (conf && conf->lc_notice && strcmp(conf->lc_notice, "last"))
		fprintf(ofp, "checked out wrong feat exp %s, got %s %s/%d\n", 
			"last", conf->lc_notice, __FILE__, __LINE__);
	lc_free_job(job[0]);

	v7_new_job(&v, &job[0]);
	st_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0, __LINE__);
	st_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)
		"START_LICENSE\n\
		\nFEATURE sort7 demo 1.0 permanent uncounted 776F3C60189F \
			HOSTID=DEMO NOTICE=first sort=50\
		\nFEATURE sort6 demo 1.0 permanent uncounted FC67F7B5D5CC \
			HOSTID=DEMO NOTICE=last sort=40\
		\nEND_LICENSE"
		, __LINE__);
	if (lc_checkout(job[0], "sort6", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout sort6 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (!(conf = lc_auth_data(job[0], "sort6")) || !conf->lc_notice)
		fprintf(ofp, "Error checkout sort6 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (conf && conf->lc_notice && strcmp(conf->lc_notice, "last"))
		fprintf(ofp, "checked out wrong feat exp %s, got %s %s/%d\n", 
			"last", conf->lc_notice, __FILE__, __LINE__);
	lc_free_job(job[0]);

	v7_new_job(&v, &job[0]);
	st_set_attr(job[0], LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0, __LINE__);
	st_set_attr(job[0], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1, __LINE__);
	st_set_attr(job[0], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)
		"START_LICENSE\n\
		\nFEATURE sort6 demo 1.0 permanent uncounted FC67F7B5D5CC \
			HOSTID=DEMO NOTICE=last sort=40\
		\nFEATURE sort7 demo 1.0 permanent uncounted 776F3C60189F \
			HOSTID=DEMO NOTICE=first sort=50\
		\nEND_LICENSE"
		, __LINE__);
	if (lc_checkout(job[0], "sort6", "1.0", 1, 0, &v, LM_DUP_NONE))
		fprintf(ofp, "Error checkout sort6 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (!(conf = lc_auth_data(job[0], "sort6")) || !conf->lc_notice)
		fprintf(ofp, "Error checkout sort6 %d %s\n", __LINE__,
			lc_errstring(job[0]));
	if (conf && conf->lc_notice && strcmp(conf->lc_notice, "last"))
		fprintf(ofp, "checked out wrong feat exp %s, got %s %s/%d\n", 
			"last", conf->lc_notice, __FILE__, __LINE__);
	lc_free_job(job[0]);

#if 0
/*
 *	Attempt Test parts of borrow that all plats support
 */
	v7_new_job(&v, &job[0]);
 	if (!(conf = lc_get_config(job[0], "borrow1")))
		fprintf(ofp, "can't get borrow1 %s/%d", __FILE__, __LINE__);
 	if (conf->users != LM_COUNT_FROM_METER)
		fprintf(ofp, "borrow1 exp %d, got %d %s/%d\n", 
			LM_COUNT_FROM_METER, conf->users, __FILE__, __LINE__);
	lc_free_job(job[0]);
#endif

        date_test("y2k1");
        date_test("y2k2");
        date_test("y2k3");
        date_test("y2k4");
        date_test("y2k5");
}

static
void
date_test(feat)
char *feat;
{
#if 0
	/*
	 *	This test is no longer valid as of 9.0 as the expected return code of LM_LONGGONE
	 *	won't be returned anymore because UD is now enabled.  Because UD is enabled, we keep
	 *	track of which server had the best chance to service the request and at the end of processing
	 *	try to checkout AGAIN from that server (we also specify in the checkout to the server that if
	 *	this checkout fails that it should log a UD).  The checkout from that server more could result
	 *	in a different error than LM_LONGGONE, depending on the how that error is prioritized
	 *	when calculating the server to log the UD on.
	 */
 int expired = 0;
 int rc;
  VENDORCODE v;
        v7_new_job(&v, &job[0]);
        if (lc_expire_days(job[0], lc_get_config(job[0], "y2k1")) <  0)
                expired = 1;
        if (rc = lc_checkout(job[0], "y2k1", "1.0", 1, 0, &v,LM_DUP_NONE))
        {
                if (rc != LM_LONGGONE)
                        fprintf(ofp, "Error, line %d exp errno %d, got %d\n",
                                __LINE__, LM_LONGGONE, rc);

        }
        else if (expired)
                fprintf(ofp, "Error, line %d exp errno 0, got %d\n",
                                __LINE__, rc);
        lc_free_job(job[0]);                    
#endif 
}                                    

test49()
{

  VENDORCODE v;
  int rc, i;
  char buf[1000];
  extern phase2_app();
  extern phase2_gen();
  char *key2str, *keynewstr, *keyoldstr, *sign2;
  char *keystr; 
  char savkeystr[500];
  char *err;

	serv_log( "49 -- v7.1 attributes \n");

	fprintf(ofp, "\t--SIGN:\n");
	st_cryptstr("FEATURE lic_string demo 1.0 permanent \
			uncounted 0 HOSTID=ANY SIGN=0", &key2str, 0, __LINE__);
	st_cryptstr("FEATURE lic_string demo 1.0 permanent \
				uncounted 0 HOSTID=ANY", &keyoldstr, 0, __LINE__);
	if (key2str) fprintf(ofp, "\t%s", key2str);
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", key2str ? key2str : "");
	ts_lc_new_job(&v, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "key2 error line %d %s: %s\n", __LINE__, 
			lc_errstring(lm_job), buf);
	}
	lc_free_job(lm_job);
/*
 *	Make sure SIGN is required 
 */
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", keyoldstr ? keyoldstr : "");
	ts_lc_new_job(&v, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	if (!(rc = lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &v, 
		LM_DUP_NONE)) || (rc != LM_SIGN_REQ))
	{
		fprintf(ofp, "key2 error line %d got %d/%d exp %d: %s\n", 
			__LINE__, rc, lm_job->err_info.min_errno, LM_SIGN_REQ, buf);
	}
	lc_free_mem(lm_job, keyoldstr);
	lc_free_job(lm_job);
	ts_lc_new_job(&v, &lm_job, __LINE__); 
/*
 *	Make sure SIGN= is required with server
 */
	if (!(rc = lc_checkout(lm_job, "oldkey", "1.0", 1, 0, &v, 
		LM_DUP_NONE)) || (rc != LM_SIGN_REQ))
	{
		fprintf(ofp, "key2 error line %d %s\n", __LINE__, 
			lc_errstring(lm_job));
	}
/*
 *	Make sure SIGN= works with server
 */
	lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_STRENGTH_OVERRIDE, (LM_A_VAL_TYPE)LM_STRENGTH_163BIT);
	if (lc_checkout(lm_job, "key2", "1.0", 1, 0, &v, LM_DUP_NONE)) 
	{
		fprintf(ofp, "key2 error line %d %s\n", __LINE__, 
			lc_errstring(lm_job));
	}
	lc_free_job(lm_job);
/*
 *	Make sure license-key is not required
 */
	/* P5203 */
	ts_lc_new_job(&v, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_STRENGTH_OVERRIDE, (LM_A_VAL_TYPE)LM_STRENGTH_113BIT);
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)"@localhost");
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "sign", "1.0", 1, 0, &v, LM_DUP_NONE)) 
	{
		fprintf(ofp, "sign error line %d %s\n", __LINE__, 
			lc_errstring(lm_job));
	}
/*
 *	Make sure old clients can work with license-key works with server
 */
	lc_free_job(lm_job);
	ts_lc_new_job(&v, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	if (lc_checkout(lm_job, "oldkey", "1.0", 1, 0, &v, 
		LM_DUP_NONE))
	{
		fprintf(ofp, "oldkey error line %d %s\n", __LINE__, 
			lc_errstring(lm_job));
	}


	lc_free_job(lm_job);
	fprintf(ofp, "\t--KEY_OLD:\n");

	st_cryptstr("FEATURE lic_string demo 1.0 permanent \
				uncounted 0 HOSTID=ANY", &keyoldstr, 0, __LINE__);
	if (keyoldstr) fprintf(ofp, "\t%s", keyoldstr);
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", keyoldstr ? keyoldstr : "");
	memset(&v, 0, sizeof(v));
	ts_lc_new_job(&v, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "keyold error line %d %s: %s\n", __LINE__, 
			lc_errstring(lm_job), buf);
	}
	lc_free_job(lm_job);
/*
 *	Make sure SIGN doesn't affect checkout for old client 
 */
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", key2str ? key2str : "");
	ts_lc_new_job(&v, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	if (lc_checkout(lm_job, "lic_string", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "keyold error line %d %s: %s\n", __LINE__, 
			lc_errstring(lm_job), buf);
	}
	lc_free_mem(lm_job, keyoldstr);
	lc_free_mem(lm_job, key2str);
	lc_free_job(lm_job);
#if 0
/*
 *	LM_A_PHASE2...
 */
	fprintf(ofp, "\t--PHASE2:\n");
	ts_lm_init("demo", &code, &lm_job, __LINE__); 
	/*lc_set_attr(lm_job, LM_A_STRENGTH_OVERRIDE, (LM_A_VAL_TYPE)LM_STRENGTH_DEFAULT);*/
	memcpy((char *)&v, (char *)&lic_vc, sizeof(v));
	v.data[1] ^= VENDOR_KEY5;
	v.data[0] ^= VENDOR_KEY5;
	v.strength = LM_STRENGTH_DEFAULT;
	lc_set_attr(lm_job, LM_A_PHASE2_GEN, (LM_A_VAL_TYPE)phase2_gen);
	if (lc_cryptstr(lm_job, 
	"FEATURE phase2 demo 1.0 permanent uncounted 0 HOSTID=ANY SIGN=0",
		&keystr, &v,  LM_CRYPT_FORCE, "string", &err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s \n", __LINE__, 
			err ? err : "", lc_errstring(lm_job));
		if (err) free(err);
	}
	fprintf(ofp, "\t%s", keystr);
	lc_free_job(lm_job);
	ts_lc_new_job(&v, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_PHASE2_APP, (LM_A_VAL_TYPE)phase2_app);
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", keystr ? keystr : "");
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	if (lc_checkout(lm_job, "phase2", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "phase2 error line %d %s: %s\n", __LINE__, 
			lc_errstring(lm_job), buf);
	}
	lc_free_job(lm_job);
/*
 *	test same license with SIGN_LEVEL = 0
 */
	ts_lc_new_job(&v, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", keystr ? keystr : "");
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	if (lc_checkout(lm_job, "phase2", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "phase2 error line %d %s: %s\n", __LINE__, 
			lc_errstring(lm_job), buf);
	}
	lc_free_mem(lm_job, keystr);
	lc_free_job(lm_job);
#endif

/*
 *	USE PHASE2 & pubkey 
 */
	fprintf(ofp, "\t--PHASE2 & PUBKEY with SIGN2 =:\n");
	ts_lm_init( "demo", &v, &lm_job, __LINE__); 
	lc_set_attr(lm_job, LM_A_PHASE2_GEN, (LM_A_VAL_TYPE)phase2_gen);
	memcpy(&v, &lic_vc, sizeof(v));
#if 0
	v.data[0] ^= vc.data[0]^VENDOR_KEY5;
	v.data[1] ^= vc.data[1]^VENDOR_KEY5;

	memcpy(&v.pubkeysize, &lm_prisize, sizeof(lm_prisize));
	for (i = 0; i < LM_PUBKEYS; i++)
		memcpy(v.pubkey[i], lm_prikey[i], lm_prisize[i]);
        v.pubkey_fptr = l_prikey_sign;
#endif
	LM_CODE_GEN_INIT(&v)
	v.strength = LM_STRENGTH_113BIT ;
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)2);
	lc_set_attr(lm_job, LM_A_PUBKEY, (LM_A_VAL_TYPE)&v);
	if (lc_cryptstr(lm_job, 
	"FEATURE sign2 demo 1.0 permanent uncounted 0 HOSTID=ANY SIGN=0 SIGN2=0",
		&keystr, &v,  LM_CRYPT_FORCE, "string", &err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s \n", __LINE__, 
			err ? err : "", lc_errstring(lm_job));
		if (err) free(err);
	}
	fprintf(ofp, "\t%s", keystr);
	lc_free_job(lm_job);
	ts_lc_new_job(&v, &lm_job, __LINE__); 
	for (i = 0; i < l_pubseedcnt; i++)
	{
		int j;
		for (j =0; j < lm_pubsize[i][j]; j++)
		{
			memcpy(&v.pubkeyinfo[i].pubkey[j], &lm_pubkey[i][j], 
			sizeof(lm_pubkey[i][j]));
		}
		v.pubkeyinfo[i].pubkey_fptr = l_pubkey_verify;
		memcpy(&v.pubkeyinfo[i].pubkeysize, &lm_pubsize, 
			sizeof(lm_pubsize));
		v.strength = LM_STRENGTH;
	}
	lc_set_attr(lm_job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)2);
	lc_set_attr(lm_job, LM_A_PUBKEY, (LM_A_VAL_TYPE)&v);
	sprintf(buf, "START_LICENSE\n%sEND_LICENSE", keystr ? keystr : "");
	lc_free_mem(lm_job, keystr);
	lc_set_attr(lm_job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	lc_set_attr(lm_job, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)buf);
	if (lc_checkout(lm_job, "sign2", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "sign2 error line %d %s: %s\n", __LINE__, 
			lc_errstring(lm_job), buf);
	}
	lc_free_job(lm_job);
}
test52(int subtest)
{
  
  LM_USERS *u;
  VENDORCODE v;
  int cnt ;
  CONFIG *conf;
  char borrowbuf[MAX_CONFIG_LINE * 2];
  extern int l_reset_env;
#define	T_LM_A_BORROW_STRING 	1
#define T_LM_BORROW		2
#define T_TOOLONG		3
#define T_NOBORROWATTR		4
#define T_EXCLUDE		6
#define T_INCLUDE		7
#define T_DEFAULTREG		50
#define T_F3			51
#define T_EXCLUDE2		52
#define T_LOWWATER		53
#define T_PACKAGE		54
#define T_2FEAT			55
#define T_EXPIRE		56

	serv_log( "52 -- v8.0 attributes \n");
        unsetenv("DEMO_LICENSE_FILE");

	if (subtest == -1 || subtest == 'a')
	{
		fprintf(ofp, "\tmultiple dup-groups\n");
		dup_test();
	}

	if (subtest == -1 || subtest == 'b')
	{
		fprintf(ofp, "\tBORROW/DONGLE\n");
		st_dongle_borrow_test();
	}

	if (subtest == -1 || subtest == 'c')
		st_registry_tests();

/*
 *	lc_userlist fix
 */

	if (subtest == -1 || subtest == 'd')
	{
		fprintf(ofp, "\tlc_userlist fix\n");
		ts_lc_new_job(&v, &job[0], __LINE__); 
		ts_lc_new_job(&v, &job[1], __LINE__); 
		if (lc_checkout(job[0], "f3", "1.0", 1, 0, &v, LM_DUP_NONE))
		{
			fprintf(ofp, "f3 error line %d %s\n", __LINE__, 
				lc_errstring(job[0]));
		}
		lc_set_attr(job[1], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
		lc_set_attr(job[1], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)
			"servtest.lic" PATHSEPARATORSTR "servtest.lic" PATHSEPARATORSTR "servtest.lic");
		for (cnt = 0, u = lc_userlist(job[1], "f3"); u; u = u->next)
			if (u->name[0]) cnt++;
		if (cnt > 1) 
			fprintf(ofp, "userlist error line %d exp 1 got %d\n", __LINE__, 
				cnt);
		lc_free_job(job[0]);
		lc_free_job(job[1]);
	}

	if (subtest == -1 || subtest >= 'e') 
		fprintf(ofp, "\tBORROW/LINGER: ");


	if (subtest == -1 || subtest == 'e' || subtest == 'z') 
		st_borrow_test(T_LM_A_BORROW_STRING);
	if (subtest == -1 || subtest == 'f' || subtest == 'z') 
		st_borrow_test(T_LM_BORROW);
	if (subtest == -1 || subtest == 'g' || subtest == 'z') 
		st_borrow_test(T_TOOLONG);
	if (subtest == -1 || subtest == 'h' || subtest == 'z') 
		st_borrow_test(T_NOBORROWATTR);
	if (subtest == -1 || subtest == 'j' || subtest == 'z') 
		st_borrow_test(T_EXCLUDE);
	if (subtest == -1 || subtest == 'k' || subtest == 'z') 
		st_borrow_test(T_INCLUDE);
	if (subtest == -1 || subtest == 'l' || subtest == 'z') 
		st_borrow_test(T_F3);
	if (subtest == -1 || subtest == 'm' || subtest == 'z') 
		st_borrow_test(T_EXCLUDE2);
	if (subtest == -1 || subtest == 'n' || subtest == 'z') 
		st_borrow_test(T_LOWWATER);
	if (subtest == -1 || subtest == 'o' || subtest == 'z') 
		st_borrow_test(T_PACKAGE);
	if (subtest == -1 || subtest == 'p' || subtest == 'z') 
		st_borrow_test(T_2FEAT);
	if (subtest == -1 || subtest == 'q' || subtest == 'z') 
		st_borrow_test(T_EXPIRE);
	if (subtest == -1 || subtest == 'r' || subtest == 'z') 
		st_borrow_test(T_DEFAULTREG);

	fprintf(ofp, "\n");
	l_reset_env = 0;


}
test53()
{
#ifdef LM_UNIX_MT
	st_unixmt();
#endif /* LM_UNIX_MT */
}

dup_test()
{
  VENDORCODE v;
	ts_lc_new_job(&v, &job[0], __LINE__); 
	lc_set_attr(job[0], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"job0");
	ts_lc_new_job(&v, &job[1], __LINE__); 
	lc_set_attr(job[1], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"job1");
	ts_lc_new_job(&v, &job[2], __LINE__); 
	lc_set_attr(job[2], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"job2");
	ts_lc_new_job(&v, &job[3], __LINE__); 
	lc_set_attr(job[3], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"job3");
	ts_lc_new_job(&v, &job[4], __LINE__); 
	lc_set_attr(job[4], LM_A_USER_OVERRIDE, (LM_A_VAL_TYPE)"job4");
	if (lc_checkout(job[0], "f1", "1.0", 1, 0, &v, LM_DUP_USER))
	{
		fprintf(ofp, "f1 error line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	}
	if (lc_checkout(job[1], "f1", "1.0", 1, 0, &v, LM_DUP_HOST))
	{
		fprintf(ofp, "f1 error line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	}
	if (lc_checkout(job[2], "f1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "f1 error line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	}
	if (lc_checkout(job[3], "f1", "1.0", 1, 0, &v, LM_DUP_HOST))
	{
		fprintf(ofp, "f1 error line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	}
        if (get_feat_info("f1", __LINE__, &fi, job[4]))
        {
                if (fi.tot_lic_in_use != 3)
                        fprintf(ofp, "error exp 3 got %d line %d\n", 
                                fi.tot_lic_in_use, __LINE__);
        }
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
	lc_free_job(job[3]);
	lc_free_job(job[4]);
	ts_lc_new_job(&v, &job[0], __LINE__); 
	lc_set_attr(job[0], LM_A_HOST_OVERRIDE, (LM_A_VAL_TYPE)"host0");
	ts_lc_new_job(&v, &job[1], __LINE__); 
	lc_set_attr(job[1], LM_A_HOST_OVERRIDE, (LM_A_VAL_TYPE)"host1");
	ts_lc_new_job(&v, &job[2], __LINE__); 
	lc_set_attr(job[2], LM_A_HOST_OVERRIDE, (LM_A_VAL_TYPE)"host2");
	if (lc_checkout(job[0], "v8dup", "1.0", 1, 0, &v, LM_DUP_USER))
	{
		fprintf(ofp, "v8dup error line %d %s\n", __LINE__, 
			lc_errstring(job[0]));
	}
	if (lc_checkout(job[1], "v8dup", "1.0", 1, 0, &v, LM_DUP_USER))
	{
		fprintf(ofp, "v8dup error line %d %s\n", __LINE__, 
			lc_errstring(job[1]));
	}
	if (!lc_checkout(job[2], "v8dup", "1.0", 1, 0, &v, LM_DUP_USER))
	{
		fprintf(ofp, "checkout should not have succeeded line %d\n", 
			__LINE__);
	}
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	lc_free_job(job[2]);
	
}


int
st_cryptstr(char *str, char** result,  int decimal, int line)
{
  LM_HANDLE *lm_job;
  int ret;
  char *err;
LM_CODE(vc, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1, 
			VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
	vc.strength = LM_STRENGTH ;
#if 0
	memcpy(&vc.pubkeysize, &lm_prisize, sizeof(lm_prisize));
	memcpy(&vc.pubkey, &lm_prikey, sizeof(lm_prikey));
        vc.pubkey_fptr = l_prikey_sign;
	vc.data[0] = vc.data[0]^VENDOR_KEY5;
	vc.data[1] = vc.data[1]^VENDOR_KEY5;
#endif
	ts_lm_init( "demo", &vc, &lm_job, __LINE__); 
	LM_CODE_GEN_INIT(&vc)
	if (ret = lc_cryptstr(lm_job, str, result, &vc,  decimal ?
				(LM_CRYPT_FORCE| LM_CRYPT_DECIMAL_FMT) :
				LM_CRYPT_FORCE, "string", &err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s \n", line, 
			err ? err : "", lc_errstring(lm_job));
		if (err) free(err);
	}
	lc_free_job(lm_job);
	return ret;
}
void
d_lc_new_job(vendor_key, job_id, line)
VENDORCODE *vendor_key;		/* Vendor's encryption code */
LM_HANDLE **job_id;
int line;
{
  int rc;
  extern LM_HANDLE *main_job;

	
	if ((rc = lc_new_job(main_job, 0, vendor_key, job_id)) && 
						rc != LM_DEMOKIT) 	
	{
		fprintf(ofp, "error line %d: %s\n", line, lc_errstring(*job_id));
	}
	st_set_attr(*job_id, LM_A_LONG_ERRMSG, (LM_A_VAL_TYPE) 0, __LINE__);
	st_set_attr(*job_id, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE) 1, __LINE__);
	st_set_attr(*job_id, LM_A_PROMPT_FOR_FILE, (LM_A_VAL_TYPE) 0, __LINE__);
}

st_dongle_borrow_test()
{
  VENDORCODE v;
  char sav2[300];
  char *cp= 0;
  extern char *l_rcfilename;
  char *realrc = l_rcfilename;

	d_lc_new_job(&v, &job[0], __LINE__); 

	l_set_registry(job[0], "LM_BORROW", 0, 0, 0); /* don't borrow */
	lc_free_job(job[0]);
/*
 *	We pretend this is a client with a different registry
 *	This client doesn't have the dongle.
 */
	l_rcfilename = "flexlmrc";
	d_lc_new_job(&v, &job[0], __LINE__); 
	l_set_registry(job[0], "HOSTID", 0, 0, 0); /* no dongle */
	l_set_registry(job[0], "DEMO_LICENSE_FILE", "@localhost", 0, 0);
/*
 *	This should be a floating checkout
 */
	if (lc_checkout(job[0], "dongle1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "dongle1 error line %d %s\n",  __LINE__, 
				lc_errstring(job[0]));
	}
	d_lc_new_job(&v, &job[1], __LINE__); 
/*
 *	This should be a floating checkout that fails -- all used up.
 */
	if (!lc_checkout(job[1], "dongle1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "dongle1 error line %d shd'v failed\n",__LINE__);
	}
	l_set_registry(job[0], "DEMO_LICENSE_FILE", "servtest.lic", 0, 0);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	d_lc_new_job(&v, &job[0], __LINE__); 
/*
 *	This should be a node-locked checkout 
 */
	if (!lc_checkout(job[0], "dongle1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
	  CONFIG *c;
		fprintf(ofp, "%s dongle1 error line %d shd'v failed\n",l_real_getenv("DEMO_LICENSE_FILE"), __LINE__);
		c = lc_auth_data(job[0], "dongle1");
		fprintf(ofp, "using %s\n", c->code);
	}
	l_set_registry(job[0], "HOSTID", "testing123", 0, 0);
	l_set_registry(job[0], "DEMO_LICENSE_FILE", "servtest.lic", 0, 0);
	lc_free_job(job[0]);
	d_lc_new_job(&v, &job[0], __LINE__); 
/*
 *	This one should fail because we're on the FLOAT_OK node
 */
	if (!lc_checkout(job[0], "dongle1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "dongle1 error line %d %s\n",  __LINE__, 
				lc_errstring(job[0]));
	}
	l_set_registry(job[0], "HOSTID", "testing456", 0, 0);
	d_lc_new_job(&v, &job[1], __LINE__); 

	if (lc_checkout(job[1], "dongle1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "dongle1 error line %d %s\n",  __LINE__, 
				lc_errstring(job[0]));
	}
	lc_free_job(job[0]);
	lc_free_job(job[1]);

	l_rcfilename = realrc;
	d_lc_new_job(&v, &job[0], __LINE__); 
	l_set_registry(job[0], "HOSTID", 0, 0, 0);
	ts_flush_server(job[0], __LINE__);
	lc_free_job(job[0]);

	d_lc_new_job(&v, &job[0], __LINE__); 
	l_set_registry(job[0], "DEMO_LICENSE_FILE", "@localhost", 0, 0);
	if (!lc_checkout(job[0], "dongle1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "dongle1 error line %d shd'v failed\n",__LINE__);
	}
	l_set_registry(job[0], "HOSTID", "testing123", 0, 0);
	lc_free_job(job[0]);
	st_reread();
	l_rcfilename = "flexlmrc";
	d_lc_new_job(&v, &job[0], __LINE__); 
	l_set_registry(job[0], "HOSTID", 0, 0, 0);
	l_set_registry(job[0], "DEMO_LICENSE_FILE", "@localhost", 0, 0);
	if (lc_checkout(job[0], "dongle1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "dongle1 error line %d %s\n",  __LINE__, 
				lc_errstring(job[0]));
	}
	lc_free_job(job[0]);
/*
 *	Now test a list
 */
	d_lc_new_job(&v, &job[0], __LINE__); 
	l_rcfilename = realrc;
	l_set_registry(job[0], "HOSTID", "testing123 testing456", 0, 0);
	st_reread();
	lc_free_job(job[0]);
	l_rcfilename = "flexlmrc";
	d_lc_new_job(&v, &job[0], __LINE__); 
	d_lc_new_job(&v, &job[1], __LINE__); 
	l_set_registry(job[0], "HOSTID", 0, 0, 0);
	l_set_registry(job[0], "DEMO_LICENSE_FILE", "@localhost", 0, 0);
	if (lc_checkout(job[0], "dongle1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "dongle1 error line %d %s\n",  __LINE__, 
				lc_errstring(job[0]));
	}
	if (lc_checkout(job[1], "dongle1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "dongle1 error line %d %s\n",  __LINE__, 
				lc_errstring(job[1]));
	}
	l_rcfilename = realrc;
	l_set_registry(job[0], "HOSTID", "testing123", 0, 0);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
	st_reread();

/*
 *	All done
 */

}

static int reconn_done_i;

static
void
reconn_done()
{
	reconn_done_i = 1;
}
static
void
st_borrow_test(int flag)
{
#define BORROW_TIME 4 /* seconds */
  VENDORCODE v;
  int cnt ;
  CONFIG *conf;
  char borrowbuf[MAX_CONFIG_LINE * 2];
  extern int l_reset_env;
  int minutes = BORROW_TIME ;
  char *feature = "f1";
  typedef unsigned int (*_borrowid) lm_args((LM_VOID_PTR, LM_VOID_PTR, int , 
				unsigned char *, int, int *));
  _borrowid borrowid;
  unsigned int b_id;
  char b_id_buf[50];
  char b_id_buf2[50];
  struct tm *tm;
#ifdef THREAD_SAFE_TIME
  struct tm tst;
#endif
  char today[20];
  int d, m, y;
  long x;
  time_t t;
  time_t exptime, real_exptime;
  time_t starttime;
  LM_BORROW_STAT *borstat;
  extern int this_mach_speed;


	minutes *= this_mach_speed;
	minutes /= 100;

	l_reset_env = 1;
	job[0] = 0;
	job[1] = 0;
	switch(flag)
	{
	case T_LM_A_BORROW_STRING:  fprintf(ofp, " LM_A_BORROW_STRING"); break;
	case T_LM_BORROW:  fprintf(ofp, ", $LM_BORROW"); break;
	case T_TOOLONG:  fprintf(ofp, ", too long");  
		minutes += 2;
		break;
	case T_NOBORROWATTR:  fprintf(ofp, ",\n\t\tno BORROW attr"); 
		feature = "f2"; break;
	case T_EXCLUDE:  fprintf(ofp, ",\n\t\tEXCLUDE_BORROW"); 
		feature = "f4";
		break;
	case T_INCLUDE:  fprintf(ofp, ", INCLUDE_BORROW"); 
		feature = "borrowfail";
		break;
	case T_DEFAULTREG:  fprintf(ofp, ",\n\t\tdefault registry"); 
			break;
	case T_PACKAGE:  fprintf(ofp, ",\n\t\tpackage"); 
		feature = "comp1";
		break;
	case T_F3:  fprintf(ofp, ",\n\t\tanother feature"); 
		feature = "f3";
		break;
	case T_EXCLUDE2:  fprintf(ofp, ", EXCLUDE2"); 
		feature = "borrowok";
		break;
	case T_LOWWATER:  fprintf(ofp, ", BORROW_LOWWATER"); 
		feature = "lowwater";
		break;
	case T_EXPIRE:  fprintf(ofp, ", Expired LM_BORROW"); break;
	case T_2FEAT:  fprintf(ofp, ", 2 pools: one borrowable"); 
		feature = "borrow2";
		break;
		
	}
	fflush(ofp);

	ts_lc_new_job(&v, &job[0], __LINE__); 
	lc_set_attr(job[0], LM_A_CHECK_BADDATE, (LM_A_VAL_TYPE)1);
/*
 *	SEtup job 10 for flushes
 */
	ts_lc_new_job(&v, &job[10], __LINE__);  /* use this job for flushes */
	if (!get_feat_info_err(feature, __LINE__, &fi, job[10], 1, 0))
	{
		fprintf(ofp, "Error connecting for flush job line %d: %s\n",
			__LINE__, lc_errstring(job[10]));
		return; /* outahere -- it will fail anyway */
	}
/*
 *	clear out registry setting
 *	We need the "borrowid" for this binary in order to do this
 */
	
	borrowid = (_borrowid)job[0]->l_new_job;
	b_id = borrowid(0, 0, 4, 0, 0, 0); /* the 4 indicates get borrowid */
	*b_id_buf2 = 0;
	if (b_id) 
	{
		b_id ^= L_BORROW_MAGIC;
		sprintf(b_id_buf, "borrow%xf1", b_id);
		if (strcmp(feature, "f1"))
			sprintf(b_id_buf2, "borrow%x%s", b_id, feature);
	}
	else
	{
		sprintf(b_id_buf, "borrowf1", b_id);
		if (strcmp(feature, "f1"))
			sprintf(b_id_buf2, "borrow%s", feature);
	}
	ts_borrow_minutes_is_seconds(job[0]);
#ifdef THREAD_SAFE_TIME
	l_get_date(&d, &m, &y, &x, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&d, &m, &y, &x);
#endif
	t = (time_t) x;
#ifdef THREAD_SAFE_TIME
	localtime_r(&t, &tst);
	tm = &tst;
#else /* !THREAD_SAFE_TIME */
	tm = localtime(&t);
#endif
	if (flag == T_EXPIRE) d -= 2;
	sprintf(today, "%d-%s-%d", d, l_month_int(m), 1900+y);
	exptime = t + (minutes * 60);
	real_exptime = t + minutes ;
	starttime = t;
/*
 *	clear out registry setting
 */
	l_set_registry(job[0], b_id_buf, 0, 0, 0);
	if (*b_id_buf2) l_set_registry(job[0], b_id_buf2, 0, 0, 0);
	l_set_registry(job[0], "LM_BORROW", 0, 0, 0);

	if (flag == T_2FEAT)
	{
	  CONFIG *pos = 0;

		ts_lc_new_job(&v, &job[2], __LINE__); 
/*
 *		Sanity test -- make sure initial levels are correct.
 */
		while (conf = lc_next_conf(job[2], feature, &pos))
		{
			if (get_feat_info_err(feature, __LINE__, &fi, job[10], 1,
					conf))
			{
				if (fi.tot_lic_in_use  || fi.borrowed)
				fprintf(ofp, "error exp %d/%d got %d/%d line %d\n", 
				0,0, fi.tot_lic_in_use, fi.borrowed, __LINE__);
			}
		}
/*
 *		Should consume non-borrow feature, and don't borrow
 */
		if (lc_checkout(job[2], feature, "1.0", 1, 0, &v , LM_DUP_NONE))
		{
			fprintf(ofp, "%s error line %d %s\n", 
				feature, __LINE__, 
				lc_errstring(job[2]));
			goto exit_borrow_test;
		}
		if (!(conf = lc_auth_data(job[2], feature)))
		{
			fprintf(ofp, "%s error line %d %s\n", 
				feature, __LINE__, 
				lc_errstring(job[2]));
			goto exit_borrow_test;
		}
		if (conf->lc_options_mask & LM_TYPE_BORROW)
		{
			fprintf(ofp, "%s error line %d -- wrong INCR line\n", 
				feature, __LINE__ );
			goto exit_borrow_test;
		}
		if (get_feat_info_err(feature, __LINE__, &fi, job[10], 1, conf))
		{
			if ((fi.tot_lic_in_use != 1) || (fi.borrowed != 0))
				fprintf(ofp, 
				"error exp %d/%d got %d/%d line %d conf %s\n", 
				1,0, fi.tot_lic_in_use, fi.borrowed, __LINE__,
				conf->code);
		}
	}
	else if (flag == T_LM_BORROW || flag == T_EXPIRE)
	{
	  char buf[200];

#ifdef THREAD_SAFE_TIME
		localtime_r(&exptime, &tst);
		tm = &tst;
#else /* !THREAD_SAFE_TIME */
		tm = localtime(&exptime );
#endif
		sprintf(buf, "%s:all:%d-%s-%d:%02d:%02d", today,
			d, l_month_int(m), 1900+y, tm->tm_hour, tm->tm_min );
		l_set_registry(job[0], "LM_BORROW", buf, 0, 0);
		l_set_registry(job[0], "borrowf1", 0, 0, 0);
		lc_free_job(job[0]);
		ts_lc_new_job(&v, &job[0], __LINE__); 

	}
	else if (flag == T_DEFAULTREG)
	{
		lc_set_attr(job[0], LM_A_USER_RECONNECT_DONE, (LM_A_VAL_TYPE)reconn_done);
	}
	lc_disconn(job[0], 1);
/* 
 *	set linger period
 */ 
	if (flag != T_LM_BORROW && flag != T_LOWWATER && flag != T_EXPIRE)
		job[0]->borrow_linger_minutes = minutes;
	if (flag == T_LM_A_BORROW_STRING)
	{
		memset(borrowbuf, 0, sizeof(borrowbuf));
		st_set_attr(job[0], LM_A_BORROW_STRING, (LM_A_VAL_TYPE) 
						borrowbuf, __LINE__);
	}
/*
 *	This checkout should do the borrow to a local file, 
 *		(except when flag == T_LOWWATER)
 */
	if (lc_checkout(job[0], feature, "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		if (flag < T_TOOLONG || flag >= T_DEFAULTREG)
		{
			fprintf(ofp, "%s error line %d %s\n", 
				feature, __LINE__, 
				lc_errstring(job[0]));
		}
		goto exit_borrow_test;
	}
	else 
	{
		
		if (flag >= T_TOOLONG && flag < T_DEFAULTREG)
		{
			fprintf(ofp, "%s error ln %d checkout shd'v failed\n", 
						feature, __LINE__);
			goto exit_borrow_test;
		}
/* 
 *		checkout succeeded and was supposed to succeed. 
 */
		else if (flag != T_LOWWATER)
		{
			int foundit = 0;
			conf = lc_auth_data(job[0], feature);
			if (flag == T_DEFAULTREG)
			{
				if (lc_heartbeat(job[0], 0, 0)||reconn_done_i) 
					fprintf(ofp, 
				"Err: reconnect_done called line %d %s\n",
					__LINE__, lc_errstring(job[0]));
				reconn_done_i = 0;
			}
			if (get_feat_info_err(feature, __LINE__, &fi, job[10], 1, conf))
			{
  			  int expbor = 1, expcnt = 1;

				if (flag == T_EXPIRE) expbor = 0;

				if ((fi.tot_lic_in_use != expcnt) || 
					(fi.borrowed != expbor))
				fprintf(ofp, "error exp %d/%d got %d/%d line %d\n", 
				expcnt,1, fi.tot_lic_in_use, fi.borrowed, __LINE__);
			}
			lc_get_attr(job[0], LM_A_BORROW_STAT, (short *)&borstat);
			for (; borstat; borstat = borstat->next)
			{
				if (!strcmp(borstat->feature , feature))
					foundit = 1;
			}
			if (!foundit && (flag != T_EXPIRE)) fprintf(ofp, 
				"LM_A_BORROW_STAT error line %d\n", __LINE__);
			
		}
	}
	if (flag != T_LOWWATER)
	{
		lc_checkin(job[0], feature, 1);
		ts_flush_server(job[10], __LINE__);
		if ((flag != T_EXCLUDE2) && 
			get_feat_info_err(feature, __LINE__, &fi, job[10], 1, conf))
		{
  		  int expcnt = 1, expbor = 1;
	
			if (flag == T_EXPIRE) expcnt = expbor = 0;
			if ((fi.tot_lic_in_use != expcnt) || 
				(fi.borrowed != expbor))
				fprintf(ofp, "error exp %d/%d got %d/%d line %d\n", 
				expcnt,expbor, fi.tot_lic_in_use, fi.borrowed, __LINE__);
		}
	}
	ts_lc_new_job(&v, &job[1], __LINE__); 
	st_set_attr(job[1], LM_A_CHECK_BADDATE, (LM_A_VAL_TYPE) 1, __LINE__);
	if (flag == T_LM_A_BORROW_STRING)
		st_set_attr(job[1], LM_A_BORROW_STRING, (LM_A_VAL_TYPE) 
					borrowbuf, __LINE__);
	if (flag != T_LOWWATER)
	{
		st_set_attr(job[1], LM_A_DISABLE_ENV, (LM_A_VAL_TYPE) 1, __LINE__);
		lc_set_attr(job[1], LM_A_LICENSE_FILE, (LM_A_VAL_TYPE) "thisshouldfail");
	}
	else 	
		job[1]->borrow_linger_minutes = (BORROW_TIME +2 )/2;
/*
 *	This checkout should use the borrowed record and not the server
 */
	if (lc_checkout(job[1], feature, "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		if (flag != T_LOWWATER && flag != T_EXPIRE)
		{
			fprintf(ofp, "borrow failed, %s error line %d real_exptime %x %s\n", feature, 
					__LINE__, real_exptime, lc_errstring(job[1]));
		}
		goto exit_borrow_test;
	}
	else if (flag == T_LOWWATER)
		fprintf(ofp, "lowwater failed line %d\n", __LINE__);
	else if (flag == T_DEFAULTREG)
	{
		if (lc_heartbeat(job[0], 0, 0)||reconn_done_i) 
			fprintf(ofp, 
		"Err: reconnect_done called line %d %s\n",
			__LINE__, lc_errstring(job[0]));
	}
	if (flag != T_LOWWATER)
	{
		conf = lc_auth_data(job[1], feature);
		if (!conf || !conf->borrow_flags)
			fprintf(ofp, "borrow auth-data wrong line %d\n", __LINE__);

	}
	lc_checkin(job[1], feature, 0);

	l_select_one(0, -1, ((real_exptime - time(0)) + 3) * 1000); /* sleep until it expires */
	if (flag == T_2FEAT) lc_free_job(job[2]);
        ts_flush_server(job[10], __LINE__);
        if (get_feat_info(feature, __LINE__, &fi, job[10]))
        {
                if (fi.tot_lic_in_use != 0)
                        fprintf(ofp, "error exp %d got %d feature %s line %d\n", 0,
                                fi.tot_lic_in_use, feature, __LINE__);
        }
/*
 *	This checkout should use the borrowed record but fail, because
 *	it's timed out
 */
        if (!lc_checkout(job[1], feature, "1.0", 1, 0, &v, LM_DUP_NONE))
        {
                fprintf(ofp, "borrow failed, %s checked out %d %s\n", feature, 
						__LINE__,
                        lc_errstring(job[0]));
        }
        lc_checkin(job[1], feature, 0);
exit_borrow_test:
	if (flag == T_LM_BORROW) l_set_registry(job[1], "LM_BORROW", 0, 0, 0);
	if (flag == T_DEFAULTREG || flag == T_PACKAGE)  
	{
		if (real_exptime < time(0))
		{
			l_select_one(0, -1, ((real_exptime - time(0)) + 2) * 1000); /* sleep until it expires */
		}
		ts_flush_server(job[10], __LINE__); /* server regain license */
	}
	if (job[0]) 
	{
		lc_free_job(job[0]);
		job[0] = 0;
	}
	if (job[1]) 
	{
		lc_free_job(job[1]);
		job[1] = 0;
	}
	lc_free_job(job[10]);
}
st_registry_tests()
{
  char *cp;
  VENDORCODE v;
  int i;
  char binary[100];
/*
 *	Binary l_set_registry
 */
	fprintf(ofp, "\tbinary l_set_registry\n", 0);
	ts_lc_new_job(&v, &job[0], __LINE__); 
	ts_lc_new_job(&v, &job[1], __LINE__); 
	l_set_registry(job[0], "binary", 0, 0, 0);
	l_set_registry(job[0], "infoborrow", 0, 0, 0);
	l_set_registry(job[0], "a", 0, 0, 0);
	l_set_registry(job[0], "b", 0, 0, 0);
	l_set_registry(job[0], "c", 0, 0, 0);
	l_set_registry(job[0], "d", 0, 0, 0);
	l_set_registry(job[0], "e", 0, 0, 0);
	l_set_registry(job[1], "longkeynamethisisverylongkeyname", 
						0, 0, 0);
	test_registry("a", 0, 0, __LINE__);
	test_registry("b", 0, 0, __LINE__);
	test_registry("c", 0, 0, __LINE__);
	test_registry("d", 0, 0, __LINE__);
	test_registry("e", 0, 0, __LINE__);
	test_registry("binary", 0, 0, __LINE__);
	test_registry("longkeynamethisisverylongkeyname", 0, 0, __LINE__);
	l_set_registry(job[1], "c", "1002", 0, 0);
	l_set_registry(job[0], "a", "1", 0, 0);
	l_set_registry(job[0], "b", "101", 0, 0);
	test_registry("a", "1", 0, __LINE__);
	l_set_registry(job[0], "d", "dddddddddd", 0, 0);
	test_registry("a", "1", 0, __LINE__);
	l_set_registry(job[1], "e", "eeeee", 0, 0);
	test_registry("a", "1", 0, __LINE__);
	l_set_registry(job[0], "longkeynamethisisverylongkeyname", "1002", 0, 0);
	test_registry("e", "eeeee", 0, __LINE__);
	test_registry("c", "1002", 0, __LINE__);
	test_registry("a", "1", 0, __LINE__);
	test_registry("b", "101", 0, __LINE__);
	test_registry("d", "dddddddddd", 0, __LINE__);
	test_registry("e", "eeeee", 0, __LINE__);

	l_set_registry(job[0], "a", "1234567890", 0, 0);
	test_registry("c", "1002", 0, __LINE__);
	test_registry("a", "1234567890", 0, __LINE__);
	test_registry("b", "101", 0, __LINE__);
	test_registry("c", "1002", 0, __LINE__);
	l_set_registry(job[1], "longkeynamethisisverylongkeyname", 
						"this one has spaces", 0, 0);
	test_registry("longkeynamethisisverylongkeyname", 
		"this one has spaces", 0, __LINE__);
	l_set_registry(job[0], "a", "1234567890", 0, 0);
	test_registry("a", "1234567890", 0, __LINE__);
	test_registry("longkeynamethisisverylongkeyname", 
		"this one has spaces", 0, __LINE__);
	test_registry("b", "101", 0, __LINE__);
	test_registry("c", "1002", 0, __LINE__);
	test_registry("d", "dddddddddd", 0, __LINE__);
	test_registry("e", "eeeee", 0, __LINE__);
	for (i = 0; i < 20; i++)
		binary[i] = i;
	l_set_registry(job[0], "binary", binary, 20, 0);
	test_registry("binary", binary, 20, __LINE__);
	l_set_registry(job[0], "binary", "This\nis\na\ntest\n", 16, 0);
	test_registry("binary", "This\nis\na\ntest\n", 16, __LINE__);
	test_registry("b", "101", 0, __LINE__);
	test_registry("c", "1002", 0, __LINE__);
	test_registry("d", "dddddddddd", 0, __LINE__);
	test_registry("e", "eeeee", 0, __LINE__);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}
static
void
test_registry(char *key, char *exp, int len, int line)
{
  char *cp;
  VENDORCODE v;
  int rlen;
	ts_lc_new_job(&v, &job[5], line); 
	cp = 0; rlen = 0; l_get_registry(job[5], key, &cp, &rlen, 0);
	if ((!exp && cp) || 
		(len && (((len != rlen)) || memcmp(cp , exp, len))) ||
		(!len && exp && cp && strcmp(cp, exp)))
		fprintf(ofp, "l_set_registry failed lines %d-%d\n",
					line, __LINE__);
	cp = 0; rlen = 0; l_get_registry(job[0], key, &cp, &rlen, 0);
	if ((!exp && cp) || 
		(len && (((len != rlen)) || memcmp(cp , exp, len))) ||
		(!len && exp && cp && strcmp(cp, exp)))
		fprintf(ofp, "l_set_registry failed lines %d-%d\n",
					line, __LINE__);
	cp = 0; rlen = 0; l_get_registry(job[1], key, &cp, &rlen, 0);
	if ((!exp && cp) || 
		(len && (((len != rlen)) || memcmp(cp , exp, len))) ||
		(!len && exp && cp && strcmp(cp, exp)))
		fprintf(ofp, "l_set_registry failed lines %d-%d\n",
					line, __LINE__);
	lc_free_job(job[5]);
}

int recon;
int done;
mt_user_reconnect() { recon++; }
mt_user_reconnect_done() { done++; }
#ifdef LM_UNIX_MT 
static
void
st_unixmt(void)
{
  VENDORCODE v;
  time_t t = time(0);
  char *u, *h, *d;

	ts_lc_new_job(&v, &job[0], __LINE__); 
	ts_lc_new_job(&v, &job[1], __LINE__); 
	st_set_attr(job[0], LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE) 1, __LINE__);
	st_set_attr(job[0], LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE) 1, __LINE__);
	st_set_attr(job[0], LM_A_USER_RECONNECT, (LM_A_VAL_TYPE) 
					mt_user_reconnect, __LINE__);
	st_set_attr(job[0], LM_A_USER_RECONNECT_DONE, (LM_A_VAL_TYPE) 
				mt_user_reconnect_done, __LINE__);
	st_set_attr(job[0], LM_A_RETRY_COUNT, (LM_A_VAL_TYPE) -1, __LINE__);
	if (lc_checkout(job[0], "f1", "1.0", 1, 0, &v, LM_DUP_NONE))
	{
		fprintf(ofp, "ckout failed, %s error line %d %s\n", "f1", 
					__LINE__, lc_errstring(job[0]));
		return;
	}
        while ((time(0) - t)  < 16)  
        {
          int i = 1;
	  time_t t2;
		t2 = time(0);
		while ((time(0) - t2) < 2) free(malloc(i++ * 100));
                if (!lc_userlist(job[1], "f1"))
			fprintf(ofp, "error line %d: %s\n", __LINE__, 
							lc_errstring(job[1]));
		l_select_one(0, -1, 100);
        }
	u = lc_username(job[1], 1);
	h = lc_hostname(job[1], 1); 
	d = lc_display(job[1], 1);
	if (lc_remove(job[1], "f1", u, h, d))
		fprintf(ofp, "error line %d %s\n", __LINE__, lc_errstring(job[1]));
	t = time(0);
        while ((time(0) - t)  < 5)  /* should check it back out again */
        {
          int i = 1;
	  time_t t2;
		t2 = time(0);
		/* 5 seconds worth of free/malloc calls */
		while ((time(0) - t2) < 2) free(malloc(i++ * 100));
                if (!lc_userlist(job[1], "f1"))
			fprintf(ofp, "error line %d: %s\n", __LINE__, 
							lc_errstring(job[1]));
		l_select_one(0, -1, 100);
        }
	if (!done) fprintf(ofp, "error line %d\n", __LINE__);
	lc_free_job(job[0]);
	lc_free_job(job[1]);
}
#endif /* LM_UNIX_MT */
