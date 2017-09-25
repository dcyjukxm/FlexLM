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
 *	Module: $Id: ls_log.c,v 1.22.6.5 2003/07/03 21:28:38 sluu Exp $
 *
 *	Functions:
 *		ls_log_asc_printf - gets called by the LOG macro
 *		ls_log_timestamp - add a timestamp
 *		ls_log_usage - changes in license usage
 *		ls_log_res_usage - changes in reservation usage
 *		ls_log_comment - log a random comment
 *		ls_log_error - log an error
 *
 *	Description:	These functions deal with outputting information to
 *			the ascii log file (the old format), and the report
 *			log file (new in version 2).
 *
 *	J. McBeath
 *	5/31/90
 *
 *
 *	Last changed:  10/29/98
 */

#include "lmachdep.h"
#ifdef ANSI
/*#define va_dcl va_list va_alist;*/
#include <stdarg.h>
#else /* ANSI */
#include <varargs.h>
#endif /* ANSI */
#include <stdio.h>
#include <sys/types.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_log.h"
#include "lsreplog.h"
#include "flex_utils.h"
#include <time.h>

extern int ls_i_am_lmgrd;

static int output_warning = 0;
/*- #define CLASSIC_DEBUG_LOG	 Define for old format */

#ifndef CLASSIC_DEBUG_LOG
static char *warning_msg =
"-----------------------------------------------\n\000\
  Please Note:\n\000\
\n\000\
  This log is intended for debug purposes only.\n\000\
  There are many details in licensing policies\n\000\
  that are not reported in the information logged\n\000\
  here, so if you use this log file for any kind\n\000\
  of usage reporting you will generally produce\n\000\
  incorrect results.\n\000\
\n\000\
-----------------------------------------------\n\000\
\n\000\
\n\000\000";

#else

static char *warning_msg =
"-----------------------------------------------\n\000\
\n\000\
  Please Note:\n\000\
\n\000\
  This log file is intended for DEBUG purposes\n\000\
  ONLY. It is not now, and never was, intended\n\000\
  for capturing product usage information; rather\n\000\
  it has always been a tool to verify that your\n\000\
  licensing system was operating correctly and\n\000\
  to inform you when license requests were being\n\000\
  denied.\n\000\
\n\000\
  There are many details in licensing policies\n\000\
  that are not reported in the information logged\n\000\
  here, so if you use this log file for any kind\n\000\
  of usage reporting you will generally produce\n\000\
  incorrect results.  This has always been the\n\000\
  case.\n\000\
\n\000\
  You may think that the results you are getting\n\000\
  are correct, but there is a good chance that\n\000\
  they are not.\n\000\
\n\000\
  For accurate usage reporting, please use the\n\000\
  REPORTLOG option (in the daemon options file)\n\000\
  and FLEXadmin's report writer.\n\000\
\n\000\
-----------------------------------------------\n\000\
\n\000\
\n\000\000";
#endif /*- CLASSIC_DEBUG_LOG */
#ifdef SUNOS5
static char *sun_ndd =
"-----------------------------------------------\n\000\
  Solaris Note:\n\000\
\n\000\
  We recommend adding the following command to the boot\n\000\
  scripts due to a bug in the Solaris operating system:\n\000\
\n\000\
  Solaris 2.1-2.6: \n\000\
   /usr/sbin/ndd -set /dev/tcp tcp_close_wait_interval 2400\n\000\
  Solaris 2.7 and higher: \n\000\
   /usr/sbin/ndd -set /dev/tcp tcp_time_wait_interval 2400\n\000\
\n\000\
  By default on Solaris, upon stopping a license server,\n\000\
  1 to 5 minutes are required for the port to free up so it\n\000\
  will restart, which can result in checkout failures.\n\000\
  The command above resets this default to 2.4 seconds\n\000\
\n\000\
-----------------------------------------------\n\000\
\n\000\
\n\000\000";
#endif /* SUN */





extern time_t time();

FILE *ls_log_ascfile = (FILE *) NULL;	/* default is setup in ls_xxx_init.c */
FILE *ls_log_repfile;
CLIENT_DATA *ls_log_repfile_client;
char ls_log_lastrepdname[LM_MSG_LEN+1] = { '\0' };
	/* daemon name in last report log message */

int ls_fflush_ok = 1; /* Should be 0 if writing to nfs-mounted system */


int
ls_close_repfile()
{
	if (ls_log_repfile)
	{
		return fclose(ls_log_repfile);
	}
	else
		return 0;
}

/*-
 *	output_to_repfile
 *	if str is null, flush
 *	cksumbuf is in this routine.  Note that we guarantee to
 *	not overflow this buffer -- if it would overflow, we
 *	print an intermediate cksum.
 */

void
ls_append_repfile(str)
char *str;
{
  extern char cksumbuf[];
  extern int buflen;
  char buf[MAX_REPFILE_LEN];
  static int cnt;
  long t = time(0);
  static long lastflush;
  static int eventcnt = 1;
  int len = 0;
#define ONEDAY 60*60*24
#define FLUSH_PERIOD (FLUSH_EVERY_N_DAYS * ONEDAY)

	if (!lastflush) lastflush = t;
	cnt++;
	if (str) len = strlen(str);
	if (!str || (buflen + len + 200) >= CKSUMBUF_SIZE)
	{
		ls_replog_cksum(cksumbuf);
		*cksumbuf = '\0';
		cnt = 0;
		buflen = 0;
	}


	if (ls_lf(str))
		eventcnt ++;

	if (str && ((cnt % PRINT_CKSUM_EVERY_N_LINES) == 0))
	{
		ls_replog_cksum(cksumbuf);
		*cksumbuf = '\0';
		buflen = 0;
	}

	if (!str)
	{

		if (str) ls_lf(0); /*- force flush of compressed stream */
		sprintf(buf, "%x\n", LL_FLUSH);
		ls_lf(buf);
		ls_dict_stat(buf);
		ls_lf(buf);
		ls_lf(0);
		ls_dict_flush();
		ls_log_client(0, LS_CLIENT_FLUSH);
		eventcnt++;
		lastflush = t;
	}
}

int ls_log_time_in_msg = 1;	/*- Log the time at the start of msgs */

/*- convert time number to string suitable for ascii log file */
static
char *
ls_log_asctime(ltime)
long ltime;
{
#ifdef THREAD_SAFE_TIME
	struct tm *localtime_r(const time_t * clock, struct tm * res), *t;
	struct tm tst;
#else /* !THREAD_SAFE_TIME */
  struct tm *localtime(), *t;
#endif
  static char buf[50];

        if (ls_log_time_in_msg)
        {
                time_t  tt = ltime;
#ifdef THREAD_SAFE_TIME
		localtime_r(&tt, &tst);
		t = &tst;
#else /* !THREAD_SAFE_TIME */
                t = localtime(&tt);
#endif
#if defined(VMS) || defined(CLASSIC_DEBUG_LOG)
                sprintf(buf,"%02d/%02d %2d:%02d:%02d",
                        t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min,
                        t->tm_sec);
#else
                sprintf(buf,"%2d:%02d:%02d",
                        t->tm_hour, t->tm_min, t->tm_sec);
#endif /* VMS */
        }
        else
        {
                buf[0] = '\0';
        }
        return(buf);
}

/*- convert delta time number to string suitable for ascii log file */
static
char *
ls_log_ascdeltatime(dtime)
long dtime;
{
  long h,m,s;	/*- long instead of int just to please lint */
  static char buf[50];

	s = dtime%60;	/*- get seconds */
	dtime /= 60;
	m = dtime%60;	/*- minutes */
	dtime /= 60;
	h = dtime%24;	/*- hours */
	dtime /= 24;	/*- what's left are days */
	if (dtime)
		sprintf(buf,"%d:%02d:%02d:%02d", dtime, h, m, s);
	else if (h)
		sprintf(buf,"%02d:%02d:%02d", h, m, s);
	else
		sprintf(buf,"%d:%02d", m, s);

	return(buf);
}

/*- get current daemon name */
char *
ls_log_daemon_name()
{
  static char buf[MAX_DAEMON_NAME+MAX_LONG_LEN+3];
  char *p;

	if (!lm_job->vendor[0])
		return("lmgrd");
	p = buf;
	strcpy(p, lm_job->vendor);
	p += strlen(p);
	*p = 0;
	return(buf);
}

void
ls_log_prefix(where,key)
int where;
int key;
{
	long ltime;
#ifdef NLM
	ThreadSwitch();
#endif /* NLM */

	ltime = (long)time(0);

	if (ls_log_ascfile && (where&LL_LOGTO_ASCII))
	{
		fprintf(ls_log_ascfile, "%s ", ls_log_asctime(ltime));
		if (key == -1)
			fprintf(ls_log_ascfile, "((%s)) ", ls_log_daemon_name());
		else
			fprintf(ls_log_ascfile, "(%s) ", ls_log_daemon_name());
	}
}

void
ls_log_endline(where)
int where;
{
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII))
	{
		fputc('\n',ls_log_ascfile);
#ifndef  PC
		if (ls_fflush_ok)
#endif
                    fflush(ls_log_ascfile);
	}
	if (where&LL_LOGTO_REPORT)
	{
		ls_append_repfile("\n");
	}
}

/*- Put a timestamp into the log file */
void
ls_log_timestamp(where)
int where;
{
	ls_log_prefix(where, LL_TIMESTAMP);
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII))
	{
		fprintf(ls_log_ascfile, lmtext("Timestamp"));
	}
	ls_log_endline(where&LL_LOGTO_ASCII);
	ls_replog_timestamp();
}

static
int
ls_log_hash(s)
char *s;
{
  int n=0;

	while (*s)
		n = (n<<1)^(*s++);
	return(n);
}

/*- Generate the encryption value */
static
int
ls_log_genlinecrypt(curtime, lickey)
long curtime;
char *lickey;
{
  char *dname;
  int nd,nf;

	dname = ls_log_daemon_name();
	nd = ls_log_hash(dname);
	nf = ls_log_hash(lickey);
	return((curtime ^ (nd+nf))&0xFFFF);
}

/*- Called by the LOG macro to put things into the ascii log file */
/* VARARGS1 */
void
#ifdef ANSI
ls_log_asc_printf(char *fmt, ...)
#else
ls_log_asc_printf(fmt, va_alist)        /* same args as printf */
char *fmt;
va_dcl
#endif /* ANSI */
{
  va_list pvar;

	if (!ls_log_ascfile) return;
#ifdef lint	/*- what a hack... */
	pvar = 0;	/*- avoid "used before set" message */
#else
#ifdef ANSI
	va_start(pvar, fmt);
#else
	va_start(pvar);
#endif
#endif
#ifndef CLASSIC_DEBUG_LOG
	if (ls_i_am_lmgrd)
#endif
	{
		if (!output_warning)
		{
		  char *cp;

			output_warning = 1;
			for (cp = warning_msg; *cp; cp += strlen(cp) + 1)
			{
				fprintf(ls_log_ascfile, "%s", cp);
				ls_log_prefix(LL_LOGTO_ASCII, 0);
			}
#ifdef SUNOS5
			for (cp = sun_ndd; *cp; cp += strlen(cp) + 1)
			{
				fprintf(ls_log_ascfile, "%s", cp);
				ls_log_prefix(LL_LOGTO_ASCII, 0);
			}
#endif
		}
	}

	vfprintf(ls_log_ascfile, fmt, pvar);
	va_end(pvar);
#ifndef PC
	if (ls_fflush_ok)
#endif
                fflush(ls_log_ascfile);

}

extern int ls_show_vendor_def;

/*- Generate log message when a feature usage changes */
void
ls_log_usage(
int where,	/*- which log file(s) to log to (LL_LOGTO_xxx) */
long handle,	/*- a (semi)unique handle for this client */
long brhandle,	/*- handle of one of the brothers of this client */
int what,	/*- what kind of transaction: (LL_USAGE_xxx: IN, OUT,
		 *  DENIED, QUEUED, DEQUEUED, UNSUPPORTED, INUSE, INQUEUE,
		    v1.2: USED, METERFAIL) */
int why,	/*- why it happened: (LL_REASON_xxx: INACTIVE, CLIENT_CRASH,
		 *  CLIENT_REQUEST, SHUTDOWN, USER_REMOVED, SERVER_CRASH,
		    v1.2: FIRST_DEC, PERIODIC_DEC, NO_COUNT */
char *user,
char *node,
char *display,
char *vendor_def,
char *feature,
char *lickey,	/*- feature 20-char key */
LS_POOLED *pooled, /*- pooled data */
char * version,
int nlicreq,		/*- requested number of licenses in this transaction */
int nlicact,		/*- actual number of licenses this change affects */
int reftime,		/*- reference time (e.g. checkout time on checkin) */
CLIENT_DATA *who,
int group,
unsigned int userflags, 	/*- userflags -- includes borrow flag */
int linger_seconds)
{
  char *whatstr, *whystr;
  long ltime, dtime;
  char *transtr;
  int printwhy;
  char *feat;
  CONFIG *conf;
  int transport  = who ? who->addr.transport : LM_TCP;
/*
 *	Set up variables
 */
	if ((conf = ls_lickey_to_conf(feature, lickey))  && conf->feature
		 && *conf->feature)
		 feat= conf->feature;
	else
		feat= feature;
	ls_log_prefix(where & ~LL_LOGTO_REPORT, LL_USAGE);	/*- V3.1 */
	ltime = time(0);
#ifdef CLASSIC_DEBUG_LOG
	dtime = ltime-reftime;	/*- get delta time */
#endif /*CLASSIC_DEBUG_LOG */
/*
 *	End of setup
 */


	switch (what)
	{
		case LL_USAGE_OUT:	whatstr=lmtext("OUT"); break;
		case LL_USAGE_METERON:	whatstr=lmtext("METERING_STARTED");
				break;
		case LL_USAGE_METEROFF:	whatstr=lmtext("METERING_STOPPED");
				break;
		case LL_USAGE_USED:	whatstr=lmtext("DECREMENT"); break;
		case LL_USAGE_METERFAIL: whatstr=lmtext("METERING_FAILURE");
				break;
		case LL_USAGE_IN:	whatstr=lmtext("IN"); break;
		case LL_USAGE_QUEUED:	whatstr=lmtext("QUEUED"); break;
		case LL_USAGE_DEQUEUED: whatstr=lmtext("DEQUEUED"); break;
		case LL_USAGE_DENIED:	whatstr=lmtext("DENIED"); break;
		case LL_USAGE_UNSUPPORTED: whatstr=lmtext("UNSUPPORTED"); break;
		case LL_USAGE_UPGRADE:	whatstr=lmtext("UPGRADE"); break;
		case LL_USAGE_INUSE:	whatstr=lmtext("INUSE"); break;
		case LL_USAGE_INQUEUE:	whatstr=lmtext("INQUEUE"); break;
		case LL_USAGE_BORROW:	whatstr=lmtext("BORROW"); break;
		case LL_USAGE_UNBORROW:	whatstr=lmtext("UNBORROW"); break;
		case LL_USAGE_BORROWFAIL:whatstr=lmtext("BORROWFAIL"); break;
		case LL_USAGE_BORROW_INCR:whatstr=lmtext("BORROW_INCREMENT"); break;
		case LL_ULTIMATE_DENIAL: whatstr=lmtext("ULTIMATE_DENIAL"); break;
		default: whatstr = lmtext("UNKNOWN");
	}
	printwhy = 1;
	switch (why)
	{
		case LL_REASON_INACTIVE: whystr=lmtext("INACTIVE"); break;
		case LL_REASON_BORROW: whystr=lmtext("BORROW"); break;
		case LL_REASON_CLIENT_CRASH: printwhy=0; break;
		case LL_REASON_CLIENT_REQUEST: printwhy=0; break;
		case LL_REASON_SHUTDOWN: whystr=lmtext("SHUTDOWN"); break;
		case LL_REASON_USER_REMOVED:
			whystr=lmtext("USER_REMOVED"); break;
		case LL_REASON_SERVER_CRASH:
			whystr=lmtext("SERVER_CRASH"); break;
		case LL_REASON_REQUEST_SERVICED:
			whystr=lmtext("REQUEST_SERVICED"); break;
		case LL_REASON_INITIAL_DEC: printwhy=0; break;
		case LL_REASON_PERIODIC_DEC: printwhy=0; break;
		case LL_REASON_NO_COUNT: whystr=lmtext("NO_COUNT_LEFT"); break;
		default:
			if (why < 0)
			{
			  int sav = lm_job->lm_errno;
				lm_job->lm_errno = why;
				whystr=lc_errstring(lm_job);
				lm_job->lm_errno = sav;
			}
			else
				whystr=lmtext("UNKNOWN"); break;
	}
	if (!user ||!*user) user=lmtext("NoUser");
	if (!node || !*node) node=lmtext("NoNode");
	if (!display || !*display) display=lmtext("NoDisplay");
	if (!vendor_def) vendor_def = "";
	transtr = (transport == LM_UDP ? "(UDP)" :
#ifdef FIFO
		transport == LM_LOCAL ? "(LOCAL)" :
#endif
						"");
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII))
	{
		fprintf(ls_log_ascfile, "%s", whatstr);
		if (what == LL_USAGE_USED)
		{
#ifdef CLASSIC_DEBUG_LOG
		    fprintf(ls_log_ascfile," of %d on counter #%d (%s:%s:%s)",
				nlicreq, nlicact,
				user,
				node,
				display);
#else
		    fprintf(ls_log_ascfile," %d on #%d (%s@%s)",
						nlicreq, nlicact,
						user,
						node);
#endif
		}
		else
		{
		    fprintf(ls_log_ascfile,": \"%s\"", feat);
		    if (what == LL_USAGE_UNSUPPORTED)
			    fprintf(ls_log_ascfile," (%s)", lickey);
		    fprintf(ls_log_ascfile, " %s@%s %s ",
				user,
				node,
				transtr);
		    if (ls_show_vendor_def && *vendor_def)
			fprintf(ls_log_ascfile, "[%s] ", vendor_def);
		    if (nlicreq>1)
		    {
			if (what==LL_USAGE_UPGRADE)
				fprintf(ls_log_ascfile,"(%d->%d licenses) ",
					nlicreq-nlicact,nlicreq);
			else if (what==LL_USAGE_BORROW_INCR)
				fprintf(ls_log_ascfile,"(+%d licenses) ",nlicreq);
			else
				fprintf(ls_log_ascfile,"(%d licenses) ",nlicreq);
		    }
		}
#ifdef CLASSIC_DEBUG_LOG
		if (what==LL_USAGE_IN)
		{
			fprintf(ls_log_ascfile, lmtext("(used: %s) "),
					ls_log_ascdeltatime(dtime));
		}
#endif
		if (printwhy)
		{
			fprintf(ls_log_ascfile,"(%s)", whystr);
		}
		ls_log_endline(LL_LOGTO_ASCII);
	}
	if ((ls_log_repfile || ls_log_repfile_client) &&
						(where&LL_LOGTO_REPORT))
		ls_replog_usage(handle,brhandle,what,why,user,node,
			display, vendor_def, feature, lickey, pooled,
			version,nlicreq, nlicact, reftime, who, group,
			userflags, linger_seconds);
}

/*- Generate log message about use of reserved licenses */
void
ls_log_res_usage(where,handle,what,licgroup,restype,nlic)
int where;
long handle;
int what;		/*- LL_USAGE_xxx: IN, OUT, INUSE */
char *licgroup;		/*- name of reserved license group, or null */
int restype;		/*- type of reservation: HOSTRES, USERRES, GROUPRES */
int nlic;		/*- number of licenses just checked out of that group */
{
	char *wlstr;
	char *rstr;

	ls_log_prefix(where & ~ LL_LOGTO_REPORT, LL_RES_USAGE);
	switch (what)
	{
		case LL_USAGE_IN: wlstr = lmtext("license checked in"); break;
		case LL_USAGE_OUT: wlstr = lmtext("license checked out"); break;
		case LL_USAGE_INUSE: wlstr = lmtext("current license usage");
			break;
		default: wlstr = lmtext("unknown usage class"); break;
	}
	switch (restype)
	{
	case HOSTRES: rstr = lmtext("host"); break;
	case USERRES: rstr = lmtext("user"); break;
	case GROUPRES: rstr = lmtext("group"); break;
	default: rstr = lmtext("(unknown)"); break;
	}
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII))
	{
		if (licgroup)
		{
			fprintf(ls_log_ascfile,
				lmtext("Reserved license %s %s - %s, %d license%s"),
				rstr, licgroup, wlstr, nlic, nlic==1?"":"s");
		}
		else
		{
			fprintf(ls_log_ascfile,
				lmtext("Unreserved license - %s, %d license%s"),
				wlstr, nlic, nlic==1?"":"s");
		}
		ls_log_endline(LL_LOGTO_ASCII);
	}
	if ((ls_log_repfile || ls_log_repfile_client) &&
						(where&LL_LOGTO_REPORT))
	{
		ls_replog_res_usage(licgroup, handle, what, restype, nlic);
	}
}

/*- Write out a random comment */
#ifdef lint
/* VARARGS2 */
/* ARGSUSED */
void
ls_log_comment(where,fmt)
int where;
char *fmt;
#else /* lint */
void
#ifdef ANSI
ls_log_comment(int where, char *fmt, ...)
#else  /* ANSI */
ls_log_comment(where, fmt, va_alist)
int where;
char *fmt;
va_dcl
#endif /* ANSI */
#endif /* lint */
{
	va_list pvar;		/*- args to format string */
	char buf[500];

#ifdef lint
	pvar = 0;
#else
#ifdef ANSI
	va_start(pvar, fmt);
#else
	va_start(pvar);
#endif
#endif
	vsprintf(buf,fmt,pvar);
		/*- TBD - check for buffer overflow? */
	ls_log_prefix(where, LL_COMMENT);
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII))
	{
		fputs(buf,ls_log_ascfile);
	}
	if ((ls_log_repfile || ls_log_repfile_client) &&
					(where&LL_LOGTO_REPORT))
	{
	  extern ls_replog_comment lm_args((char *));
		ls_replog_comment(buf);
	}
	ls_log_endline(where & ~LL_LOGTO_REPORT);
	va_end(pvar);
}

/*- Log an error to the log file(s) */

#ifdef lint
/* VARARGS4 */
/* ARGSUSED */
void
ls_log_error(fmt)
char *fmt;
#else /* lint */
void
#ifdef ANSI
ls_log_error(char *fmt, ...)
#else
ls_log_error(fmt, va_alist)
char *fmt;
va_dcl
#endif /* ANSI */
#endif /* lint */
{
	va_list pvar;	/*-  followed by the varargs list for the printf */
	char buf[MAX_CONFIG_LINE];

#ifdef lint
	pvar = 0;
#else
#ifdef ANSI
	va_start(pvar, fmt);
#else
	va_start(pvar);
#endif
#endif
	ls_log_prefix(LL_LOGTO_ASCII, 0);
	vsprintf(buf,fmt,pvar);
		/*- TBD - check for buffer overflow? */
	if (ls_log_ascfile)
	{
		fputs(buf, ls_log_ascfile);
		fputs("\n", ls_log_ascfile);
	}
	if (ls_log_repfile || ls_log_repfile_client)
	{
		ls_replog_error(buf);
	}
	va_end(pvar);
}

void
ls_log_cpu_usage(client)
CLIENT_DATA *client;
{
	ls_replog_cpu_usage(client);
}

void
ls_log_msg_cnt()
{
  extern int ls_max_msgs_per_minute;
  extern int ls_msg_cnt;
  extern int ls_msg_cnt_this_minute;
  static time_t sav_time;
  extern int ls_currtime;
  extern LM_QUORUM quorum;	/* The LM_QUORUM in this server */
#ifdef RELEASE_VERSION
#define SIX_HOURS (60 * 60 * 6)
#define MAX_PER_MINUTE  (20 * 60)
#define PER_SECOND_AVG  10
#else
#define MAX_PER_MINUTE  1
#define PER_SECOND_AVG  1
#define SIX_HOURS (60 * 3) /* actually 3 minutes */
#endif

	if (ls_msg_cnt_this_minute > ls_max_msgs_per_minute)
		ls_max_msgs_per_minute = ls_msg_cnt_this_minute;
	ls_msg_cnt_this_minute = 0;
	if (!sav_time)
	{
		sav_time = ls_currtime;
		return;
	}
	if (ls_currtime - sav_time > SIX_HOURS)
	{
		sav_time = ls_currtime;
/*
 *		If we ever get more than 50 msgs per second , or
 *		if the last 6 hours we got more than average 10
 *		msgs per second, log it
 */
		if ((ls_max_msgs_per_minute > MAX_PER_MINUTE ) ||
			(ls_msg_cnt > (PER_SECOND_AVG * SIX_HOURS)))
		{
			ls_log_prefix(LL_LOGTO_ASCII, 0);
			fprintf(ls_log_ascfile,
				"The following message is informational,\n\tand does not indicate a problem.\n");
			fprintf(ls_log_ascfile,
				"\tIt appears if message volume is not light.\n");
			fprintf(ls_log_ascfile,
				lmtext("\tMessage volume in last 6 hours: \n"));
			ls_log_prefix(LL_LOGTO_ASCII, 0);
			fprintf(ls_log_ascfile, "\t%d", ls_max_msgs_per_minute);
			fprintf(ls_log_ascfile, " max msgs per minute, ");
			fprintf(ls_log_ascfile, "%d", ls_msg_cnt);
			fprintf(ls_log_ascfile,
				lmtext(" total msgs\n"));
			ls_replog_msg_vol(SIX_HOURS);
		}
		ls_msg_cnt = 0;
		ls_max_msgs_per_minute = 0;

		if (ls_i_am_lmgrd &&
			(quorum.quorum > 1) &&
			(quorum.quorum == quorum.count))
		{
			ls_log_prefix(LL_LOGTO_ASCII, 0);
			fprintf(ls_log_ascfile,
			lmtext("One of the redundant servers is down\n"));
		}
	}
}
