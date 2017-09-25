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

 *****************************************************************************/
/*	
 *	Module:	lsv4_log.c v1.11.0.0
 *
 *	v4 report.log compatibility package
 *	Functions:
 *		ls_log_asc_printf - gets called by the LOG macro
 *		ls_log_timestamp - add a timestamp
 *		ls_log_feature - static feature info
 *		ls_log_usage - changes in license usage
 *		ls_log_res_usage - changes in reservation usage
 *		ls_log_comment - log a random comment
 *		ls_log_error - log an error
 *
 *	Description:	These functions deal with outputting information to
 *			the ascii log file (the old format), and the report
 *			log file (new in version 2).
 *
 *	D. Birns
 *	4/5/96
 *
 *
 *	Last changed:  10/30/97
 */

#include "lmachdep.h"
#if defined( PC) || defined(OSF) 
#define va_dcl va_list va_alist;
#if defined(NLM) || defined(OS2)
#include <stdarg.h>                     
#endif /* define(NLM) || defined(OS2) */                        
#else /* PC */
#include <varargs.h>	  
#endif /* PC */
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lsv4server.h"
#include "../app/lsfeatur.h"
#include "lsv4_sprot.h"
#include "ls_log.h"
#if defined( sco) || defined(sinix)
#include <time.h>
#endif
int ls_fflush_ok = 1;

extern time_t time();

FILE *ls_log_ascfile = (FILE *) NULL;	/* default is setup in ls_xxx_init.c */
FILE *ls_log_repfile;
CLIENT_DATA *ls_log_repfile_client;
static int start_time;
long ls_log_lastreptime= -1;		/* time of last report log message */
#define NUM (sizeof(time_t) * 32)
char ls_log_lastrepdname[LM_MSG_LEN+1] = { '\0' };
	/* daemon name in last report log message */
int ls_log_pass = 0;

void ls_replog_delete_client(client) CLIENT_DATA *client; {}

void
ls_log_quote_string(file,str)
FILE *file;
char *str;
{
	char *p;
	int c;

	(void) fputc('"',file);
	for (p=str; *p; p++) 
	{
		c = *p;
		if (isgraph(c)) 
		{
			if (c=='"' || c=='\\')
				(void) fputc('\\',file);
			(void) fputc(c,file);
		}
		else 
		{
			switch (c) 
			{
				case ' ':  (void) fputc(' ',file); break;
				case '\t': (void) fputs("\t",file); break;
				case '\n': (void) fputs("\\n",file); break;
				default: (void) fprintf(file,"\\%03o",c); break;
			}
		}
	}
	(void) fputc('"',file);
}

int ls_log_time_in_msg = 1;	


static char *
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
#ifdef VMS
		(void) sprintf(buf,"%02d/%02d %2d:%02d:%02d",
			t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min,
			t->tm_sec);
#else
		(void) sprintf(buf,"%2d:%02d:%02d", 
			t->tm_hour, t->tm_min, t->tm_sec);
#endif /* VMS */
	}
	else
	{
		buf[0] = '\0';
	}
	return(buf);
}


static char *
ls_log_ascdeltatime(dtime)
long dtime;
{
  long h,m,s;	
  static char buf[50];

	s = dtime%60;	
	dtime /= 60;
	m = dtime%60;	
	dtime /= 60;
	h = dtime%24;	
	dtime /= 24;	
	if (dtime)
		(void) sprintf(buf,"%d:%02d:%02d:%02d", dtime, h, m, s);
	else if (h)
		(void) sprintf(buf,"%02d:%02d:%02d", h, m, s);
	else
		(void) sprintf(buf,"%d:%02d", m, s);

	return(buf);
}


char *
ls_log_daemon_name()
{
  static char buf[MAX_DAEMON_NAME+MAX_LONG_LEN+3];
  char *p;

	if (!lm_job->vendor[0])
		return("lmgrd");
	p = buf;
	(void) strcpy(p, lm_job->vendor);
	p += strlen(p);
#ifdef MULTIPLE_VD_SPAWN
	if (snum) 
	{
		(void) sprintf(p,"_%d",(int)snum);
		p += strlen(p);
	}
#endif	/* MULTIPLE_VD_SPAWN */
	*p = 0;
	return(buf);
}
void
ls_log_prefix(where,keyword)
int where;
char *keyword; /* ignored */
{
	long ltime;
	char *dname;

	ltime = (long)time(0);

	if (ls_log_ascfile && (where&LL_LOGTO_ASCII))
	{
		(void) fprintf(ls_log_ascfile, "%s ", ls_log_asctime(ltime));
		fprintf(ls_log_ascfile, "(%s) ", ls_log_daemon_name());
	}

}

void
lsv4_log_prefix(where,keyword)
int where;
char *keyword; 
{
	long ltime;
	char *dname;

	ltime = (long)time(0);

	if (ls_log_ascfile&& (where&LL_LOGTO_ASCII)) 
	{
		(void) fprintf(ls_log_ascfile, "%s ", ls_log_asctime(ltime));
		if (*keyword == 'D')
			fprintf(ls_log_ascfile, "<%s> ", ls_log_daemon_name());
		else
			fprintf(ls_log_ascfile, "(%s) ", ls_log_daemon_name());
	}


	if (ls_log_repfile && (where&LL_LOGTO_REPORT)) 
	{
		if (ls_log_pass > 0) { ls_log_pass--; ltime -= start_time; }
		else { ls_log_pass = NUM; start_time = ltime; }
		dname = ls_log_daemon_name();
		(void) strcpy(ls_log_lastrepdname,dname);
		ls_log_lastreptime=ltime;
		(void) fprintf(ls_log_repfile,"%s %d ",keyword,ltime);
		ls_log_quote_string(ls_log_repfile,dname);
		(void) fputc(' ',ls_log_repfile);
	}
}

void
ls_log_endline(where)
int where;
{
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII)) 
	{
		(void) fputc('\n',ls_log_ascfile);
		(void) fflush(ls_log_ascfile);
	}
	if (ls_log_repfile && (where&LL_LOGTO_REPORT)) 
	{
		(void) fputc('\n',ls_log_repfile);
		(void) fflush(ls_log_repfile);
	}
}


void
ls_log_timestamp(where)
int where;
{
	(void) lsv4_log_prefix(where,"lm_timestamp");
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII)) 
	{
		(void) fprintf(ls_log_ascfile, lmtext("Timestamp"));
	}
	ls_log_endline(where);
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


static
int
ls_log_genlinecrypt(curtime, feature)
long curtime;
char *feature;
{
  char *dname;
  int nd,nf;

	dname = ls_log_daemon_name();
	nd = ls_log_hash(dname);
	nf = ls_log_hash(feature);
	return((curtime ^ (nd+nf))&0xFFFF);
}


/* VARARGS1 */
void
#if defined(NLM) || defined(OS2) || defined(OSF) 
ls_log_asc_printf(char *fmt, ...)       
#else                                   
ls_log_asc_printf(fmt, va_alist)        /* same args as printf */
char *fmt;
va_dcl
#endif /* defined(NLM) || defined(OS2) */                        
{
	va_list pvar;

	if (!ls_log_ascfile) return;
#ifdef lint	
	pvar = 0;	
#else
#if defined( PC) || defined(OSF) 
	va_start(pvar, fmt);
#else
	va_start(pvar);
#endif
#endif
	(void) vfprintf(ls_log_ascfile,fmt,pvar);
	va_end(pvar);
	(void) fflush(ls_log_ascfile);
}


void
ls_log_feature(where,feature,nlic,overdraft,nres,version)
int where;	
char *feature;	
int nlic;	
int overdraft;	
int nres;	
char *version;

{
	(void) lsv4_log_prefix(where,"lm_feature");
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII)) 
	{
		(void) fprintf(ls_log_ascfile,
				lmtext(" Feature %s v%s"), feature, version);
		if (nlic>0) 
		{
			(void) fprintf(ls_log_ascfile,
					lmtext(" %d unreserved license%s"),
					nlic,(nlic==1)?"":"s");
		}
		else 
		{
			(void) fprintf(ls_log_ascfile,
				lmtext(" no unreserved licenses"));
		}
		if (nres>0) 
		{
			(void) fprintf(ls_log_ascfile,
					lmtext(" %d reserved license%s"),
					nres,(nres==1)?"":"s");
		}
		else 
		{
			(void) fprintf(ls_log_ascfile,
					lmtext(" no reserved licenses"));
		}
	}
	if (ls_log_repfile && (where&LL_LOGTO_REPORT)) 
	{
		ls_log_quote_string(ls_log_repfile,feature);
		(void) fprintf(ls_log_repfile," %s %d %d %d ",
				version, nlic-overdraft, nres, overdraft);
	}
	ls_log_endline(where);
}

extern int ls_show_vendor_def;


void
ls_log_usage(where,handle,brhandle,what,why,user,node,display, vendor_def,
	feature,lickey,pooled, version, nlicreq, nlicact, reftime, who, group)
int where;	
long handle;	
long brhandle;	
int what;	
int why;	
char *user;
char *node;
char *display;
char *vendor_def;
char *feature;
		/* inserted */
char * lickey; 
LS_POOLED *pooled;
		/* end insert */
char *version;
int nlicreq;		
int nlicact;		
int reftime;		
CLIENT_DATA *who;
int group;
#if 0 
int transport;		
short *inet_addr;	
long pid;		
char *platform;		
#endif
{
	char *whatstr, *swhatstr, *whystr, *swhystr;
	long ltime, dtime;
	char *transtr;
	char inet_str[20]; 
	CONFIG *conf;
	char *platform = who->platform;
	char *feat;
	int transport = who ? who->addr.transport : LM_TCP;

/*
 *	Set up variables
 */
	if ((conf = ls_lickey_to_conf(feature, lickey))  && conf->feature
                 && *conf->feature)
                 feat= conf->feature;
        else
                feat= feature;

	if (*who->inet_addr)
		l_addr_to_inet(who->inet_addr, 4, inet_str);
	else
		*inet_str = '\0';
	lsv4_log_prefix(where,"lm_usage4");		
	ltime = time(0);
	dtime = ltime-reftime;	
/*
 *	End of setup
 */


	switch (what) 
	{
		case LL_USAGE_OUT:	
				whatstr=lmtext("OUT"); 
				swhatstr="O"; 
				break;
		case LL_USAGE_METERON:	
				whatstr=lmtext("METERING_STARTED"); 
				swhatstr="MON"; 
				break;
		case LL_USAGE_METEROFF:	
				whatstr=lmtext("METERING_STOPPED"); 
				swhatstr="MOFF"; 
				break;
		case LL_USAGE_USED:	
				whatstr=lmtext("DECREMENT"); 
				swhatstr="D"; 
				break;
		case LL_USAGE_METERFAIL:	
				whatstr=lmtext("METERING_FAILURE"); 
				swhatstr="F"; 
				break;
		case LL_USAGE_IN:	
				whatstr=lmtext("IN"); 
				swhatstr="I"; 
				break;
		case LL_USAGE_QUEUED:	
				whatstr=lmtext("QUEUED"); 
				swhatstr="Q"; 
				break;
		case LL_USAGE_DEQUEUED: 
				whatstr=lmtext("DEQUEUED"); 
				swhatstr="DQ"; 
				break;
		case LL_USAGE_DENIED:	
				whatstr=lmtext("DENIED"); 
				swhatstr="DEN"; 
				break;
		case LL_USAGE_UNSUPPORTED: 
				whatstr=lmtext("UNSUPPORTED"); 
				swhatstr="UNS";
				break;
		case LL_USAGE_UPGRADE:	
				whatstr=lmtext("UPGRADE"); 
				swhatstr="UP"; 
				break;
		case LL_USAGE_INUSE:	
				whatstr=lmtext("INUSE"); 
				swhatstr="INUSE"; 
				break;
		case LL_USAGE_INQUEUE:	
				whatstr=lmtext("INQUEUE"); 
				swhatstr="INQ"; 
				break;
		default:
				whatstr=lmtext("UNKNOWN"); 
				swhatstr="X"; 
				break;
	}
	switch (why) 
	{
		case LL_REASON_INACTIVE:
			whystr=lmtext("INACTIVE"); swhystr="INACT"; break;
		case LL_REASON_CLIENT_CRASH:
			swhystr="CEXIT"; break;
		case LL_REASON_CLIENT_REQUEST:
			whystr=lmtext("CLIENT_REQUEST"); swhystr="CREQ"; break;
		case LL_REASON_SHUTDOWN:
			whystr=lmtext("SHUTDOWN"); swhystr="SHUT"; break;
		case LL_REASON_USER_REMOVED:
			whystr=lmtext("USER_REMOVED"); swhystr="UREM"; break;
		case LL_REASON_SERVER_CRASH:
			whystr=lmtext("SERVER_CRASH"); swhystr="SCRASH"; break;
		case LL_REASON_REQUEST_SERVICED:
			whystr=lmtext("REQUEST_SERVICED"); swhystr="REQSERV"; 
			break;
		case LL_REASON_INITIAL_DEC:
			whystr=lmtext("INITIAL"); swhystr="IDEC"; 
			break;
		case LL_REASON_PERIODIC_DEC:
			whystr=lmtext("PERIODIC"); swhystr="PDEC";
			break;
		case LL_REASON_NO_COUNT:
			whystr=lmtext("NO_COUNT_LEFT"); swhystr="NOCOUNT";
			break;
		default:
			whystr=lmtext("UNKNOWN"); swhystr="UNK"; break;
	}
	if (!user ||!*user) user=lmtext("NoUser");
	if (!node || !*node) node=lmtext("NoNode");
	if (!display || !*display) display=lmtext("NoDisplay");
	if (!platform || !*platform) platform=lmtext("NoPlatform");
	if (!vendor_def) vendor_def = "";
	if (!feature || !*feature) feature=lmtext("NoFeature");
	transtr = (transport == LM_UDP ? "(UDP)" : 
#ifdef FIFO
		transport == LM_LOCAL ? "(LOCAL)" : 
#endif
						"");
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII)) 
	{
		(void) fprintf(ls_log_ascfile, "%s", whatstr);
		if (why!=LL_REASON_CLIENT_REQUEST && why != LL_REASON_CLIENT_CRASH) 
		{
			(void) fprintf(ls_log_ascfile," (%s)", whystr);
		}
		if (what == LL_USAGE_USED)
		{
		    (void) fprintf(ls_log_ascfile," of %d on counter #%d (%s:%s:%s)",
				nlicreq, nlicact, user, node, display);
		}
		else
		{
		    (void) fprintf(ls_log_ascfile,": %s %s@%s %s ",
		       feature, user, node, transtr);
		    if (ls_show_vendor_def && *vendor_def)
			(void) fprintf(ls_log_ascfile, "[%s] ", vendor_def);
		    if (nlicreq>1) 
		    {
			if (what==LL_USAGE_UPGRADE)
				(void) fprintf(ls_log_ascfile,"(%d->%d licenses) ",
					nlicreq-nlicact,nlicreq);
			else
				(void) fprintf(ls_log_ascfile,"(%d licenses) ",nlicreq);
		    }
		}
#if 0
		if (what==LL_USAGE_IN) 
		{
			(void) fprintf(ls_log_ascfile, lmtext("(used: %s) "),
					ls_log_ascdeltatime(dtime));
		}
#endif
	}
	if (ls_log_repfile && (where&LL_LOGTO_REPORT)) 
	{
		(void) fprintf(ls_log_repfile,"0X%x 0X%x %s %s ",
					handle, brhandle, swhatstr, swhystr);
		ls_log_quote_string(ls_log_repfile,feature);
		(void) fprintf(ls_log_repfile," %s %d %d ",
					version, nlicreq, nlicact);
		ls_log_quote_string(ls_log_repfile, user);
		(void) fputc(' ',ls_log_repfile);
		ls_log_quote_string(ls_log_repfile, node);
		(void) fputc(' ',ls_log_repfile);
		ls_log_quote_string(ls_log_repfile, display);
		(void) fputc(' ',ls_log_repfile);		
		if (!ls_show_vendor_def) 
				vendor_def = lmtext("Hidden");	
		else if (*vendor_def == 0) 
				vendor_def = lmtext("NoVendor");
		ls_log_quote_string(ls_log_repfile,vendor_def);	
		(void) fprintf(ls_log_repfile," %d",dtime); 
		(void) fprintf(ls_log_repfile," %d ",
			ls_log_genlinecrypt(ltime,feature));
		ls_log_quote_string(ls_log_repfile, transtr);
		(void) fputc(' ',ls_log_repfile);		
		ls_log_quote_string(ls_log_repfile,inet_str);  
		(void) fprintf(ls_log_repfile," %d ",who->pid); 
		ls_log_quote_string(ls_log_repfile, platform);  
	}
	ls_log_endline(where);
}


void
ls_log_res_usage(where,handle,what,licgroup,restype,nlic)
int where;
long handle;
int what;		
char *licgroup;		
int restype;		
int nlic;		
{
	char *wlstr, *swhatstr;
	char *rstr, *srstr;

	(void) lsv4_log_prefix(where, "lm_res_usage");
	switch (what) 
	{
		case LL_USAGE_IN:
			swhatstr = "I";
			wlstr = lmtext("license checked in");
			break;
		case LL_USAGE_OUT:
			swhatstr = "O";
			wlstr = lmtext("license checked out");
			break;
		case LL_USAGE_INUSE:
			swhatstr = "U";
			wlstr = lmtext("current license usage");
			break;
		default:
			swhatstr="X";
			wlstr = lmtext("unknown usage class");
			break;
	}
	switch (restype) 
	{
	case HOSTRES:
		rstr = lmtext("host"); srstr="H"; break;
	case USERRES:
		rstr = lmtext("user"); srstr="U"; break;
	case GROUPRES:
		rstr = lmtext("group"); srstr="G"; break;
	default:
		rstr = lmtext("(unknown)"); srstr="X"; break;
	}
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII)) 
	{
		if (licgroup) 
		{
			(void) fprintf(ls_log_ascfile,
				lmtext("Reserved license %s %s - %s, %d license%s"),
				rstr, licgroup, wlstr, nlic, nlic==1?"":"s");
		}
		else 
		{
			(void) fprintf(ls_log_ascfile,
				lmtext("Unreserved license - %s, %d license%s"),
				wlstr, nlic, nlic==1?"":"s");
		}
	}
	if (ls_log_repfile && (where&LL_LOGTO_REPORT)) 
	{
		(void) fprintf(ls_log_repfile,"0X%x %s %s ",handle,swhatstr,srstr);
		if (!licgroup) licgroup="";
		ls_log_quote_string(ls_log_repfile,licgroup);
		(void) fprintf(ls_log_repfile," %d",nlic);
	}
	ls_log_endline(where);
}


#ifdef lint
/* VARARGS2 */
/* ARGSUSED */
void
ls_log_comment(where,fmt)
int where;
char *fmt;
#else /* lint */
void
#if defined(NLM) || defined(OS2) || defined(OSF) 
ls_log_comment(int where, char *fmt, ...)
#else  /* defined(NLM) || defined(OS2) */
ls_log_comment(where, fmt, va_alist)
int where;              
char *fmt;              
va_dcl
#endif /* NLM */                        
#endif /* lint */
{
	va_list pvar;		
	char buf[500];

#ifdef lint
	pvar = 0;
#else
#if defined( PC) || defined(OSF) 
	va_start(pvar, fmt);
#else
	va_start(pvar);
#endif
#endif
	(void) vsprintf(buf,fmt,pvar);
		
	(void) lsv4_log_prefix(where,"lm_comment");
	if (ls_log_ascfile && (where&LL_LOGTO_ASCII)) 
	{
		(void) fputs(buf,ls_log_ascfile);
	}
	if (ls_log_repfile && (where&LL_LOGTO_REPORT)) 
	{
		ls_log_quote_string(ls_log_repfile,buf);
	}
	ls_log_endline(where);
	va_end(pvar);
}



#ifdef lint
/* VARARGS4 */
/* ARGSUSED */
void
ls_log_error(fmt)
char *fmt;
#else /* lint */
void
#if defined(NLM) || defined(OS2) || defined(OSF) 
ls_log_error(char *fmt, ...)
#else                                                   
ls_log_error(fmt, va_alist)
char *fmt;      
va_dcl
#endif /* defined(NLM) || defined(OS2)  */
#endif /* lint */
{
	va_list pvar;	
	char buf[500];
	char *cstr;
	int where = LL_LOGTO_ASCII;

#ifdef lint
	pvar = 0;
#else
#if defined( PC) || defined(OSF) 
	va_start(pvar, fmt);
#else
	va_start(pvar);
#endif
#endif
	(void) vsprintf(buf,fmt,pvar);
		
	(void) lsv4_log_prefix(where,"lm_error");
	if (ls_log_ascfile)
	{
		(void) fputs(buf,ls_log_ascfile);
	}
	if (ls_log_repfile || ls_log_repfile_client)
	{
		ls_log_quote_string(ls_log_repfile,buf);
	}
	ls_log_endline(where);
	va_end(pvar);
}
CONFIG *
ls_lickey_to_conf(feature, lickey)
char *feature;
char *lickey;
{
  CONFIG *conf;
        for (conf = lm_job->line; conf; conf = conf->next)
        {
                if (!strcmp(conf->code, lickey) &&
                        !strcmp(conf->feature, feature))
                {
                        break;
                }
        }
        return conf;
}


void 
ls_feat_info() 
{
  extern FEATURE_LIST *ls_flist;
  FEATURE_LIST *f;
	for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
	{
		ls_log_feature(LL_LOGTO_REPORT, f->feature, f->nlic,
				f->overdraft, f->res, f->version);
	}
										}
void		
ls_log_cpu_usage (client)
CLIENT_DATA * client;
{
}

void
ls_log_msg_cnt()
{
  extern int ls_max_msgs_per_minute;
  extern int ls_msg_cnt;
  extern int ls_msg_cnt_this_minute;
  static time_t sav_time;
  extern long ls_currtime;
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
				lmtext("Message volume in last 6 hours: \n"));
			ls_log_prefix(LL_LOGTO_ASCII, 0);
			fprintf(ls_log_ascfile, "\t%d", ls_max_msgs_per_minute);
			fprintf(ls_log_ascfile, " max msgs per minute, ", ls_max_msgs_per_minute);
			fprintf(ls_log_ascfile, "%d", ls_msg_cnt);
			fprintf(ls_log_ascfile, 
				lmtext(" total msgs\n"));
		}
		ls_msg_cnt = 0;
		ls_max_msgs_per_minute = 0;
	}
}
