/******************************************************************************

	    COPYRIGHT (c) 1994, 2003  by Macrovision Corporation.
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
 *	Module: $Id: lm_lsapi.c,v 1.3 2003/04/16 17:36:26 brolland Exp $
 *
 *	Description: 	LSAPI function call interface standard.  This module
 *			provides standard calls that work with FLEXlm.
 *
 *	M. Christiano
 *	2/12/94
 *
 *	Last changed:  12/8/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_attr.h"
#include "lm_comm.h"
#include "lm_lsapi.h"
#include <stdio.h>

#define LICENSESYSTEM "GSI (FLEXlm)"
static LS_STATUS_CODE lsapi_error lm_args((int i));
static void lsapi_challenge lm_args((LM_HANDLE *, LS_CHALLENGE *c));

typedef struct lsapi_handle {
			      LM_HANDLE *l_handle;
			      char l_feature[MAX_FEATURE_LEN];
			      char l_v[MAX_VER_LEN+1];
			      LS_ULONG l_units;
			    } LSAPI_HANDLE;

static LS_STATUS_CODE do_checkout lm_args((LSAPI_HANDLE *h, LS_CHALLENGE *c,
					LS_ULONG *granted, LS_STR *comment));

/*
 *	LSEnumProviders() - Provide list of licensing system providers.
 */

LS_STATUS_CODE API_ENTRY
LSEnumProviders(index, buffer)
LS_ULONG index;
LS_STR *buffer;
{
  LS_STATUS_CODE stat = LS_SUCCESS;

	if (index == 0) (void) strcpy((char *) buffer, LICENSESYSTEM);
	else stat = LS_BAD_INDEX;
	return(stat);
}

void API_ENTRY
LSFreeHandle(handle)
LS_HANDLE handle;
{
  LM_HANDLE *h;

	if (!handle) return;

	h = ((LSAPI_HANDLE *)handle)->l_handle;

	if (h)
	{
		lc_free_job(h);			/* Free the FLEXlm job */
	}
	(void) memset((char *)handle, '\0', sizeof(LSAPI_HANDLE));
	(void) free((char *) handle);	/* ... and the LSAPI_HANDLE */
}

static char *errmsg[] = {
"No error",
"Handle used on call did not describe a valid licensing system context",
"The licensing system could not locate enough available licensing resources to satisy the request",
"No licensing system could be found with which to perform the function invoked",
"The resources used to satisfy a previous request are no longer granted",
"No licensing resources are available that could satisfy the request",
"Licensing resources are temporarily unavailable to satisfy the request",
"Insufficient resources (such as memory) are avaialble to complete the request",
"The network is unavailable",
"A warning occurred while looking up an error message string",
"An unrecognized status code was pased into the LSGetMessage() function",
"An invalid index was specified in LSEnumProviders() or LSQuery()",
"The license associated with the current context has expired",
"Supplied buffer too small to LSQuery() call",
"One or more arguments to an LSAPI call is incorrect"
};

/*
 *	LSGetMessage() - Get Message from Licensing system.
 */

/* ARGSUSED */
LS_STATUS_CODE API_ENTRY
LSGetMessage(handle, value, buffer, bufsiz)
LS_HANDLE handle;
LS_STATUS_CODE value;
LS_STR *buffer;
LS_ULONG bufsiz;
{
  LS_STATUS_CODE stat = LS_SUCCESS;

	if ( !handle )			
		return LS_BAD_HANDLE;	

  
	if ((value < LS_FIRST_ERROR) ||
	     ((value > LS_LAST_ERROR) && (value != LS_OTHER_FLEX_ERROR)))
			 stat = LS_UNKNOWN_STATUS;
	else if (value == LS_OTHER_FLEX_ERROR)
	{
	  LM_HANDLE *lmhandle = ((LSAPI_HANDLE *) handle)->l_handle;

/* TODO: This isn't exactly right if other errors have happened since */
/*	 we could return "other FLEXlm error" instead ??? */
		(void) strncpy((char *) buffer, 
			lmtext(lc_errstring(lmhandle)), 
								(int) bufsiz);
		buffer[(int) bufsiz-1] = '\0';
	}
	else
	{
	  int which = 0;

		if (value != 0) which = (int) (value - (LS_FIRST_ERROR)) + 1;
		(void) strncpy((char *) buffer, lmtext(errmsg[which]), 
							(int) bufsiz);
		buffer[(int) bufsiz-1] = '\0';
	}
	return(stat);
}

typedef struct ls_counted_data { unsigned long count; 
				 union { char string[1000]; 
					 LM_HANDLE *job;
				       } data;
			       } COUNTED_DATA;
/*
 *	LSQuery() - Query Licensing system information.
 */

LS_STATUS_CODE API_ENTRY
LSQuery(handle, info, buffer, bufsiz, actual_bufsiz)
LS_HANDLE handle;
LS_ULONG info;
LS_VOID *buffer;
LS_ULONG bufsiz;
LS_ULONG *actual_bufsiz;
{
  LS_STATUS_CODE stat = LS_SUCCESS;
  CONFIG *c;
  unsigned long val;
  int i;
  LM_HANDLE *h = ((LSAPI_HANDLE *)handle)->l_handle;
  char *feature = ((LSAPI_HANDLE *)handle)->l_feature;
  COUNTED_DATA *cd = (COUNTED_DATA *)buffer;
 
	switch (info)
	{
		case LS_INFO_NONE:
			*(char *)buffer = '\0';
			break;

		case LS_INFO_SYSTEM:
			(void) strncpy((char *) buffer, LICENSESYSTEM, (int) bufsiz);
			((char *)buffer)[(int) (bufsiz-1)] = '\0';
			*actual_bufsiz = strlen((char *) buffer);
			if (strlen(LICENSESYSTEM) > bufsiz)
				stat = LS_BUFFER_TOO_SMALL;
			break;

		case LS_INFO_DATA:
			c = lc_auth_data(h, feature);
			if (c && c->lc_vendor_def) 
			{
				cd->count = strlen(c->lc_vendor_def) + 1;
				if (cd->count > bufsiz)
				{
					stat = LS_BUFFER_TOO_SMALL;
					cd->count = bufsiz;
				}
				(void) strncpy(cd->data.string, 
							c->lc_vendor_def, 
							(int) cd->count);
				cd->data.string[(int) cd->count] = '\0';
				*actual_bufsiz = (LS_ULONG) cd->count - 1 +
							sizeof(cd->count);
			}
			else
			{
				cd->count = 0;
				*actual_bufsiz = (LS_ULONG) 0;
			}
			break;

		case LS_UPDATE_PERIOD:
			(void) lc_get_attr(h, LM_A_UDP_TIMEOUT, (short *)&i);
			val = (unsigned long) i;
			val /= 60;
			val--;	    /* recommend more often than is required */
			if (val <= 2) val = 2;
			if (bufsiz < sizeof(unsigned long))
			{
				stat = LS_BUFFER_TOO_SMALL;
				*actual_bufsiz = (LS_ULONG) 0;
			}
			else
			{
				*((unsigned long *) buffer) = val;
				*actual_bufsiz = 
					(LS_ULONG) sizeof(unsigned long);
			}
			break;

		case LS_LICENSE_CONTEXT:
			if (bufsiz < sizeof(unsigned long))
			{
				stat = LS_BUFFER_TOO_SMALL;
				*actual_bufsiz = (LS_ULONG) 0;
			}
			else
			{
				cd->count = 4;
				cd->data.job = h;
				*actual_bufsiz = (LS_ULONG) 
				    (sizeof(unsigned long) + sizeof(cd->count));
			}
			break;

		default:
			stat = LS_BAD_INDEX;
			*actual_bufsiz = (LS_ULONG) 0;
			break;
	}
	return(stat);
}

/*
 *	LSRelease() - Release a license.
 */

/* ARGSUSED */
LS_STATUS_CODE API_ENTRY
LSRelease(handle, consumed, comment)
LS_HANDLE handle;
LS_ULONG consumed;
LS_STR *comment;
{
  LS_STATUS_CODE stat = LS_SUCCESS;
  int i;
  LM_HANDLE *h; 
  char *feature; 
 
	if (!handle) 	
		stat = LS_BAD_ARG;
	else 
	{
		h = ((LSAPI_HANDLE *)handle)->l_handle;
		feature = ((LSAPI_HANDLE *)handle)->l_feature;
		if (comment && *comment) (void) lc_log(h, (char *) comment);
		lc_checkin(h, feature, 0);
	}
	return(stat);
}

#ifndef USE_WINSOCK
extern VENDORCODE lmcode;
#endif

/*
 *	LSRequest() - Request a license.
 */

LS_STATUS_CODE API_ENTRY
LSRequest(system, vendor, product, version, units_requested, comment, 
						challenge, granted, handle)
LS_STR *system;
LS_STR *vendor;
LS_STR *product;
LS_STR *version;
LS_ULONG units_requested;
LS_STR *comment;
LS_CHALLENGE *challenge;
LS_ULONG *granted;
LS_HANDLE *handle;
{
  LS_STATUS_CODE stat = LS_SUCCESS;
  int i = 0;
  LM_HANDLE *lmhandle;
  
	*handle = (LS_HANDLE) NULL;
	if ((system != (LS_STR *) LS_ANY) && 
				strcmp((char *) system, LICENSESYSTEM))
	{
		stat = LS_SYSTEM_UNAVAILABLE;
	}

	if (!challenge) 	return(LS_BAD_ARG);

	if ((stat == LS_SUCCESS))
	{
		if ( challenge->Protocol == LS_FLEXLM_PROTOCOL )
			i = lc_init((LM_HANDLE *) NULL,
				    ((LS_CHALLENGE_FLEXLM *)challenge)->
					    ChallengeData.VendorName,
				    &(((LS_CHALLENGE_FLEXLM *)challenge)->
					    ChallengeData.VendorCode),
				    &lmhandle);
		else
#ifndef PC
			/* For backward compatibility on Unix. */
			i = lc_init((LM_HANDLE *) NULL, (char *) vendor,
				    &lmcode, &lmhandle);
#else
			i = LM_BADKEYDATA;
#endif
	}
	else
		i = LM_BADKEYDATA;

	if (i && i != LM_DEMOKIT) 
	{
		if (i == LM_NONETWORK)  stat = LS_NETWORK_UNAVAILABLE;
		else			stat = LS_SYSTEM_UNAVAILABLE;
	}
	else 
	{
	  int num;
	  LSAPI_HANDLE *h;

		h = (LSAPI_HANDLE *) malloc((unsigned)sizeof(LSAPI_HANDLE));

		if (h == (LSAPI_HANDLE *) NULL)
		{
			stat = LS_RESOURCES_UNAVAILABLE;
		}
		else
		{
			(void) memset((char *)h, 0, sizeof(LSAPI_HANDLE));
			h->l_handle = lmhandle;
			(void) strncpy(h->l_feature, (char *) product, 
							MAX_FEATURE_LEN);
			h->l_feature[MAX_FEATURE_LEN-1] = '\0';
			*handle = (LS_HANDLE) h;
			strncpy(h->l_v, version, MAX_VER_LEN);
			h->l_v[MAX_VER_LEN] = '\0';
	  		num = (int) units_requested;
			if (units_requested == LS_DEFAULT_UNITS) num = 1;
			h->l_units = (LS_ULONG) num;
			(void) lc_set_attr(lmhandle, LM_A_CHECK_INTERVAL, 
							(LM_A_VAL_TYPE) -1);
			(void) lc_set_attr(lmhandle, LM_A_RETRY_INTERVAL, 
							(LM_A_VAL_TYPE) -1);
			(void) lc_set_attr(lmhandle, LM_A_COMM_TRANSPORT, 
							(LM_A_VAL_TYPE) LM_UDP);
#ifdef SUPPORT_IPX
			{
				char *ct;
				LM_HANDLE_PTR job=lmhandle;
				if (ct = getenv("FLEXLM_COMM_TRANSPORT"))	/* overrun checked */
				{
					if (!stricmp("SPX", ct) ||
					    !stricmp("IPX", ct))
						lc_set_attr(lmhandle,
							LM_A_COMM_TRANSPORT, 
						(LM_A_VAL_TYPE) LM_SPX);
				}
			}
#endif /* SUPPORT_IPX */			

			stat = do_checkout(h, challenge, granted, comment);
		}
	}
	return(stat);
}

/*
 *	LSUpdate() - Inform license system that a license is still in use
 */

/* ARGSUSED */
LS_STATUS_CODE API_ENTRY
LSUpdate(handle, consumed, reserved, comment, challenge, granted)
LS_HANDLE handle;
LS_ULONG consumed;
LS_ULONG reserved;
LS_STR *comment;
LS_CHALLENGE *challenge;
LS_ULONG *granted;
{
  LS_STATUS_CODE stat = LS_SUCCESS;
  int i;
  LSAPI_HANDLE *l = (LSAPI_HANDLE *) handle;
  LM_HANDLE *h = l->l_handle;
  char *feature = l->l_feature;

	LM_SET_ERRNO(h, 0, 190, 0);
	/*
	h->lm_errno = 0;
	*/
	lc_timer(h);
	if (reserved > l->l_units)
	{
/*
 *			Need to go get some more
 */
		l->l_units = reserved;
		stat = do_checkout(l, challenge, granted, comment);
	}
	else
	{
/*
 *			Just check what we have
 */
		i = lc_status(h, feature);
		if (h->lm_errno) stat = lsapi_error(h->lm_errno);;
		if (i == LM_CANTCONNECT) stat = LS_LICENSE_TERMINATED;
		if (stat != LS_LICENSE_TERMINATED)
		{
			if (comment && *comment) 
				(void) lc_log(h, (char *) comment);
			lsapi_challenge(h, challenge);
		}
	}
	return(stat);
}

/*
 *	Turn a FLEXlm error into an LSAPI error status
 */

static
LS_STATUS_CODE
lsapi_error(i)
int i;
{
  LS_STATUS_CODE stat;

	switch (i)
	{
		case LM_MAXUSERS:
		case LM_USERSQUEUED:
		case LM_CANTCONNECT:
		case LM_BUSY:
			stat = LS_LICENSE_UNAVAILABLE;
			break;

		case LM_CANTMALLOC:
			stat = LS_RESOURCES_UNAVAILABLE;
			break;

		case LM_NOFEATURE:
		case LM_OLDVER:
		case LM_TOOMANY:
		case LM_NOSERVSUPP:
			stat = LS_AUTHORIZATION_UNAVAILABLE;
			break;

		case LM_LONGGONE:
		case LM_SERVLONGGONE:
			stat = LS_LICENSE_EXPIRED;
			break;

		case 0:
			stat = LS_SUCCESS;

		default:
			stat = LS_OTHER_FLEX_ERROR;
			break;
	}
	return(stat);
}

/*
 *	Perform the challenge
 */

static
void
lsapi_challenge(job, c)
LM_HANDLE *job;		/* Current license job */
LS_CHALLENGE *c;
{
  char msg[LM_MSG_LEN+1];
  char type, *param;	/* Returned message */

	if (c == (LS_CHALLENGE *) NULL ||
	   ((c->Protocol!=LS_BASIC_PROTOCOL)&&(c->Protocol!=LS_FLEXLM_PROTOCOL))
	    ||	c->Size < sizeof(LS_CHALLDATA))
	{
		if (c) c->Size = 0;
	}
	else
	{
		long index;
		long random;
		char resp_msgdigest[LS_MSG_DIGEST_SIZE];
		char *p_msgdigest;
		 
		if (c->Protocol==LS_BASIC_PROTOCOL)
		{ 
			   index = c->ChallengeData.SecretIndex; 
			   random = c->ChallengeData.Random;
			   p_msgdigest = c->ChallengeData.MsgDigest.MessageDigest;
		} 
		else
		{
			   index = 1; 
			   random = 1;
			   p_msgdigest = resp_msgdigest;

		}
		
		(void) memset(msg, '\0', LM_MSG_LEN);

		l_encode_long(&msg[LS_CHALLENGEMSG_SECRET], 
					(long) index);
		l_encode_long(&msg[LS_CHALLENGEMSG_RANDOM], 
					(long) random);
		(void) memcpy(&msg[LS_CHALLENGEMSG_MSGDIGEST],
			(char *) p_msgdigest, LS_MSG_DIGEST_SIZE);
		if (!l_sndmsg(job, LM_CHALLENGE, &msg[MSG_DATA]))
		{
			c->Size = 0;
		}
		else
		{
			if (!l_rcvmsg(job, &type, &param) || 
						type != LM_CHALLENGE_RESP)
			{
				c->Size = 0;
			}
			else
			{
				(void) memcpy((char *) p_msgdigest,
				     &param[LS_CHALLENGEMSG_MSGDIGEST-MSG_DATA],
							LS_MSG_DIGEST_SIZE);
				c->Size = sizeof(LS_CHALLDATA);
			}
		} 
	}
}


static
LS_STATUS_CODE
do_checkout(h, challenge, granted, comment)
LSAPI_HANDLE *h;
LS_CHALLENGE *challenge;
LS_ULONG *granted;
LS_STR *comment;
{
  int i;
  LS_STATUS_CODE stat;
  LM_HANDLE *job = h->l_handle;

	if ( challenge->Protocol == LS_FLEXLM_PROTOCOL )
		i = lc_checkout(job, (char *) h->l_feature, h->l_v, 
				(int) h->l_units, LM_CO_NOWAIT,
				&(((LS_CHALLENGE_FLEXLM *)challenge)->
						ChallengeData.VendorCode),
				LM_DUP_NONE);
	else
#ifndef PC
		/* For backward compatibility on Unix. */
		i = lc_checkout(job, (char *) h->l_feature, h->l_v, 
			(int) h->l_units, LM_CO_NOWAIT, &lmcode, LM_DUP_NONE);
#else
		i = LM_BADKEYDATA;
#endif
	if (i)
	{
		stat = lsapi_error(i);
		*granted = (LS_ULONG) 0;
	}
	else
	{ 
		stat = LS_SUCCESS;
		*granted = (LS_ULONG) h->l_units;
		if (comment && *comment) 
		(void) lc_log(job, (char *)comment);
		lsapi_challenge(job, challenge);
	}
	return(stat);
}


