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
 *	Module: $Id: ts_config.c,v 1.4 2003/01/13 22:55:18 kmaclean Exp $
 *
 *	Function: ts_config(file, filename, feature, type, daemon, version,
 *			  fromversion, nusers, date, sdate, user_string, hostid)
 *
 *	Description: Creates the FEATURE line for testsuite programs.
 *
 *	Parameters:	(FILE **) file - File to output to.
 *			(char *) filename - The file name for output.
 *			(char *) feature - Feature 
 *			(int) type - Type of line (FEATURE/INCREMENT/UPGRADE) 
 *			(char *) daemon - daemon serving this feature
 *			(char *) version - version of this feature.
 *			(char *) fromversion - from version (for UPGRADE)
 *			(int) nusers - Number of licenses.
 *			(char *) date - expiration date.	
 *			(char *) sdate - start date (use current if NULL).	
 *			(char *) user_string - vendor-defined string.
 *			(char *) hostid - The host (or NULL)
 *
 *	Return:		Feature line written to 'file'
 *
 *	M. Christiano
 *	8/21/89
 *
 *	Last changed:  9/17/98
 *
 */

#ifndef LM_INTERNAL
#define LM_INTERNAL
#endif /* LM_INTERNAL */
#include "lmachdep.h"
#include "lmclient.h"
#include <sys/param.h>
#include <stdio.h>
#include "testsuite.h"
#include "l_openf.h"
#include "l_prot.h"
#include "code.h"
#ifdef PC
#include <stdlib.h>
#include <string.h>
#endif /* PC */
LM_DATA_REF;

ts_config(file, filename, feature, type, daemon, version, fromversion, nusers,
					date, sdate, user_string, hostid)
FILE **file;
char *filename;
char *feature, *daemon;
int type;
char *version;
char *fromversion;
int nusers;
char *date;
char *sdate;
char *user_string;
char *hostid;
{
  void ts_master_list();
  CONFIG conf;
  char *us, buf[MAX_CONFIG_LINE+1];
  static counter = 0;
  LM_SERVER servers[MAX_SERVERS];
  VENDORCODE vc;
  int i;

	memset(servers, 0, sizeof(servers));
	l_config(&conf, type, feature, version, daemon, date, nusers,
		 					user_string, NULL);
	strncpy(conf.fromversion,  fromversion, MAX_VER_LEN);
	conf.fromversion[MAX_VER_LEN] = '\0';
/*
 *	Sometimes print lc_issuer and sometimes lc_vendor_def
 *	to test both old and new version license files
 */
	if (nusers < 4)
		conf.lc_issuer = "GSI";
	else if (!user_string && ((counter++ % 2) == 0))
	{
		conf.lc_vendor_def = "vendor text";
	}
	if (hostid)
	{
		l_get_id(lm_job, &conf.idptr, hostid);
		if (!conf.idptr)
		{
			(void) printf("WARNING: Invalid hostid specified (%s), none used\n",
					hostid);
		}
	}
	else
		conf.idptr = (HOSTID *)0;
#ifdef VMS
	fclose(*file);
#endif
	ts_master_list(lm_job, servers);
	if (conf.server = servers)
	{
	  LM_SERVER_LIST_PTR slp;
                slp = (LM_SERVER_LIST_PTR)l_malloc(lm_job, 
						sizeof(LM_SERVER_LIST));
                slp->s = conf.server;
                slp->next = lm_job->conf_servers;
                lm_job->conf_servers = slp;

	}
#ifdef VMS
	*file = fopen(filename, "a");
	if (*file == (FILE *) NULL) 
	{
		perror("re-open of license file");
		return(0);
	}
#endif
	if (!conf.server && !conf.idptr)
	{
		lm_perror("No hostid specified and can't get server");
		exit(2);
	}

        if (sdate == (char *) NULL) sdate = l_bin_date((char *) NULL);
	(void)memcpy((char *)&vc, (char *)&code, sizeof(vc));
	vc.data[0] ^= VENDOR_KEY5;
	vc.data[1] ^= VENDOR_KEY5;
	us = lc_crypt(lm_job, &conf, sdate, &vc);
	(void) strncpy(conf.code, us, MAX_CRYPT_LEN);

	l_print_config(lm_job, &conf, buf);
	(void) fprintf(*file, "%s\n", buf);
	if (conf.lc_vendor_def) free(conf.lc_vendor_def);
	/*if (conf.idptr) free(conf.idptr);*/
	return(0);
}

/*
 *	ts_master_list -- like lc_master_list, but works even when
 *	there's no FEATURES in a license file, which should only
 *	happen in the testsuite
 */
void
ts_master_list(job, servers)
LM_HANDLE *job;
LM_SERVER *servers; /* should point to an array of MAX_SERVERS servers */
{
  LICENSE_FILE *lf;

	lf = l_open_file(job, LFPTR_CURRENT);
	if (lf != (LICENSE_FILE *) NULL)
	{
		(void)l_master_list_lfp(job, lf, servers); /* sets servers */
		l_lfclose(lf);
	}
}


void API_ENTRY
l_config(conf, type, feature, version, daemon, date, users, vendor_def, server)
CONFIG *conf;
int type;
char *feature;
char * version;
char *daemon;
char *date;
int users;
char *vendor_def;
LM_SERVER *server;
{
	(void) memset((char *) conf, '\0', sizeof(CONFIG));
	conf->type = type;
	(void) strncpy(conf->feature, feature, MAX_FEATURE_LEN);
	strcpy(conf->version, version);
	(void) strncpy(conf->daemon, daemon, MAX_DAEMON_NAME);
	(void) strncpy(conf->date, date, DATE_LEN);
	conf->users = users;
	if (vendor_def && *vendor_def)
	{
		/* need to figure out how to clean this up */
		conf->lc_vendor_def = malloc((strlen(vendor_def)+1));
		if (conf->lc_vendor_def) 
			(void) strcpy(conf->lc_vendor_def, vendor_def);
	}
	conf->server = server;
}
