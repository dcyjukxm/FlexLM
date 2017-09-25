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
 *	Module: $Id: ts_reread.c,v 1.9 2003/01/13 22:55:18 kmaclean Exp $
 *
 *
 *	Last changed:  7/28/96
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "lm_attr.h"
#include "l_prot.h"
#ifdef VMS
#include "lsmaster.h"
#endif /* VMS */
#define lmtext(x) x
#ifdef FREE_VERSION
#include "free.h"
LM_CODE(reread_code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
#define SIG VENDOR_KEY5
#else
#include "code.h"
#define TEST_DEMOF
#endif
extern int l_borrow_in_seconds;

ts_borrow_minutes_is_seconds(job)
LM_HANDLE *job;
{
  LM_HANDLE *lm_job;
  char *ret;
	l_borrow_in_seconds = 1;
	if (!(ret = lc_vsend(job, "seconds")) || strcmp(ret, "okseconds"))
		lc_perror(job, "seconds error");
	if (!(ret = lc_vsend(job, "test")) || strcmp(ret, "ok"))
		lc_perror(job, "flush error");
}

ts_flush_server(LM_HANDLE *job, int line)
{
  LM_HANDLE *lm_job;
  char *ret;
	if (!(ret = lc_vsend(job, "flush")) || strcmp(ret, "flushed"))
		fprintf(stderr, "flush error line %d: %s\n", line, lc_errstring(job));
	if (!(ret = lc_vsend(job, "test")) || strcmp(ret, "ok"))
		fprintf(stderr, "flush error line %d: %s\n", line, lc_errstring(job));
}
