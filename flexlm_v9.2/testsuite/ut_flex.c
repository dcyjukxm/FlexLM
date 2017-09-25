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
 *	Module: $Id: ut_flex.c,v 1.10.4.1 2003/07/02 23:50:41 sluu Exp $
 *
 *	Function: ut_unix.c -- utiltest for unix.
 *
 *	D. Birns
 *	3/23/95
 *
 *	Last changed:  7/31/97
 *
 */
#include <stdlib.h>
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "code.h"
#include "lm_attr.h"
LM_CODE(key, ENCRYPTION_SEED1, ENCRYPTION_SEED2,
	  VENDOR_KEY1, VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
LM_HANDLE *job;

void
exitcall()
{
	printf("\nERROR:\treread failed");
}
void
reconn_done(feat)
char *feat;
{
	printf("\t%s successfully reconnected\n", feat);
}

LM_HANDLE *thisjob;
void
ut_free_job(void)
{
	lc_free_job(thisjob);
}

void
ut_clear_borrow(void)
{
#ifdef PC
  HKEY happ;

        if (RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\FLEXlm License Manager", 0,KEY_WRITE,&happ) ==
						ERROR_SUCCESS)
	{
		if (RegDeleteKey(happ,
			   "Borrow") != ERROR_SUCCESS)
		{
			fprintf(stderr, "can't delete borrow registry\n");
		}
	}
	RegCloseKey(happ);
#else /* PC */
	unlink("test_flexlmrc");
	unlink("test_borrow");
	{
	  char *cp;
	  char buf[200];
		cp = getenv("HOME");
		sprintf(buf, "%s/.flexlmborrow", cp);
		unlink (buf);
	}
#endif /* UNIX */
	 l_set_registry(thisjob, "DEMO_LICENSE_FILE", 0, 0, 0);
	 l_set_registry(thisjob, "infoborrow", 0, 0, 0);
}

int
ut_checkout(feature, samejob)
char *feature;
int samejob;
{
 int ret;
 static VENDORCODE co_code;
 extern char *lm_borrow;
	lm_borrow = (char *)NULL;
	if (!samejob)
	{
		if (lc_new_job((LM_HANDLE *)0, 0, &co_code, &thisjob) &&
			lc_get_errno(thisjob) != LM_DEMOKIT)
				printf("can't lc_init: %s\n", lc_errstring(thisjob));
		job = thisjob;
		lc_set_attr(thisjob, LM_A_LICENSE_FILE, (LM_A_VAL_TYPE)".");
#if 0
		lc_set_attr(thisjob, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)5);
		lc_set_attr(thisjob, LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)2);
#endif
		lc_set_attr(thisjob, LM_A_USER_EXITCALL, (LM_A_VAL_TYPE)exitcall);
		lc_set_attr(thisjob, LM_A_USER_RECONNECT_DONE, (LM_A_VAL_TYPE)reconn_done);
	}
	if (ret = lc_checkout(job, feature, "1.0", 1, LM_CO_NOWAIT, &co_code,
							LM_DUP_NONE))
			lc_perror(job, "utcheckout failed");
	return ret;
}

void
ut_checkin(feature)
char *feature;
{
	lc_checkin(job, feature, 0);
}

char *
ut_hostname()
{
	if (!job)
	{
		lc_init((LM_HANDLE *)0, "demo", &code, &job);
	}
	return lc_hostname(job, 0);
}
char *
ut_display()
{
	if (!job)
	{
		lc_init((LM_HANDLE *)0, "demo", &code, &job);
	}
	return lc_display(job, 0);
}
char *
ut_username()
{
	if (!job)
	{
		lc_init((LM_HANDLE *)0, "demo", &code, &job);
	}
	return lc_username(job, 0);
}


