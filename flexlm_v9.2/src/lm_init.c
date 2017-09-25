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
/******************************************************************************
 *
 *
 *	NOTE:	The purchase of FLEXlm source does not give the purchaser
 *
 *		the right to run FLEXlm on any platform of his choice.
 *
 *		Modification of this, or any other file with the intent 
 *
 *		to run on an unlicensed platform is a violation of your 
 *
 *		license agreement with Macrovision Corporation. 
 *
 *
 *****************************************************************************/
/*	
 *	Module: $Id: lm_init.c,v 1.115 2003/04/18 23:48:07 sluu Exp $
 *
 *	Description: 	FLEXlm client initialization routine.
 *
 *	M. Christiano
 *	6/18/90
 *
 *	Last changed:  12/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include "lmselect.h"
#include "lm_attr.h"
#include "lgetattr.h"
#include "flex_file.h"
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#ifndef PC
#ifdef USE_SYS_TIMES_H
#include <sys/times.h>
#else
#include <sys/time.h>
#endif
#endif
#ifdef USE_WINSOCK  
char *l_gcspim;
#include <winsock.h>      
#endif
#ifdef ANSI
#include <stdlib.h>
#endif
int l_reset_env = 0;
extern char *(* l_borrow_dptr)(LM_HANDLE *job, char *);

#ifdef VMS
static void get_vms_ef();
#endif

#ifdef NO_INIT_SHARE
char ** _lm_errlist;
static char *flexlm_errlist[] = {
#include "lmerrors.h"
				};
int _lm_nerr = (sizeof (flexlm_errlist) / sizeof (flexlm_errlist[0]));
#endif	/* NO_INIT_SHARE */

/*-
 *	last_chance used to be an LM_HANDLE, but that's too big, and 
 *	all we need is to make sure it's big enough for err_info.
 */
static char last_chance[50 + sizeof(LM_ERR_INFO)]; 
char * lm_borrow = NULL;


#ifdef NO_getdtablesize
int lu_getdtablesize lm_args((lm_noargs));
#endif

#if defined(DGUX) || defined (MOTO_88K)

#ifdef DGUX
#ifdef SVR4
#include <sys/_int_unistd.h>
#else
#include <sys/m88kbcs.h>
#endif
#endif	/* DGUX */

#ifdef MOTO_88K
#include <unistd.h>
#endif	/* MOTO_88K */

long sysconf();
#endif	/* defined(MOTO_88K) || defined(DGUX) */

#ifdef NO_INIT_SHARE
extern char *our_daemon;
#endif /* NO_INIT_SHARE */

typedef int (*PFI)();
typedef char *(*PCSTAR)();
typedef HOSTID * (*PFHOSTID) lm_args((short idtype));
typedef LM_PFV (*PFPFV)();

#if !defined(NO_TIMERS) && !defined(NLM) && !defined(OS2)
extern setitimer();
#endif /* !defined(NO_TIMERS) && !defined(NLM) && !defined(OS2) */
#ifdef VOID_SIGNAL
extern LM_PFV signal();
#elif ! defined(VXWORKS)
extern PFI signal();
#endif

#ifdef PC
extern FILE * our_std_error;
int l_win95;
#endif


#ifdef PC
#define LIBNAME "lmgr.lib"
#else 
#define LIBNAME "liblmgr.a"
#endif 
static char *VERSION =  "@(#) FLEXlm v" FLEXLM_VERSION_REVISION_STRING " (" LIBNAME "), " COPYRIGHT_STRING(1988) ;
						
						/* Not used anywhere */

#ifndef RELEASE_VERSION         /* Don't let this appear in user versions */
int _lm_debug = 0;              /* Global debugging variable */
#endif




/* 
 *	variables used for select()
 *      Initialize them to zero for DEC C will allocate some storage
 *      for them.
 */
static int first = 1;
int lm_bpi = 0;
int lm_max_masks = 0;
int lm_nofile = 0;
#define INIT_NUM_MSGQ 5 /* malloc this many MSGQUEUE's -- avoid async malloc */

void
l_add_key_filter(
	LM_HANDLE *		job, 
	char *			app_filter, 
	char *			gen_filter,  
	int				sign_level, 
	unsigned int	flags, 
	int				pubkeysize[LM_PUBKEYS],
	unsigned char	pubkey[LM_PUBKEYS][LM_MAXPUBKEYSIZ],
	LM_VENDORCODE_PUBKEYINFO *pubkeyinfo,
	int				strength)
{
	L_KEY_FILTER * f = NULL, * sav = NULL;

	sav = (L_KEY_FILTER *)job->key_filters;
	for (f = sav; f; f = f->next)
	{
		if (sign_level == f->sign_level)
			break;
	}
	if (!f)
	{
		f = (L_KEY_FILTER *)l_malloc(job, sizeof(L_KEY_FILTER));
		f->next = (L_KEY_FILTER *)job->key_filters;
		job->key_filters = (char *)f;
	}
	f->app_filter = app_filter;
	f->gen_filter = gen_filter;
	f->sign_level = sign_level;
	f->pubkeyinfo = pubkeyinfo;
	f->pubkeyinfo1 = strength;
	if (pubkeysize)
		memcpy(f->pubkeysize, pubkeysize, sizeof(f->pubkeysize));
	if (pubkey)
		memcpy(f->pubkey, pubkey, sizeof(f->pubkey));
	f->flags = flags;
}

/*
 *	lc_init() - Initialize License System Activities and get a unique job_id
 */
int
API_ENTRY
lc_init(
	LM_HANDLE *		oldjob,		/* OLD license job */
	char *			vendor_id,	/* vendor ID */
	VENDORCODE *	vendor_key,	/* Vendor's encryption seeds */
	LM_HANDLE **	job_id)
{
	return l_init(oldjob, vendor_id, vendor_key, job_id, 1);
}

int
API_ENTRY
l_init(
	LM_HANDLE *		oldjob,		/* OLD license job */
	char *			vendor_id,	/* vendor ID */
	VENDORCODE *	vendor_key,	/* Vendor's encryption seeds */
	LM_HANDLE **	job_id,
	int				external_api)
{
  int saved_lm_errno = 0;
  int ret;
  int saved_uerrno = 0;
  int saved_minor = 0;
  LM_HANDLE *job;
  LM_DAEMON_INFO *cur_daemon;
  LM_OPTIONS *cur_options;
  int cache_file = 0;
  int disable_finder = 1;
  MSGQUEUE *msgqueue = (MSGQUEUE *)NULL, *mqp = (MSGQUEUE *)NULL, *tmqp;
  int i = *VERSION; /* This forces VERSION to be compiled in */
  VENDORCODE vc;
  int sign_level = 0;


/*
 *	LITE support:  make sure they don't call lc_init more than once
 *			unless MULTIPLE_JOBS is turned on
 */

	if (oldjob && (l_getattr(oldjob, MULTIPLE_JOBS) != MULTIPLE_JOBS_VAL)
		&& (oldjob->type != LM_JOB_HANDLE_DSPECIAL_TYPE))
	{
		LM_SET_ERRNO(oldjob, LM_FUNCNOTAVAIL, 178, 0);
		return (LM_FUNCNOTAVAIL);
	}

#ifdef NO_INIT_SHARE
	_lm_errlist = flexlm_errlist;	/* Initialize error strings */
	our_daemon = vendor_id;		/* Set up our_daemon */
#endif

	if (oldjob && oldjob->options) 
	{
		cache_file = oldjob->options->cache_file;
		disable_finder = oldjob->options->disable_finder;
	}

	if (first)
	{
		lm_nofile = getdtablesize();
		if (lm_nofile > FD_SETSIZE)
			lm_nofile = FD_SETSIZE - 1;
		lm_bpi = sizeof(int) * 8;
		lm_max_masks = lm_nofile / lm_bpi;
		if ((lm_max_masks * lm_bpi) < lm_nofile)
			lm_max_masks++;
		first = 0;
	}
/*
 *	Allocate all the global data and point the current handle to
 *	it.
 */
	errno = 0;
	job = (LM_HANDLE *) calloc(sizeof(LM_HANDLE), 1);
	cur_daemon = (LM_DAEMON_INFO *) calloc(sizeof(LM_DAEMON_INFO), 1);
	cur_options = (LM_OPTIONS *) calloc(sizeof(LM_OPTIONS), 1);
	for (i=0;i<INIT_NUM_MSGQ;i++)
	{
		if (!(tmqp = (MSGQUEUE *)calloc(sizeof(MSGQUEUE), 1)))
			break;
		if (!msgqueue)
			msgqueue = tmqp;
		if (mqp)
			mqp->next = tmqp;
		mqp = tmqp;
	}
	if (cur_daemon == (LM_DAEMON_INFO *) NULL ||
	    job == (LM_HANDLE *) NULL ||
	    cur_options == (LM_OPTIONS *) NULL ||
	    tmqp == (MSGQUEUE *)NULL ||
	    !job_id)
	{
		if (!job_id)
			return LM_BADPARAM;
		else 
		{
			*job_id = (LM_HANDLE *)&last_chance[0];
			(*job_id)->lm_errno = LM_CANTMALLOC;
			return LM_CANTMALLOC;
		}
	}
	if (job_id) *job_id = job;


#if defined(PC) && !defined(NLM)
		
	if (getenv("FLEXLM_DIAGNOSTICS") && !our_std_error)	/* overrun checked */
	{
		char t[MAX_LONGNAME_SIZE] = {'\0'};
		sprintf(t,"flex%i.log",(abs(getpid())%1000));	/* OVERRUN */
		 our_std_error = l_flexFopen(job, t,"a+");
	}
	{
		WSADATA wsadata;
		OSVERSIONINFO p;
    
		p.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&p);
		if (p.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			l_win95 = 1;
	}
#endif /* USE_WINSOCK */

/*
 *		set up msgq_free 
 */
	job->msgq_free = msgqueue;
	job->type = LM_JOB_HANDLE_TYPE;
	job->daemon = cur_daemon;
	(void) strncpy(job->vendor, vendor_id, MAX_VENDOR_NAME);
	job->options = cur_options;
	job->lfptr = LFPTR_FILE1;
	job->savemsg = (char *)l_malloc(job, LM_MSG_LEN + 1);
	job->last_udp_msg = (char *)l_malloc(job, LM_MSG_LEN + 1);
/* 
 *		l_more_featdata initializes job->featdata 
 */
	job->featdata = (char *)l_more_featdata(job, job->featdata); 
	cur_daemon->type = LM_DAEMON_INFO_TYPE;
	cur_daemon->commtype = LM_TCP;
	cur_daemon->socket = LM_BAD_SOCKET;
	cur_daemon->our_comm_version = COMM_NUMVER;
	cur_daemon->our_comm_revision = COMM_NUMREV;
	/*cur_daemon->heartbeat      = 1;*/
	cur_daemon->udp_timeout     = 60*45; /*45 minutes*/
	cur_daemon->tcp_timeout     = 60*60*2; /* 2 hours */

#ifdef VMS
	get_vms_ef();
#endif
	cur_options->timer_type     = LM_REAL_TIMER;
	cur_options->check_interval = 120;
	cur_options->retry_interval = 60;
	cur_options->retry_count    = 5;
	cur_options->conn_timeout   = 10;
	cur_options->normal_hostid  = 1;
	cur_options->max_timediff   = -1;
	cur_options->commtype = LM_TCP;
	cur_options->setitimer      = 
#if defined( NO_TIMERS) || defined(PC)
					(LM_PFV) 0;
#else
					(LM_PFV) setitimer;
#endif /* defined( NO_TIMERS) || defined(PC) */
	cur_options->sighandler	    = (PFPFV) 0;
	cur_options->cache_file     = cache_file;
	cur_options->disable_finder = disable_finder;
				

/* 
 *		set boolean flags that need to default to TRUE
 */
	cur_options->flags |= LM_OPTFLAG_PORT_HOST_PLUS;
	cur_options->flags |= LM_OPTFLAG_DIAGS_ENABLED;
	cur_options->flags |= LM_OPTFLAG_LONG_ERRMSG;
	cur_options->flags |= LM_OPTFLAG_PC_PROMPT_FOR_FILE;
	cur_options->flags |= LM_OPTFLAG_RETRY_CHECKOUT;
	cur_options->flags |= LM_OPTFLAG_CKOUT_INSTALL_LIC;
	cur_options->flags |= LM_OPTFLAG_PERROR_MSGBOX;
	cur_options->flags |= LM_OPTFLAG_LCM;
	cur_options->flags |= LM_OPTFLAG_MT_HEARTBEAT;
	L_SECLEN_SET_SHORT;
	strcpy(cur_options->license_fmt_ver, LM_BEHAVIOR_CURRENT);
	strcpy(cur_options->behavior_ver, LM_BEHAVIOR_CURRENT);


	memset(&cur_options->alt_vendorcode, 0, 
					sizeof(VENDORCODE));
/*
 *		allow users to
 *		set transport via FLEXLM_COMM_TRANSPORT
 * 		or COMM_TRANSPORT license file line
 */
	cur_options->allow_set_transport  = 1; 

	/* Note that ALLOW_SET_TRANSPORT can't be set yet */
	{
	    char *	ct = NULL;
		if (ct = getenv("FLEXLM_COMM_TRANSPORT"))	/* overrun checked */
		{
			if (!strcmp("UDP", ct) || 
				!strcmp("LM_UDP", ct) ||
				!strcmp("udp", ct)|| 
				!strcmp("Udp", ct))
			{
				cur_options->transport_reset = 
					LM_RESET_BY_USER;
				cur_options->commtype = LM_UDP;
			}
			else if (!strcmp("TCP", ct) || 
				!strcmp("LM_TCP", ct))
			{
				cur_options->transport_reset = 
					LM_RESET_BY_USER;
				cur_options->commtype = LM_TCP;
			}
#ifdef SUPPORT_IPX
			else if (!strcmp("SPX", ct) ||
				!strcmp("IPX", ct))		 
			{
				cur_options->transport_reset = 
					LM_RESET_BY_USER;
				cur_options->commtype = LM_SPX;
			}
#endif /* SUPPORT_IPX */
			
			/* else it's a user error, which I don't see any
			 * easy way to report to the user */
		}	
	}
/*		
 *		Make sure user is not cheating using TZ
 */
	{
	
#ifdef PC
/* 
 * We are sure that 24*365*(local->tm_year-70) will not overflow
 * interger on PC.
 */
#pragma warning ( disable : 4756 )
#endif /* PC */	
#ifdef THREAD_SAFE_TIME
		struct tm *local = NULL;
#ifndef VXWORKS		
		struct tm *localtime_r(const time_t * clock, struct tm * res);
#endif 
		struct tm tst;
#else /* !THREAD_SAFE_TIME */
	  struct tm *local, *localtime();
#endif
		time_t t;
		int local_hours;
		
/* 
 *			Check difference with GMT
 */
		
		t = (time_t)24*60*60; /* pretend jan 2 1970 */
#ifdef THREAD_SAFE_TIME
		localtime_r(&t, &tst);
		local = &tst;
#else /* !THREAD_SAFE_TIME */
		local = localtime(&t);
#endif
		local_hours = (24 *365 * (local->tm_year - 70)) + 
			(24 * local->tm_yday) + local->tm_hour;
/*
 *			If it's more than 25 hours different -- they're 
 *			cheating
 */
		if ((local_hours > 50) || (local_hours < 0)) 
		{
			LM_SET_ERRNO(job, LM_BAD_TZ, 182, 0);
			return(LM_BAD_TZ);
		}
	}

	if (vendor_key)
	{
		if (vendor_key->type == VENDORCODE_BIT64)
		{
			(void) bcopy((char *) vendor_key, 
				(char *) &(job->code), 
						sizeof(VENDORCODE1));
			LM_SET_ERRNO(job, LM_NOKEYDATA, 183, 0);
			return(NOKEYDATA);
		}
		else if (vendor_key->type == VENDORCODE_BIT64_CODED ||
			 vendor_key->type == VENDORCODE_3)
		{
			LM_SET_ERRNO(job, LM_OLDVENDORDATA, 184, 0);
			return(LM_OLDVENDORDATA);
		}
		else if (vendor_key->type == VENDORCODE_4)
		{
			short version = 0, revision = 0;
			extern void (*L_UNIQ_KEY5_FUNC)();


			l_get_attr(job, LM_A_VERSION, &version);
			l_get_attr(job, LM_A_REVISION,  &revision);
			
			if ((vendor_key->flexlm_version != version) ||
			    (vendor_key->flexlm_revision != revision))
			{
				LM_SET_ERRNO(job, LM_LIBRARYMISMATCH, 235, 0);
			}
			memcpy(&job->code, vendor_key, sizeof(VENDORCODE));
			if (!vendor_key->keys[0] && !vendor_key->keys[1] &&
					!vendor_key->keys[2])
			{
				LM_SET_ERRNO(job, LM_BADKEYDATA, 543, 0);
				return(LM_BADKEYDATA);
			}
			if (ret = l_getattr_init(job, vendor_key, vendor_id))
				return(ret);
/*-
 *				This means:  If you're not using lc_new_job,
 *				and this is not a demo vendor daemon, then
 *				it's safe to check the seeds, since this
 *				is not being shipped to a customer
 *				Therefore, this should get checked in the
 *				license generator...
 */
			if (!(job->options->flags & 
						LM_OPTFLAG_CUSTOM_KEY5)
				&& !L_STREQ(job->vendor, "demo") 
				&& (l_getattr(job, LMADMIN_API) != 
					LMADMIN_API_VAL))
			{
				memcpy(&vc, &job->code, sizeof(vc));
				l_sg(job, job->vendor, &vc);
				if ((vc.data[0] == 0x87654321) 
				 ||(vc.data[1] == 0x12345678 ))
				{
					LM_SET_ERRNO(job, LM_DEFAULT_SEEDS, 318, 0);
				}
			}
			memset(&vc, 0, sizeof(vc));
		}
		else
		{
			LM_SET_ERRNO(job, LM_BADVENDORDATA, 186, 0);
			return(LM_BADVENDORDATA);
		}
	}
	else
	{
		LM_SET_ERRNO(job, LM_NOKEYDATA, 187, 0);
	}
	saved_lm_errno = job->lm_errno;
	saved_uerrno = job->u_errno;
	saved_minor = job->errno_minor;
/*
 *		Set some globals, if this is a mini-FLEXlm kit
 */
	if ((saved_lm_errno != EXPIREDKEYS) &&
	    (saved_lm_errno != BADPLATFORM) &&
	    (saved_lm_errno != BADKEYDATA) &&
	    (saved_lm_errno != NOKEYDATA) )
	{
		if (l_getattr(job, FULL_FLEXLM) != FULL_FLEXLM_VAL)
		{
			cur_options->check_interval = -1;
			cur_options->retry_interval = -1;
		}
	}
/*
 *		Add options from VENDORCODE struct
 *		This must come after all default settings.
 */
	l_set_attr(job, LM_A_BEHAVIOR_VER, (LM_A_VAL_TYPE)(vendor_key->behavior_ver));
/*
 *		Any tests dependent on l_getattr must be done here 
 */
	{
		static char *	flexlm_batch = NULL;
		static char *	flexlm_no_ckout_install_lic = NULL;
		char *			cp = NULL;
		if ((lm_borrow == NULL) || l_reset_env)
		{
			if (lm_borrow)
				free(lm_borrow);
			lm_borrow = NULL;
			cp = l_getenv(job, "LM_BORROW");	/* overrun checked */
			if (cp && *cp) 
			{
				lm_borrow = (char *)l_malloc(job, strlen(cp) + 1);
				strcpy(lm_borrow, cp);
			}
				
		}
		if ((flexlm_batch == NULL) || l_reset_env)
		{
			if (flexlm_batch)
			{
				free(flexlm_batch);
			}
			flexlm_batch = NULL;
			cp = l_getenv(job, "FLEXLM_BATCH");	/* overrun checked */
			if (cp && *cp) 
			{
				flexlm_batch = (char *)l_malloc(job, strlen(cp) + 1);
				strcpy(flexlm_batch, cp);
			}
		}
                if ((flexlm_no_ckout_install_lic == NULL) || l_reset_env)
		{
			if (flexlm_no_ckout_install_lic)
			{
				free(flexlm_no_ckout_install_lic);
				flexlm_no_ckout_install_lic = 0;
			}
			flexlm_no_ckout_install_lic = 0;
			cp = l_getenv(job, "FLEXLM_NO_CKOUT_INSTALL_LIC");	/* overrun checked */
			if (cp && *cp) 
			{
				flexlm_no_ckout_install_lic = (char *)l_malloc(job, strlen(cp) + 1);
				strcpy(flexlm_no_ckout_install_lic, cp);
			}
		}
		if (flexlm_batch)
        {
			cur_options->flags &= ~LM_OPTFLAG_PC_PROMPT_FOR_FILE;
			cur_options->flags &= ~LM_OPTFLAG_PERROR_MSGBOX;
        }
		if (flexlm_no_ckout_install_lic)
		{
			cur_options->flags &= ~LM_OPTFLAG_CKOUT_INSTALL_LIC;
		}
		if (lm_borrow)
		{
			/*
			 *	Convert the envir var to LM_A_BORROW_EXPIRE format
			 */
			char *buf = NULL;
			char *date = NULL;
			char *exp = NULL;
			char *vendor = NULL;
			time_t t = 0;
			time_t datet = 0;
			struct tm datetm, todaytm, *tm;
#ifdef THREAD_SAFE_TIME
			struct tm tst;
#ifndef VXWORKS		
			struct tm *localtime_r(const time_t * clock, struct tm * res);
#endif 
#endif

			buf = l_malloc(job, strlen(lm_borrow) + 1);
			strcpy(buf, lm_borrow);
			date = buf;
			if (!(vendor = strchr(date, ':')))
				goto novalidborrowenv;
			*vendor++ = 0; /* null terminate date */
			datet = (time_t)l_date_to_time(job, date);
/*
 *			Since localtime returns data static to it, 
 *			to compare times we must make copies
 */
#ifdef THREAD_SAFE_TIME
			localtime_r(&datet, &tst);
			tm = &tst;
#else /* !THREAD_SAFE_TIME */
			tm = localtime(&datet);
#endif 
			memcpy(&datetm, tm, sizeof(datetm));
			t = time(0);
#ifdef THREAD_SAFE_TIME
			localtime_r(&t, &tst);
			tm = &tst;
#else /* !THREAD_SAFE_TIME */
			tm = localtime(&t);
#endif
			memcpy(&todaytm, tm, sizeof(todaytm));
			if ((datetm.tm_yday != todaytm.tm_yday) ||
					(datetm.tm_year != todaytm.tm_year))
			{
				goto novalidborrowenv;
			}
			if (!*vendor)
				goto novalidborrowenv;
			if (!(exp = strchr(vendor, ':')) )
				goto novalidborrowenv;
			*exp++ = 0; /* terminate vendor */
			if ((!l_keyword_eq(job, vendor, vendor_id) 
				&& !l_keyword_eq(job, "all", vendor)) 
				|| !*exp)
			{
				goto novalidborrowenv;
			}
			/* finally! */

			l_set_attr(job, LM_A_BORROW_EXPIRE, (LM_A_VAL_TYPE)exp);
novalidborrowenv:
			l_free(buf);
		}
	}
	if (l_getattr(job, LM_SET_ATTR) != LM_SET_ATTR_VAL)
		cur_options->retry_count = -1; /* big number */
	cur_options->capacity = -1; /* not set */
/*
 *		He may change the license file after the lc_init() call,
 *		so an error from l_init_file is not important compared to
 *		an error from initializing the FLEXlm vendor keys.
 */
	LM_SET_ERRNO(job, saved_lm_errno, saved_minor, saved_uerrno);

#if defined (DGUX) || defined (MOTO_88K)
	{
		int network = sysconf(_SC_BSDNETWORK);
		if (network == -1)
		{
			LM_SET_ERRNO(job, LM_NONETWORK, 189, errno);
		}
	}
#endif
/*
 *	OK, now link this job into the list for this process
 */
	if (job)
	{
		if (oldjob)
		{
			if (oldjob->next)
				job->next = oldjob->next;
			oldjob->next = job;
			job->first_job = oldjob->first_job;
		}
		else
		{
			job->first_job = job;
		}
	}
	
	job->port_end = LMGRD_PORT_END;
	
	{
		LM_VENDORCODE_PUBKEYINFO *	p = NULL;
		for (i = 0, p = &job->code.pubkeyinfo[0]; 
			(i < LM_MAXSIGNS) && p->pubkeysize[0] ; i++, p++)
		{
			p->strength = vendor_key->strength;
			l_add_key_filter(job, 
				p->strength >= LM_STRENGTH_PUBKEY ?
					(char *)p->pubkey_fptr : 0, 
				p->strength >= LM_STRENGTH_PUBKEY ?
					(char *)p->pubkey_fptr : 0, 
					p->sign_level, LM_KF_DIG_SIG,
					p->pubkeysize,
					p->pubkey, 
					p,
					p->strength);
			sign_level++;
		}
	}
	if ((char *)sign_level > job->L_SIGN_LEVEL)
		job->L_SIGN_LEVEL = (char *)sign_level;

	return(job->lm_errno);
}

void (*L_UNIQ_KEY5_FUNC)() = 0;

void
L_SET_KEY5_FUNC( void (*f)())
{
	if (!L_UNIQ_KEY5_FUNC)
		L_UNIQ_KEY5_FUNC = f;
}


/*
 *----------------------------------------------------------------------------
 *	Get dynamic event flags for VMS
 */

#ifdef VMS

#define FPC 32  /* # of event flags per cluster */


/*
 *      samec() - Returns 1 if the two event flags are in the same cluster.
 */

static
int
samec(int f1, int f2)
{
  int c1 = f1/FPC, c2=f2/FPC;

	return (c1 == c2);
}

/*
 *	Allocate the VMS event flags for our use
 */

static int _flex_efs_allocated = 0;

static
void
get_vms_ef()
{
  int i, err, ef[4];

/*
 *      We can get called lots of times now with multiple jobs,
 *      so protect against allocating more than we need of this
 *      precious, finite resource.
 */
        if (!_flex_efs_allocated)
                _flex_efs_allocated = 1;
        else
                return;

/*
 *      Create event flags for our internal use.  We need 4, so we are
 *      assured of getting 2 in the same cluster for READ_EF and TIMEOUT_EF
 */
	for (i=0; i< 4; i++)
	{
		err = lib$get_ef(&ef[i]);
		if (err && 0x1 == 0)
		{
			ef[0] = 63;
			ef[1] = 62;
			ef[2] = 61;
			ef[3] = 60;
		}
	}
	if (samec(ef[0], ef[1]))
	{
		READ_EF = ef[0]; TIMEOUT_EF = ef[1]; TIMER_EF = ef[2];
		TEMP_EF = ef[3];
	}
	else if (samec(ef[0], ef[2]))
	{
		READ_EF = ef[0]; TIMEOUT_EF = ef[2]; TIMER_EF = ef[1];
		TEMP_EF = ef[3];
	}
	else if (samec(ef[0], ef[3]))
	{
		READ_EF = ef[0]; TIMEOUT_EF = ef[3]; TIMER_EF = ef[1];
		TEMP_EF = ef[2];
	}
	else if (samec(ef[1], ef[2]))
	{
		READ_EF = ef[1]; TIMEOUT_EF = ef[2]; TIMER_EF = ef[0];
		TEMP_EF = ef[3];
	}
	else if (samec(ef[1], ef[3]))
	{
		READ_EF = ef[1]; TIMEOUT_EF = ef[3]; TIMER_EF = ef[0];
		TEMP_EF = ef[2];
	}
	else if (samec(ef[2], ef[3]))
	{
		READ_EF = ef[2]; TIMEOUT_EF = ef[3]; TIMER_EF = ef[0];
		TEMP_EF = ef[1];
	}
	else
	{
		printf("FLEXlm internal event flag error: flags: %d %d %d %d\n",
					ef[0], ef[1], ef[2], ef[3]);
		READ_EF = 63;
		TIMEOUT_EF = 62;
		TIMER_EF = 61;
		TEMP_EF = 60;
	}
	err = lib$get_ef(&L_TIMER_EF);
	if (err && 0x1 == 0) L_TIMER_EF = 59;
}
#endif	/* VMS */

#ifndef NO_FLEXLM_CLIENT_API
/*
 *	lc_first_job -- returns job's pointer the first job
 */
LM_HANDLE * API_ENTRY
lc_first_job(LM_HANDLE * job)
{
	return job->first_job;
}

/*
 *	lc_next_job -- returns job's pointer the next job
 *		NOTE:   The "job" arg MUST be the job returned by
 *			lc_first_job or lc_next_job
 */
LM_HANDLE * API_ENTRY
lc_next_job(LM_HANDLE * job)
{
	return job->next;
}
#endif /* NO_FLEXLM_CLIENT_API */

void
lc_add_key_filter(
	LM_HANDLE *		job,
	char *			app_filter,
	char *			gen_filter,
	int				sign_level,
	unsigned int	flags)
{
	if (LM_API_ERR_CATCH)
		return;
	l_add_key_filter(job, app_filter, gen_filter,  sign_level, flags,
		0, 0, 0, 0);
	LM_API_RETURN_VOID()
}

#ifdef PC
char *l_gcspim;
#endif
