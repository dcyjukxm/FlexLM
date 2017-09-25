/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_ckin.c,v 1.20.8.2 2003/07/01 17:04:20 sluu Exp $
 *
 *	Function: 	lc_checkin()
 *
 *	Description: 	Handle LM_CHECKIN message
 *
 *	D. Birns
 *	9/21/96
 *
 *	Last changed:  10/13/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lsfeatur.h"
#include "ls_aprot.h" 
#include "ls_log.h" 
#include "flex_utils.h"
extern int (*ls_infilter)();              /* Checkin filter */
static int client_using_suite_token lm_args(( CLIENT_DATA *, FEATURE_LIST *));

#ifdef PC
typedef int (LM_CALLBACK_TYPE * LS_CB_INFILTER) (void);
#define LS_CB_INFILTER_TYPE (LS_CB_INFILTER)
#else
#define	 LS_CB_INFILTER_TYPE
#endif /* PC */


FEATURE_LIST * findfeat(char *, char *, FEATURE_LIST *last);

/*
 *	This function checks to see if any reservations can be freed.
 */

static
int
sCanDynamicUnreserve(
	FEATURE_LIST *	pFeature,
	CLIENT_DATA *	pClient)
{
	FEATURE_LIST *	pParentFeature = NULL;
	FEATURE_LIST *	pComponent = NULL;
	USERLIST *		pUser = NULL;
	USERLIST *		pCurrUser = NULL;
#define INVALID_RETURN_VALUE		2147483646
	int				iNumToFree = INVALID_RETURN_VALUE;
	int				iNumResForFeature = 0;

	/*
	 *	Make sure we're working only with the BUNDLE itself and not the
	 *	components of the BUNDLE.
	 */
	if(pFeature->flags & LM_FL_FLAG_BUNDLE_COMPONENT)
	{
		/*
		 *	Get parent for this feature
		 */
		pParentFeature = f_get_feat(pFeature->package->feature,
									pFeature->package->code, 1);
		if(!pParentFeature)
			goto done;
	}
	else
		pParentFeature = pFeature;


	/*
	 *	Now iterate through the list of users to see if any users match this user
	 */
	while(pComponent = findfeat(pParentFeature->feature, NULL, pComponent))
	{
		if(strcmp(pComponent->code, pParentFeature->code))
			continue;
		if(pComponent->flags & LM_FL_FLAG_BUNDLE_PARENT)
		{
			continue;
		}
		iNumResForFeature = f_dynamic_res_avail(pComponent, pClient, 0, NULL);

		/*
		 *	Might want to skip the package itself
		 */

		if(iNumResForFeature < iNumToFree)
		{
			iNumToFree = iNumResForFeature;
		}

		if(!iNumToFree)
			break;
	}

done:
	return iNumToFree == INVALID_RETURN_VALUE ? 0 : iNumToFree;
}

int
fCanDynamicUnreserve(
	FEATURE_LIST *	pFeature,
	CLIENT_DATA *	pClient)
{
	return sCanDynamicUnreserve(pFeature, pClient);
}

/*
 *	Check to see if a DYNAMIC_RESERVE currently in use
 *	can be swapped for a static RESERVE that is available.
 *	The reason for doing this is because you want to consume the
 *	static RESERVE before the DYNAMIC_RESERVE.  This can happen for
 *	instance when you do the following:
 *	
 *	1. RESERVE 3 licenses of a BUNDLE component for user1.
 *	2. Start a license server with 6 licenses
 *	3. Checkout 4 licenses of one component (f1) as user1
 *	4. Checkout 6 licneses of another component (f2) as user1
 *	5. Checkout 2 remainig licenses of f1 as user1.
 *	6. Checkin 4 licenses checked out in step #3.  You want the license server
 *		to make sure that the licenses checked out in step #5 are RESERVE and not
 *		dynamic_reserve.
 */
static
int
sCanSwitch(
	CLIENT_DATA *	pClient,
	OPTIONS *		pOptA,
	OPTIONS *		pOptB)
{
	int	ret = 0;

	if( (pOptB->type2 | OPT_HOST) && pOptA->pszHost
		&& (strcmp(pOptB->name, pOptA->pszHost) == 0) )
	{
		ret = 1;
	}
	else if( (pOptB->type2 | OPT_USER) && pOptA->pszUser
		&& (strcmp(pOptB->name, pOptA->pszUser) == 0) )
	{
		ret = 1;
	}
	else if( (pOptB->type2 | OPT_DISPLAY) && pOptA->pszDisplay
		&& (strcmp(pOptB->name, pOptA->pszDisplay) == 0) )
	{
		ret = 1;
	}
	else if( ((pOptB->type2 | OPT_GROUP) && pOptB->name)
		&& pClient && ls_ingroup(OPT_GROUP, pClient, pOptB->name) )
	{
		ret = 1;
	}
	else if( ((pOptB->type2 | OPT_HOST_GROUP) && pOptB->name)
		&& pClient && ls_ingroup(OPT_HOST_GROUP, pClient, pOptB->name) )
	{
		ret = 1;
	}
	else if( ((pOptB->type2 | OPT_PROJECT) && pOptB->name)
		&& pClient && pClient->project && (strcmp(pClient->project, pOptB->name) == 0) )
	{
		ret = 1;
	}
	else if( ((pOptB->type2 | OPT_INTERNET) || (pOptB->type2 == OPT_HOST && !pOptB->name))
		&& pClient && !l_inet_cmp(&pClient->inet_addr[0], &pOptB->inet_addr[0]) )
	{
		ret = 1;
	}
	
	return ret;
}



static
void 
sCheckReserves(
	CLIENT_DATA *	pClient,
	FEATURE_LIST *	pParent)
{
	FEATURE_LIST *	pFeat = NULL;
	USERLIST *		pUser = NULL;
	OPTIONS *		pOption = NULL;
	OPTIONS *		pOptionLast = NULL;
	OPTIONS *		pOptionRes = NULL;
	OPTIONS *		pOptionResLast = NULL;
	OPTIONS *		pTemp = NULL;
	int				iNumRes = 0;	/* Number of regular RESERVES available */
	int				iNumDynRes = 0;	/* Number of DYNAMIC_RESERVES in use */

	pFeat = findfeat(pParent->feature, NULL, pFeat);
	while(pFeat)
	{
		/*
		 *	Determine how many RESERVES we actually have
		 */
		iNumRes = iNumDynRes = 0;

		for(pOption = pFeat->opt; pOption; pOption = pOption->next)
		{
			if(pOption->type == RESERVE)
				iNumRes++;
		}

		if(iNumRes)
		{
			/*
			 *	Iterate through list of users and see if any are using OPTIONS(DYNAMIC_RESERVE) that
			 *	can be moved to OPTIONS(RESERVE).
			 */
			for(pUser = pFeat->u; pUser && iNumRes; pUser = pUser->next)
			{
				pOptionLast = NULL;
				for(pOption = pUser->o; pOption && iNumRes; pOption = pOption->next)
				{
					if(pOption->type == DYNAMIC_RESERVE)
					{
						/*
						 *	Walk pFeat->opt to see if any of the RESERVES there can be moved over to this
						 *	user to replace DYNAMIC_RESERVE.
						 */
						pOptionResLast = NULL;
						for(pOptionRes = pFeat->opt; pOptionRes && iNumRes; pOptionRes = pOptionRes->next)
						{
							if(pOptionRes->type == RESERVE && sCanSwitch(pClient, pOption, pOptionRes))
							{
								/*
								 *	Move DYNAMIC_RESERVE back to f->opt and move RESERVE to USERLIST.o.
								 */
								if(pUser->o == pOption)
								{
									/* 
									 *	First entry in list, make sure to update pUser->o.
									 */
									pUser->o = pOptionRes;
									pOptionLast = pUser->o;
								}
								if(pFeat->opt == pOptionRes)
								{
									/*
									 *	First entry in pFeat->opt.
									 */
									pFeat->opt = pOption;
									pOptionResLast = pFeat->opt;
								}
								pTemp = pOptionRes->next;
								if(pOptionLast)
								{
									pOptionLast->next = pOptionRes;
								}
								pOptionRes->next = pOption->next;
								if(pOptionResLast)
								{
									pOptionResLast->next = pOption;
									pOption->next = pTemp;
								}
								iNumRes--;
								pOption = pOptionRes;
								break;
							}
							pOptionResLast = pOptionRes;
						}
					}
					pOptionLast = pOption;
				}
			}
		}
		pFeat = findfeat(pParent->feature, NULL, pFeat);
	}

	return;
}

int
ls_checkin(
	CLIENT_DATA *	thisuser,
	char *			cmd)
{

	FEATURE_LIST *fl;
	char msg[LM_MSG_LEN+1];	/* For ls_client_send */
	int usevendor;
	int rc;
	int docheckin = 1;
	int iNumToFree = 0;

	memset(msg, 0, sizeof(msg));
	if (thisuser->comm_revision < 3)
		usevendor = 0;
	else
		usevendor = cmd[MSG_IN_USE_VENDOR];
	if (ls_infilter) 
	{
		ls_attr_setup(&cmd[MSG_IN_FEATURE], "*",
			"*", thisuser, "*", "*", 
			"*", &cmd[MSG_IN_CODE]);
		LM_SET_ERRNO(lm_job, 0, 329, 0);
		docheckin = (* LS_CB_INFILTER_TYPE ls_infilter)();
		rc = lm_job->lm_errno;
	}
	if (docheckin)
	{
		if (usevendor)
		{
			(void) l_zcp(
				thisuser->vendor_def,
				&cmd[MSG_IN_VENDOR],
			      MAX_VENDOR_CHECKOUT_DATA);
			thisuser->use_vendor_def = 1;
		}
		else
			thisuser->use_vendor_def = 0;

		rc = f_remove(thisuser, 
			&cmd[MSG_IN_FEATURE],
			LL_REASON_CLIENT_REQUEST,
			&cmd[MSG_IN_CODE], 1, 0);
		if (!rc)
		{
			LOG(("Checkin failed feature \"%s\": %s %s %s\n",
				&cmd[MSG_IN_FEATURE], 
				thisuser->name,
				thisuser->node,
				thisuser->display));
			LOG(("\t\"%s\"\n", lc_errstring(lm_job)));
		}

		if (cmd[MSG_IN_FEATURE] )
		{
			fl = f_get_feat(&cmd[MSG_IN_FEATURE], &cmd[MSG_IN_CODE], 0);

			if (fl && (fl->flags & LM_FL_FLAG_SUITEBUNDLE_COMPONENT) &&
				!client_using_suite_token(thisuser, fl->package))
			{

				/*
				 *	Check back in SUITE/BUNDLE itself
				 */
				if(fl->flags & LM_FL_FLAG_SUITE_COMPONENT)
				{
					rc = f_remove(thisuser, fl->package->feature,
									LL_REASON_CLIENT_REQUEST, fl->package->code, 1, 0);
				}
				else
				{

					/*
					 *	Iterate through the components and see if any users who are using DYNAMIC_RESERVES
					 *	can be moved over to using RESERVES.  That way, the following call to
					 *	sCanDynamicUnreserve() will free up licenses that can be used by others who
					 *	wouldn't qualify to use a RESERVE.  Make sense?  Hope so.....
					 */
					sCheckReserves(thisuser, fl->package);

					iNumToFree = sCanDynamicUnreserve(fl, thisuser);
					if(iNumToFree)
					{
						char	buffer[1024] = {'\0'};
						if(fl->dup_select && fl->suite_dup_group)
						{
							if(fl->suite_dup_group & LM_DUP_USER)
							{
								sprintf(buffer, thisuser->name);
							}
							strcat(buffer, ":");
							if(fl->suite_dup_group & LM_DUP_HOST)
							{
								strcat(buffer, thisuser->node);
							}
							strcat(buffer, ":");
							if(fl->suite_dup_group & LM_DUP_DISP)
							{
								strcat(buffer, thisuser->display);
							}
							strcat(buffer, ":");
							if(fl->suite_dup_group & LM_DUP_VENDOR)
							{
								strcat(buffer, thisuser->vendor_def);
							}
						}
						else
						{
							strcpy(buffer, "*:*:*:*");
						}

						f_dynamic_reserve_remove(fl->package, buffer, iNumToFree);
						/*
						 *	Dequeue any users who are queued
						 */
						{
							FEATURE_LIST *	pFeat = NULL;
							while(pFeat = findfeat(fl->package->feature, NULL, pFeat))
							{
								while(pFeat->queue)
								{
									if(!f_dequeue(pFeat, 1))
										break;
								}
							}
						}
					}
				}
			}
		}
	}
		
	if (rc)
	{	
		ls_client_send(thisuser, LM_OK, msg); 
	}
	else
	{	
		ls_client_send(thisuser, LM_NUSERS, msg);
	}
	if (thisuser->lastcomm == LS_DELETE_THIS_CLIENT)
		ls_delete_client(&thisuser->addr);
	return rc;
}

/*
 *	client_using_suite_token
 *	If this client is using the suite token for another component,
 *	return true
 */
static 
int
client_using_suite_token(
	CLIENT_DATA *	client,
	FEATURE_LIST *	suite_fl)
{
	FEATURE_LIST *fl;
	USERLIST *u, *b;
	extern FEATURE_LIST *ls_flist;

/*
 *	Examine every USERLIST in the entire database
 */
	for (fl = ls_flist; fl; fl = fl->next)
	{
		for (u = fl->u; u; u = u->next)
		{
			if ((client->handle == u->handle) && /* same client */
				(fl->flags & LM_FL_FLAG_SUITEBUNDLE_COMPONENT) && 
				l_keyword_eq(lm_job, fl->package->feature, suite_fl->feature) &&
				l_keyword_eq(lm_job, fl->package->code, suite_fl->code))
			{
				return 1;
			}
			/* same test for each brother also */
			for (b = u->brother; b; b = b->next)
			{
				if ((client->handle == b->handle) &&
					(fl->flags & LM_FL_FLAG_SUITEBUNDLE_COMPONENT) && 
					l_keyword_eq(lm_job, fl->package->feature, suite_fl->feature) &&
					l_keyword_eq(lm_job, fl->package->code, suite_fl->code))
				{
					return 1;
				}
			}
		}
	}
	return 0;
}
