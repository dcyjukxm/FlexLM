/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_log_open.c,v 1.23.2.3 2003/07/01 17:04:21 sluu Exp $
 *
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
 *	J. McBeath
 *	5/31/90
 *
 *	Last changed:  12/13/98
 *
 */
#include "lmachdep.h"
#include <stdio.h>
#include <errno.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_log.h"
#include "flex_file.h"
#include "flex_utils.h"
#include "flexevent.h"
#include <fcntl.h>
#include "lsreplog.h"
#include "../machind/lsfeatur.h"
#include "../app/ls_aprot.h"
#include <time.h>
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
  extern char *sys_errlist[];
#endif
#ifdef PC
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/locking.h>
#include <io.h>
#define setlinebuf(x)
#endif /* PC */

#ifdef NLM
#include <fcntl.h>
#include <io.h>
#include <share.h>

#endif

#define LOG_VER_MAJOR 2
#define LOG_VER_MINOR 3

/*
 *	Log of Minor version bug fixes:
 *	VER_MINOR = 1
 *	P1385, 6/3/96 -- if nlicact != 1, it prints nlicreq instead of nlicact.
 *
 *	VER_MINOR = 2
 *	Added CPU event, and CPU args to checkout and checkin
 */



extern FILE *ls_log_ascfile;
extern int ls_fflush_ok;
extern FILE *ls_log_repfile;
extern CLIENT_DATA *ls_log_repfile_client;

extern int ls_log_pass;
extern char ls_log_lastrepdname[LM_MSG_LEN+1];

static char ascfilename[LM_MSG_LEN+1] = "stdout";
static char repfilename[LM_MSG_LEN+1] = { '\0' };
static char prevascfilename[LM_MSG_LEN+1] = { '\0' };
static char prevrepfilename[LM_MSG_LEN+1] = { '\0' };
static char *months[] = { "January", "February", "March", "April", "May",
		"June", "July", "August", "September", "October", "November",
		"December" , ""};

#ifndef RELEASE_VERSION
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)l_real_getenv(c);\
	}

#define DEBUG(x) if (debug) printf x
#else
#define DEBUG_INIT
#define DEBUG(x)
#endif


/*- opens named file for output; checks for stdout or stderr, looks for
 * leading '+' in name to mean append mode */
static
FILE *
ls_log_open(
	char *	filename,
	int		debuglog)	/* flag */
{
	FILE *fp;
	char *mode;
	extern int ls_fflush_ok;

	if (!filename)
		return (FILE *)0;
	if (strcmp(filename,"stdout")==0)
		return stdout;
	else if (strcmp(filename,"stderr")==0)
		return stderr;
	if (filename[0]=='+' || (debuglog && (lm_job->flags & LM_FLAG_IS_VD)) )
        {
		if (*filename == '+') filename++;
#ifdef WINNT
		mode = "a+";
#else
		mode = "a";
#endif
	} else {
		mode = "w";
	}

	fp = l_flexFopen(lm_job, filename, mode);
	if (ls_fflush_ok && fp)
		setlinebuf(fp);
	return fp;
}

void
ls_log_open_ascii(char * filename)
{
	char *	ppszInStr[20] = {NULL};

	if (ls_log_ascfile)
	{
				/*- close old log file before opening new one */
		ls_log_close_ascii();
	}
	ls_log_ascfile = ls_log_open(filename, 1);
	if (!ls_log_ascfile)
	{

		if(l_flexEventLogIsEnabled())
		{
			ppszInStr[0] = filename;
			ppszInStr[1] = SYS_ERRLIST(errno);

			l_flexEventLogWrite(lm_job,
								FLEXEVENT_ERROR,
								CAT_FLEXLM_LMGRD,
								MSG_FLEXLM_DEBUG_LOG_OPEN_ERROR,
								2,
								ppszInStr,
								0,
								NULL);
		}

		fprintf(stderr,"Can't open ascii log file %s: %s\n",
				filename?filename:"<null>", SYS_ERRLIST(errno));

		/* P7319
		 * kmaclean 6/16/03
		 *
		 * If the open failed and the previous file was stderr or stdout
		 * re-open it so logging will continue. */
		if ( strcmp(prevascfilename, "stdout") == 0 )
			ls_log_ascfile = ls_log_open("stdout", 1);
		if (strcmp(prevascfilename, "stderr") == 0 )
			ls_log_ascfile = ls_log_open("stdout", 1);
		return;
	}
	/*- No opening message... */
	strcpy(ascfilename,filename);
	if (ls_log_ascfile && ls_fflush_ok)
		fflush(ls_log_ascfile);
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
		return;		/*- open of new logfile failed */
#ifdef VMS
	ls_log_comment(LL_LOGTO_ASCII, lmtext("Logfile switched from %s"),
						prevascfilename);
#endif
}

/*
 *	ls_log_open_report
 *	returns:	1 == success
 *			0 == failure
 */
int
ls_log_open_report(
	char *			filename,
	int				loghead,
	CLIENT_DATA *	client)
{
	char *	ppszInStr[20] = {NULL};
	if (ls_log_repfile || ls_log_repfile_client)
	{
		/*- close old log file before opening new one */
		/* if loghead, then logtail */
		ls_log_close_report(loghead);
	}
	ls_log_repfile = (FILE *)0;
	ls_log_repfile_client = (CLIENT_DATA *)0;
	ls_dict_init();
	if (client)
		ls_log_repfile_client = client;
	else if (filename)
	{
		ls_log_repfile = ls_log_open(filename, 0);
		if (!ls_log_repfile)
		{

			LOG((lmtext("Can't open report log file %s: %s, %d\n"),
				filename ? filename : "<null>",
				SYS_ERRLIST(errno), __LINE__));
			if(l_flexEventLogIsEnabled())
			{
				ppszInStr[0] = filename ? filename : "<NULL>";
				ppszInStr[1] = SYS_ERRLIST(errno);


				l_flexEventLogWrite(lm_job,
									FLEXEVENT_ERROR,
									CAT_FLEXLM_LMGRD,
									MSG_FLEXLM_VENDOR_REPORT_LOG_OPEN_ERROR,
									2,
									ppszInStr,
									0,
									NULL);
			}
			return 0;
		}
	}
	else
	{
		DLOG((lmtext("INTERNAL ERROR at %d\n"), __LINE__));
		return 0;
	}

	if (loghead)
	{
	  char buf[500], *cp = buf;	/* OVERRUN */
	  extern char cksumbuf[];
	  extern int buflen;
	  char hostname[41];
	  int d, m, y;
	  time_t t;
#ifdef THREAD_SAFE_TIME
	  struct tm tst;
#endif
	  struct tm *tm;


		t = time(0);
#ifdef THREAD_SAFE_TIME
		localtime_r(&t, &tst);
		tm = &tst;
#else /* !THREAD_SAFE_TIME */
		tm = localtime(&t);
#endif
		gethostname(hostname, 40);
		sprintf(buf,
			"# FLEXlm Report Log, %d %s, %d (%2d:%02d), \"%s\" on \"%s\" FLEXlm v%d.%d%s\n",
			tm->tm_mday, months[tm->tm_mon], tm->tm_year + 1900,
			tm->tm_hour, tm->tm_min, lm_job->vendor, hostname,
			lm_job->code.flexlm_version,
			lm_job->code.flexlm_revision,
			lm_job->code.flexlm_patch
			);
		ls_append_repfile(buf);
		*buf = 0;
		*cksumbuf = buflen = 0;
		(void) sprintf(buf,"%x %x %x",
					LL_START, LOG_VER_MAJOR, LOG_VER_MINOR);
		cp += strlen(cp);
		if (l_getattr(lm_job, END_USER_OPTIONS)!=END_USER_OPTIONS_VAL)
		{
			sprintf(cp, " %x%c%x", LL_START_RESTRICTED, LL_EQUAL,
									1);
			cp += strlen(cp);
		}
		strcat(cp, "\n");
		ls_append_repfile(buf);
		ls_feat_info();	/*- dump static feature info */
		ls_log_timestamp(LL_LOGTO_REPORT);    /*- put in a timestamp */
		ls_feat_dump();/*- dump all current features into report file */
	}
	if (!client)
		(void)strcpy(repfilename,filename);
	else
		sprintf(repfilename, "%s-%s-%s-%d", client->name,
			client->node, client->display, client->handle);
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

/*
 *	ls_log_reopen_report
 *	returns:	<0 error, 0--nothing to do, 1--success
 */

int
ls_log_reopen_report(
	char *			filename,
	CLIENT_DATA *	client,
	int				mode)	/* switch or newfile */
{
  char *newname = (char *)0;
  char *switchto;
  char *savefilename = filename;

	DEBUG_INIT

	if (!filename && !client)
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

	switchto = filename;
	if (mode == REPLOG_NEWFILE)
	{
		switchto = repfilename;
		if (*switchto == '+') switchto++;
	}


	if (ls_log_repfile || ls_log_repfile_client)
	{
		ls_log_switchto(switchto, client);

	}

	if (mode == REPLOG_NEWFILE)
	{
	  char *f = filename;
		if (*filename == '+')
			f++;
		if (l_flexRename(lm_job, switchto, f))
			return -1;
		filename = repfilename;
	}
	ls_log_lastrepdname[0] = 0;
	ls_log_pass = 0;
	if (!ls_log_open_report(filename, 1, client))
		return -1;
	if (!ls_log_repfile && !ls_log_repfile_client) return -1;
				/*- open of new logfile failed */
	if (prevrepfilename && *prevrepfilename)
	{
		ls_log_switchfrom(prevrepfilename);
	}
	if (newname) free(newname);

	if (mode == REPLOG_NEWFILE)   /* Only log this message if report log is successfully switched. */
	{
		LOG(("LMNEWLOG: Moving existing REPORTLOG to %s\n", savefilename));
		LOG(("LMNEWLOG: Starting new REPORTLOG %s\n", filename));
	}
	/* return 0; */
	/* This function needs to return a 1 if it succeeds(according to its precondition), bug fix P6380 */
	return 1;
}

void
ls_log_close_ascii()
{
	int t = 0;

	if (!ls_log_ascfile) return;  /*- TBD - write out postscript messages */
#ifdef VMS
	t = fflush(ls_log_ascfile);
#endif
	/* P7319
	 * kmaclean 6/16/03
	 *
	 * Don't close stderr and stdout*/
	if (strcmp(ascfilename,"stdout") != 0 && strcmp(ascfilename,"stderr") != 0)
		t = fclose(ls_log_ascfile);

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
	if (!ls_log_repfile && !ls_log_repfile_client) return;
	if (logtail)
	{
		ls_feat_dump();		/*- dump out final status */
		ls_replog_end();
		ls_append_repfile((char *)0);
		ls_append_repfile("# END REPORT\n");
	}
	if (ls_close_repfile())
		LOG((lmtext( "Error closing report log file %s: %s, %d\n"),
				repfilename, SYS_ERRLIST(errno), __LINE__));
	(void)strcpy(prevrepfilename,repfilename);
	ls_log_repfile = (FILE *)0;
	ls_log_repfile_client = (CLIENT_DATA *)0;
}
