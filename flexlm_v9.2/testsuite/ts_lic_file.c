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
 *	Module: $Id: ts_lic_file.c,v 1.6 2003/01/13 22:55:18 kmaclean Exp $
 *
 *	Function: ts_lic_file(file, hostname, type, hostid, dpath, dopts, 
 *				feature, daemon, ver, nlic, date, string, 
 *				fhostid);
 *
 *	Description: Creates the FEATURE line for testsuite programs.
 *
 *	Parameters:	(FILE *) file - The file to output to.
 *			(char *) hostname - Name of server host.
 *			(int)    type -- CONFIG_{FEATURE/UPGRADE/INCREMENT}
 *			(char *) hostid - Our host ID
 *			(char *) dpath - Path to daemon serving this feature
 *			(char *) dopts - Daemon options (or NULL, for none)
 *			(char *) feature - Feature 
 *			(char *) daemon - daemon serving this feature
 *			(char *) ver - version of this feature.
 *			(int) nlic - Number of licenses.
 *			(char *) date - expiration date.	
 *			(char *) string - vendor-defined string.
 *			(char *) fhostid - The host for this feature (or NULL)
 *			(char **) morehosts - null-terminated array of 
 *				           additional hosts or NULL
 *
 *	Return:		License file written to 'file'
 *
 *	M. Christiano
 *	8/21/89
 *
 *	Last changed:  12/31/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include <sys/param.h>
#include <stdio.h>
#include <errno.h>
#include "testsuite.h"
#include "code.h"
#include "l_prot.h"

#ifdef PC		    
#include <string.h>
#include <stdlib.h>
#include <io.h>		    
#include <direct.h>
#ifndef OS2
#define rm(file)	    _unlink(file)
#define X_OK 0
#endif /* OS2 */
#define cp(from, to)	    CopyFile(from, to, 0)	
#endif /* PC */

#ifdef USE_WINSOCK	
#include <winsock.h>	
#endif			

static int origport = 2837;
int portinc = 100;	/* Amount to increment port # on subsequent calls
				to ts_lic_file */
lm_extern char *l_bin_date lm_args((char *));

LM_DATA_REF;

ts_lic_file(file, hostname, type, hostid, dpath, dopts, feature, daemon, ver, 
		nlic, date, sdate, string, fhostid, morehosts )
char *file;
char *hostname;
int type; 
char *hostid;
char *dpath;
char *dopts;
char *feature;
char *daemon;
char * ver;
int nlic;
char *date;
char *sdate;
char *string;
char *fhostid;
char **morehosts;
{
	return ts_lic_file_vcode(file, hostname, type, hostid, dpath, dopts, 
				feature, daemon, ver, nlic, date, sdate, 
				string, fhostid, morehosts, (VENDORCODE *)0);
}
ts_lic_file_vcode(file, hostname, type, hostid, dpath, dopts, feature, daemon, 
		ver, nlic, date, sdate, string, fhostid, morehosts, vcode)
char *file;
char *hostname;
int type; 
char *hostid;
char *dpath;
char *dopts;
char *feature;
char *daemon;
char * ver;
int nlic;
char *date;
char *sdate;
char *string;
char *fhostid;
char **morehosts;
VENDORCODE *vcode; /* optional vendorcode -- passed to ts_config*/
{
  FILE *cf;
  char **cpp;
  int port = 2837;

	cf = fopen(file, "w");
	if (cf == (FILE *) NULL)
	{
		perror("fopen");
		return(-1);
	}
	if (getenv("TSPORT"))
		sscanf(getenv("TSPORT"), "%d", &port);
	(void) fprintf(cf, "SERVER %s %s %d\n", hostname, hostid, port);
	port += portinc;
	for(cpp=morehosts; cpp && *cpp; cpp++)
	{
		(void) fprintf(cf, "SERVER %s %s %d\n", *cpp,hostid , port);
		port += portinc;
	}
		
		
		
	(void) fprintf(cf, "DAEMON %s %s %s%s\n", daemon, dpath, dopts ? dopts : "",
#ifdef VMS
				" 200"
#else
				""
#endif
					);
	(void) fclose(cf);
	setenv("LM_LICENSE_FILE", file);
	/*lc_flush_config(lm_job);*/
	cf = fopen(file, "a");
	if (cf == (FILE *) NULL)
	{
		perror("fopen");
		return(-1);
	}
	ts_config(&cf, file, feature, type, daemon, ver, "0.0", nlic, 
					date, sdate, string, fhostid);
	unsetenv("LM_LICENSE_FILE");
	(void) fclose(cf);
	/*lc_flush_config(lm_job);*/
	return(0);
}

ts_lic_append(file, feature, type, daemon, ver, fromver, nlic, date, sdate, 
							string, fhostid)
char *file;
char *feature;
int type;	/* Type of line: FEATURE/INCREMENT/UPGRADE */
char *daemon;
char * ver;
char * fromver;	/* from version for UPGRADE */
int nlic;
char *date;
char *sdate;
char *string;
char *fhostid;
{
	return ts_lic_append_vcode(file, feature, type, daemon, ver, fromver, 
		nlic, date, sdate, string, fhostid, (VENDORCODE *)0);
}
ts_lic_append_vcode(file, feature, type, daemon, ver, fromver, nlic, date, 
						sdate, string, fhostid, vcode)
char *file;
char *feature;
int type;	/* Type of line: FEATURE/INCREMENT/UPGRADE */
char *daemon;
char * ver;
char * fromver;	/* from version for UPGRADE */
int nlic;
char *date;
char *sdate;
char *string;
char *fhostid;
VENDORCODE *vcode; /* optional vendorcode */
{
  FILE *cf;

	setenv("LM_LICENSE_FILE", file);
	/*lc_flush_config(lm_job);*/
	cf = fopen(file, "a");
	if (cf == (FILE *) NULL)
	{
		perror("fopen");
		return(-1);
	}
	ts_config(&cf, file, feature, type, daemon, ver, fromver, nlic, date, 
						sdate, string, fhostid);
	unsetenv("LM_LICENSE_FILE");
	(void) fclose(cf);
	/*lc_flush_config(lm_job);*/
	return(0);
}

ts_opt_file (filename, optstring)
char *filename;
char *optstring;
{
	FILE *f;

	/*
	 * Open and then close, to be sure we overwrite if it already exists
	 */
#ifdef VMS
	{
        char *p;
        if (p = strrchr (filename, '/')) filename = p+1;
	}
#endif /* VMS */
	f =  fopen (filename, "w");
        if (f == (FILE *) NULL)
        {
                perror ("fopen");
                return (-1);
        }
	fclose (f);
	return (ts_opt_file_append (filename, optstring));
}

ts_opt_file_append (filename, optstring)
char *filename;
char *optstring;
{
	FILE *f;

#ifdef VMS
{
        char *p;
        if (p = strrchr (filename, '/')) filename = p+1;
}
#endif /* VMS */
	f = fopen (filename, "a");
	if (f == (FILE *) NULL)
	{
		perror ("fopen");
		return (-1);
	}
	fprintf (f, "%s\n", optstring);
	fclose (f);
	return (0);
}

ts_lic_append_vendor_opt(file, daemon, dpath, optfile)
char *file;
char *daemon;
char *dpath;
char *optfile;
{
  FILE *cf;

        setenv("LM_LICENSE_FILE", file);
        /*lc_flush_config(lm_job);*/
        cf = fopen(file, "a");
        if (cf == (FILE *) NULL)
        {
                perror("fopen");
                return(-1);
        }
        (void) fprintf(cf, "DAEMON %s %s%s %s\n", daemon, dpath,
#ifdef VMS
                                " 200"
#else
                                ""
#endif
                       ,optfile);
        unsetenv("LM_LICENSE_FILE");
        (void) fclose(cf);
        /*lc_flush_config(lm_job);*/
        return(0);
}


ts_lic_append_vendor(file, daemon, dpath)
char *file;
char *daemon;
char *dpath;
{
	return(ts_lic_append_vendor_opt (file, daemon, dpath, ""));
}


ts_lic_pkg(file, pkg)
char *file;
char *pkg;
{
  FILE *fp, *efp;
  VENDORCODE vc;
  char *result;
  char *errors;


	memcpy((char *)&vc, (char *)&code, sizeof (vc));
	vc.data[0] ^= VENDOR_KEY5;
	vc.data[1] ^= VENDOR_KEY5;
	errno = 0;
	if (lc_cryptstr(lm_job, pkg, &result, &vc, 0, 0, &errors))
	{
		efp = fopen("tslic.err", "a");
		fputs(errors, efp);
		fclose(efp);
		free(errors);
	}
	if (!(fp = fopen(file, "a")))
	{
		perror("fopen");
		return(-1);
	}
	fprintf(fp, "%s", result);
	(void) fclose(fp);
	free(result);
	return (0);
}
