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
 *	Module: $Id: lm_ckin.c,v 1.5 2003/03/06 21:39:19 sluu Exp $
 *
 *	Function: lc_checkin(feature, keep)
 *
 *	Description: "checks in" a copy of the specified feature, when done.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) feature - The ascii feature name to be 
 *						checked back in.
 *			(int) keep - "keep the connection" flag
 *					(0 => no keep, <> 0 => keep)
 *
 *	Return:		(int) - 0 - OK, feature checked in.
 *				<> 0 - Feature checkin failed (see job->lm_errno).
 *
 *	Notes:	This call is optional, since the cleanup will be done when
 *		the process exits, but it may be desirable if features
 *		are to be used for only a part of the time an application
 *		is running.
 *
 *	M. Christiano
 *	2/18/88
 *
 *	Last changed:  9/30/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include "lm_attr.h"

void API_ENTRY
lc_checkin(job, feature, keep)
LM_HANDLE *job;		/* Current license job */
const LM_CHAR_PTR feature;		/* The feature to be checked in */
int keep;		/* "Keep the connection" flag */
{
	if (LM_API_ERR_CATCH) return;
	l_checkin(job, feature, keep);
	LM_API_RETURN_VOID();
}

void 
l_checkin(job, feature, keep)
LM_HANDLE *job;		/* Current license job */
const LM_CHAR_PTR feature;		/* The feature to be checked in */
int keep;		/* "Keep the connection" flag */
{
  char type, *param;	/* Returns from l_rcvmsg() */
  char msg[LM_MSG_LEN+1];
  char code[MAX_CRYPT_LEN+1];
  short udp = 0;


/*
 *	Turn off checking (which lets lm_status() know 
 *	about this feature being checked in).
 */
	if (l_checkoff(job, feature, code))
	{

		/*
		 *	Check to see if the feature is "borrowed", if yes
		 *	don't do anything as the license was checked in
		 *	when the borrow was initialized.  If this license was
		 *	checked out from the local borrow file, there's no server
		 *	to send the checkin message to.
		 */
		if(l_feature_is_borrowed(job, feature, code))
			return;

		if (job->daemon->commtype == LM_FILE_COMM)
		{
		  int ret;

			ret = l_file_checkin(job, feature, code,
					job->options->vendor_checkout_data);
			if (ret != LM_OK) 
			{
				LM_SET_ERRNO(job, LM_CHECKINBAD, 110, 0);
			}
		}
		else
		{
/*
 *			Send "checkin" message to server for this feature.
 */
			memset(msg, 0, LM_MSG_LEN);
			if (feature != LM_CI_ALL_FEATURES)
			{
				strncpy(&msg[MSG_IN_FEATURE-MSG_DATA], feature, 
							MAX_FEATURE_LEN);
				strncpy(&msg[MSG_IN_CODE-MSG_DATA], code, 
							MAX_CRYPT_LEN);
				if (job->options->vendor_checkout_data[0])
				{
					msg[MSG_IN_USE_VENDOR-MSG_DATA] = '1';
					l_zcp(&msg[MSG_IN_VENDOR-MSG_DATA], 
				      	job->options->vendor_checkout_data,
				      	MAX_VENDOR_CHECKOUT_DATA);
				}
			}
			(void) l_sndmsg(job, LM_CHECKIN, msg);
			if (job->lm_errno == LM_NOSOCKET)
				return;
			if (l_rcvmsg(job, &type, &param))
			{
			  int i;
				switch(type)
				{
					case LM_OK:
						break;
	
					default:
						i = 0;
						l_decode_int(param, &i);
						if (i >= 0) i = LM_CHECKINBAD;
						LM_SET_ERRNO(job, i, 111, 0);
						break;
				}
			}
			udp = job->options->commtype;
/*-
 *			We need to close the connection if we're using UDP, and
 *			we have no remaining features checked out, because:
 *			the server removes this client from it's database upon
 *			checkin of its last feature.  if we don't remove the
 *			connection, the next checkout occurs with an 
 *			encryption that the server doesn't expect, and the
 *			checkout fails.
 */
			if ((udp == LM_UDP) && (job->feat_count == 0))
							lc_disconn(job, 1);
			else if (keep == 0) 
							lc_disconn(job, 0);
			else if (job->daemon->usecount > 0) 
							job->daemon->usecount--;
		}
	}
	return;
}
