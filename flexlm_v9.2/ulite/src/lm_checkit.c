/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision  Inc and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision  Inc.

 *****************************************************************************/
/*
 *	Module:	$Id: lm_checkit.c,v 1.5 2003/01/14 21:47:24 kmaclean Exp $
 *
 *	Function: lc_checkit(vendor, feature, lic_key, hostid, key)
 *
 *	Description: "checks out" a copy of the specified feature for use.
 *
 *	Parameters:	(char *) vendor - Vendor name
 *			(char *) feature - The ascii feature name desired.
 *			(char *) lic_key - The license key.
 *			(char *) hostid - The hostid for this license.
 *			(VENDORCODE *) key - The vendor-specific encryption
 *						seeds for this feature.
 *
 *	Return:		(int) - 0 - OK, ie. we are running on the specified
 *					host.
 *				NOTTHISHOST - We are not running on the
 *						specified host.
 *
 *	M. Christiano
 *	2/18/88
 *
 *	Last changed:  11/27/02
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"

#if 0
#define LM_CHECKIT_DEBUG
#endif

#ifdef LM_CHECKIT_DEBUG

void *cmd_string;
#define DEBUG(x) xprintf(x)
#define DEBUG_DEC(x) output_dec_word(x, 1, cmd_string);
#ifdef TARGET
#define putchar CustomPrintChar
#endif
xprintf(str) char *str; { char *p = str; while (*p) { putchar(*p); p++; } }
xputchar(c) char c; { putchar(c); }

#else

#define DEBUG(x)
#define DEBUG_DEC(x)
#endif

/*-
 *	We have our own private copies of many functions, as a way
 *	to thwart theft
 */
#define l_crypt_private l_ckout_crypt
#define l_string_key	l_ckout_string_key
static LM_CHAR_PTR API_ENTRY    l_ckout_crypt lm_args((LM_HANDLE_PTR job,
				      CONFIG_PTR conf, LM_CHAR_PTR sdate,
					      VENDORCODE_PTR code));
static LM_U_CHAR_PTR API_ENTRY
		l_ckout_string_key	lm_args((LM_HANDLE_PTR, LM_U_CHAR_PTR, 
				int, VENDORCODE_PTR, unsigned long, char *));

static int good_lic_key lm_args((LM_HANDLE *, CONFIG *, VENDORCODE *));




API_ENTRY
lc_checkit(vendor, feature, lic_key, hostid, key)
const LM_CHAR_PTR vendor;		/* The vendor name (from lm_code.h) */
const LM_CHAR_PTR feature;		/* The feature to be checked in */
const LM_CHAR_PTR lic_key;		/* license key */
const LM_CHAR_PTR hostid;		/* hostid for this license */
const VENDORCODE_PTR key;		/* The vendor's key */
{
  CONFIG conf;
  LM_HANDLE _job, *job = &_job;	/* Allocate the job */
  LM_OPTIONS opt;
  HOSTID hid;

	bzero(job, sizeof(LM_HANDLE));
	bzero(&opt, sizeof(LM_OPTIONS));
	job->options = &opt;
	(void) strcpy(job->vendor, vendor);
	L_SECLEN_SET_SHORT;

	if ((feature == (char *) NULL) || *feature == (char) NULL ||
			(key == (VENDORCODE *) NULL))
	{
		return(LM_BADPARAM);
	}
	else if (hostid == (char *) NULL || *hostid == (char) NULL)
	{
		return(LM_BADFILE);
	}
	else if ((lic_key == (char *) NULL) || (*lic_key == (char) NULL))
	{
		return(LM_BADCODE);
	}
/*
 *	Build the CONFIG
 */
	bzero(&conf, sizeof(CONFIG));
	conf.type = CONFIG_FEATURE;	
	(void) strcpy(conf.feature, feature);
	(void) strcpy(conf.date, "1-jan-0");
	(void) strcpy(conf.version, "1.0");
/*
 *	Insert the hostid
 */
	bzero(&hid, sizeof(HOSTID));
	conf.idptr = &hid;
	
	conf.idptr->type = HOSTID_STRING;
	(void) strcpy(conf.idptr->hostid_string, hostid);

	conf.users = 0;
	(void) strcpy(conf.code, lic_key);
/*
 *	re-encrypt the data, compare to the code in the file.
 */
	if (!good_lic_key(job, &conf, key)) return(LM_BADCODE);
	else return(0);	
}
						   

/*
 *	return 1 if good, 0 if bad
 */
static
int
good_lic_key(job, conf, key)
LM_HANDLE *job;
CONFIG *conf;
VENDORCODE *key;
{
  VENDORCODE vc;
  int ok = 0;
  unsigned long signature;
  char *code;
  int str_res;
  char x[5];

DEBUG_DEC(1234);

	(void) bzero(x, 5);

	if (!(conf->lc_keylist && job->L_SIGN_LEVEL))
	{
		job->flags |= LM_FLAG_MAKE_OLD_KEY;
	}
	else
	{
		signature = l_svk(job->vendor, key);
		DEBUG_DEC(signature);
		(void) memcpy((char *)&vc, (char *)key, sizeof(vc));
		vc.data[0] = key->data[0] ^ signature;
		DEBUG_DEC(vc.data[0]);
		vc.data[1] = key->data[1] ^ signature;
		DEBUG_DEC(vc.data[1]);
	} 
	(void) memcpy((char *)&vc, (char *)key, sizeof(vc));
	 
	code = l_crypt_private(job, conf, x, &vc);
   	if (!(conf->lc_keylist && job->L_SIGN_LEVEL))
		job->flags &= ~LM_FLAG_MAKE_OLD_KEY;

	DEBUG("code: ");
	DEBUG(code);
	DEBUG("\n");
	DEBUG("conf->code: ");
	DEBUG(conf->code);
	DEBUG("\n");
	
	STRNCMP(code, conf->code, MAX_CRYPT_LEN, str_res);
	if (str_res)
	{
	}
	else ok = 1;

	DEBUG_DEC(4321);

	return ok;
}


#define SIGSIZE 4

unsigned long l_svk(char *vendor_id, VENDORCODE *key)
{
	unsigned long keys[4];
	unsigned long signature;
	char sig[SIGSIZE];
	/*- unsigned long x = 0xa8f38730;                   v3.1 */
	/*-unsigned long x = 0x7648b98e;                    v7.0 */
	unsigned long x = 0x6f7330b8;                   /*- v8.x */
	int i = SIGSIZE-1;

	/*-
	 *      First, verify the key
	 */
	l_key(vendor_id, &(key->keys[0]),keys,4);
	/*-                             JUNK only in top 16 bits as of v2.2 */
	/*-                             JUNK XORed with date as of v2.4 */


	sig[0] = sig[1] = sig[2] = sig[3] = '\0';

	while (*vendor_id)
	{
	        sig[i] ^= *vendor_id++;
	        i--;
	        if (i < 0) i = SIGSIZE-1;
	}
	signature = (long)sig[0] |
	            ((long)sig[1] << 8) |
	            ((long)sig[2] << 16) |
	            ((long)sig[3] << 24);
	signature ^= x;
	signature ^= keys[1];
	signature ^= keys[2];
	signature &= 0xffffffff;
	if (signature == 0) 
		signature = x;      /* 0 invalid */
	return(signature);
}


/*-
 *	Include l_crypt.c, so that these functions won't be global
 *	on Unix, at least not from here.
 */
#define LM_CRYPT
#define LM_CKOUT

#include "l_crypt.c"
