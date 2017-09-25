/******************************************************************************

	    COPYRIGHT (c) 2001, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_path.c,v 1.11.12.2 2003/07/01 17:04:22 sluu Exp $
 *
 *	Function:	lmpath
 *
 *	D. Birns
 *	April, 2001
 *
 */
#include "lmutil.h"
#include "flex_utils.h"
#include <time.h>
#include <string.h>
#define PATH_ADD 1
#define PATH_OVERRIDE 2

static int lmpath_usage(void);
static int getfilelist(LM_HANDLE **jobp, char *vendor, char ***filelist);

static
int 
lm_path_status(void)
{
  char *vendornames, *thisname, *nextname, **filelist, *cp;
  char buf[50];
  int ret;
  LM_HANDLE *job;

/*
 *	get list of vendornames in registry and environment
 */
	for (thisname = vendornames = l_vendor_license_file(lm_job, 1);
		thisname; thisname = nextname)
	{
  	  char vendor[MAX_VENDOR_NAME + 1];
	  int i;
		if (nextname = strchr(thisname, PATHSEPARATOR))
			*nextname++ = 0;
		if (thisname == vendornames)
		{
			fprintf(ofp, "Known Vendors:\n");
			fprintf(ofp, "_____________\n\n");
		}
		for (i = 0; 
		thisname[i] && (!(thisname[i] == '_' && thisname[i+1] == 'L')); 
		i++)
			vendor[i] = thisname[i];
		vendor[i] = 0; /* null terminate */
		l_lowercase(vendor);
		if (ret = getfilelist(&job, vendor, &filelist))
			return ret;
		{
                  char buf[50];
                        sprintf(buf, "%s:", vendor);
                        fprintf(ofp, "%-15s", buf);
                }
			
		while (*filelist)
		{
			fprintf(ofp, "%s", *filelist);
			filelist++;
			if (*filelist && **filelist)
				fprintf(ofp, "%c", PATHSEPARATOR);
		}
		fprintf(ofp, "\n");
		lc_free_job(job);
	}
	if (vendornames) free(vendornames);
	fprintf(ofp, "_____________\n\n");
	fprintf(ofp, "Other Vendors:\n");
	fprintf(ofp, "______________\n\n");
	if (lc_init(0, "lmgrd", &code, (LM_HANDLE **) &job))
	{
		printf("lmutil: can't initialize: %s\n", 
				lc_errstring(job));
		exit(job->lm_errno);
	}

	l_init_file(job);
	if (lc_get_attr(job, LM_A_LF_LIST, (short *)&filelist))
	{
		fprintf(ofp, "lmpath: get license file list: %s\n", 
				lc_errstring(job));
		return job->lm_errno;
	}
	while (*filelist)
	{
		
		fprintf(ofp, "%15s%s", " ", *filelist++);
		if (*filelist && **filelist) fprintf(ofp, "%c", PATHSEPARATOR);
	}
	fprintf(ofp, "\n");
	lc_free_job(job);
	return 0;
	
}
static
int 
lm_path(int argc, char **argv, int what)
{
  char *key;
  char buf[200];
  char *cp;
  char *newpath = argv[3];
  char *vendor = argv[2];
  char *value;
	if (argc != 4) return lmpath_usage();

	if (l_keyword_eq(lm_job, vendor, "all"))
	{
		key = LM_DEFAULT_ENV_SPEC;
	}
	else  key = vendor;
/*
 *	Check if the name is already VENDOR_LICENSE_FILE
 */
	if ((cp = strchr(key, '_')) && 
		l_keyword_eq(lm_job, cp, "_LICENSE_FILE"))
		; /* nothing to do */
	else
	{
		l_zcp(buf, key, 180);
		strcat(buf, "_LICENSE_FILE");
		key = buf;
	}
	l_uppercase(key);
	if (strlen(key) > (MAX_DAEMON_NAME + strlen("_LICENSE_FILE")))
	{
		fprintf(ofp, "Vendor name invalid: too long\n");
		return LM_BADPARAM;
	}
	if (what == PATH_ADD)
	{
	  char **filelist;
	  LM_HANDLE *job;
	  int found;
		for (found = 0, getfilelist(&job, vendor, &filelist); 
					*filelist && !found; filelist++)
		{
			if (!strcmp(*filelist, newpath))
			{
				found = 1;
				fprintf(ofp, "Warning: %s already in path, not added\n", 
					*filelist);
			}
		}

	  	lc_get_registry(lm_job, key, &value);
		if (found && value) newpath = value;
		else if (value)
		{
			newpath = l_malloc(lm_job, strlen(value) + 
					strlen(argv[3]) + 4);
			sprintf(newpath, "%s%c%s", argv[3], PATHSEPARATOR, 
				value);
		}
	}
		
	if (lc_set_registry( lm_job, key, newpath))
	{
		fprintf(ofp, "Can't reset path: %s\n", lc_errstring(lm_job));
		return lm_job->lm_errno;
	}
	fprintf(ofp, "New path for %s: %s\n", key, newpath);
	return 0;
}
int 
lmpath_main(int argc, char **argv)
{
  int arg;

/*
 *	call correct option, or print usage
 */
	if ((argc != 4) && (argc != 2)) return lmpath_usage(); 

	if ((argc == 2) && l_keyword_eq(lm_job, argv[1], "-status"))
		return lm_path_status();

	if (argc != 4) return lmpath_usage();

	if (l_keyword_eq(lm_job, argv[1], "-add"))
		return lm_path(argc, argv, PATH_ADD);

	if (l_keyword_eq(lm_job, argv[1], "-override"))
		return lm_path(argc, argv, PATH_OVERRIDE);

	return lmpath_usage();
}
static
int
lmpath_usage(void)
{
	
	fprintf(ofp, "usage: lmpath\n");
	fprintf(ofp, "\t\t-status                         (display current path settings)\n");
	fprintf(ofp, "\t\t-override {all | vendor } path  (set path for vendor)\n");
	fprintf(ofp, "\t\t-add {all | vendor } path       (add to path for vendor)\n");
	return 0;
}
static
int
getfilelist(LM_HANDLE **jobp, char *vendor, char ***filelist)
{
 LM_HANDLE *job;
/*
 *	Now pretend we're a regular client
 */
	if (lc_init(0, "lmgrd", &code, (LM_HANDLE **) &job))
	{
		fprintf(ofp, "lmpath: can't initialize: %s\n", 
				lc_errstring(job));
		return job->lm_errno;
	}

	strcpy(job->alt_vendor, vendor);
	l_init_file(job);
	*jobp = job;

	if (lc_get_attr(job, LM_A_LF_LIST, (short *)filelist))
	{
		fprintf(ofp, "lmpath: get license file list: %s\n", 
				lc_errstring(job));
		return job->lm_errno;
	}
	return 0;
}
