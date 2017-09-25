/******************************************************************************

	    COPYRIGHT (c) 1995, 2002 by Macrovision Corporation.
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
 *	Module: $Id: lmpolicy.c,v 1.18 2003/04/18 23:48:08 sluu Exp $
 *
 *	Function:	simple, policy-based API interface
 *
 *	D. Birns
 *	7/30/95
 *
 *	Last changed:  10/15/98
 *
 */
#include "lmachdep.h"
#define LMPOLICY_C
#include "lmpolicy.h"
#include "lm_attr.h"
#include "l_prot.h"
#include <errno.h>
#ifdef UNIX /* for DISPLAY setting */
#include <sys/types.h>
#ifndef NO_UIO_H
#include <sys/uio.h>
#endif
#include "l_socket.h" 
#endif /* UNIX */
#ifdef PC
#include <windows.h>
#include <winsock.h>
#endif

#ifdef VXWORKS
#include "l_socket.h" 
#endif

static LP_HANDLE lpdummy;
static void stub lm_args((lm_noargs));
void (*lp_key2_ptr)();

typedef LM_PFV (*SIGPTR)();
/*
 *	lp_checkout
 */
int API_ENTRY
lp_checkout(lpcode, policy, feature, version, nlic, licpath, lpp)
LPCODE_HANDLE *lpcode;
int policy;
char *feature;
char *version;
int nlic;
char *licpath;
LP_HANDLE **lpp; 	/* returns handle */
{
  LM_HANDLE *job;
  LP_HANDLE *lp_handle;
  int rc;
  int queue = LM_CO_NOWAIT;
  CONFIG *c = (CONFIG *)0;
  char *display_env;
  char display_buf[MAX_HOSTNAME + 5];
  char ipaddr[MAX_HOSTNAME + 5];
  int initrc;
  int retry_count = 0;

/*
 *	failsafe
 */
	*lpp = &lpdummy;
#if (defined (WINNT) && !defined(FLEX_STATIC)) || defined(PC16)
        initrc = lc_init((LM_HANDLE *)0, lpcode->vendor_name, lpcode->code,
#else
        initrc = lc_new_job((LM_HANDLE *)0, 0, lpcode->code,
#endif
								&job);
	lpdummy.job = job;
/*
 *	return if lc_init fails
 */
        if (!initrc || (initrc == LM_DEFAULT_SEEDS))
        {
                ;
        }
        else return initrc; 

	if (!(lp_handle = (LP_HANDLE *)calloc(1, sizeof (LP_HANDLE))))
		return LM_CANTMALLOC;
	*lpp = lp_handle;
	lp_handle->job = job;
	lp_handle->policy = policy;

/*
 *	Set attributes
 */
	if (lp_key2_ptr) (*lp_key2_ptr)();
	if (licpath && *licpath)
		l_set_license_path(job, licpath, LM_A_LICENSE_DEFAULT);

	if (!(policy & LM_RETRY_RESTRICTIVE))
	{
		/* retry forever */
		job->options->retry_count = -1;
	} 
	if (policy & LM_CHECK_BADDATE) 
	{
		job->options->flags |= LM_OPTFLAG_CHECK_BADDATE;
	}
	if (policy & LM_USE_LICENSE_KEY) 
	{
		job->L_SIGN_LEVEL = 0;
		job->flags |= LM_FLAG_MAKE_OLD_KEY;
		job->flags &= ~LM_FLAG_MAKE_PUBLIC_KEY; 
	}
#ifdef WINNT
	if (policy & LM_FLEXLOCK)
	{
                job->options->flags |= LM_OPTFLAG_FLEXLOCK;
	}
#endif
		
	if (policy & LM_MANUAL_HEARTBEAT)
	{
		job->options->check_interval = -1;
		job->timer = 0;
		job->options->retry_interval = -1;
		job->options->setitimer = (LM_PFV) stub;
		job->options->sighandler = (SIGPTR) stub;
	} 
	else
	{
		job->daemon->udp_timeout = 4*60;
	}
	job->options->flags |= LM_OPTFLAG_RETRY_CHECKOUT;

/* 
 *	Set up the display name
 */

#ifdef PC
	display_env = "unix:0";  /* later we turn this into ip address */
#else
	if (display_env = getenv("DISPLAY"))
#endif
	{
	  char *suffix_ptr, suffix[30];

	  struct hostent *host;

		*suffix = 0;
		l_zcp(display_buf, display_env, sizeof(display_buf) - 1);
#if !(defined( VMS) || defined(PC) )

		suffix_ptr = strchr(display_buf, ':');
		if (suffix_ptr)
		{
			*suffix_ptr = 0;
			suffix_ptr++;
			strcpy(suffix, suffix_ptr);
		}
/*
 *		Check for unix:0.0 or :0.0
 */
		if (!strcmp(display_buf, "unix") || !*display_buf)
		{
			*display_buf = 0;
			gethostname(display_buf, MAX_HOSTNAME);
		}
retry:
		if (!l_ipaddr(display_buf))
		{
			host = gethostbyname(display_buf);
			if (host != NULL)
			{
				sprintf(ipaddr, "%d.%d.%d.%d", 
				(int)(unsigned char)host->h_addr_list[0][0],
				(int)(unsigned char)host->h_addr_list[0][1],
				(int)(unsigned char)host->h_addr_list[0][2],
				(int)(unsigned char)host->h_addr_list[0][3]);
				if (++retry_count < 3 &&
#if 1
				  !strcmp(ipaddr, "127.0.0.1"))
#else /* for testing purposes */
				  (!strcmp(ipaddr, "192.156.198.180")
				|| !strcmp(ipaddr, "192.156.198.19")))
#endif
				{
					if (!gethostname(display_buf, MAX_HOSTNAME))
						goto retry;
					else
						l_zcp(display_buf, display_env,
							sizeof(display_buf));
				}
				else
				{
					strcpy(display_buf, ipaddr);
				}
			}
		}
#endif /* !VMS && !PC */
#ifndef PC
		l_zcp(job->options->display_override, display_buf, 
						MAX_DISPLAY_NAME - 1);
#endif /* PC */
	}
	


	switch(policy & 0xff)
	{
	case LM_FAILSAFE: 	queue = LM_CO_QUEUE;  break;
	case LM_QUEUE:		queue = LM_CO_WAIT; break;
	}

	rc = lc_checkout(job, feature, version, nlic, queue, lpcode->code, 
							LM_DUP_NONE);
	if (!rc) 
	{
		c = lc_auth_data(job, feature);
	}
	if (c)
	{
		l_zcp(lp_handle->feature, c->feature, MAX_FEATURE_LEN);
		l_zcp(lp_handle->lickey, c->code, MAX_CRYPT_LEN);
	}
	else
	{
		l_zcp(lp_handle->feature, feature, MAX_FEATURE_LEN);
	}
	if (rc && !job->lm_errno) 
	{
		LM_SET_ERRNO(job, rc, 286, 0);
	}
	switch(policy & 0xff)
	{
	case LM_LENIENT:
		if (!rc) break;
		if (rc != LM_MAXUSERS)
		{
			job->err_info.warn = job->lm_errno;
			job->lm_errno = 0;
			rc = 0;
		} 
		break;
	case LM_FAILSAFE: 
		if (!rc) break;
		job->err_info.warn = job->lm_errno;
		job->lm_errno = 0;
		rc = 0; 
		break;
	}

	if (initrc && !rc) return initrc;
	return rc;
}
void
API_ENTRY
lp_checkin(lp_handle)
LP_HANDLE *lp_handle;
{
	if (!lp_handle || !lp_handle->job )
		return;
	if (lp_handle == &lpdummy) 
		return;

	lc_checkin(lp_handle->job, lp_handle->feature, 0);
	lc_free_job(lp_handle->job);
	memset(lp_handle, 0, sizeof(LP_HANDLE)); 
	free(lp_handle);
	return;
}
/*
 *	lp_heartbeat Calls lc_heartbeat.
 */
int
API_ENTRY
lp_heartbeat(lp_handle, ret_num_reconnects, minutes)
LP_HANDLE *lp_handle;
int *ret_num_reconnects;
int minutes;
{
	return lc_heartbeat(lp_handle->job, ret_num_reconnects, minutes);
}
LM_CHAR_PTR
API_ENTRY
lp_warning(lp_handle)
LP_HANDLE *lp_handle;
{
	if (!lp_handle->job->err_info.warn)
		return 0;
	return lc_errstring(lp_handle->job);
}

LM_CHAR_PTR
API_ENTRY
lp_errstring(lp_handle)
LP_HANDLE *lp_handle;
{
	if (!lp_handle->job) return "no valid job";
	return lc_errstring(lp_handle->job);
}	
void
API_ENTRY
lp_perror(lp_handle, str)
LP_HANDLE *lp_handle;
LM_CHAR_PTR str;
{
	lc_perror(lp_handle->job, str);
}
void
API_ENTRY
lp_pwarn(lp_handle, str)
LP_HANDLE *lp_handle;
LM_CHAR_PTR str;
{
	if (lp_handle->job->err_info.warn)
		lc_perror(lp_handle->job, str);
}


static
void
stub()
{}
