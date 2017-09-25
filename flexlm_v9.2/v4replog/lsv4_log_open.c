/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation Software Inc.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation Software Inc and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation Software Inc.

/*****************************************************************************/
/*	
 *	Module:	lsv4_log_open.c v1.4.0.0
 *
 *	v4 report.log compatibility package
 *	Functions:
 *		ls_log_open_ascii - opens the ascii log file
 *		ls_log_open_report - opens the report log file
 *		ls_log_report_name - Returns filename of reportlog
 *		ls_log_ascii_name - Returns filename of ascii log 
 *		ls_log_reopen_ascii - close old, open new ascii log file
 *		ls_log_reopen_report - close old, open new report log file
 *		ls_log_close_ascii - closes the ascii log file
 *		ls_log_close_report - closes the report log file
 *
 *	Description:	These functions deal with opening and closing the
 *			ascii log file (the old format), and the report
 *			log file (new in version 2).
 *
 *	D. Birns
 *	4/5/96
 *
 *	Last changed:  9/11/97
 *
 */
static char *sccsid = "@(#) ls_log_open.c v3.19.0.0";
#include "lmachdep.h"
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
/*#include "lsv4server.h"*/
#include "lsv4_sprot.h"
#include "ls_log.h"
#include "../app/lsfeatur.h"
#include "../app/ls_aprot.h"
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
  extern char *sys_errlist[];
#endif

#define LOG_VER_MAJOR 1
#define LOG_VER_MINOR 2

extern FILE *ls_log_ascfile;
extern FILE *ls_log_repfile;
extern long ls_log_lastreptime;
extern int ls_log_pass;
extern char ls_log_lastrepdname[LM_MSG_LEN+1];

static char ascfilename[LM_MSG_LEN+1] = "stdout";
static char repfilename[LM_MSG_LEN+1] = { '\0' };
static char prevascfilename[LM_MSG_LEN+1] = { '\0' };
static char prevrepfilename[LM_MSG_LEN+1] = { '\0' };

#ifndef RELEASE_VERSION
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)getenv(c);\
	}

#define DEBUG(x) if (debug) printf x
#else
#define DEBUG_INIT 
#define DEBUG(x) 
#endif



static FILE *
ls_log_open(filename)
char *filename;
{
	FILE *fp;
	char *mode;

	if (!filename)
		return (FILE *)0;
	if (strcmp(filename,"stdout")==0)
		return stdout;
	else if (strcmp(filename,"stderr")==0)
		return stderr;
	if (filename[0]=='+') {
		filename++;
		mode = "a";
	} else {
		mode = "w";
	}
	fp = fopen(filename,mode);
	return fp;
}

void
ls_log_open_ascii(char * filename)
{
	if (ls_log_ascfile) 
	{
				
		ls_log_close_ascii();
	}
	ls_log_ascfile = ls_log_open(filename);
	if (!ls_log_ascfile) 
	{
		(void) printf(lmtext("Can't open ascii log file %s: %s\n"),
				filename?filename:"<null>", SYS_ERRLIST(errno));
		return;
	}
	
	(void)strcpy(ascfilename,filename);
	(void) fflush(ls_log_ascfile);
}

void
ls_log_reopen_ascii(char * filename)
{
	if (ls_log_ascfile) {
		ls_log_comment(LL_LOGTO_ASCII, lmtext("Logfile switching to %s")
				, filename);
		ls_log_close_ascii();
	}
	ls_log_open_ascii(filename);
	if (!ls_log_ascfile)
		return;		
	ls_log_comment(LL_LOGTO_ASCII, lmtext("Logfile switched from %s"),
						prevascfilename);
}

int
ls_log_open_report(
	char *			filename,
	int				loghead,
	CLIENT_DATA *	client)		/* unused */
{
	if (ls_log_repfile) 
	{
		
		/* if loghead, then logtail */
		ls_log_close_report(loghead);	
	}
	ls_log_repfile = ls_log_open(filename);
	if (!ls_log_repfile) 
	{
		LOG((lmtext("Can't open report log file %s: %s, %d\n"),
				filename?filename:"<null>", 
				SYS_ERRLIST(errno), __LINE__));
		return 0;
	}
	if (loghead)
	{
		(void) fprintf(ls_log_repfile,"lm_log_start %d %d\n",
					LOG_VER_MAJOR, LOG_VER_MINOR);
		ls_log_timestamp(LL_LOGTO_REPORT);    
		ls_feat_info();	
		ls_feat_dump();
		(void) fflush(ls_log_repfile);
	}
	(void)strcpy(repfilename,filename);
	return 1;
}

/*
 *	ls_log_report_name - Returns filename of reportlog
 */

char *
ls_log_report_name()
{
	return(repfilename);
}

/*
 *	ls_log_ascii_name - Returns filename of ascii log
 */
char *
ls_log_ascii_name()
{
	return(ascfilename);
}

int
ls_log_reopen_report(
	char *			filename,
	CLIENT_DATA *	client,		/* unused */
	int				mode)		/* unused */
{
  char *newname = (char *)0;
	DEBUG_INIT
	if (!filename)
	{
		if (!*repfilename)
			return 0; /* nothing to do */
		if (*repfilename == '+')
		{
			DEBUG(("filename has a '+' already: %s\n", repfilename));
			filename = repfilename;
		}
		else
		{
			DEBUG(("prepending '+' to %s\n", repfilename));
			newname = LS_MALLOC(strlen(repfilename) + 2);
			sprintf(newname, "+%s", repfilename);
			filename = newname;
		}
	}
	DEBUG(("filename is %s, repfilename is %s\n", 
		filename ? filename : "null", repfilename));

	
	if (ls_log_repfile) 
	{
		(void) lsv4_log_prefix(LL_LOGTO_REPORT,"lm_switchto");
		ls_log_quote_string(ls_log_repfile,filename);
		ls_log_endline(LL_LOGTO_REPORT);
		ls_log_close_report(1);
	} 
	ls_log_lastreptime = -1;
	ls_log_lastrepdname[0] = 0;
	ls_log_pass = 0;
	ls_log_open_report(filename, 1, 0);
	if (!ls_log_repfile) return 1;   
	if (prevrepfilename && *prevrepfilename) 
	{
		(void) lsv4_log_prefix(LL_LOGTO_REPORT,"lm_switchfrom");
		ls_log_quote_string(ls_log_repfile,prevrepfilename);
		ls_log_endline(LL_LOGTO_REPORT);
	}
	if (newname) free(newname);
	return 1;
}

void
ls_log_close_ascii()
{
	int t;

	if (!ls_log_ascfile) return;  
	t = fflush(ls_log_ascfile);
#ifdef VMS 
	t |= fclose(ls_log_ascfile);
#endif
	ls_log_ascfile = (FILE *)0;
	(void)strcpy(prevascfilename, ascfilename);
	if (t) 
	{
		(void) printf(lmtext("Error closing ascii log file %s: %s\n"),
					ascfilename, SYS_ERRLIST(errno));
	}
}

void
ls_log_close_report(logtail)
int logtail;
{
	int t;

	if (!ls_log_repfile) return;
	if (logtail)
	{
		ls_feat_dump();		
		(void) lsv4_log_prefix(LL_LOGTO_REPORT,"lm_log_end");
		fputc('\n',ls_log_repfile);
	}
	t = fflush(ls_log_repfile);
	t |= fclose(ls_log_repfile);
	(void)strcpy(prevrepfilename,repfilename);
	ls_log_repfile = (FILE *)0;
	if (t) {
		LOG((lmtext("Error closing report log file %s: %s, %d\n"),
				repfilename, SYS_ERRLIST(errno), __LINE__));
	}
}
ls_flush_replog() {}
