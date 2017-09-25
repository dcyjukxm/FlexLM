/******************************************************************************

	    COPYRIGHT (c) 2000, 2003 by Macrovision Corporation.
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
 *	Module: $Id: st_8bit.c,v 1.20 2003/01/13 22:55:16 kmaclean Exp $
 *
 *	D. Birns
 *	8/97
 *
 *	Last changed:  11/18/98
 *
 */
#include "lmachdep.h"
#include "code.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lmpolicy.h"
#include "lmpubkey.h"
#include "lmprikey.h"
#include "lm_attr.h"
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
#include <locale.h>
#include "testsuite.h"
static void license(void);
extern VENDORCODE lic_vc;
extern FILE *ofp;
extern LM_HANDLE *job[NUM_JOBS];
#define STR_8BIT "\340\337\347\320fghijklm\361"
/* 
 * note that 160 (0240) is controversial, since it can be a space! 
 */
#define LONG_8BIT \
"\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222" \
"\223\224\225\226\227\230\231\232\233\234\235\236\237\241\242\243\244" \
"\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266" \
"\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310" \
"\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332" \
"\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354" \
"\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377"
static void counted_8bit(void);
static void uncounted_8bit(void);
static char lic_7bit[2000];
static char lic_8bit_vs[2000];
static char lic_8bit[2000];
static char lic_long_8bit[2000];
static void setup_8bit(void);

test_8bit(int counted_flag)
{
  char *savfilep;
  char savfile[1024];
  char buf[100];
  char *languages[20] =
	{
		"fr",
		"es", 
		"it", 
		"ru", 
		"ja", 
		"tr",
		"en" 
	};
  char **cpp;
  char *curr = setlocale(LC_ALL, 0);
	savfilep = getenv("LM_LICENSE_FILE");
	strcpy(savfile, savfilep);
	unsetenv ("LM_LICENSE_FILE");
	
	serv_log( "  ==>8 bit \"" STR_8BIT "\"\n");
	if (curr) 
  	{
		sprintf(buf, "current locale is %s\n", curr);
		serv_log(buf);
	}
	if (counted_flag)
		counted_8bit();
	else
		uncounted_8bit();
	for (cpp = languages; *cpp; cpp++)
	{
		
		if (setlocale(LC_ALL, *cpp))
		{
			sprintf(buf, "locale: %s\n", *cpp);
			serv_log( buf);
			if (counted_flag) counted_8bit();
			else uncounted_8bit();
		}
		else fprintf(ofp, "Can't setlocale to %s, skipping\n", *cpp);
	}
	if (curr) setlocale(LC_ALL, curr);
	else if (curr) setlocale(LC_ALL, "C");

	setenv("LM_LICENSE_FILE", savfile);		
}
int
LM_CALLBACK_TYPE
checkout_filter_8bit( CONFIG *c)
{
	if (!strcmp(c->lc_vendor_def, STR_8BIT))
		return(0); 
	return(1);
}
static
void
setup_8bit(void)
{
  VENDORCODE vc;
  char *lic, *err;
  char buf[2000];

	memcpy(&vc, &lic_vc, sizeof(VENDORCODE));
	LM_CODE_GEN_INIT(&vc);
	lc_init((LM_HANDLE *)0, VENDOR_NAME, &vc, &job[0]);
	if (lc_cryptstr(job[0], "FEATURE lic_string demo 1.0 permanent \
			uncounted 0 HOSTID=ANY SIGN=0 SIGN2=0 NOTICE=text", &lic, &vc, 
				LM_CRYPT_FORCE, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s \n", __LINE__, 
			err ? err : "", lc_errstring(job[0]));
	}
	sprintf(lic_7bit, "START_LICENSE\n%sEND_LICENSE", lic ? lic : "");
	sprintf(buf, 
"FEATURE lic_string demo 1.0 permanent uncounted 0 HOSTID=ANY SIGN=0 SIGN2=0 VENDOR_STRING=\"%s\"", STR_8BIT);
	if (err) free(err);
	if (lic) free(lic);
	if (lc_cryptstr(job[0], buf, &lic, &vc, LM_CRYPT_FORCE, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s \n", __LINE__, 
			err ? err : "", lc_errstring(job[0]));
	}
	sprintf(lic_8bit_vs, "START_LICENSE\n%sEND_LICENSE", lic ? lic : "");
	if (err) free(err);
	if (lic) free(lic);
	if (lc_cryptstr(job[0], 

"FEATURE " STR_8BIT " demo 1.0 permanent uncounted 0 HOSTID=ANY SIGN=0 SIGN2=0",
		&lic, &vc, LM_CRYPT_FORCE, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s \n", __LINE__, 
			err ? err : "", lc_errstring(job[0]));
	}
	sprintf(lic_8bit, "START_LICENSE\n%sEND_LICENSE", lic ? lic : "");
	if (err) free(err);
	if (lic) free(lic);
	if (lc_cryptstr(job[0], 

"FEATURE long_8bit demo 1.0 permanent uncounted 0 HOSTID=ANY SIGN=0 SIGN2=0 NOTICE="
	LONG_8BIT,
		&lic, &vc, LM_CRYPT_FORCE, "string", 
					&err))
	{
		fprintf(ofp, "cryptstr failed line %d %s %s \n", __LINE__, 
			err ? err : "", lc_errstring(job[0]));
	}
	sprintf(lic_long_8bit, "START_LICENSE\n%sEND_LICENSE", lic ? lic : "");
	if (err) free(err);
	if (lic) free(lic);
	lc_free_job(job[0]);
}
static
void
uncounted_8bit(void)
{
  LM_USERS *users;


	{
	  int c = 0xe4;
	if (isspace(0xe4))
		fprintf(ofp, "results are incorrect -- isspace is wrong!\n");
		if (isspace(c))
		fprintf(ofp, "results are incorrect -- isspace is wrong!\n");
	}

	setup_8bit();
	
	if (CHECKOUT(LM_RESTRICTIVE, "lic_string", "1.0", lic_7bit))
		fprintf(ofp, "lic_string failed line %d %s\n", __LINE__, 
			ERRSTRING());
	else
		CHECKIN();
	

/*
 *	VENDOR_STRING
 */
	if (CHECKOUT(LM_RESTRICTIVE, "lic_string", "1.0", lic_8bit_vs))
		fprintf(ofp, "lic_string failed line %d %s\n", __LINE__, 
			ERRSTRING());
	else
		CHECKIN();
/*
 *	Checkout filter based on vendor-string
 */
	ts_lc_new_job( &code, &job[1], __LINE__); 
	st_set_attr(job[1], LM_A_CHECKOUTFILTER, 
		(LM_A_VAL_TYPE) checkout_filter_8bit, __LINE__);
	st_set_attr(job[1], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE) lic_8bit_vs, 
		__LINE__);
	if (lc_checkout(job[1], "lic_string", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "8bit vstring failed line %d: %s\n", __LINE__, 
				lc_errstring(job[1]));
	lc_free_job(job[1]);
/*
 *	feature name
 */

	if (CHECKOUT(LM_RESTRICTIVE, STR_8BIT, "1.0", lic_8bit))
		fprintf(ofp, "8bitstring failed line %d %s\n", __LINE__, 
			ERRSTRING());
	else
		CHECKIN();
/*
 *	uppercase ??
 */
 	{
	  char upper[300];

	  	strcpy(upper, STR_8BIT);
		l_uppercase(upper);
		if (CHECKOUT(LM_RESTRICTIVE, STR_8BIT, "1.0", lic_8bit))
			fprintf(ofp, "8bitstring failed line %d %s\n", __LINE__, 
				ERRSTRING());
		else
			CHECKIN();
	}
	if (CHECKOUT(LM_RESTRICTIVE, "long_8bit", "1.0", lic_long_8bit))
		fprintf(ofp, "8bitstring failed line %d %s\n", __LINE__, 
			ERRSTRING());
	else
		CHECKIN();
	
}
static
void
counted_8bit(void)
{
  LM_USERS *users;


	
/*
 *	Checkout from server
 */
	if (CHECKOUT(LM_RESTRICTIVE|LM_USE_LICENSE_KEY, STR_8BIT, "1.0", "."))
		fprintf(ofp, STR_8BIT " failed line %d %s\n", __LINE__, 
			ERRSTRING());
	else
		CHECKIN();
/*
 *	Checkout filter based on vendor-string
 */
	ts_lc_new_job( &code, &job[1], __LINE__); 
	st_set_attr(job[1], LM_A_CHECKOUTFILTER, 
		(LM_A_VAL_TYPE) checkout_filter_8bit, __LINE__);
	st_set_attr(job[1], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE) ".", 
		__LINE__);
	if (lc_checkout(job[1], STR_8BIT, "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "8bit vstring failed line %d: %s\n", __LINE__, 
				lc_errstring(job[1]));
	lc_free_job(job[1]);
/*
 *	User info
 */
	ts_lc_new_job( &code, &job[1], __LINE__); 
	st_set_attr(job[1], LM_A_USER_OVERRIDE, 
		(LM_A_VAL_TYPE) STR_8BIT, __LINE__);
	st_set_attr(job[1], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE) ".", 
		__LINE__);
	if (lc_checkout(job[1], "f2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "8bit vstring failed line %d: %s\n", __LINE__, 
				lc_errstring(job[1]));
	
	for(users = lc_userlist(job[1], "f2"); users; users = users->next)
	{
		if (*users->name)
		{
			if (strcmp(users->name, STR_8BIT)) 
				fprintf(ofp, 
				"8bit error line %d, expected %s got %s\n",
				__LINE__, STR_8BIT, users->name);
		}
	}
	lc_free_job(job[1]);
	ts_lc_new_job( &code, &job[1], __LINE__); 
	st_set_attr(job[1], LM_A_HOST_OVERRIDE, 
		(LM_A_VAL_TYPE) STR_8BIT, __LINE__);
	st_set_attr(job[1], LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE) ".", 
		__LINE__);
	if (lc_checkout(job[1], "f2", "1.0", 1, 0, &code, LM_DUP_NONE))
		fprintf(ofp, "8bit vstring failed line %d: %s\n", __LINE__, 
				lc_errstring(job[1]));
	
	for(users = lc_userlist(job[1], "f2"); users; users = users->next)
	{
		if (*users->name)
		{
			if (strcmp(users->node, STR_8BIT)) 
				fprintf(ofp, 
				"8bit error line %d, expected %s got %s\n",
				__LINE__, STR_8BIT, users->node);
		}
	}
	lc_free_job(job[1]);
	
}
