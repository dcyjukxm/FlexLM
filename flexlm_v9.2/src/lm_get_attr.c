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
 *	Module: $Id: lm_get_attr.c,v 1.25 2003/04/18 23:48:06 sluu Exp $
 *
 *	Function:	lc_get_attr(job, key, val) - Gets a FLEXlm attribute.
 *
 *	Description: 	Returns an attribute value.
 *			(see lm_attr.h for attribute keys)
 *
 *	M. Christiano
 *	5/3/90
 *
 *	Last changed:  9/17/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_attr.h"
#include "lgetattr.h"
typedef LM_PFV (*SIGPTR)(); 
typedef int (**PFI)(); 
typedef char * (**PCSTAR)();


typedef HOSTID_PTR  (LM_CALLBACK_TYPE **PPHOSTID)lm_args((short idtype));
typedef int  (**CRYPT_FUNC) lm_args((LM_VOID_PTR, LM_U_CHAR_PTR, int,int));
typedef void  (**CRYPT_FUNC_GEN) lm_args((LM_VOID_PTR, LM_U_CHAR_PTR, int));




#ifdef VMS
extern int lm_vms_max_links;
#endif /* VMS */

/* VARARGS1 */	/* Second arg can be different type */
API_ENTRY
lc_get_attr(job, key, val)
LM_HANDLE *job;		/* Current license job */
int key;
short *val;
{

	if (LM_API_ERR_CATCH) return job->lm_errno;
	LM_API_RETURN(int, l_get_attr(job, key,val))
}
API_ENTRY
l_get_attr(job, key, val)
LM_HANDLE *job;		/* Current license job */
int key;
short *val;
{
  int *i;
  char *c, **cp;
  PFI pfi;
  PPHOSTID phostid;
  LM_PFV *pfv;
  SIGPTR *sigptr;
  char ***charstarptr;
  PCSTAR pcstar;
  CRYPT_FUNC crypt_func;
  CRYPT_FUNC_GEN  crypt_func_gen;
  LM_VENDOR_HOSTID **vendor_id;
  int error = 0;

	if (job == (LM_HANDLE *) 0 || job->lm_errno == NOFLEXLMINIT)
	{
		return(NOFLEXLMINIT);
	}
	i = (int *) val;

	switch(key)
	{
		case LM_A_CAPACITY:
			*val = job->options->capacity;
			break;

		case LM_A_CHECK_BADDATE:
			*val = job->options->flags & LM_OPTFLAG_CHECK_BADDATE ?
				1 : 0;
			break;

		case LM_A_CHECK_INTERVAL:
			*i =  job->options->check_interval;
			break;

		case LM_A_CHECKOUTFILTER:
			pfi = (PFI) val;
			*pfi = job->options->outfilter;
			break;

		case LM_A_CHECKOUTFILTER_EX:
			pfi = (PFI) val;
			*pfi = job->options->outfilter_ex;
			break;

		case LM_A_CHECKOUTFILTERLAST_EX:
			pfi = (PFI) val;
			*pfi = job->options->outfilter_last_ex;
			break;

		case LM_A_CHECKOUT_DATA:
			c = (char *) val;
			(void) strncpy(c, 
				       job->options->vendor_checkout_data
						, MAX_VENDOR_CHECKOUT_DATA);
			break;
		
		case LM_A_COMM_TRANSPORT:
			*val = job->options->commtype;
			break;

		case LM_A_BEHAVIOR_VER:
			cp = (char **) val;
			*cp = job->options->behavior_ver;
			break;

		case LM_A_BORROW_STAT:
			l_borrow_stat(job, (LM_BORROW_STAT **)val, 0);
			break;

		case LM_A_CONN_TIMEOUT:
			*i = job->options->conn_timeout;
			break;

		case LM_A_CRYPT_CASE_SENSITIVE:
			*val = job->options->crypt_case_sensitive;
			break;


		case LM_A_DIAGS_ENABLED:
			*val = job->options->flags & LM_OPTFLAG_DIAGS_ENABLED ?
				1 : 0;
			break;

		case LM_A_DISABLE_ENV:
			*val = job->options->disable_env;
			break;

		case LM_A_DISPLAY_OVERRIDE:
			c = (char *) val;
			(void) strncpy(c, job->options->display_override,
						MAX_DISPLAY_NAME);/* LONGNAMES */
			break;

		case LM_A_ETHERNET_BOARDS:
			charstarptr = (char ***) val;
			*charstarptr = job->options->ethernet_boards;
			break;

		case LM_A_FLEXLOCK:
			*i = job->options->flags & LM_OPTFLAG_FLEXLOCK ? 1 : 0;
			break;

		case LM_A_HOSTID_PARSE:
			pfi = (PFI) val;
			*pfi = job->options->parse_vendor_id;
			break;
			
		case LM_A_HOST_OVERRIDE:
			c = (char *) val;
			(void) strncpy(c, job->options->host_override, 
							MAX_SERVER_NAME);/* LONGNAMES */
			break;

		case LM_A_JAVA_LIC_FMT:
			*i = job->options->flags & LM_OPTFLAG_JAVA_LIC_FMT 
							? 1 : 0;
			break;

		case LM_A_LF_LIST:
			charstarptr = (char ***)val;
			*charstarptr = job->lic_files;			
			break;

		case LM_A_LICENSE_FMT_VER:
			cp = (char **) val;
			*cp = job->options->license_fmt_ver;
			break;


		case LM_A_LICENSE_FILE:
			c = (char *) val;
			if (job->options->config_file)
			  (void) strcpy(c, job->options->config_file);
			else
			  *c = '\0';
			break;

		case LM_A_LICENSE_FILE_PTR:
		case LM_A_LICENSE_DEFAULT:
			cp = (char **) val;
			*cp =  job->options->config_file;
			break;

		case LM_A_LINGER:
			*(long *)i = job->options->linger_interval;
			break;

		case LM_A_LONG_ERRMSG:
			*i = job->options->flags & LM_OPTFLAG_LONG_ERRMSG ?
				1 : 0;
			break;


		case LM_A_MAX_LICENSE_LEN:
			*val = job->options->max_license_len;
			break;

		case LM_A_MAX_TIMEDIFF:
			*i = job->options->max_timediff;
			break;

		case LM_A_MT_HEARTBEAT:
#ifdef LM_SUPPORT_MT
			*i = job->options->flags & LM_OPTFLAG_MT_HEARTBEAT 
							? 1 : 0;
#else
			*i = 0;
#endif
			break;

		case LM_A_LCM:
			*i = job->options->flags & LM_OPTFLAG_LCM ? 1 : 0;
			break;

		case LM_A_LKEY_START_DATE:
			*i = job->options->flags & LM_OPTFLAG_LKEY_START_DATE 
				? 1 : 0;
			break;

		case LM_A_LKEY_LONG:
			*i = job->options->flags & LM_OPTFLAG_LKEY_LONG 
				? 1 : 0;
			break;


		case LM_A_NO_TRAFFIC_ENCRYPT:
			*val = job->options->no_traffic_encrypt;
			break;

		case LM_A_NORMAL_HOSTID:
			*val = job->options->normal_hostid;
			break;

		case LM_A_PC_PROMPT_FOR_FILE:
			*i = job->options->flags & 
				LM_OPTFLAG_PC_PROMPT_FOR_FILE ? 1 : 0;
			break;

		case LM_A_PERROR_MSGBOX:
			*i = job->options->flags & LM_OPTFLAG_PERROR_MSGBOX;
			break;


		case LM_A_PERIODIC_CALL:
			pfi = (PFI) val;
			*pfi = job->options->periodic_call;
			break;

		case LM_A_PERIODIC_COUNT:
			*i = job->options->periodic_count;
			break;

		case LM_A_PLATFORM_OVERRIDE:
			c = (char *) val;
			(void) strncpy(c, job->options->platform_override, 
							MAX_PLATFORM_NAME);
			break;

		case LM_A_REDIRECT_VERIFY:
			pfi = (PFI) val;
			*pfi = job->options->redirect_verify;
			break;

		case LM_A_RETRY_CHECKOUT:
			*i = job->options->flags & LM_OPTFLAG_RETRY_CHECKOUT;
			break;

		case LM_A_RETRY_COUNT:
			*i = job->options->retry_count;
			break;

		case LM_A_RETRY_INTERVAL:
			*i = job->options->retry_interval;
			break;

		case LM_A_REVISION:
			*val = FLEXLM_REVISION;
			break;

		case LM_A_SETITIMER:
			pfv = (LM_PFV *) val;
			*pfv = job->options->setitimer;
			break;

		case LM_A_SIGNAL:
			sigptr = (SIGPTR *) val;
			*sigptr = job->options->sighandler;
			break;

		case LM_A_LICENSE_CASE_SENSITIVE:
			*i = job->options->flags & 
				LM_OPTFLAG_STRINGS_CASE_SENSITIVE ?
				1 : 0;
			break;

		case LM_A_TCP_TIMEOUT:
			*i = job->daemon->tcp_timeout;
			break;

		case LM_A_TIMER_TYPE:
			*i = job->options->timer_type;
			break;

		case LM_A_TRY_COMM:
			*val = job->options->flags & LM_OPTFLAG_TRY_OLD_COMM ?
				1 : 0;
			break;

		case LM_A_PORT_HOST_PLUS:
			*val = job->options->flags & LM_OPTFLAG_PORT_HOST_PLUS ?
				1 : 0;
			break;
			
		case LM_A_USE_START_DATE:
			*val = job->options->flags & LM_OPTFLAG_USE_START_DATE ? 
				1 : 0;
			break;

		case LM_A_USER_CRYPT_FILTER_GEN:
			crypt_func_gen = (CRYPT_FUNC_GEN) val;
			*crypt_func_gen = job->user_crypt_filter_gen;
			break;

		case LM_A_USER_CRYPT_FILTER:
			crypt_func = (CRYPT_FUNC) val;
			*crypt_func = job->user_crypt_filter;
			break;

		case LM_A_USER_CRYPT:
			pcstar = (PCSTAR) val;
			*pcstar = job->options->user_crypt;
			break;

		case LM_A_USER_EXITCALL:
			pfi = (PFI) val;
			*pfi = job->options->user_exitcall;
			break;

		case LM_A_USER_EXITCALL_EX:
			pfi = (PFI) val;
			*pfi = job->options->user_exitcall_ex;
			break;

		case LM_A_VENDOR_CALLBACK_DATA:
			val = job->options->pUserData;
			break;

		case LM_A_USER_OVERRIDE:
			c = (char *) val;
			(void) strncpy(c, job->options->user_override, 
								MAX_USER_NAME);/* LONGNAMES */
			break;

		case LM_A_USER_RECONNECT:
			pfi = (PFI) val;
			*pfi = job->options->user_reconnect;
			break;

		case LM_A_USER_RECONNECT_EX:
			pfi = (PFI) val;
			*pfi = job->options->user_reconnect_ex;
			break;

		case LM_A_USER_RECONNECT_DONE:
			pfi = (PFI) val;
			*pfi = job->options->user_reconnect_done;
			break;

		case LM_A_USER_RECONNECT_DONE_EX:
			pfi = (PFI) val;
			*pfi = job->options->user_reconnect_done_ex;
			break;

		case LM_A_VENDOR_CHECKID:
			pfi = (PFI) val;
			*pfi = job->options->check_vendor_id;
			break;

		case LM_A_VENDOR_ID_DECLARE:
			vendor_id = (LM_VENDOR_HOSTID **)val;
			*vendor_id = job->options->vendor_hostids;
			break;

		case LM_A_VENDOR_GETHOSTID:
			phostid = (PPHOSTID) val;
			*phostid = job->options->get_vendor_id;
			break;

		case LM_A_VENDOR_PRINTHOSTID:
			pcstar = (PCSTAR) val;
			*pcstar = job->options->print_vendor_id;
			break;

		case LM_A_VERSION:
			*val = FLEXLM_VERSION;
			break;

		case LM_A_RELEASE_PATCH:
			cp = (char **) val;
			*cp = FLEXLM_PATCH;
			break;

		case LM_A_UDP_TIMEOUT:
			*i = job->daemon->udp_timeout;
			break;

		case LM_A_ALLOW_SET_TRANSPORT:
			*i = (job->options->allow_set_transport ? 1 : 0);
			break;

		case LM_A_ALT_ENCRYPTION:
			c = (char *) val;
			(void) memcpy(c, (char *)&job->options->alt_vendorcode,
							sizeof(VENDORCODE));
			break;

#ifndef NO_FLEXLM_CLIENT_API			
		case LM_A_VD_GENERIC_INFO:
			if (!val)
			{
				error = LM_BADPARAM;
				break;
			}
			((LM_VD_GENERIC_INFO *)val)->type = 
						LM_VD_GENINFO_HANDLE_TYPE; 
			error = l_serv_msg(job, 
					((LM_VD_GENERIC_INFO *)val)->feat, 
							(LM_HANDLE *)val);
			break;

		case LM_A_VD_FEATURE_INFO:
		{
		  LM_VD_FEATURE_INFO *p = (LM_VD_FEATURE_INFO *)val;

			if (!p || !p->feat)
			{
				error = LM_BADPARAM;
				break;
			}
				
			/* P3503 */
			l_zcp(job->err_info.feature, p->feat->feature, 
							MAX_FEATURE_LEN);

			p->type = LM_VD_FEATINFO_HANDLE_TYPE; 
			error = l_serv_msg(job, p->feat, (LM_HANDLE *)val);
		}
			break;
#endif /* NO_FLEXLM_CLIENT_API */

#ifdef WINNT
		case LM_A_FLEXLOCK_INSTALL_ID:
			*(long *)i = job->options->flexlock_install_id;
			break;

		case LM_A_LCM_URL:

			cp = (char **) val;
			*cp = job->options->lcm_url;
		        break;

		case LM_A_WINDOWS_MODULE_HANDLE:
			*(long *)i = job->options->windows_module_handle;
			break;

#endif /* WINNT */
			
#ifdef VMS
		case LM_A_EF_1:
			*val = FLEXLM_VMS_EF1;
			break;

		case LM_A_EF_2:
			*val = FLEXLM_VMS_EF2;
			break;

		case LM_A_EF_3:
			*val = FLEXLM_VMS_EF3;
			break;

		case LM_A_EF_4:
			*val = FLEXLM_VMS_EF4;
			break;

		case LM_A_EF_5:
			*val = FLEXLM_VMS_EF5;
			break;

		case LM_A_VMS_MAX_LINKS:
			*val = lm_vms_max_links;
			break;

#endif
		default:
			LM_SET_ERRNO(job, LM_NOSUCHATTR, 168, 0);
			break;
	}
	return(error);
}
