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
 *	Module: $Id: lm_set_attr.c,v 1.66 2003/05/05 16:10:55 sluu Exp $
 *
 *	Function:	lc_set_attr(job, key, value) - Set a FLEXlm attribute.
 *
 *	Description: 	Sets an attribute in the specified job
 *			(see lm_attr.h for attribute keys)
 *
 *	M. Christiano
 *	4/17/90
 *
 *	Last changed:  12/16/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_attr.h"
#include "lgetattr.h"
#include "l_timers.h"
#include <stdio.h>

#ifdef EMBEDDED_FLEXLM
#define ITIMER_REAL 0
#define ITIMER_VIRTUAL 1
#endif  /* EMBEDDED_FLEXLM */

#ifdef PC
#include <sys/types.h>
/* 
 * Suppress Microsoft Compiler warning against function parameter list 
 * mismatch.  All reference if PFI would cause such warning.
 */
#pragma warning ( disable : 4113 )
#elif defined(USE_SYS_TIMES_H)
#include <sys/times.h>
#else
#include <sys/time.h>
#endif

typedef int (*PFI)();
typedef char * (*PCSTAR)();
typedef LM_PFV (*SIGPTR)();


typedef HOSTID_PTR  (LM_CALLBACK_TYPE *PHOSTID)lm_args((short idtype));
typedef int  (*CRYPT_FUNC) lm_args((LM_VOID_PTR, LM_U_CHAR_PTR, int,int));
typedef void  (*CRYPT_FUNC_GEN) lm_args((LM_VOID_PTR, LM_U_CHAR_PTR, int));

static void remove_dups_in_path	lm_args((LM_HANDLE *, char *));


#ifdef VMS
extern int lm_vms_set_max_links();
#endif /* VMS */

		
int
API_ENTRY
lc_set_attr(
	LM_HANDLE *		job,		/* Current license job */
	int				key,
	LM_A_VAL_TYPE	value)
{
	if (LM_API_ERR_CATCH)
		return job->lm_errno;
	LM_API_RETURN(int, l_set_attr(job, key, value))
}


int
API_ENTRY
l_set_attr(
	LM_HANDLE *		job,		/* Current license job */
	int				key,
	LM_A_VAL_TYPE	value)  
{
	char *cstar = (char *) value;
	int error = 0;
	int i = (int)(value);
	short sh = (short)(i & 0xffff);

	if (job == (LM_HANDLE *) NULL || 
	    job->type != LM_JOB_HANDLE_TYPE)
	{
		return(NOFLEXLMINIT);
	}
	if (l_getattr(job, LM_SET_ATTR) != LM_SET_ATTR_VAL)
	{
		LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 204, 0);
		return (LM_FUNCNOTAVAIL);
	}

	/*if (key != LM_A_LONG_ERRMSG) l_clear_error(job);*/

	switch(key)
	{

		case LM_A_BORROW_EXPIRE:
		{
			long exp = 0;
			if (exp = l_borrow_string_to_time(job, cstar))
			{
				/*
				 *	Check to make sure this time hasn't already elapsed.
				 *	This fixes P7138.
				 */
				time_t	now = time(&now);
				if(now < exp)
				{
					job->borrow_linger_minutes = (exp - time(0))/60;
				}
			}
			else
			{
				LM_SET_ERROR(job, LM_BADPARAM, 569, 0, cstar ? cstar : "BORROW_EXPIRE null", LM_ERRMASK_ALL);
				return (LM_BADPARAM);
			}
			break;
		}

		case LM_A_BORROW_OK:
			if (sh)
			{
				/* only allow disabling */
				LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 568, 0);
				return (LM_FUNCNOTAVAIL);
			}
			else
				job->options->flags &= ~LM_OPTFLAG_BORROW_OK;
			break;

		case LM_A_BORROW_STRING:
			job->L_BORROW_STRING = cstar;
			break;

		case LM_A_CAPACITY:
			job->options->capacity = sh;
			break;

		case LM_A_CHECKOUTFILTER:
			job->options->outfilter = (PFI) value;
			break;

		case LM_A_CHECKOUTFILTER_EX:
			job->options->outfilter_ex = (PFI) value;
			break;

		case LM_A_CHECKOUTFILTERLAST_EX:
			job->options->outfilter_last_ex = (PFI) value;
			break;

		case LM_A_CHECK_BADDATE:
			if (sh)
				job->options->flags |= LM_OPTFLAG_CHECK_BADDATE;
			else
				job->options->flags &= ~LM_OPTFLAG_CHECK_BADDATE;
			break;
		
		case LM_A_CHECK_INTERVAL:
			if ((int) value < 0 || (int) value >= 30 ||
				l_getenv(job, "FLEXLM_INTERVAL_OK") ||	/* overrun checked */
				((job->options->timer_type == 
					LM_VIRTUAL_TIMER) && (int) value > 0))
			{
				job->options->check_interval=(int)value;
				/*
				 *	if there already is a timer, reset the timer
				 */
				if (job->timer) 
				{
					int tim = (int) value;
					/*
					 *	Note that 0 turns off timers, so
					 *	we convert negative to 0
					 */
					if ((int)value < 0)
						tim = 0;

					l_timer_change(job, job->timer, 
						(int) LM_TIMER_UNSET,
						(int) tim * 1000, 
						(FP_PTRARG) lc_timer, 
						LM_TIMER_CHECK,
						(int) value * 1000);
				}
				/* 
				 *	turning off timers when value <= 0
				 */
				if ((int)value <= 0 ) 
					job->timer = (LM_TIMER) 0;
						
#ifdef RELEASE_VERSION
				else
				{
					LM_SET_ERRNO(job, LM_BADPARAM, 205, 0);
					error = job->lm_errno;

				}
#endif
			}
			break;

		case LM_A_CHECKOUT_DATA:
			if (cstar == (char *) NULL)
				cstar = "";
			(void)strncpy(job->options->vendor_checkout_data, 
					cstar , MAX_VENDOR_CHECKOUT_DATA);
			job->options->vendor_checkout_data[MAX_VENDOR_CHECKOUT_DATA] = '\0';
			break;

		case LM_A_COMM_TRANSPORT:
			job->options->commtype = sh;
			if (value == (LM_A_VAL_TYPE)LM_UDP)
				job->daemon->udp_sernum = 0;
			job->options->transport_reset = LM_RESET_BY_APPL;
			break;

		case LM_A_BEHAVIOR_VER:
			l_zcp(job->options->behavior_ver, (char *) value, 
				LM_MAX_BEH_VER);
			if (strcmp( job->options->behavior_ver, 
							LM_BEHAVIOR_V7_1) < 0)
			{
				l_set_attr(job, LM_A_INTERNAL2, (LM_A_VAL_TYPE)1);
				l_set_attr(job, LM_A_SIGN_LEVEL, (LM_A_VAL_TYPE)0);
			}
			if (strcmp( job->options->behavior_ver, 
							LM_BEHAVIOR_V6) < 0)
			{
				job->options->flags |= 
						LM_OPTFLAG_LKEY_START_DATE;
				job->options->flags |= 
						LM_OPTFLAG_USE_START_DATE;
				job->options->flags &= 
						~LM_OPTFLAG_RETRY_CHECKOUT;
				L_SECLEN_SET_LONG;
				job->options->flags &= ~LM_OPTFLAG_LONG_ERRMSG;
				job->options->flags |= 
					LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
				job->options->flags |= 
					LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
			}
			else
			{
				job->options->flags &= 
						~LM_OPTFLAG_LKEY_START_DATE;
				job->options->flags &= 
						~LM_OPTFLAG_USE_START_DATE;
				job->options->flags |= 
						LM_OPTFLAG_RETRY_CHECKOUT;
				L_SECLEN_SET_SHORT;
				job->options->flags |= LM_OPTFLAG_LONG_ERRMSG;
				job->options->flags &= 
					~LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
			}
			if (strcmp( job->options->behavior_ver, 
							LM_BEHAVIOR_V7) < 0)
			{
				job->options->flags &= 
						~LM_OPTFLAG_CKOUT_INSTALL_LIC;
			}
			else
			{
				job->options->flags |= 
						LM_OPTFLAG_CKOUT_INSTALL_LIC;
			}
			break;

		case LM_A_CKOUT_INSTALL_LIC:
			if ((int)value)
			{
				job->options->flags |= LM_OPTFLAG_CKOUT_INSTALL_LIC;
			}
			else
			{
				job->options->flags &= ~LM_OPTFLAG_CKOUT_INSTALL_LIC;
			}
			break;

		case LM_A_CONN_TIMEOUT:
			if ((int) value >= 0)
				job->options->conn_timeout = (int) value;
			else
			{
				LM_SET_ERRNO(job, LM_BADPARAM, 206, 0);
				error = job->lm_errno;
			}
			break;

		case LM_A_CRYPT_CASE_SENSITIVE:
			job->options->crypt_case_sensitive = sh;
			break;


		case LM_A_DIAGS_ENABLED:
			if (sh)
				job->options->flags |= LM_OPTFLAG_DIAGS_ENABLED;
			else
				job->options->flags &= ~LM_OPTFLAG_DIAGS_ENABLED;
			break;
		
#if 0
		case LM_A_DIG_SIG_AUTH:
			l_add_key_filter(job, value, 0, 1, LM_KF_DIG_SIG, 0, 
								0, 0);
			job->flags |= LM_FLAG_KEY_NEW; 
			job->flags &= ~(LM_FLAG_KEY_OLD|LM_FLAG_MAKE_OLD_KEY); 
			break;

		case LM_A_DIG_SIG_GEN:
			l_add_key_filter(job, 0, value, 1, LM_KF_DIG_SIG, 0, 0, 
									0);
			job->flags |= LM_FLAG_KEY_NEW; 
			job->flags &= ~(LM_FLAG_KEY_OLD|LM_FLAG_MAKE_OLD_KEY); 
			break;
#endif

		case LM_A_DISPLAY_OVERRIDE:
			if (cstar == (char *) NULL)
				cstar = "";
			(void)strncpy(job->options->display_override, 
					cstar, MAX_DISPLAY_NAME);/* LONGNAMES */
			job->options->display_override[MAX_DISPLAY_NAME]
									= '\0';
			break;

		case LM_A_DISABLE_ENV:
		{
			int flush = 0;
			static char *use_finder = (char *)-1;

			if (use_finder == (char *)-1)
				use_finder = l_getenv(job, "FLEXLM_USE_FINDER");	/* overrun checked */

/*
 *			If we're changing the value, then flush
 */
			if ((job->options->disable_env != sh) &&
				job->line && job->lic_files)  
			{
				flush = 1;
			}
			job->options->disable_env = sh;

			if (use_finder)
				job->options->disable_finder = sh;

			if (flush) l_flush_config(job);
			break;
		}

		case LM_A_ETHERNET_BOARDS:
			job->options->ethernet_boards = (char **) value;
			break;


		case LM_A_HOSTID_PARSE:
			job->options->parse_vendor_id = (PFI) value;
			break;

		case LM_A_HOST_OVERRIDE:
			if (cstar == (char *) NULL)
				cstar = "";
			(void)strncpy(job->options->host_override, cstar, MAX_SERVER_NAME);/* LONGNAMES */
			job->options->host_override[MAX_SERVER_NAME] = '\0';
			break;

		case LM_A_INTERNAL1:
			if (sh)
				job->options->flags |= LM_OPTFLAG_P3130;
			else
				job->options->flags &= ~LM_OPTFLAG_P3130;
			break;


		case LM_A_SIGN_LEVEL:
			if (value)
			{
				job->flags &= ~LM_FLAG_MAKE_OLD_KEY;
				job->flags |= LM_FLAG_MAKE_PUBLIC_KEY; 
				job->L_SIGN_LEVEL = (char *)value;
			}
			else
			{
				job->flags |= LM_FLAG_MAKE_OLD_KEY;
				job->flags &= ~LM_FLAG_MAKE_PUBLIC_KEY; 
				job->L_SIGN_LEVEL = 0;
			}
			break;

		case LM_A_JAVA_LIC_FMT:
			if (sh)
			{
				job->options->flags |= LM_OPTFLAG_JAVA_LIC_FMT;
				job->options->flags |= 
						LM_OPTFLAG_USE_START_DATE;
				L_SECLEN_SET_LONG;
				job->options->flags |= 
					LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
			}
			else
			{
				job->options->flags &= ~LM_OPTFLAG_JAVA_LIC_FMT;
				job->options->flags &= 
						~LM_OPTFLAG_USE_START_DATE;
				L_SECLEN_SET_SHORT;
				job->options->flags &= 
					~LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
			}
			break;

		case LM_A_LCM:
			if (sh)
				job->options->flags |= LM_OPTFLAG_LCM;
			else
				job->options->flags &= ~LM_OPTFLAG_LCM;
			break;

		case LM_A_LICENSE_FMT_VER:
			l_zcp(job->options->license_fmt_ver, 
				(char *) value, LM_MAX_BEH_VER);
		
			if (strcmp( job->options->license_fmt_ver, 
							LM_BEHAVIOR_V6) < 0)
			{
				job->options->flags |= 
						LM_OPTFLAG_LKEY_START_DATE;
				job->options->flags |= 
						LM_OPTFLAG_USE_START_DATE;
				L_SECLEN_SET_LONG;
				job->options->flags |= 
					LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
			}
			else
			{
				job->options->flags &= 
						~LM_OPTFLAG_LKEY_START_DATE;
				job->options->flags &= 
						~LM_OPTFLAG_USE_START_DATE;
				L_SECLEN_SET_SHORT;
				job->options->flags &= 
					~LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
			}
			break;

		case LM_A_LICENSE_FILE:
		case LM_A_LICENSE_FILE_PTR:
		case LM_A_LICENSE_DEFAULT:
			{
				int ret = 0;
				if (ret = l_set_license_path(job, cstar, key))
					error = ret;
			}
			break;

		case LM_A_LKEY_LONG:
			if (sh)
			{
				L_SECLEN_SET_LONG;
			}
			else
			{
				L_SECLEN_SET_SHORT;
			}
			break;

		case LM_A_LKEY_START_DATE:
			if (sh)
				job->options->flags |=
					LM_OPTFLAG_LKEY_START_DATE;
			else
				job->options->flags &= 
					~LM_OPTFLAG_LKEY_START_DATE;
			break;

		case LM_A_LINGER:
			if (l_getattr(job, LINGERING) == LINGERING_VAL)
			{
				job->options->linger_interval = 
								(long) value;
				if (job->options->linger_interval < 0L)
				{
				      job->options->linger_interval = (long) 0;
				}
			}
			else
			{
				LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 208, 0);
				error = job->lm_errno;
			}
			break;

		case LM_A_LONG_ERRMSG:
			if (sh)
				job->options->flags |= LM_OPTFLAG_LONG_ERRMSG;
			else
				job->options->flags &= ~LM_OPTFLAG_LONG_ERRMSG;
			break;

		case LM_A_MAX_LICENSE_LEN:
			if (value > (LM_A_VAL_TYPE)MAX_CONFIG_LINE) 
				value = (LM_A_VAL_TYPE)MAX_CONFIG_LINE;
			job->options->max_license_len = sh;
			break;

		case LM_A_MAX_TIMEDIFF:
			job->options->max_timediff = (int) value;
			break;

		case LM_A_MT_HEARTBEAT:
			if (sh)
				job->options->flags |= LM_OPTFLAG_MT_HEARTBEAT;
			else
				job->options->flags &= ~LM_OPTFLAG_MT_HEARTBEAT;
			break;

		case LM_A_MT_CONNECT:
			if (sh)
				job->flags &= ~LM_FLAG_CONNECT_NO_THREAD;
			else
				job->flags |= LM_FLAG_CONNECT_NO_THREAD;
			break;

		case LM_A_NO_MT_CONNECT: /* for v8.0 compatibility */
			if (sh)
				job->flags |= LM_FLAG_CONNECT_NO_THREAD;
			else
				job->flags &= ~LM_FLAG_CONNECT_NO_THREAD;
			break;

		case LM_A_NO_TRAFFIC_ENCRYPT:
			job->options->no_traffic_encrypt = sh;
			break;

		case LM_A_NORMAL_HOSTID:
			job->options->normal_hostid = sh;
			break;

		case LM_A_PC_PROMPT_FOR_FILE:
			if (value)
			{
				job->options->flags |= 
					LM_OPTFLAG_PC_PROMPT_FOR_FILE;
			}
			else
			{
				job->options->flags &= 
					~LM_OPTFLAG_PC_PROMPT_FOR_FILE;
			}
			break;

		case LM_A_PERIODIC_CALL:
			job->options->periodic_call = (PFI) value;
			break;

		case LM_A_PERIODIC_COUNT:
			job->options->periodic_count = (int) value;
			job->options->periodic_counter = 0;			
			break;

		case LM_A_PERROR_MSGBOX:
			if (value)
			{
				job->options->flags |= 
					LM_OPTFLAG_PERROR_MSGBOX;
			}
			else
			{
				job->options->flags &= 
					~LM_OPTFLAG_PERROR_MSGBOX;
			}
			break;

		case LM_A_PHASE2_GEN:
			l_add_key_filter(job, 0, (char *)value, 
				(int)job->L_SIGN_LEVEL, 0, 0, 0, 0, 0);
			break;

		case LM_A_PHASE2_APP:
			if (l_getattr(job, LM_PHASE2_ATTR) != 
				LM_PHASE2_ATTR_VAL)
			{
				LM_SET_ERRNO(job, LM_NOCROSUPPORT, 538, 0);
				return job->lm_errno;
			}
			l_add_key_filter(job, (char *)value, 0, 
				(int)job->L_SIGN_LEVEL, 0, 0, 0, 0, 0);
			break;

		case LM_A_PLATFORM_OVERRIDE:
			if (cstar == (char *) NULL) cstar = "";
			l_zcp(job->options->platform_override, 
						cstar, MAX_PLATFORM_NAME);
			break;

		case LM_A_PUBKEY:
		{
			VENDORCODE *v = (VENDORCODE *)value;
			LM_VENDORCODE_PUBKEYINFO *p = NULL;
			int i = 0;

		  	for (i = 0, p = v->pubkeyinfo; i < LM_MAXSIGNS; i++, p++)
			{
				if (!p->pubkey_fptr)
					break;
				l_add_key_filter(job, p->strength >= LM_STRENGTH_PUBKEY ? (char *)p->pubkey_fptr : 0, 
					p->strength >= LM_STRENGTH_PUBKEY ?	(char *)p->pubkey_fptr : 0, 
					(int)p->sign_level, LM_KF_DIG_SIG,
					p->pubkeysize,
					p->pubkey,
					p,
					p->strength);
			}
		}
			break;
		case LM_A_STRENGTH_OVERRIDE:
			job->L_STRENGTH_OVERRIDE = (char *)i;
			break;

		case LM_A_REDIRECT_VERIFY:
			job->options->redirect_verify = (PFI) value;
			l_do_redir(job);
			break;

		case LM_A_RETRY_CHECKOUT:
			if (value)
			{
				job->options->flags |= 
					LM_OPTFLAG_RETRY_CHECKOUT;
			}
			else
			{
				job->options->flags &= 
					~LM_OPTFLAG_RETRY_CHECKOUT;
			}
			break;

		case LM_A_RETRY_COUNT:

			job->options->retry_count = (int) value;
			break;

		case LM_A_RETRY_INTERVAL:
			

			if ((int) value < 0 || (int) value >= 30 ||
				((job->options->timer_type == 
					LM_VIRTUAL_TIMER) && (int) value > 0) 
					|| l_getenv(job, "FLEXLM_INTERVAL_OK"))	/* overrun checked */
			{
				job->options->retry_interval = (int) value;
			}
			else
			{
				LM_SET_ERRNO(job, LM_BADPARAM, 209, 0);
				error = job->lm_errno;
			}
			break;

		case LM_A_SETITIMER:
			job->options->setitimer = (LM_PFV) value;
			break;

		case LM_A_SIGNAL:
			job->options->sighandler = (SIGPTR) value;
			break;

		case LM_A_LICENSE_CASE_SENSITIVE:
			if (value) 
			{
				job->options->flags |= 
					LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
			}
			else
			{
				job->options->flags &= 
					~LM_OPTFLAG_STRINGS_CASE_SENSITIVE;
			}
			break;

		case LM_A_SUPPORT_HP_IDMODULE:
			if (value) 
			{
				job->options->flags |= 
						LM_OPTFLAG_SUPPORT_HP_IDMODULE;
			}
			else
			{
				job->options->flags &= 
						~LM_OPTFLAG_SUPPORT_HP_IDMODULE;
			}
			break;

		case LM_A_TCP_TIMEOUT:
			if ((i >0) && (i < LM_TCP_TIMEOUT_INCREMENT)) 
			{
				LM_SET_ERRNO(job, LM_BADPARAM, 285, 0);
				error = job->lm_errno;
				job->daemon->tcp_timeout = (int) LM_TCP_TIMEOUT_INCREMENT;
			}
			else
				job->daemon->tcp_timeout = i;
			break;
		

		case LM_A_TIMER_TYPE:
#ifndef PC
			job->options->timer_type = (int)value;
/* 
 *				Handle the case of him setting 
 *				ITIMER_REAL or ITIME_VIRTUAL 
 */
			if ((int) value == ITIMER_REAL)
				job->options->timer_type = LM_REAL_TIMER;
			if ((int) value == ITIMER_VIRTUAL)
				job->options->timer_type =
							     LM_VIRTUAL_TIMER;
#endif
			break;

		case LM_A_TRY_COMM:
			if (sh)
				job->options->flags |= LM_OPTFLAG_TRY_OLD_COMM;
			else
				job->options->flags &= ~LM_OPTFLAG_TRY_OLD_COMM;
			break;

		case LM_A_PORT_HOST_PLUS:
			if (sh)
				job->options->flags |=
					LM_OPTFLAG_PORT_HOST_PLUS;
			else
				job->options->flags &= 
					~LM_OPTFLAG_PORT_HOST_PLUS;
			break;
		
		case LM_A_UDP_TIMEOUT:
			job->daemon->udp_timeout = (int) value;
			break;
		
		case LM_A_USE_START_DATE:
			if (sh)
				job->options->flags |=
					(LM_OPTFLAG_USE_START_DATE |
					LM_OPTFLAG_LKEY_START_DATE);
			else
				job->options->flags &= 
					~LM_OPTFLAG_USE_START_DATE;
			break;
			

		case LM_A_USER_EXITCALL:
			job->options->user_exitcall = (PFI) value;
			break;

		case LM_A_USER_EXITCALL_EX:
			job->options->user_exitcall_ex = (PFI) value;
			break;

		case LM_A_VENDOR_CALLBACK_DATA:
			job->options->pUserData = (void *)value;
			break;

		case LM_A_USER_RECONNECT:
			job->options->user_reconnect = (PFI) value;
			break;

		case LM_A_USER_RECONNECT_EX:
			job->options->user_reconnect_ex = (PFI) value;
			break;

		case LM_A_USER_RECONNECT_DONE:
			job->options->user_reconnect_done = (PFI) value;
			break;

		case LM_A_USER_RECONNECT_DONE_EX:
			job->options->user_reconnect_done_ex = (PFI) value;
			break;

		case LM_A_USER_CRYPT:
			job->options->user_crypt = (PCSTAR) value;
			break;

		case LM_A_USER_CRYPT_FILTER:
			job->user_crypt_filter = (CRYPT_FUNC) value;
			break;

		case LM_A_USER_CRYPT_FILTER_GEN:
			job->user_crypt_filter_gen = (CRYPT_FUNC_GEN) value;
			break;

		case LM_A_USER_OVERRIDE:
			if (cstar == (char *) NULL) cstar = "";
			(void)strncpy(job->options->user_override, 
						cstar, MAX_USER_NAME);/* LONGNAMES */
			job->options->user_override[MAX_USER_NAME] ='\0';
			break;

		case LM_A_V7_0_HANDSHAKE:
			if (value)
				job->options->flags |=
					LM_OPTFLAG_V7_0_HANDSHAKE;
			else
				job->options->flags &= 
					LM_OPTFLAG_V7_0_HANDSHAKE;
			break;
			
		case LM_A_VENDOR_CHECKID:
			job->options->check_vendor_id = (PFI) value;
			break;

		case LM_A_VENDOR_ID_DECLARE:
		{
			LM_VENDOR_HOSTID *h = NULL;

			h = (LM_VENDOR_HOSTID *)l_malloc(job, sizeof (LM_VENDOR_HOSTID));
			memcpy(h, (LM_VENDOR_HOSTID *)value, sizeof(LM_VENDOR_HOSTID));
			h->next = job->options->vendor_hostids;
			job->options->get_vendor_id = h->get_vendor_id;
			job->options->vendor_hostids = h;
			l_flush_config(job);
		}
			break;

		case LM_A_VENDOR_GETHOSTID:
			job->options->get_vendor_id = (PHOSTID) value;
			break;

		case LM_A_VENDOR_PRINTHOSTID:
			job->options->print_vendor_id = (PCSTAR) value;
			break;

		case LM_A_ALLOW_SET_TRANSPORT:
			job->options->allow_set_transport = (int) value;
			break;

		case LM_A_ALT_ENCRYPTION:
			(void)memcpy(&job->options->alt_vendorcode, 
					(char *)value, sizeof(VENDORCODE));
			break;

#ifdef WINNT
		case LM_A_FLEXLOCK:
			if (sh)
				job->options->flags |= LM_OPTFLAG_FLEXLOCK;
			else
				job->options->flags &= ~LM_OPTFLAG_FLEXLOCK;
			break;

		case LM_A_FLEXLOCK_INSTALL_ID:
            job->options->flexlock_install_id = (long)value;
            break;

		case LM_A_LCM_URL:

		    if (job->options->lcm_url)
		            l_free(job->options->lcm_url);
		    if (value)
		    {
		        job->options->lcm_url = (char *)l_malloc(job,
		                strlen((char *)value) + 1);
		        strcpy(job->options->lcm_url, (char *)value);
		    }
		    else
				job->options->lcm_url = 0;
		    break;

		case LM_A_WINDOWS_MODULE_HANDLE:
			job->options->windows_module_handle = (long) value;
			break;

#endif /* WINNT */



#ifdef VMS
		case LM_A_EF_1:
			FLEXLM_VMS_EF1 = value;
			break;

		case LM_A_EF_2:
			FLEXLM_VMS_EF2 = value;
			break;

		case LM_A_EF_3:
			FLEXLM_VMS_EF3 = value;
			break;

		case LM_A_EF_4:
			FLEXLM_VMS_EF4 = value;
			break;

		case LM_A_EF_5:
			FLEXLM_VMS_EF5 = value;
			break;

		case LM_A_VMS_MAX_LINKS:
			(void) lm_vms_set_max_links((int) value);
			break;

#endif
		default:
			LM_SET_ERRNO(job, LM_NOSUCHATTR, 210, 0);
			error = job->lm_errno;
			break;
	}
	return(error);
}

int API_ENTRY
l_set_license_path(
	LM_HANDLE *	job,		/* Current license job */
	char *		newpath,
	int			key)
{
	char *			oldfile = NULL;
	int				oldgot = 0;
	short			disable = 0, disable_finder = 0;
	int				saverrno = job->lm_errno;
	int				savu_errno = job->u_errno;
	int				savminor = job->errno_minor;
	int				error = 0;
	char *			envptr = (char *)NULL;
	char *			vdenvptr = (char *)NULL;
	static char *	diag = (char *)NULL;
	static char *	lm_license_file_env = (char *)NULL;
	static char *	vd_license_file_env = (char *)NULL;
	static char		lm_license_buffer[MAX_CONFIG_LINE * 2] = {'\0'};
	static char		vd_license_buffer[MAX_CONFIG_LINE * 2] = {'\0'};
	char			szTempBuffer[MAX_CONFIG_LINE * 2] = {'\0'};

/*
 *	If LM_LICENSE_FILE is set, warn user if they ask for it.
 */

	if (diag == (char *)NULL)
		diag = l_getenv(job, "FLEXLM_DIAGNOSTICS");	/* overrun checked */
	if ((lm_license_file_env == (char *)NULL) &&
		(lm_license_file_env = l_getEnvUTF8(job, LM_DEFAULT_ENV_SPEC, szTempBuffer, sizeof(szTempBuffer))))	
	{
		strcpy(lm_license_buffer, lm_license_file_env);
		lm_license_file_env = lm_license_buffer;
	}

	if (vd_license_file_env == (char *)NULL) 
	{
		char buf[100] = {'\0'};
		sprintf(buf, "%s_LICENSE_FILE", job->vendor);
		l_uppercase(buf);
		if (vd_license_file_env = l_getEnvUTF8(job, buf, szTempBuffer, 
									sizeof(szTempBuffer)))	/* overrun checked */
		{
			strcpy(vd_license_buffer, vd_license_file_env);
			vd_license_file_env = vd_license_buffer;
		}
	}
	disable = job->options->disable_env;
	disable_finder = job->options->disable_finder;
	oldfile = job->options->config_file;
	oldgot = job->options->got_config_file;
	if (key == LM_A_LICENSE_DEFAULT)
	{
		job->options->disable_env = 1;
	}
	else
	{
/*
 *		These settings ensure we're testing ONLY the
 *		path they've specified
 */
		job->options->disable_env = 1;
		job->options->disable_finder = 1;
	}

	if (key == LM_A_LICENSE_DEFAULT && !disable)
	{
		envptr = lm_license_file_env;
		vdenvptr = vd_license_file_env;
		if ((envptr && *envptr) || (vdenvptr && *vdenvptr))
			job->options->disable_finder = 1;
		else
			vdenvptr = envptr = 0;
	}
			
	if (envptr || vdenvptr)
	{
		job->options->config_file = 
			(char *)l_malloc(job, (envptr ? strlen(envptr) : 0) +
				(vdenvptr ? strlen(vdenvptr) : 0) +	strlen(newpath) +3);
	}
	else
	{
		job->options->config_file = (char *)l_malloc(job, 
							strlen(newpath) + 1);
	}

	if (key == LM_A_LICENSE_DEFAULT && (envptr || vdenvptr))
	{
		char ps[2] = {'\0'};

		sprintf(ps, "%c", PATHSEPARATOR);
		sprintf(job->options->config_file, "%s%s%s%s%s", 
						vdenvptr ? vdenvptr : "",  
						vdenvptr ? ps : "",  
						envptr ? envptr : "",  
						envptr ? ps : "",  
						newpath);
	}
	else
		strcpy(job->options->config_file, newpath);

	job->options->got_config_file = 1;  

	if ((key !=  LM_A_LICENSE_DEFAULT) || job->lic_files)
	{
		l_flush_config(job); 	/* Try to read it */

		if (job->lm_errno != NOCONFFILE)  
		{
			if (key != LM_A_LICENSE_DEFAULT)
			{
				job->options->disable_env = disable;
				job->options->disable_finder = disable_finder;
/*
 *			If we are NOT disabling the environment variable or 
 *			the finder, re-flush the license file data, so that
 *			we will get the result of LM_LICENSE_FILE and/or the 
 *			finder.
 */
				if ((job->options->disable_env == 0) ||
				    (job->options->disable_finder == 0))
				{
					l_flush_config(job);
				}
			}
			/* set it back here */
			if ( job->lm_errno != LM_CANTMALLOC)
				LM_SET_ERRNO(job, saverrno, savminor, savu_errno);
			if (oldgot && oldfile)
				(void) free(oldfile);	/* Free old one */
		}
		else
		{
/*
 *		Couldn't find it -- put back the old one
 */
			if (job->options->config_file)
				(void) free(job->options->config_file);
			job->options->config_file = oldfile;
			job->options->got_config_file = oldgot;
			job->options->disable_env = disable;
			job->options->disable_finder = disable_finder;
			l_flush_config(job);
			LM_SET_ERRNO(job, LM_NOCONFFILE, 212, 0);
			error = LM_NOCONFFILE;
		}
	}
	if (!error)
	{
		remove_dups_in_path(job, job->options->config_file);
	}
	return(error);
}

void
API_ENTRY
lc_set_errno(
	LM_HANDLE *	job,
	int			err)
{
	LM_SET_ERRNO(job, err, 234, 0);
}

static
void
remove_dups_in_path(
	LM_HANDLE *	job,
	char *		path)
{
	char *	tmp = NULL, * otmp = NULL;
	char *	endword = NULL, * opathp = NULL;
	char *	nextiword = NULL, * nextoword = NULL;
	int		foundone = 0;

	if (!path || !*path)
		return;
  	tmp = l_malloc(job, strlen(path) + 1);
  	nextoword = otmp = l_malloc(job, strlen(path) + 2);
	strcpy(tmp, path);
	nextiword = tmp;
	if (endword = strchr(nextiword, PATHSEPARATOR)) *endword = 0; 
	while( nextiword)
	{
		char *	nextpword = NULL;
	
		for (opathp = endword ? endword + 1 : 0; opathp && *opathp; 
			opathp = nextpword)
		{
			int l = strlen(nextiword);
			nextpword = strchr(opathp + 1, PATHSEPARATOR); 
			if (L_STREQ_N(nextiword, opathp, l) &&  /* P4943 */
				(opathp[l] == 0 || &opathp[l+1] == nextpword) )
			{
				break;
			}
			
		}
		if (!opathp || !*opathp)
		{
			strcpy(nextoword, nextiword);
			nextoword += strlen(nextoword);
			*nextoword++ = PATHSEPARATOR;
			*nextoword = 0; 
			foundone = 1;
		}
		if (endword)
		{
			nextiword = endword + 1;
			if (endword = strchr(nextiword, PATHSEPARATOR)) 
				*endword = 0; 
		}
		else
			nextiword = 0;
	 }
	 if (foundone)
	 	nextoword[-1] = 0; /* remove last PATHSEPARATOR */
	strcpy(path, otmp);
	free(otmp);
	free(tmp);
}
