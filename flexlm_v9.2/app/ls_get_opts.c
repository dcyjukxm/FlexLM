
/******************************************************************************

            COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_get_opts.c,v 1.47.2.4 2003/07/01 17:04:21 sluu Exp $
 *
 *      Function: ls_get_opts(options)
 *
 *      Description: Reads the options file and links the options to featlist.
 *
 *      Parameters:     (char *) options - the options file.
 *
 *      Return:         featlist->opt pointers filled in.
 *
 *      M. Christiano
 *      6/12/88
 *
 *	Last changed:  12/31/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lsfeatur.h"
#include "ls_aprot.h"
#include "ls_log.h"
#include "ls_aprot.h"
#include "ls_sprot.h"
#include <stdio.h>
#include "flex_file.h"
#include "flex_utils.h"
#include "flexevent.h"
#ifndef NLM
#include <sys/file.h>
#endif

extern int ls_log_in, ls_log_out, ls_log_denied, ls_log_queued;
extern FILE *ls_log_repfile;
extern GROUP *groups;           /* Server's group definitions */
extern GROUP *hostgroups;
extern FEATURE_LIST *ls_flist;
extern char ls_our_hostname[];
int ls_got_opt_max;
long ls_reread_time;

/* Fix for P7208 */
static int ls_group_case_insensitive;

#define strsave(x) strcpy(strptr, x); strptr += strlen(x) + 1
FEATURE_LIST * findfeat lm_args((char *, char *, FEATURE_LIST *last));

static void log_option lm_args(( char *, char *, OPTIONS *, char *));
static void set_lowwater();
static void nolog();
static void groupcaseinsensitve();
static void ls_process_group    lm_args((int, char *, char *, char *, char *,
                                        char *, char *));
static void process_opts        lm_args((int, char *, char *, char *, char *,
                                        char *, char *));
static int scan_opt_line        lm_args((char *, char *, char *, char *,
                                                char *, char *, char *));
static void parse_featspec      lm_args((char *, char **feature, char **));
static void skip_word           lm_args((char **));
#define SKIP_SPACES 0
#define SKIP_WORD 1
static char * getword           lm_args((char **, char *));
static void proc_log(char *, char *, char *, int);

static void build_each_opt      lm_args(( int , char *, char *, char *,
                                        FEATURE_LIST *, char *, int ));


extern OPTIONS *includeall, *excludeall;
extern OPTIONS *b_includeall, *b_excludeall;
extern char *ls_serv_down_notify;       /* Whom to notify on server down */
extern char *ls_new_master_notify;      /* Whom to notify on new master */
extern int ls_minimum_user_timeout;     /* From ls_vendor.c: minimum
                                           user-selected feature timeout value
                                           (seconds) */
extern int ls_cpu_usage_interval;
extern int ls_cpu_usage_delta;
int reread;
char *sav_options;
char *sav_master_name;

static
void
ls_build_opt(int			type,		/* What it is: RESERVE, INCLUDE, EXCLUDE */
			 char *			num,		/* Number of option */
			 char *			featspec,	/* Feature for option, and optional spec */
			 char *			kind,		/* Kind of option: USER/HOST/DISPLAY/GROUP */
			 char *			name,		/* Name of "kind" */
			 FEATURE_LIST *	featlist,	/* Daemon's feature list */
			 int			borrow);	/* This is a BORROW include/exclude option */

void
ls_get_opts(char *	options,
			char *	master_name,
			char **	reread_replog_name,
			char **	reread_deblog_name)
{
	FILE *option_file;
#ifndef OPENVMS
	FILE *fopen();
#endif
	char line[MAX_CONFIG_LINE+1];
	char f1[MAX_CONFIG_LINE+1], f2[LM_MAXPATHLEN+1], f3[MAX_CONFIG_LINE+1];
	char f4[MAX_CONFIG_LINE+1], f5[MAX_CONFIG_LINE+1], f6[MAX_CONFIG_LINE+1];
	int nflds;
	OPTIONS *o;
	FEATURE_LIST *f;
	char rlog[MAX_LONGNAME_SIZE+1] = {'\0'};	/* LONGNAMES */
	char dlog[MAX_LONGNAME_SIZE+1] = {'\0'};	/* LONGNAMES */
	LICENSE_FILE lf;
	char *	ppszInStr[20] = {NULL};

	reread = 0;
	ls_group_case_insensitive = 0;
	if (options && !sav_options)
	{
		/*	P6394	*/
		if(master_name && master_name[0] != '\0')
		{
			sav_master_name = LS_MALLOC(strlen(master_name) + 1);
			strcpy(sav_master_name, master_name);
		}
		if(options && options[0] != '\0')
		{
			sav_options = LS_MALLOC(strlen(options) + 1);
			strcpy(sav_options, options);
		}
		else
		{
			return;
		}
	}
	else if (!options)
	{
		reread = 1;
		if(!sav_options)
		{
			char ls_options[LM_MAXPATHLEN+1];	/* LONGNAMES */
			find_default_options_file(ls_options);
			options = ls_options;
			master_name = sav_master_name;
		}
		else
		{
			options = sav_options;
			master_name = sav_master_name;
		}
		if (!options)
			return;
	}


    memset(&lf, 0, sizeof(lf));
    *rlog = dlog[0] = '\0';

	/*
	 *	make sure we have valid options file name
	 */
	if(!options || (options[0] == '\0'))
		return;
	if (!(option_file = l_flexFopen(lm_job, options, "r")))
    {
		LOG((lmtext("CANNOT OPEN options file \"%s\"\n"), options));
		if(l_flexEventLogIsEnabled())
		{
			ppszInStr[0] = options;

			l_flexEventLogWrite(lm_job,
								FLEXEVENT_WARN,
								CAT_FLEXLM_LMGRD,
								MSG_FLEXLM_VENDOR_OPTIONS_FILE_OPEN_ERROR,
								1,
								ppszInStr,
								0,
								NULL);
		}
    }
    else
    {

        lf.type = LF_FILE_PTR;
        lf.ptr.f = option_file;
        while (l_lfgets(lm_job, line, MAX_CONFIG_LINE, &lf, 0))
        {

			nflds = scan_opt_line(line, f1, f2, f3, f4, f5, f6);
/*
 *                      First look for options that don't require
 *                      END_USER_OPTIONS attribute
 */

            if (nflds >= 2 && (l_keyword_eq(lm_job,f1, "REPORTLOG")))
				proc_log(f2, rlog, line, 0);
            else if (nflds >= 2 && (l_keyword_eq(lm_job,f1, "DEBUGLOG")))
				proc_log(f2, dlog, line, 1);
            else
            {
                if (l_getattr(lm_job, END_USER_OPTIONS) ==
                                        END_USER_OPTIONS_VAL)
                {
					process_opts(nflds, f1, f2, f3, f4, f5, f6);
                }
                else
                {
					/*
					 *	FLEXlm LITE
					 */
					LOG(("User Option %s not supported by this server\n", f1));
                }
            }
        }
        fclose(option_file);
	}
/*
 *      Now, trim the options structures based on what is there.
 */

    for (f = ls_flist; f && f->feature && *(f->feature); f = f->next)
    {

		int cnt = 0;
		int last_type = 0;
		char *last_u = 0;

		if (f->flags & (LM_FL_FLAG_LS_HOST_REMOVED | LM_FL_FLAG_REREAD_REMOVED))
			continue;
        for (o = f->opt; o; o = o->next)
        {
            if (o->type == RESERVE)
            {
				char *u, _s[MAX_CONFIG_LINE];


				if ((o->type2 == OPT_INTERNET) ||
					((o->type2 == OPT_HOST) && !o->name))
				{
					l_addr_to_inet(&o->inet_addr[0], o->addr_type, _s);
					u = _s;
				}
				else
					u = o->name;
                f->nlic--;
                f->res++;

                if (last_u && (!L_STREQ(last_u, u) || (o->type2 != last_type)))
                {
					if (L_STREQ(last_u, u) && (o->type2 == last_type))
						cnt++;
					LOG((lmtext("RESERVING %d %s/%s license%s for %s %s\n"),
					cnt,
					f->feature, f->code,
					cnt > 1 ? "s" : "",
					last_type == OPT_HOST ? "HOST" :
					last_type == OPT_USER ? "USER" :
					last_type == OPT_DISPLAY ? "DISPLAY" :
					last_type == OPT_INTERNET ? "INTERNET" :
					last_type == OPT_GROUP ? "USER_GROUP" :
					last_type == OPT_HOST_GROUP ? "HOST_GROUP" :
					last_type == OPT_PROJECT ? "PROJECT" :
											"???", last_u));
					cnt = 0;
                }
                cnt++;
                if (!o->next || f->nlic == 0)
                {
                    LOG((lmtext("RESERVING %d %s/%s license%s for %s %s\n"),
                    cnt,
                    f->feature, f->code,
                    cnt > 1 ? "s" : "",
                    o->type2 == OPT_HOST ? "HOST" :
                    o->type2 == OPT_USER ? "USER" :
                    o->type2 == OPT_PROJECT ? "PROJECT" :
                    o->type2 == OPT_DISPLAY ? "DISPLAY" :
                    o->type2 == OPT_INTERNET ? "INTERNET" :
                    o->type2 == OPT_GROUP ? "USER_GROUP" :
                    o->type2 == OPT_HOST_GROUP ? "HOST_GROUP" :
                                            "???", u));
                }
                last_type = o->type2;
                last_u = u;
                if (f->nlic == 0)
                {
					OPTIONS *del, *next;

					f->flags |= LM_FL_FLAG_ALL_RESERVED;

					for(del = o->next; del; del = next)
					{
						next = del->next;

						if ((del->type2 == OPT_INTERNET) ||
							((del->type2 == OPT_HOST) &&
							!del->name))
						{
							l_addr_to_inet(
									&del->inet_addr[0],
									del->addr_type, _s);
							u = _s;
						}
						else
							u = del->name;
						LOG((lmtext("RESERVATION of %s for %s %s exceeds license count - ignored.\n")
								, f->feature,
								del->type2 == OPT_HOST ? "HOST" :
								del->type2 == OPT_USER ? "USER" :
								del->type2 == OPT_PROJECT ? "PROJECT" :
								del->type2 == OPT_DISPLAY ? "DISPLAY" :
								del->type2 == OPT_INTERNET ? "INTERNET":
								del->type2 == OPT_GROUP ? "USER_GROUP" :
								del->type2 == OPT_HOST_GROUP ? "HOST_GROUP" :
								lmtext("???"), u));
						free_opt(del);
						del = next;
					}
					/*
					 *	Truncate list
					 */
					o->next = NULL;
                    break;
                }
            }
            else if ((o->type == OPT_MAX) /*&& !reread*/)
            {
                ls_got_opt_max = 1;
                log_option(lmtext("MAX"), lmtext("for"), o, f->feature);
            }
        }
        for (o = f->reread_include ? f->reread_include : f->include; o; o = o->next)
        {
            log_option(lmtext("INCLUDE"), "", o, f->feature);
        }
        for (o = includeall; o; o = o->next)
        {
            log_option(lmtext("INCLUDE"), lmtext("in"), o,
                                                    lmtext("ALL FEATURES"));
        }
        for (o = f->exclude; o; o = o->next)
        {
            log_option(lmtext("EXCLUDE"), lmtext("from"), o, f->feature);
        }
        for (o = excludeall; o; o = o->next)
        {
            log_option(lmtext("EXCLUDE"), lmtext("from"), o,
                                                    lmtext("ALL FEATURES"));
        }
        for (o = f->b_include; o; o = o->next)
        {
            log_option(lmtext("INCLUDE_BORROW"), lmtext("in"), o,
                                                            f->feature);
        }
        for (o = b_includeall; o; o = o->next)
        {
            log_option(lmtext("INCLUDEALL_BORROW"), lmtext("in"), o,
                                                    lmtext("ALL FEATURES"));
        }
        for (o = f->b_exclude; o; o = o->next)
        {
            log_option(lmtext("EXCLUDE_BORROW"), lmtext("from"), o,
                                                            f->feature);
        }
        for (o = b_excludeall; o; o = o->next)
        {
            log_option(lmtext("EXCLUDEALL_BORROW"), lmtext("from"), o,
                                                    lmtext("ALL FEATURES"));
        }
    }
/*
 *      Finally, open the report log, if specified
 */
    if ((*rlog || *dlog) && l_keyword_eq(lm_job, master_name, ls_our_hostname))
    {
		if (reread)
		{
			if (*rlog)
			{
				*reread_replog_name = LS_MALLOC(strlen(rlog) + 1);
				strcpy(*reread_replog_name, rlog);
			}
			if (*dlog)
			{
				*reread_deblog_name = LS_MALLOC(strlen(dlog) + 1);
				strcpy(*reread_deblog_name, dlog);
			}
		}
		else
		{
			if (*rlog )
			{
				if ( ls_log_open_report(rlog, 1, (CLIENT_DATA *)0))
				{
					LOG((lmtext("Report log started (%s).\n"), rlog));
					if(l_flexEventLogIsEnabled())
					{
						ppszInStr[0] = rlog;


						l_flexEventLogWrite(lm_job,
											FLEXEVENT_INFO,
											CAT_FLEXLM_LMGRD,
											MSG_FLEXLM_VENDOR_REPORT_LOG_STARTED,
											1,
											ppszInStr,
											0,
											NULL);
					}
				}
				else
				{
					LOG(("Report log didn't start (%s).\n", rlog));
					if(l_flexEventLogIsEnabled())
					{
						ppszInStr[0] = rlog;

						l_flexEventLogWrite(lm_job,
											FLEXEVENT_INFO,
											CAT_FLEXLM_LMGRD,
											MSG_FLEXLM_VENDOR_REPORT_LOG_FAILED,
											1,
											ppszInStr,
											0,
											NULL);
					}
				}
			}
			if (*dlog )
			{
				LOG((lmtext("Debug log started (%s).\n"), dlog));
				if(l_flexEventLogIsEnabled())
				{
					ppszInStr[0] = dlog;

					l_flexEventLogWrite(lm_job,
										FLEXEVENT_INFO,
										CAT_FLEXLM_LMGRD,
										MSG_FLEXLM_VENDOR_DEBUG_LOG_STARTED,
										1,
										ppszInStr,
										0,
										NULL);
				}
				ls_log_open_ascii(dlog);
				ls_print_feats();
			}
		}
	}
	if (!*rlog)
		ls_log_repfile = 0;
	ls_user_based(sav_options, sav_master_name);
}

/*
 *      process all options, except for REPORTLOG
 */
static
void
process_opts(int	nflds,
			 char *	f1,
			 char *	f2,
			 char *	f3,
			 char *	f4,
			 char *	f5,
			 char *	f6)
{
/*
 *	BORROW_LOWWATER option
 *		BORROW_LOWWATER feature number
 */
    if   (nflds >= 3 && l_keyword_eq(lm_job,f1, "BORROW_LOWWATER"))
    {
        LOG(("BORROW_LOWWATER   %s %s\n", f2, f3));
        set_lowwater(f2, f3);
    }
/*
 *	EXCLUDE option
 *		EXCLUDE feature U/H/D/G/I name
 */
    if   (nflds >= 4 && l_keyword_eq(lm_job,f1, "EXCLUDE"))
    {
		ls_build_opt(EXCLUDE, "1", f2, f3, f4, 0, 0);
    }
/*
 *	EXCLUDEALL option
 */
    else   if (nflds >= 3 && l_keyword_eq(lm_job,f1, "EXCLUDEALL"))
    {
		ls_build_opt(EXCLUDE, "1", "", f2, f3, 0, 0);
    }
/*
 *	EXCLUDE_BORROW option
 *		EXCLUDE_BORROW feature H name
 */
    if   (nflds >= 4 && l_keyword_eq(lm_job,f1, "EXCLUDE_BORROW"))
    {
		ls_build_opt(EXCLUDE, "1", f2, f3, f4, 0, 1);
    }
/*
 *  EXCLUDEALL_BORROW option
 *		EXCLUDEALL_BORROW H name
 */
    else if(nflds >= 3 && l_keyword_eq(lm_job,f1, "EXCLUDEALL_BORROW"))
    {
		ls_build_opt(EXCLUDE, "1", "", f2, f3, 0, 1);
    }
/*
 *  GROUP option
 *		GROUP name list-of-users
 *		HOST_GROUP name list-of-hosts
 *		(A second GROUP line REPLACES pre-v3.0)
 *		(A second GROUP line concatenates post-v3.0)
 */
    else if(nflds >= 3 && (l_keyword_eq(lm_job,f1, "GROUP") ||
            l_keyword_eq(lm_job,f1,   "HOST_GROUP") || !strcmp(f1,   "USER_GROUP")))
    {
		ls_process_group(nflds,   f1, f2, f3, f4, f5, f6);
    }
/*
 *	INCLUDE option
 *		INCLUDE feature U/H/D/G/I name
 */
    if(nflds >= 4 && l_keyword_eq(lm_job,f1, "INCLUDE"))
    {
		ls_build_opt(INCLUDE, "1", f2, f3, f4, 0, 0);
    }
/*
 *  INCLUDEALL option
 *		INCLUDEALL U/H/D/G/I name
 */
    if(nflds >= 3 && l_keyword_eq(lm_job,f1, "INCLUDEALL"))
    {
		ls_build_opt(INCLUDE, "1", "", f2, f3, 0, 0);
    }
/*
 *	INCLUDE_BORROW option
 *		INCLUDE_BORROW feature HOST name
 */
    if(nflds >= 4 && l_keyword_eq(lm_job,f1, "INCLUDE_BORROW"))
    {
		ls_build_opt(INCLUDE, "1", f2, f3, f4, 0, 1);
    }
/*
 *  INCLUDEALL_BORROW option
 *		INCLUDEALL_BORROW HOST name
 */
    if(nflds >= 3 && l_keyword_eq(lm_job,f1, "INCLUDEALL_BORROW"))
    {
		ls_build_opt(INCLUDE, "1", "", f2, f3, 0, 1);
    }

/*
 *	MAX_BORROW_HOURS option
 *		MAX_BORROW_HOURS feature number_of_hours
 */
	if(nflds >= 3 && l_keyword_eq(lm_job, f1, "MAX_BORROW_HOURS"))
	{
		int				iBorrowPeriod = atoi(f3);
		FEATURE_LIST *	f = NULL;
		char *			feature = NULL;
		char *			spec = NULL;

		if(iBorrowPeriod > 0)
		{
			parse_featspec(f2, &feature, &spec);
			if(f = findfeat(f2, spec, f))
			{
				/*
				 *	Check to see if new borrow period less that
				 *	that specified in license, if yes, set FEATURE_LIST.maxborrow
				 *	to new max borrow period.
				 */
				if(f->conf->lc_type_mask & LM_TYPE_BORROW)
				{
					if(f->conf->lc_max_borrow_hours > iBorrowPeriod)
					{
						f->maxborrow = iBorrowPeriod;
						LOG(("Max borrow interval for feature, %s, set to %d hours\n",
							f->feature, f->maxborrow));
					}
				}
				else
				{
					LOG(("Feature %s does not support borrowing, MAX_BORROW_HOURS ignored\n", f2));
				}
			}
			else
			{
				LOG(("%s is not a valid feature name\n", f2));
			}
		}
		else
		{
			LOG(("Invalid MAX_BORROW_HOURS value (%s) specified for feature %s", f3, f2));
		}
	}
/*
 *  MAX option
 *		MAX count feature {USER|GROUP|USER_GROUP|HOST_GROUP} name
 */
    if(nflds >= 5 && l_keyword_eq(lm_job,f1, "MAX"))
    {
		char num[MAX_LONG_LEN + 2];

		sprintf(num, "c%s", f2);
		ls_build_opt(OPT_MAX, num, f3, f4, f5, 0, 0);
    }
/*
 *  NOLOG option
 *		NOLOG IN/OUT/DENIED/QUEUED
 */
    else if(nflds >= 2 && l_keyword_eq(lm_job,f1, "NOLOG"))
    {
		nolog(f2);
    }

/* Fix for P7208 */
/*
 *  GROUPCASEINSENSITIVE option
 *		GROUPCASEINSENSITIVE ON/OFF
 */
    else if(nflds >= 2 && l_keyword_eq(lm_job,f1, "GROUPCASEINSENSITIVE"))
    {
		groupcaseinsensitve(f2);
    }

/*
 *  RESERVE
 *		RESERVE num feature U/H/D/G/I name
 */
	else if(nflds >= 5 && l_keyword_eq(lm_job,f1, "RESERVE"))
	{
		int n;
		n   = atoi(f2);
		if(n <= 0)
		{
			LOG(("\n"));
			LOG((lmtext("ERROR - RESERVE of %s \"%s\" licenses specified.\n"),
									f2,   f3));
			LOG((lmtext("RESERVE options must specify # of licenses > 0\n")));
			LOG((lmtext("No reservation performed\n")));
			LOG(("\n"));
			LOG_INFO((INFORM, "The RESERVE option did \
					not   specify a positive number \
					of   licenses."));
		}
		else
		{
			ls_build_opt(RESERVE, f2, f3, f4, f5, 0, 0);
		}
	}
/*
 *  TIMEOUT - Feature timeout
 *		TIMEOUT feature timeout_in_secs
 */
    else if(nflds >= 2 && (l_keyword_eq(lm_job, f1, "TIMEOUT") ||
			l_keyword_eq(lm_job, f1, "TIMEOUTALL")) &&
			ls_minimum_user_timeout > 0)
    {
		int timeout;
		FEATURE_LIST *fl;
		int all = 0;
        if (l_keyword_eq(lm_job, f1, "TIMEOUTALL"))
        {
			all = 1;
			timeout = atoi(f2);
			LOG((lmtext(
			"ALL FEATURES: INACTIVITY TIMEOUT set to %d seconds\n"),
											timeout));
        }
        else if(nflds >= 3)
			timeout = atoi(f3);
        else
			return; /* error */

        if (timeout < ls_minimum_user_timeout)
			timeout = ls_minimum_user_timeout;

		for (fl=ls_flist; fl && fl->feature && *(fl->feature);
										fl = fl->next)
		{
			if (fl->flags & (LM_FL_FLAG_LS_HOST_REMOVED | LM_FL_FLAG_REREAD_REMOVED))
				continue;
			if (all || l_keyword_eq(lm_job, f2, fl->feature))
			{
				fl->timeout   = timeout;
				if (!all)
					LOG((lmtext("FEATURE   %s INACTIVITY TIMEOUT set to %d seconds\n"),
						f2,   timeout));
			}
		}
    }
/*
 *  LINGER - Feature lingering
 *		LINGER feature linger_in_minutes
 */
	else if(nflds >= 3 && l_keyword_eq(lm_job,f1, "LINGER"))
	{
		int linger = atoi(f3);
		FEATURE_LIST *fl;

		for   (fl = ls_flist; fl && fl->feature; fl=fl->next)
		{
			if (fl->flags & (LM_FL_FLAG_LS_HOST_REMOVED |
				LM_FL_FLAG_REREAD_REMOVED))
			{
				continue;
			}
			if (l_keyword_eq(lm_job,f2, fl->feature))
			{
				fl->linger = linger;
				LOG((lmtext("FEATURE   %s LICENSE LINGER set to %d seconds\n"),
							f2,   linger));
				LOG_INFO((INFORM,   "The specified \
				feature   is set to have \
				licenses   linger after the user \
				stops   using it."));
			}
		}
	}
#define   NOTIFY_SUPPORT
#ifdef   NOTIFY_SUPPORT
/*
 *  NOTIFY feature
 */
	else   if (nflds >= 3 && l_keyword_eq(lm_job,f1, "NOTIFY"))
	{
		int size = strlen(f3) + 1;
		char *p;

		if(nflds > 3)
			size += strlen(f4) + 1;
		p   = LS_MALLOC(size);
		if(l_keyword_eq(lm_job,f2, "server_down") || !strcmp(f2, "SERVER_DOWN"))
		{
			ls_serv_down_notify   = p;
		}
		else if(l_keyword_eq(lm_job,f2, "new_master") ||
				   l_keyword_eq(lm_job,f2, "NEW_MASTER"))
		{
			ls_new_master_notify   = p;
		}
		strcpy(p, f3);
		if(nflds > 3)
		{
			strcat(p, " ");
			strcat(p, f4);
		}
	}
/*
 *	TRANSPORT - TCP or UDP or IPX
 */
	else if(nflds == 2 && (l_keyword_eq(lm_job,f1, "TRANSPORT") ||
			l_keyword_eq(lm_job,f1,   "COMM_TRANSPORT")))
	{

		lm_job->options->transport_reset   =
						LM_RESET_BY_USER;
		if(!strncmp(f2, "TCP", 3) ||
			!strncmp(f2, "LM_TCP", 6) ||
			!strncmp(f2, "tcp", 3))
		{
			lm_job->options->commtype   = LM_TCP;
			LOG((lmtext("TRANSPORT:   TCP REQUESTED\n")));
		}
		else if(!strncmp(f2, "UDP", 3) ||
				   !strncmp(f2, "LM_UDP", 6) ||
				   !strncmp(f2, "udp", 3))
		{
			lm_job->options->commtype   = LM_UDP;
			LOG((lmtext("TRANSPORT:   UDP REQUESTED\n")));
		}
#ifdef SUPPORT_IPX
		else if(!strncmp(f2, "SPX", 3) ||
				   !strncmp(f2, "IPX", 3) ||
				   !strncmp(f2, "LM_IPX", 6) ||
				   !strncmp(f2, "LM_SPX", 6) ||
				   !strncmp(f2, "ipx", 3) ||
				   !strncmp(f2, "spx", 3))
		{
			lm_job->options->commtype   = LM_SPX;
			LOG((lmtext("TRANSPORT:   IPX REQUESTED\n")));
		}
#endif /* SUPPORT_IPX */
		else
		{
			lm_job->options->transport_reset   = 0;
			LOG((lmtext("Invalid   TRANSPORT REQUESTED: %s\n"),
																f2));
		}
	}
#endif
    else   if (nflds == 3 && l_keyword_eq(lm_job,f1, "MAX_OVERDRAFT"))
    {
		int overdraft = atoi(f3);
		FEATURE_LIST *f = 0;
		char *feat, *spec; /* feat is set but unused */

		parse_featspec(f2, &feat, &spec);
		if (f = findfeat(f2, spec, f))
		{
			if (f->overdraft > overdraft)
			{
				LOG((lmtext("FEATURE %s OVERDRAFT reduced from %d to %d\n"),
						f->feature, f->overdraft,  overdraft));
				f->nlic -= (f->overdraft - overdraft);
				f->overdraft = overdraft;
			}
			else
			{
				LOG(("FEATURE %s MAX_OVERDRAFT (%d) > than OVERDRAFT %d \n",
					f->feature, overdraft,  overdraft, f->overdraft));
			}
		}
    }
}

FEATURE_LIST *
findfeat(char *			feat,
		 char *			spec,
		 FEATURE_LIST *	last)
{
	FEATURE_LIST *f, *start = (last ? last->next : ls_flist);
	char buf[MAX_CONFIG_LINE];
	char *field, *value = 0, *cp;
	int quote = 0;
	LS_POOLED *pool;

    if (spec)
    {
		strncpy(buf, spec, MAX_CONFIG_LINE);
		field = buf;
		for (cp = field; *cp; cp++)
		{
			if (*cp == '=')
			{
				*cp = '\0'; /* null terminate field */
				value = cp+1;
				continue;
			}
			if (value == cp && *cp == '"') /* skip 1st quote */
			{
				quote = 1;
				value++;
				continue;
			}
			if (quote && *cp == '"')
			{
				*cp = '\0'; /* erase quote ending */
				break;
			}
		}
    }
    for (f = start; f && f->feature && *(f->feature); f = f->next)
    {
        if (l_keyword_eq(lm_job,f->feature, feat) &&
                (!last || strcmp(f->code, last->code)) )
        {
			HOSTID *hid;

			if (!spec)
				return(f);
			if (l_keyword_eq(lm_job,field, "VERSION") &&
                        !l_compare_version(lm_job, f->version, value))
			{
				return f;
			}
			if  (l_keyword_eq(lm_job,field, "HOSTID"))
			{
				int rc;

				ls_i_fhostid(f);
				if (value && f->id);
				l_get_id(lm_job, &hid, value);
				rc = l_hostid_cmp(lm_job, f->hostid, hid);
				free(hid);
				if (rc)
					return f; /* P5529, was backwards */
			}
            for (pool = f->pooled; pool; pool = pool->next)

            {
                if(
					(l_keyword_eq(lm_job,field, "EXPDATE") &&
					l_keyword_eq(lm_job,pool->date, value)) ||

					(l_keyword_eq(lm_job,field, "KEY") &&
					l_keyword_eq(lm_job,pool->code, value)) ||
					(l_keyword_eq(lm_job,field, "SIGN") &&
					pool->lc_sign &&
					l_keyword_eq(lm_job,pool->lc_sign, value)) ||

					(l_keyword_eq(lm_job,field, "VENDOR_STRING") &&
					!lc_xstrcmp(lm_job,
							pool->lc_vendor_def, value)) ||

					(l_keyword_eq(lm_job,field, "dist_info") &&
					!lc_xstrcmp(lm_job,
							pool->lc_dist_info, value)) ||

					(l_keyword_eq(lm_job,field, "user_info") &&
					!lc_xstrcmp(lm_job,
							pool->lc_user_info, value)) ||

					(l_keyword_eq(lm_job,field, "asset_info") &&
					!lc_xstrcmp(lm_job,
							pool->lc_asset_info, value)) ||

					(l_keyword_eq(lm_job,field, "ISSUER") &&
					!lc_xstrcmp(lm_job,
							pool->lc_issuer, value)) ||

					(l_keyword_eq(lm_job,field, "NOTICE") &&
					!lc_xstrcmp(lm_job,
							pool->lc_notice, value))
					)
                {
					return f;
                }
            }
        }
        else
        {
/*
 *		See if this is part of the package...
 */
			for (pool = f->pooled; pool; pool = pool->next)
			{
				if (pool->parent_featname &&
					l_keyword_eq(lm_job,pool->parent_featname, feat))
				{
					return f;
				}
			}
        }
    }
	return 0;
}

static FEATURE_LIST dummy;

static
void
ls_build_opt(int			type,		/* What it is: RESERVE, INCLUDE, EXCLUDE */
			 char *			num,		/* Number of option */
			 char *			featspec,	/* Feature for option, and optional spec */
			 char *			kind,		/* Kind of option: USER/HOST/DISPLAY/GROUP */
			 char *			name,		/* Name of "kind" */
			 FEATURE_LIST *	featlist,	/* Daemon's feature list */
			 int			borrow)		/* This is a BORROW include/exclude option */
{
	char *feature, *spec;
	FEATURE_LIST *f = 0;
	int bFoundParent = 0;


	parse_featspec(featspec, &feature, &spec);
/*
 *  First, see if the feature exists.
 */
	if (!*feature || featlist)
    {
		if(featlist)
			f = featlist;
		else
			f = &dummy; /* !*feature */
		build_each_opt(type, num, kind, name, f, feature, borrow);
    }
    else
	{
		while(f = findfeat(feature, spec, f))
		{
			if(f->flags & LM_FL_FLAG_BUNDLE_PARENT)
			{
				bFoundParent = 1;
				continue;
			}
			if(!bFoundParent && (f->flags & LM_FL_FLAG_BUNDLE_COMPONENT))
			{
				/*
				 *	Can only RESERVE the BUNDLE itself, which results in all
				 *	the bundle components being reserved.  Don't allow users
				 *	to RESERVE individual bundle components.
				 */
				DLOG(("SUITE_RESERVED component '%s' cannot be RESERVED - ignoring", feature));
				break;
			}
			build_each_opt(type, num, kind, name, f, feature, borrow);
		}
	}
}

void
ls_build_reserve_opt(int			type,		/* What it is: RESERVE, INCLUDE, EXCLUDE */
					 char *			num,		/* Number of option */
					 char *			featspec,	/* Feature for option, and optional spec */
					 char *			kind,		/* Kind of option: USER/HOST/DISPLAY/GROUP */
					 char *			name,		/* Name of "kind" */
					 FEATURE_LIST *	featlist,	/* Daemon's feature list */
					 int			borrow)		/* This is a BORROW include/exclude option */
{
	char *feature, *spec;
	FEATURE_LIST *f = 0;


	parse_featspec(featspec, &feature, &spec);
/*
 *  First, see if the feature exists.
 */
	while(f = findfeat(feature, spec, f))
	{
		/*
		 *	We check the type here because this code is called when doing a dynamic_reserve
		 *	which would result in the number of components of the BUNDLE being reserved.
		 */

		if(f->flags & LM_FL_FLAG_BUNDLE_COMPONENT &&
			type == RESERVE && f->package)
		{
			/*
			 *	If a component of a BUNDLE, needs to be done to all
			 *	components of the BUNDLE
			 */
			FEATURE_LIST *	pFeature = NULL;
			FEATURE_LIST *	pParent = NULL;


			pFeature = pParent = f_get_feat(f->package->feature, f->package->code, 1);
			if(pParent)
			{
				do
				{
					build_each_opt(type, num, kind, name, pFeature, pParent->feature, borrow);
				} while (pFeature = findfeat(pParent->feature, spec, pFeature));
			}
			else
			{
				/*
				 *	Just do the component itself I guess......
				 */
				build_each_opt(type, num, kind, name, f, feature, borrow);
			}

		}
		else
		{
			if(type == DYNAMIC_RESERVE_EXCLUDE_PACKAGE)
			{
				type = DYNAMIC_RESERVE;
				if(f->flags & LM_FL_FLAG_BUNDLE_PARENT)
					continue;
			}
			if(featlist)
			{
				if(strcmp(featlist->code, f->code))
					continue;
			}
			if(!f->conf)
				continue;
			build_each_opt(type, num, kind, name, f, feature, borrow);
		}
	}
}


static
void
remove_each_opt(
	int				type,	/* What it is: RESERVE, INCLUDE, EXCLUDE */
	char *			num,	/* Number of option */
	char *			kind,	/* Kind of option: USER/HOST/DISPLAY/GROUP */
	char *			name,	/* Name of "kind" */
	FEATURE_LIST *	f,		/* Daemon's feature list */
	char *			feature,/* feature name */
	int				borrow)	/* This is a BORROW include/exclude option */
{
	int			n = 0;
	int			i = 0;
	int			count = 0;
	int			bHead = 0;
	int			bFound = 0;

	OPTIONS *	pHead = NULL;
	OPTIONS *	pCurr = NULL;
	OPTIONS *	pNext = NULL;
	OPTIONS *	pPrev = NULL;

	OPTIONS *	pLast = NULL;

	count = i = n = atoi(num);


	/*
	 *	Should add a check to make sure we don't go over the number of licenses to serve
	 */

	pHead = pCurr = f->opt;
	pPrev = NULL;

	while(i && pCurr)
	{
		pNext = pCurr->next;
		if(pCurr->type == type && strcmp(pCurr->name, name) == 0)
		{
			if(pHead == pCurr)
			{
				/*
				 *	First in the list
				 */
				pHead = pNext;
			}
			i--;
			free_opt(pCurr);
			if(pPrev)
				pPrev->next = pNext;
		}
		else
		{
			pPrev = pCurr;
		}
		pCurr = pNext;
	}
	f->opt = pHead;

	/*
	 *	Add back to available pool.
	 */
	f->nlic += (n - i);
	f->res -= (n - i);

	/*
	 *	If this flag was set, unset it since we just freed at least one.
	 */
	if(n > i && f->flags & LM_FL_FLAG_ALL_RESERVED)
		f->flags ^= LM_FL_FLAG_ALL_RESERVED;


	/*
	 *	If type is DYNAMIC_RESERVE, delete one entry from f->res_list as when
	 *	you reserve X licenses for a feature, X OPTIONS are placed onto f->opt,
	 *	but a single OPTIONS is placed onto f->res_list.
	 */
	if (type == DYNAMIC_RESERVE)
	{
		/*
		 *	Walk FEATURE_LIST.res_list and delete the OPTIONS struct
		 *	that pertains to this user
		 */
		pHead = pCurr = f->res_list;
		pPrev = NULL;
		while(pCurr)
		{
			pNext = pCurr->next;
			if(pCurr->type == DYNAMIC_RESERVE && strcmp(pCurr->name, name) == 0)
			{
				/*
				 *	Check to see if we can actually delete this OPTION struct from
				 *	the reservation list.  If not, just decrement the count.  If
				 *	"num" is more than this one, delete this guy and continue on to
				 *	next OPTION struct that matches until we've deleted the right
				 *	number of OPTION structs or have decremented it "num" times.
				 */
				if(pCurr->numRes == count || pCurr->numRes < count)
				{
					count -= pCurr->numRes;
					/*
					 *	Delete this entry and fix up pointers, if o->count < count,
					 *	continue looking for more to decrement/delete
					 */
					if(pHead == pCurr)
					{
						pHead = pNext;
					}
					free_opt(pCurr);
					pCurr = NULL;
					if(pPrev)
					{
						pPrev->next = pNext;
					}
					if(!count)
						break;
					else
					{
						pCurr = pNext;
						continue;
					}
				}
				else
				{
					/*
					 *	Just decrement this count and break out of loop.
					 */
					pCurr->numRes -= count;
					break;
				}
			}
			pPrev = pCurr;
			pCurr = pNext;
		}
		f->res_list = pHead;
	}
}


void
ls_remove_opt(
	int				type,		/*	What it is: RESERVE, DYNAMIC_RESERVE, INCLUDE, EXCLUDE */
	char *			num,		/*	Number of options	*/
	char *			feat,		/*	Feature for option */
	char *			kind,		/*	Kind of option: USER/HOST/DISPLAY/GROUP */
	char *			name,		/*	Name of "kind" */
	FEATURE_LIST *	featlist,	/*	Daemon's feature list */
	int				borrow)		/*	This is a BORROW include/exclude option */
{
	char *			feature = NULL;
	char *			spec = NULL;
	FEATURE_LIST *	f = NULL;


	parse_featspec(feat, &feature, &spec);
	while(f = findfeat(feature, spec, f))
	{
		if(strcmp(f->code, featlist->code))
			continue;	/* Not the right one */
		remove_each_opt(type, num, kind, name, f, feature, borrow);
	}

}

/*
 *      build_each_opt
 *      note:  (OPT_MAX) num is the number of reservations, etc, and
 *              is normally used to make "num" opt records, 1 for
 *              each reservation.
 *              If num (which is a string) is prefaced with a "c", it's
 *              a "count", and there's only 1 opt record to be made,
 *              but count is set to "num"'s value.
 */
static
void
build_each_opt(int				type,	/* What it is: RESERVE, INCLUDE, EXCLUDE */
			   char *			num,	/* Number of option */
			   char *			kind,	/* Kind of option: USER/HOST/DISPLAY/GROUP */
			   char *			name,	/* Name of "kind" */
			   FEATURE_LIST *	f,		/* Daemon's feature list */
			   char *			feature,/* feature name */
			   int				borrow)	/* This is a BORROW include/exclude option */
{
	int n;
	int	nlic = 0;
	OPTIONS *o, *firsto = 0, *last = 0;  /* Pointer to the (malloc'ed) option structures */
	char *s;
	int otype2;
	OPTIONS **listhead;
	int count = 0;
#ifdef NLM
    ThreadSwitch();
#endif

    if (*num == 'c') /* OPT_MAX */
    {
        n = 1;
        count = atoi(num+1);
    }
    else
		n = atoi(num);
	nlic = n;

	/*
	 *              get the appropriate number of options structs.
	 *              malloc 1 extra for reserves to go in res_list
	 */

    if (l_keyword_eq(lm_job,kind, "HOST"))
		otype2 = OPT_HOST;
    else if (l_keyword_eq(lm_job,kind, "USER"))
		otype2 = OPT_USER;
    else if (l_keyword_eq(lm_job,kind, "DISPLAY"))
		otype2 = OPT_DISPLAY;
    else if (l_keyword_eq(lm_job,kind, "USER_GROUP") || !strcmp(kind, "GROUP"))
		otype2 = OPT_GROUP;
    else if (l_keyword_eq(lm_job,kind, "HOST_GROUP"))
		otype2 = OPT_HOST_GROUP;
    else if (l_keyword_eq(lm_job,kind, "INTERNET"))
		otype2 = OPT_INTERNET;
    else if (l_keyword_eq(lm_job,kind, "PROJECT"))
		otype2 = OPT_PROJECT;
	else if (l_keyword_eq(lm_job,kind, "BUNDLE"))
		otype2 = OPT_BUNDLE;
    else
    {
        LOG((lmtext("Unknown option: %s\n"), kind));
        otype2 = 0;
    }

    while (n--)
    {
		o = (OPTIONS *) LS_MALLOC(sizeof(OPTIONS));
		if ((otype2 != OPT_INTERNET) &&
			!((otype2 == OPT_HOST) && l_is_inet(name)))
		{
			s = LS_MALLOC(strlen(name) + 1);
			strcpy(s, name);
		}
		if (!firsto)
			firsto = o;
		if (last)
			last->next = o;
		o->type = type;
		o->type2 = otype2;
		if ((otype2 == OPT_INTERNET) ||
			((otype2 == OPT_HOST) && l_is_inet(name)))
		{
			l_inet_to_addr(name, &o->addr_type, &o->inet_addr[0]);
		}
		else
			o->name = s;
		/*
		 *	Check for a BUNDLE component, if yes, parse the DUP settings
		 */
		if(o->type2 == OPT_BUNDLE)
		{
			char *	pszData = NULL;
			char *	pszCurr = o->name;

			o->pszUser = o->pszHost = o->pszDisplay = o->pszVendor = NULL;

			if(o->name)
			{
				pszData = strstr(o->name, ":");
				if(pszData && pszData != pszCurr)
				{
					o->pszUser = LS_MALLOC(pszData - pszCurr + 1);
					if(o->pszUser)
					{
						memset(o->pszUser, 0, pszData - pszCurr);
						memcpy(o->pszUser, pszCurr, pszData - pszCurr);
					}
				}
				pszCurr = ++pszData;
				pszData = strstr(pszCurr, ":");
				if(pszData && pszData != pszCurr)
				{
					o->pszHost = LS_MALLOC(pszData - pszCurr + 1);
					if(o->pszHost)
					{
						memset(o->pszHost, 0, pszData - pszCurr);
						memcpy(o->pszHost, pszCurr, pszData - pszCurr);
					}
				}
				pszCurr = ++pszData;
				pszData = strstr(pszCurr, ":");
				if(pszData && pszData != pszCurr)
				{
					o->pszDisplay = LS_MALLOC(pszData - pszCurr + 1);
					if(o->pszDisplay)
					{
						memset(o->pszDisplay, 0, pszData - pszCurr);
						memcpy(o->pszDisplay, pszCurr, pszData - pszCurr);
					}
				}
				pszCurr = ++pszData;
				if(pszCurr && *pszCurr != '\0')
				{
					o->pszVendor = LS_MALLOC(strlen(pszCurr) + 1);
					if(o->pszVendor)
					{
						strcpy(o->pszVendor, pszCurr);
					}
				}
			}
			if(f->conf)
				o->dup_select = f->conf->lc_suite_dup;

		}
		o->count = count;
		last = o;
    }
/*
 *              The option struct is filled in, now link
 *              it to the appropriate feature
 */
    if (type == INCLUDE)
    {
		if (reread  && !borrow && *feature &&
			((f->type_mask & LM_TYPE_USER_BASED) ||
			(f->type_mask & LM_TYPE_HOST_BASED)))
		{
			if (!ls_reread_time)
				ls_reread_time = ls_currtime;
			listhead = &(f->reread_include);
		}
		else if (*feature)
		{
			if (borrow)
				listhead = &(f->b_include);
			else
				listhead = &(f->include);
		}
		else
			listhead = &includeall;
    }
    else if(type == EXCLUDE)
    {
		if (*feature)
		{
			if (borrow)
				listhead = &(f->b_exclude);
			else
				listhead = &(f->exclude);
		}
		else
		{
			if (borrow)
				listhead = &b_excludeall;
			else
				listhead = &excludeall;
		}
    }
    else
		listhead = &(f->opt);

	if (type == RESERVE || type == DYNAMIC_RESERVE)
    {

		o = (OPTIONS *) LS_MALLOC((unsigned) (sizeof(OPTIONS)));
		o->next = f->res_list;
		f->res_list = o;
		o->type = type;
		o->type2 = otype2;
		if ((otype2 == OPT_INTERNET) ||
			((otype2 == OPT_HOST) && l_is_inet(name)))
		{
			l_inet_to_addr(name, &o->addr_type,	&o->inet_addr[0]);
			o->name = 0;
		}
		else
		{
			/*
			 *	Allocate memory for copy of name.
			 */
			o->name = LS_MALLOC(strlen(name) + 1);
			strcpy(o->name, name);
		}
		/*
		 *	Check for a BUNDLE component, if yes, parse the DUP settings
		 */
		if(o->type2 == OPT_BUNDLE)
		{
			char *	pszData = NULL;
			char *	pszCurr = o->name;

			o->pszUser = o->pszHost = o->pszDisplay = o->pszVendor = NULL;

			if(o->name)
			{
				pszData = strstr(o->name, ":");
				if(pszData && pszData != pszCurr)
				{
					o->pszUser = LS_MALLOC(pszData - pszCurr + 1);
					if(o->pszUser)
					{
						memset(o->pszUser, 0, pszData - pszCurr);
						memcpy(o->pszUser, pszCurr, pszData - pszCurr);
					}
				}
				pszCurr = ++pszData;
				pszData = strstr(pszCurr, ":");
				if(pszData && pszData != pszCurr)
				{
					o->pszHost = LS_MALLOC(pszData - pszCurr + 1);
					if(o->pszHost)
					{
						memset(o->pszHost, 0, pszData - pszCurr);
						memcpy(o->pszHost, pszCurr, pszData - pszCurr);
					}
				}
				pszCurr = ++pszData;
				pszData = strstr(pszCurr, ":");
				if(pszData && pszData != pszCurr)
				{
					o->pszDisplay = LS_MALLOC(pszData - pszCurr + 1);
					if(o->pszDisplay)
					{
						memset(o->pszDisplay, 0, pszData - pszCurr);
						memcpy(o->pszDisplay, pszCurr, pszData - pszCurr);
					}
				}
				pszCurr = ++pszData;
				if(pszCurr && *pszCurr != '\0')
				{
					o->pszVendor = LS_MALLOC(strlen(pszCurr) + 1);
					if(o->pszVendor)
					{
						strcpy(o->pszVendor, pszCurr);
					}
				}
			}
			if(f->conf)
				o->dup_select = f->conf->lc_suite_dup;
			o->numRes = nlic;

		}
		o->count = count;
    }

    if (*listhead)
    {
        for (o = *listhead; o->next; o = o->next)   ;
        o->next = firsto;
    }
    else
		*listhead = firsto;
	if(type == DYNAMIC_RESERVE)
	{
		f->nlic -= nlic;
		if(f->nlic < 0)
			f->nlic = 0;
		f->res += nlic;

	}
}

/*
 *      parse_featspec
 *      featspec is "feature:spec", and spec is "config_field=value"
 *      Examples:       f1:key=20-charcode
 *                      f1:asset_info="this-one"
 *                      This parsing happens in place, and the colon
 *                      is replaced with a '\0';
 */
static
void
parse_featspec(char *	featspec,
			   char **	feature,	/* return value */
			   char **	spec)		/* return value */
{
    *spec = '\0';
    *feature = featspec;
    for(;*featspec;featspec++)
    {
        if (((*featspec == ':') ||(*featspec == ' ')) &&
			strchr(featspec, '='))
        {
            *featspec = '\0'; /* null terminate feature */
            *spec = featspec+1;
            return;
        }
    }
}

static
void
nolog(char * string)
{
	int msg = 1;

	if (l_keyword_eq(lm_job,string, "IN"))
	{
		ls_log_in = 0;
	}
	else if (l_keyword_eq(lm_job,string, "OUT"))
	{
		ls_log_out = 0;
	}
	else if (l_keyword_eq(lm_job,string, "QUEUED"))
	{
		ls_log_queued = 0;
	}
	else if (l_keyword_eq(lm_job,string, "DENIED"))
	{
		ls_log_denied = 0;
	}
	else
		msg = 0;

	if (msg)
	{
		LOG((lmtext("Not logging %s messages\n"), string));
		LOG_INFO((INFORM, "This daemon is not logging this type of \
				message, as specified in the options file."));
	}
}

/* Fix for P7208 */
static
void
groupcaseinsensitve(char * string)
{
	if (l_keyword_eq(lm_job,string, "ON"))
	{
		ls_group_case_insensitive = 1;
	}
	else if (l_keyword_eq(lm_job,string, "OFF"))
	{
		ls_group_case_insensitive = 0;
	}
	else
		ls_group_case_insensitive = 0;
}

/* accessor function to get groupcaseinsensitive flag setting */
int getgroupcaseinsensitiveflag()
{
	return ls_group_case_insensitive;
}

/*
 *      log_option() - Log the options
 */
static
void
log_option(char *		type,	/*	Type of option */
		   char *		prep,
		   OPTIONS *	o,		/*	The option */
		   char *		which)	/*	Which feature */
{
	char count_info[MAX_LONG_LEN + 25];

    if (o->count)
		sprintf(count_info, "%d", o->count);
    else
		*count_info = 0;

    if ((o->type2 == OPT_INTERNET) || ((o->type2 == OPT_HOST) && !o->name))
    {
		char addr[20];

        l_addr_to_inet(&o->inet_addr[0], o->addr_type, addr);
        LOG((lmtext("%s Internet Address %s %s %s\n"), type, addr,
                                                        prep, which));
        LOG_INFO((INFORM, "The specified Internet Address \
                        has been INCLUDED/EXCLUDED in this feature."));
    }
    else
    {
		char *t = o->type2 == OPT_USER ? "USER" :
				o->type2 == OPT_HOST ? "HOST" :
				o->type2 == OPT_GROUP ? "USER_GROUP" :
				o->type2 == OPT_HOST_GROUP ? "HOST_GROUP" :
				o->type2 == OPT_DISPLAY ? "DISPLAY" :
				o->type2 == OPT_PROJECT ? "PROJECT" :
                                                            "???";

		LOG(("%s %s%s%s %s %s%s%s\n", type,
			count_info, *count_info ? " " : "",
			t, o->name, prep, *prep ? " " : "",
			which));
		LOG_INFO((INFORM, "The specified user/host/display/group \
			has been INCLUDED/EXCLUDED in this feature."));
    }
}

static
void
set_lowwater(char *	feature,	/* Feature for option */
			 char *	low)		/* Lowwater value for this feature */
{
	int n = atoi(low);
	FEATURE_LIST *f;

/*
 *      First, see if the feature exists.
 */
    if (*feature)
		f = findfeat(feature, 0, 0);
    else
		f = &dummy;
/*
 *      If it exists, set the lowwater mark
 */
	if(f)
		f->lowwater = n;
}

static
void
ls_process_group(int	nflds,
				 char *	f1,
				 char *	f2,
				 char *	f3,
				 char *	f4,
				 char *	f5,
				 char *	f6)
{
	GROUP *g, *gl, *lg, **groupsp;
	int len;
	char *s;

	if (reread && ls_user_or_hostbased(f2, 1))
	{
		return;
	}
    if (l_keyword_eq(lm_job,f1, "GROUP") || !strcmp(f1, "USER_GROUP"))
		groupsp = &groups;
    else
		groupsp = &hostgroups;

/*
 *	Compute length of user names
 */
    len = strlen(f3) + 2;
    if (nflds > 3)
		len += strlen(f4) + 1;
    if (nflds > 4)
		len += strlen(f5) + 1;
    if (nflds > 5)
		len += strlen(f6) + 1;
/*
 *	First, see if this group is here
 */
    for (g = *groupsp; g; g = g->next)
    {
		if (l_keyword_eq(lm_job,g->name, f2))
			break;
    }
    if (g)
    {
/*
 *	Got it already.  Update
 */
        len += strlen(g->list);
        s = LS_MALLOC((unsigned) len +2) ;
        strcpy(s, g->list);
        free(g->list);
        strcat(s, " ");
	}
    else
    {
/*
 *	New group - link onto the group listhead
 */
		g = (GROUP *)LS_MALLOC((unsigned)sizeof(GROUP));
		lg = 0;
		for (gl = *groupsp; gl; gl = gl->next)
		{
			lg = gl;
		}
		if (lg)
			lg->next = g;
		else
			*groupsp = g;
		g->next = 0;
		s = LS_MALLOC((unsigned) strlen(f2) +1);
	/*
	 *	Save group name
	 */
		strcpy(s, f2);
		g->name = s;
		s = LS_MALLOC((unsigned) len + 2);
		memset(s, 0, len);
	}

/*
 *	Now save the (new) group members
 */
    strcat(s, f3);
    if (nflds > 3)
    {
        strcat(s, " ");
        strcat(s, f4);
    }
    if (nflds > 4)
    {
        strcat(s, " ");
        strcat(s, f5);
    }
    if (nflds > 5)
    {
        strcat(s, " ");
        strcat(s, f6);
    }
    g->list = s;
}

static
int
scan_opt_line(char * line,
			  char * f1,
			  char * f2,
			  char * f3,
			  char * f4,
			  char * f5,
			  char * f6)
{

	int field;
	int quote = 0;
	int nflds = 0;

    *f1 = *f2 = *f3 = *f4 = *f5 = *f6 = '\0';
    for (field = 1; *line && field < 6; *line ? line++ : 0, field++)
    {
        while (*line && isspace(*line))
			line++;

        while (*line)
        {
            if(isspace(*line) && !quote)
                    break;
            if(*line == '"' )
            {
                if(!quote)
					quote = 1;
                else
					quote = 0;
                line ++;
                if(!quote)
					break;
            }
            switch (field)
            {
				case 1: nflds=1; *f1++ = *line++; break;
				case 2: nflds=2; *f2++ = *line++; break;
				case 3: nflds=3; *f3++ = *line++; break;
				case 4: nflds=4; *f4++ = *line++; break;
				case 5: nflds=5; *f5++ = *line++; break;
            }
        }
        switch(field)
        {
			case 1: *f1 = '\0'; break;
			case 2: *f2 = '\0'; break;
			case 3: *f3 = '\0'; break;
			case 4: *f4 = '\0'; break;
			case 5: *f5 = '\0'; break;
        }
    }
    while (*line && isspace(*line))
		line++;
    while (*line && (*line != '\n'))
    {
        nflds = 6;
        *f6++ = *line++;
    }
    if (nflds == 6)
		*f6 = '\0';
    return nflds;
}

static
void
skip_word(char ** cpp)
{
	char *cp = *cpp;

    while (*cp && isspace(*cp))
		cp++;  /* leading spaces */
    while (*cp && !isspace(*cp))
		cp++;  /* the word */
    while (*cp && isspace(*cp))
		cp++;  /* trailing spaces */
    *cpp = cp;
}

static
char *
getword(char **	cpp,
		char *	buf)
{
	char *cp = *cpp;
	char *bufp = buf;
    while (*cp && isspace(*cp))
		cp++; /* skip leading spaces */
    while (*cp && !isspace(*cp))
            *bufp++ = *cp++;
    while (*cp && isspace(*cp))
		cp++; /* skip trailing spaces */
    *bufp = 0; /* null terminate. */
    *cpp = cp;
    if(*buf)
		return buf;
    else
		return 0;
}

static
void
proc_log(char *	name,
		 char *	logname,
		 char *	line,
		 int	debuglog)
{
	char *cp;


/*
 *      REPORTLOG
 *      REPORTLOG [+]filename
 *
 *      Save the name now - do it later, after we
 *      have processed all the reservations.
 */
	cp = strstr(line, name);
	if (cp)
	{
		/* strip quotes */
		if (*cp == '"')
			cp++;
		if (cp[strlen(cp) -1] == '"')
			cp[strlen(cp) -1] = 0;
		strcpy(logname, cp);
	}
	else /* huh? */
		strcpy(logname, name);
#if 0 /* removing in v8, so that we can more easily parse spaces in a path */
    cp = line;
    skip_word(&cp); /* REPORTLOG */
    skip_word(&cp); /* filename */
    while (getword(&cp, buf))
    {
        if (val = strchr(buf, '='))
        {
/*
 *	Null terminate and move val to next word
 */
            *val++ = 0;
            while (*val && isspace(*val))
				val++;  /* skip spaces */
        }

        if (!debuglog && !strncmp(buf, "CPU_USAGE_INTERVAL",
                strlen("CPU_USAGE_INTERVAL")))
        {
            if (!val)
            {
                LOG((lmtext("CPU_USAGE_INTERVAL not set -- missing value\n")));
                continue;
            }
            if (!strncmp(val, "CONTINUOUS", strlen("CONTINUOUS")))
				ls_cpu_usage_interval = 0;
            else
            {
                if (!(i = atoi(val)))
                {
                        LOG((lmtext(
                "CPU_USAGE_INTERVAL not set -- invalid value\n")));
                        continue;
                }
                ls_cpu_usage_interval = i;
            }
            if (ls_cpu_usage_interval == 0)
            {
                LOG((lmtext(
                "CPU_USAGE_INTERVAL = CONTINUOUS\n")));
            }
            else
            {
                LOG((lmtext(
                "CPU_USAGE_INTERVAL = %d minute%c\n"),
                                ls_cpu_usage_interval,
                                ls_cpu_usage_interval == 1 ?
                                ' ' : 's'));
            }
        }
        else if (!strncmp(buf, "CPU_USAGE_DELTA",
                                        strlen("CPU_USAGE_DELTA")))
        {
            if (!val)
            {
                LOG((lmtext(
                "CPU_USAGE_DELTA not set -- missing value\n")));
                continue;
            }
            if (!(i = atoi(val)))
            {
				LOG((lmtext(
					"CPU_USAGE_INTERVAL not set -- invalid value\n")));
				continue;
            }
            ls_cpu_usage_delta = i;
            LOG((lmtext(
            "CPU_USAGE_DELTA = %d/10th second CPU usage\n"),
                            ls_cpu_usage_delta));
        }
    }
#endif
}

