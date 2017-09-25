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
 *	Module: $Id: ls_lmgrd.c,v 1.32.2.9 2003/07/01 17:04:21 sluu Exp $
 *
 *	Function:	main() - main routine for lmgrd
 *
 *	Description:	Main program for the "master" license manager server.
 *
 *	Parameters:	None
 *
 *	Return:		None.
 *
 *	M. Christiano
 *	2/15/88
 *
 *	Last changed:  8/6/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsmaster.h"
#include "../machind/lsfeatur.h"
/*#include "../app/ls_aprot.h" */
#include "l_m_prot.h" 
#include "flex_utils.h"
#include "flexevent.h"

#if defined(MOTO_88K) || defined(MIPS) || defined (sco) || defined(sinix)
#include <sys/types.h>
#include <time.h>
#endif

DAEMON *master_daemons = (DAEMON *)0;
int ls_i_am_lmgrd = 1;		/* We are lmgrd */
int ls_read_wait = 10;		/* How long to wait for solicited reads */
int ls_dump_send_data = 0;	/* Set to non-zero value for debug output */
int ls_conn_timeout = MASTER_WAIT; /* How long to wait for a connection */
extern char ls_our_hostname[];	/* Our hostname from license file */
int use_v1_spawn = 0;		/* Use the FLEXlm v1.x daemon spawn code */
				/* Default changed to 1 in v2.38 */
				/* Default changed to 0 in v6.0 */
int ls_allow_lmdown = 1;	/* toggled with '-x lmdown' arg */
int ls_allow_lmremove = 1;	/* toggled with '-x lmremove' arg */
int ls_lmgrd_tcp_port = -99;

static SELECT_MASK select_mask; /* File descriptors to select on */

/*
 *	Stubs for app library routines that don't exist in lmgrd
 */
void f_nousers() { }		/* (Vendor-daemon routine --- UNUSED) */
void ls_feat_dump() { }		/* (Vendor-daemon routine --- UNUSED) */
void ls_get_info() { }		/* (Vendor-daemon routine --- UNUSED) */
int ls_show_vendor_def = 0;	/* Vendor-daemon variable */
char *ls_flist;			/* vendor-daemon variable, UNUSED */
char *ls_license_finder = (char *) NULL; /* "license finder" desc. file */
char * groups;/* vendor-daemon variable, UNUSED */
char * hostgroups;/* vendor-daemon variable, UNUSED */
int ls_cpu_usage_delta; /* vendor-daemon variable, UNUSED */
long ls_user_init1 ; /* UNUSED */
long ls_user_init2 ; /* UNUSED */
long ls_outfilter ; /* UNUSED */
long ls_infilter ; /* UNUSED */
long ls_incallback ; /* UNUSED */
long ls_vendor_msg ; /* UNUSED */
long ls_vendor_challenge ; /* UNUSED */
char *ls_user_lockfile ; /* UNUSED */
int ls_conn_timeout; /* UNUSED */
int ls_enforce_startdate; /* UNUSED */
int ls_tell_startdate; /* UNUSED */
int ls_minimum_user_timeout; /* UNUSED */
int ls_min_lmremove; /* UNUSED */
int ls_use_featset ; /* UNUSED */
int ls_do_checkroot; /* UNUSED */
int ls_show_vendor_def; /* UNUSED */
long ls_daemon_periodic; /* UNUSED */
int ls_use_all_feature_lines ; /* UNUSED */
int ls_compare_vendor_on_increment; /* UNUSED */
int ls_compare_vendor_on_upgrade; /* UNUSED */
char *ls_a_behavior_ver; /* UNUSED */
int ls_a_check_baddate; /* UNUSED */
int ls_a_lkey_start_date; /* UNUSED */
int ls_a_lkey_long; /* UNUSED */
int ls_a_license_case_sensitive; /* UNUSED */

#ifndef MAC10
void *l_borrow_dptr; /* UNUSED */
#endif
LM_SOCKET tcp_s = LM_BAD_SOCKET;	/* The TCP socket file descriptor */
LM_SOCKET spx_s = LM_BAD_SOCKET;	/* The SPX socket file descriptor */
LM_SERVER *main_master_list;

char **argv_sav;
int argc_sav;

#ifdef WINNT
void
main_service_thread(int argc, char * argv[])
#else
int
main(int argc, char * argv[])
#endif /* WINNT */
{
	LM_SERVER *s;
	char	szBuffer[MAX_PATH] = {'\0'};
	char *	ppszInStr[20] = {NULL};	/* should never have more that 20 insertion strings */
 
#ifdef USE_WINSOCK
#if defined( OS2) || defined(WINNT)

  typedef void (*_onexit_t)(void);

#undef WSACleanup
#define WSACleanup l__WSACleanup
#endif /* OS2 */  
#define WINSOCK_VERSION 0x0101
	argc_sav=argc;
	argv_sav=argv;
	{
		WSADATA wsadata;
		WSAStartup( (WORD)WINSOCK_VERSION, &wsadata );
		onexit( (_onexit_t)WSACleanup );
	}
#endif		
  
	ls_m_init(argc, argv, &main_master_list, 0);
	if (ls_license_finder)
	{
		ls_finder(ls_license_finder, &select_mask);  /* Never returns */
	}
	{
	  char arch[MAX_HOSTTYPE_NAME+5];
	  HOSTTYPE *x;
#ifdef THREAD_SAFE_TIME
	  struct tm *ls_gettime(struct tm * ptst), *t;
	  struct tm tst;
#else /* THREAD_SAFE_TIME */
	  struct tm *ls_gettime(), *t;
#endif

		x = lm_hosttype(0);
		if (x && x->name)
		{
			(void) sprintf(arch, " (%s)", x->name);
		}
		else
			arch[0] = '\0';

#ifdef THREAD_SAFE_TIME
		ls_gettime(&tst);
		t = &tst;
#else /* THREAD_SAFE_TIME */
		t = ls_gettime();
#endif

		if(l_flexEventLogIsEnabled())
		{
			char	szVersion[20] = {'\0'};
			char	szRev[20] = {'\0'};
			char	szPatch[20] = {'\0'};
			char	szMonth[20] = {'\0'};
			char	szDay[20] = {'\0'};
			char	szYear[20] = {'\0'};

			l_flexEventLogWrite(lm_job,
								FLEXEVENT_INFO,
								CAT_FLEXLM_SERVER_EVENT,
								MSG_FLEXLM_EVENT_LOG_ACTIVE,
								0,
								ppszInStr,
								0,
								NULL);
			sprintf(szVersion, "%d", lm_job->code.flexlm_version);
			sprintf(szRev,"%d", lm_job->code.flexlm_revision);
			sprintf(szPatch, "%s", lm_job->code.flexlm_patch);
			sprintf(szMonth, "%d", t->tm_mon + 1);
			sprintf(szDay, "%d", t->tm_mday);
			sprintf(szYear, "%d", t->tm_year + 1900);

			ppszInStr[0] = szVersion;
			ppszInStr[1] = szRev;
			ppszInStr[2] = szPatch;
			ppszInStr[3] = ls_our_hostname;
			ppszInStr[4] = arch;
			ppszInStr[5] = szMonth;
			ppszInStr[6] = szDay;
			ppszInStr[7] = szYear;

			l_flexEventLogWrite(lm_job,
								FLEXEVENT_INFO,
								CAT_FLEXLM_SERVER_EVENT,
								MSG_FLEXLM_LMGRD_STARTMSG,
								8,
								ppszInStr,
								0,
								NULL);
		}


		LOG((lmtext("FLEXlm (v%d.%d%s) started on %s%s (%d/%d/%d)\n"),
				lm_job->code.flexlm_version, 
				lm_job->code.flexlm_revision, 
				lm_job->code.flexlm_patch, 
				ls_our_hostname,
				arch, t->tm_mon+1, t->tm_mday, 
				t->tm_year + 1900));
		LOG((COPYRIGHT_STRING(1988)"\n"));
		LOG((lmtext("US Patents 5,390,297 and 5,671,412.\n")));
		LOG((lmtext("World Wide Web:  http://www.macrovision.com\n")));
		LOG_INFO((INFORM, "The license server logs its start time \
			along with the node name whenever it starts up."));
		{
		  extern int ls_local;
			if (ls_local)
				LOG(("lmdown/lmreread only allowed on this node\n"));
		}
		LOG((lmtext("License file(s):")));
		{
			int		i = 0;
			int		dummy = 0;

			for (i = 0; lm_job->lic_files[i]; i++)
			{
				_LOG((" %s", lm_job->lic_files[i]));
				if(l_flexEventLogIsEnabled())
				{
					ppszInStr[0] = lm_job->lic_files[i];
					l_flexEventLogWrite(lm_job,
										FLEXEVENT_INFO,
										CAT_FLEXLM_LICENSE_FILE,
										MSG_FLEXLM_LMGRD_LICENSE_FILE,
										1,
										ppszInStr,
										0,
										NULL);
				}
			}
		}
		_LOG(("\n"));

		LOG_INFO((INFORM, "The license server logs the current \
			active license file."));
	}
	if (!main_master_list)
	{
/*
 *		Look for features that bind to a node with usercount = 0
 *		If we find none, this is an illegal configuration
 */
		lm_perror("can't get host list");
		ls_statfile_rm();
		exit(1);
	}
	for (s = main_master_list; s && !ls_on_host(s->name); s = s->next)
		;
	if (s) 
	{
		if (ls_lmgrd_tcp_port != -1)
			ls_lmgrd_tcp_port = s->port;
		LOG((lmtext("lmgrd tcp-port %d\n"), s->port));
		if(l_flexEventLogIsEnabled())
		{
			sprintf(szBuffer, "%d", s->port);
			ppszInStr[0] = szBuffer;
			l_flexEventLogWrite(lm_job,
								FLEXEVENT_INFO,
								CAT_FLEXLM_LMGRD,
								MSG_FLEXLM_LMGRD_PORT,
								1,
								ppszInStr,
								0,
								NULL);
		}
	}


		
	if (!master_daemons)
	{
		LOG((lmtext("The license daemon has found no vendor daemons to start\n")));
		LOG((lmtext(" (There are no VENDOR (or DAEMON) lines in the license file),\n\tlmgrd exiting.\n")));

		if(l_flexEventLogIsEnabled())
		{
			l_flexEventLogWrite(lm_job,
								FLEXEVENT_ERROR,
								CAT_FLEXLM_SERVER_EVENT,
								MSG_FLEXLM_LMGRD_DAEMON_NOT_FOUND,
								0,
								ppszInStr,
								0,
								NULL);
		}

		if (_lm_errno)
		{
			_LOG((" (%s)", lmtext(lc_errstring(lm_job))));
		}
		_LOG(("\n"));
		LOG_INFO((CONFIG, "The license daemon found no vendor daemons \
			to start, thus it exited."));

		ls_statfile_rm();		
		exit(2);
	}
	lm_job->options->flags &= ~LM_OPTFLAG_LONG_ERRMSG;
	while (1)
	{
		ls_quorum(main_master_list, "", 0); /* Make sure quorum is up */
		ls_m_main(&master_daemons, &select_mask);
	}
#ifndef PC
	return 0;
#endif
}

void
ls_mask_clear(int i)
{
	if (i >= 0 && select_mask)
	{
		MASK_CLEAR(select_mask, (unsigned int)i);
	}
}
