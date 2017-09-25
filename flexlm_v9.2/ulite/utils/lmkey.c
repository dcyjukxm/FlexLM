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
 *
 *	Module:	lmkey.c v1.1.0.0
 *
 *	Function: lmkey
 *
 *	Description: Create a license file for the license manager
 *
 *	Parameters:	None.
 *
 *	Return:		None - license file is written
 *
 *	NOTE:		The salient function in this file is l_crypt_private(),
 *			which returns the 20-character code for the
 *			FEATURE/INCREMENT/UPGRADE line.  Everything
 *			else is I/O, checking, and setting up the call
 *			to l_crypt_private().  Vendors are permitted to modify
 *			this source as needed.
 *	
 *			Normally, lmcrypt is a better tool for making
 *			licenses.
 *
 *	M. Christiano
 *	2/25/88
 *	8/11/98 - Adapted from FLEXlm v5.0 makekey.c
 *
 *	Last changed:  8/18/98
 *
 */

#include "lmclient.h"
#include "lm_code.h"
#include <stdio.h>
#if defined(PC) || defined (__DECC)
#include <stdlib.h>
#include <string.h>
#endif /*  defined(PC) || defined (__DECC) */
extern char *strncpy();

#if 0
#define USE_DEBUG		/* Define this for debug key output */
#endif

#ifdef USE_DEBUG
#define DEBUG(x)  (void) printf x
#else
#define DEBUG(x)
#endif

#if 0
#define NO_CHECKSUM		/* Define this to remove the cksum= part */
				/*   from FEATURE/INCREMENT/UPGRADE lines */
#endif

lm_extern void API_ENTRY
	l_print_config   lm_args((LM_HANDLE_PTR, CONFIG_PTR, LM_CHAR_PTR));
lm_extern void API_ENTRY
	l_get_id lm_args((LM_HANDLE_PTR, HOSTID_PTR_PTR, LM_CHAR_PTR, int ));
lm_extern int  API_ENTRY
        l_parse_feature_line lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
                                      CONFIG_PTR, LM_CHAR_PTR_PTR, int));

LM_CHAR_PTR     l_crypt_private lm_args((LM_HANDLE_PTR job,
				      CONFIG_PTR conf, LM_CHAR_PTR sdate,
					      VENDORCODE_PTR code));

static int makeconfig lm_args((char *feature, char *hostid)); 

LM_HANDLE *lm_job;

#ifdef VMS
#ifndef __DECC
extern exit();
extern strcpy();
extern strlen();
extern memset();
#endif /* __DECC */
#endif  /* VMS */

#undef VENDOR_KEY5
#define VENDOR_KEY5 0xcc96ea43

LM_CODE(site_seeds, 0, 0, 0xbd2b7ad3, 
			0xe5eaf2f2, 0xf07ae883, 0xa796ddbe, 0xcc96ea43);


#ifndef NO_CHECKSUM
/*
 *	Do NOT modify this LM_CODE macro for dummycode - otherwise the
 *	checksums you produce won't work with lmutil.
 */
LM_CODE(dummycode, 0, 0, VENDOR_KEY1,
			VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
#endif

main(argc, argv)
int argc;
char *argv[];
{
  int status, rc;
  FILE *keys, *fopen();
  char key[100];
#if defined (_WINDOWS) && !defined(WINNT)
  struct _wsizeinfo wsize;	

	/*
	 * Set QuickWin window behavior customization.
	 */
	_wsetexit( _WINEXITPERSIST );
	_wabout( "FLEXlm Create License Program\n" COPYRIGHT_STRING(1988));
	wsize._version = _QWINVER;	
	wsize._type = _WINSIZEMAX;
	_wsetsize( _wgetfocus(), &wsize );	
#endif  

	if ((rc = lc_init((LM_HANDLE *)0, 
		"demo", &site_seeds, (LM_HANDLE **) &lm_job)) &&
							rc != LM_DEMOKIT)
		lc_perror(lm_job, "lm_init failed");

	system("grep \"#define ENCRYPTION_SEED\" lm_code.h | sed -e 's/^.*0x//' > /tmp/zz");
	keys = fopen("/tmp/zz", "r");
	if (keys)
	{
		fgets(key, 100, keys);
		sscanf(key, "%x", &site_seeds.data[0]);
		fgets(key, 100, keys);
		sscanf(key, "%x", &site_seeds.data[1]);
		DEBUG(("keys from lm_code.h are %x and %x\n",
			site_seeds.data[0],
			site_seeds.data[1]));
	}

	if (site_seeds.data[0] == 0 && site_seeds.data[1] == 0)
	{
		(void) printf("Error - can't read encryption seeds from lm_code.h\n");
		exit(1);
	}

	if (argc >= 2)
	{
		status = makeconfig(argv[1], argv[2]);
		exit(status);
	}
	else
	{
		printf("Usage: lmkey feature hostid\n");
		exit(1);
	}
}

static
makeconfig(feature, hostid)
char *feature;
char *hostid;
{
  char *u, *us;
  CONFIG conf;
  LM_SERVER _servers;
  char buf[MAX_CONFIG_LINE+1];
  char nodelock[MAX_HOSTID_LEN];
  HOSTID _id, *idptr = &_id;

	(void) memset((char *) &conf, '\0', sizeof(CONFIG));
	conf.type = CONFIG_FEATURE;
	(void) strncpy(conf.feature, feature, MAX_FEATURE_LEN);
	(void) strncpy(conf.version, "1.0", MAX_VER_LEN); 
	(void) strcpy(conf.date, "1-jan-0");
	conf.users = 0;
	conf.server = lm_job->servers = &_servers;
	memset(lm_job->servers, 0, sizeof (LM_SERVER));

	if (hostid[0] != '\0')
	{
		if (strlen(hostid) + strlen("ID_STRING=") > MAX_HOSTID_LEN)
		{
			(void) printf("Error in hostid: %s: too long\n",
					hostid);
			exit(1);
		}
		sprintf(nodelock, "ID_STRING=%s", hostid);
		l_get_id(lm_job, &idptr, nodelock, 0);
		conf.idptr = idptr;
		if (!conf.idptr)
		{
			   (void) printf("WARNING: Invalid hostid specified (%s), none used\n",
					hostid);
		}
	}
	else
	{
		conf.idptr = (HOSTID *)0;
	}

	us = l_crypt_private(lm_job, &conf, "0000", &site_seeds);
	if (us == (char *) NULL || *us == '\0')  
	{
		lc_perror(lm_job, "license generation error");
	}
	else
	{
		(void) strncpy(conf.code, us, MAX_CRYPT_LEN);
	}
#ifndef NO_CHECKSUM
	{
	  LM_SERVER *stmp;
	  unsigned z;
	  int dummy = 0;
	  CONFIG c;

		stmp = conf.server;
		conf.server = (LM_SERVER *) NULL;
		l_print_config(lm_job, &conf, buf);
		l_parse_feature_line(lm_job, buf, &c, 0, 1);
		z = l_cksum(lm_job, &c, &dummy, &dummycode);
		conf.lc_got_options |= LM_LICENSE_CKSUM_PRESENT;
		conf.lc_cksum = (int) z;
		conf.server = stmp;
		if (c.idptr) lc_free_hostid(lm_job, c.idptr);
	}
#endif
	/* l_print_config(lm_job, &conf, buf); */
	(void) printf("License for \"%s\" on hostid \"%s\" is \"%s\"\n", 
			feature, hostid, conf.code);
	if (conf.idptr) lc_free_hostid(lm_job, conf.idptr);
		
	return(0);
}
