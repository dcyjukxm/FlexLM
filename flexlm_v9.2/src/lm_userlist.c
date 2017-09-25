/******************************************************************************

	    COPYRIGHT (c) 1988, 2003 by Macrovision Corporation
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
 *	Module: $Id: lm_userlist.c,v 1.19 2003/04/18 23:48:08 sluu Exp $
 *
 *	Function: lc_userlist(job, feature)
 *
 *	Description: Provides a list of users of a specified feature.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) feature - The ascii feature name.
 *		
 *	Return:		(LM_USERS *) - Ptr to LM_USERS structure
 *
 *			The returned LM_USERS structure is from the dynamic
 *			memory area.  Subsequent calls to lm_userlist()
 *			will free the old data and re-allocate space.
 *
 *	M. Christiano
 *	2/18/88
 *
 *	Last changed:  4 Nov 2002
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include "lgetattr.h"
#include <stdio.h>
#include <errno.h>

static void decode lm_args(( int comm_rev, char *msg, LM_USERS *who));
static int is_not_bug_p6476(LM_HANDLE * job, CONFIG *config, int num_entries);
		
	/*- P5985:  job->userlist used to be static to this file */

/**********************************************************************
 * Get the list of users using the named feature.
 * 
 * Parameters:
 * 		LM_HANDLE *job;		Current license job
 * 		char *feature;		The feature we want to know about
 * 
 * Return:
 * 		Linked list of users
 * 
 *********************************************************************/ 
LM_USERS * API_ENTRY lc_userlist(LM_HANDLE * job, char * feature)
{
 	char type;
 	char *name;
 	LM_USERS *who;		/* Who the users are */
 	CONFIG *conf;
 	char msg[LM_MSG_LEN+1];
 	int first = 1;
 	int cursize = 0, curnum = 0;
 	int comm_rev = 0;
 	char vendor_data[MAX_VENDOR_CHECKOUT_DATA+1];
 	int usefile = 0;
 	int got = 0;
 	int num_users;
 	CONFIG *pos = 0;
 	char last_server[MAX_HOSTNAME + 1 ];

	if (LM_API_ERR_CATCH) return 0;

	*last_server = 0;
	vendor_data[0] = '\0';
	if (l_getattr(job, ULIST) != ULIST_VAL)
	{
		LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 217, 0);
		LM_API_RETURN(LM_USERS *, NULL)
	}
	if (job->userlist != (LM_USERS *) NULL)
	{
		(void) free(job->userlist);
		job->userlist = (LM_USERS *) NULL;
	}
	for (conf = l_next_conf_or_marker(job, feature, &pos, 0, 0); conf; 
		conf = l_next_conf_or_marker(job, feature, &pos, 0, 0))
	{
		if ( (job->lm_errno == LM_HOSTDOWN) && conf->server 
			&& !L_STREQ(last_server, conf->server->name))
		{	
			/* clear it */
			l_clear_error(job);
		}
		if (!l_keyword_eq(job, feature, conf->feature)) continue;
		if ((conf->type != CONFIG_FEATURE) &&
		    (conf->type != CONFIG_UPGRADE) &&
		    (conf->type != CONFIG_INCREMENT)) continue;

		if ( ! is_not_bug_p6476(job,conf,got) )
			continue;	/* We hit the bug. ignore this config */

		if (conf->server && conf->server->commtype == LM_FILE_COMM)
		{
			LM_SERVER *tmp;

			tmp = job->daemon->server;
			job->daemon->server = conf->server;
			l_file_userlist(job, conf, (LM_USERS **)&job->userlist);
			job->daemon->server = tmp;
			usefile = 1;
			break;
		}
		/*
		 *		If we haven't connected yet, do it now
		 */
		(void) l_connect(job, conf->server, conf->daemon,
					job->options->commtype);
		if (job->daemon->socket == LM_BAD_SOCKET) continue;
		comm_rev = job->daemon->comm_revision;
		/*
		 *		Send "list" message to server for this feature, 
		 *		and receive the list of users
		 *
		 *		Make a copy of the message so l_sndmsg() doesn't run 
		 *		off the end of the input string, which could cause a 
		 *		segment violation.
		 */
		(void) bzero(msg, LM_MSG_LEN);
		(void) strncpy(&msg[MSG_LIST_FEATURE-MSG_DATA], feature,
						MAX_FEATURE_LEN+1);
		(void) strncpy(&msg[MSG_LIST_CODE-MSG_DATA], conf->code,
						MAX_CRYPT_LEN+1);
		msg[MSG_LIST_BFLAG-MSG_DATA] = '1';  
					/* We can handle BORROW */
		if (l_sndmsg(job, LM_LIST, msg) == 0) break;
		if (l_rcvmsg(job, &type, &name) == 0) break;
		if (type != LM_NUSERS)
		{
			
			LM_SET_ERRNO(job, LM_BADCOMM, 218, 0);
			continue;
		}

		l_decode_int(&name[MSG_NUSERS_N-MSG_DATA], &num_users);
		if (num_users < 0)
		{
			LM_SET_ERRNO(job, LM_BADCOMM, 219, 0);
			continue;
		}
/*
 *		allocate memory for the structs and fill in the names.
 */
		if (first)
		{
			cursize = (num_users+1) * sizeof(LM_USERS);
			got = curnum = num_users + 1;
			who = job->userlist =
			(LM_USERS *) l_malloc(job, (unsigned) cursize);
			first = 0;
		}
		else
		{
			int more = 0;
			LM_USERS *u;
			int oldsize = cursize;

			if (comm_rev) more = 1;
			cursize += (num_users+more) * sizeof(LM_USERS);
			got = curnum += num_users + more;
			job->userlist = (LM_USERS *)realloc(job->userlist, cursize);
			who = (LM_USERS *)job->userlist + (curnum-num_users-more);
			memset((char *) who, (char) 0,
				sizeof(LM_USERS) * (num_users + more));
		}


		if (comm_rev)
		{
			l_decode_long(&name[MSG_NUSERS_TIME-MSG_DATA],
							&(who->time));
			l_decode_int(&name[MSG_NUSERS_TOT-MSG_DATA], 
							&(who->nlic));
		}
		else
		{
			who->time = 0;
			who->nlic = 0;
		}
		/* kmaclean 1/1/2002
		 * It seems to me that setting the next pointer here is usless 
		 * and misleading. If we get another list of users from the server
		 * we will realloc the array of LM_USERS that this 'who' comes from 
		 * so that all the next pointers will be invalid. I don't really 
		 * see why this is here. I took it out I hope the sky does not fall.
		 *  */
		/*  who->next = who+1; */
		who++;
		for ( ; num_users > 0; num_users--)
		{
			if (l_rcvmsg(job, &type, &name) == 0)
			{
			      LM_SET_ERRNO(job, LM_BADCOMM, 221, 0);
			      got --;
			}
			else if (type == LM_USERNAME)
			{
				/*  who->next = who+1; */
				who->opts = 0;
				who->ul_conf = conf;
				decode(comm_rev, name, who);
				if (*vendor_data)
				{
					(void) strncpy(who->vendor_def,
						vendor_data, 
						MAX_VENDOR_CHECKOUT_DATA);
					who->vendor_def[
						MAX_VENDOR_CHECKOUT_DATA]
							= '\0';
					vendor_data[0] = '\0';
				}
				who++;
			}
			else if (type == LM_USERNAME2)
			{
				num_users++;    /* Don't count as a user */
				(void) strncpy(vendor_data, name,
					  MAX_VENDOR_CHECKOUT_DATA);
			}
		}
		
		/* 
		 *		Only look at 1 server on old daemons 
		 */
		if (job->daemon->comm_revision < 3) break;
	}
	if (first && !conf) 
	{
		LM_SET_ERRNO(job, LM_NOFEATURE, 222, 0);
	}
	if (job->userlist && !usefile)
	{
		int n;

	  	/* set all the next pointers on the userlist linked list.
		 * Need to do this because the list may have been realloc()'ed above */
		for (n = 0; n < got-1; n++) 
			((LM_USERS *)job->userlist)[n].next = ((LM_USERS *)job->userlist) + n+1;
		((LM_USERS *)job->userlist)[got-1].next = 0;  /* The last one */
	}
	LM_API_RETURN(LM_USERS *,(LM_USERS *)job->userlist)
}

/*
 *	decode() - Decode the message into the userlist struct
 */
static void decode(int comm_rev, char *msg, LM_USERS *who)
{
	int flags;
	if (comm_rev)
	{
		long l;
		l_decode_long(&msg[MSG_U_TIME-MSG_DATA], &l);
		who->time = l;
		strncpy(who->version, &msg[MSG_U_VER-MSG_DATA], MAX_VER_LEN);
		if (comm_rev >= 2)
		{
			l_decode_long(&msg[MSG_U_LINGER-MSG_DATA], &who->linger);
			if (comm_rev >= 3)
			{
			  int flags;
				l_decode_int(&msg[MSG_U_HANDLE-MSG_DATA], 
						    &who->ul_license_handle);
				/*
				 *		For historic reasons, there's exactly 3 bytes
				 *		in binary format in MSG_U_FLAGS
				 */
				flags = 0;
				flags |= (msg[MSG_U_FLAGS] << 16) & 0xff0000;
				flags |= (msg[MSG_U_FLAGS + 1] << 8) & 0xff00;
				flags |= msg[MSG_U_FLAGS + 2];
				if (flags & LM_ULF_BORROWED)
					who->opts |= LM_USERS_BORROW;
					
			}
			else
			{
				who->ul_license_handle = 0;
			}
		}
		else
		{
			who->linger = who->ul_license_handle = 0;
		}
		who->opts = 0;
		if (who->time == 0)   /* RESERVATION */
		{
			(void) strncpy(who->name, &msg[1], MAX_USER_NAME);/* LONGNAMES */
			if (*msg == 'U')
			{
	     			who->opts = USERRES;
			}
			else if (*msg == 'H')
			{
				who->opts = HOSTRES;
			}
			else if (*msg == 'D')
			{
				who->opts = DISPLAYRES;
			}
			else if (*msg == 'G')
			{
				who->opts = GROUPRES;
			}
			else if (*msg == 'K')
			{
				who->opts = HOSTGROUPRES;
			}
			else if (*msg == 'I')
			{
				who->opts = INTERNETRES;
			}
			else if (*msg == 'B')
			{
				who->opts = BORROWEDLIC;
			}
			else if (*msg == 'L' || *msg == 'V')
			{
				who->opts = BUNDLERES;
				strncpy(who->node, &msg[MSG_U_NODE - MSG_DATA], MAX_SERVER_NAME);/* LONGNAMES */
				strncpy(who->display, &msg[MSG_U_DISP - MSG_DATA], MAX_DISPLAY_NAME);/* LONGNAMES */
				if(*msg == 'V')
					strcpy(who->vendor_def, "VENDOR");	/* VENDOR info is never displayed as that's up to the ISV */
			}
			else
			{
				who->opts = UNKNOWNRES;
			}
			if (*msg == 'P')
			{
	     			who->opts = PROJECTRES;
			}
		}
		else	/* Normal */
		{
			l_zcp(who->name, &msg[MSG_U_NAME-MSG_DATA], 
							MAX_USER_NAME);/* LONGNAMES */
			l_zcp(who->node, &msg[MSG_U_NODE-MSG_DATA], 
							MAX_SERVER_NAME);/* LONGNAMES */
			l_zcp(who->display, &msg[MSG_U_DISP-MSG_DATA],
							MAX_DISPLAY_NAME);/* LONGNAMES */
		}
		l_decode_int(&msg[MSG_U_NUM-MSG_DATA], &who->nlic);
		if (who->nlic < 0)
		{
			who->opts = INQUEUE;
			who->nlic = -who->nlic;
		}
	}
	else
	{
		long l;
		l_decode_long(&msg[MSG_U_TIME_v1_0-MSG_DATA], &l);
		who->time = l;
		l_decode_int(&msg[MSG_U_NUM_v1_0-MSG_DATA], &who->nlic);
		sscanf(&msg[MSG_U_NAME_v1_0-MSG_DATA], "%[^@]@%[^@]", 	/* overrun threat */
						&who->name[0], &who->node[0]);
		if (who->nlic < 0)
		{
			who->opts = INQUEUE;
			who->nlic = -who->nlic;
		}
		who->opts = 0;
		who->ul_license_handle = 0;
		who->linger = 0;
		strcpy(who->version, "0.0");
		who->display[0] = '\0';
	}
	if(who->opts != BUNDLERES)
		who->vendor_def[0] = '\0';	/* Always filled in later */
}

/**********************************************************************
 * kmaclean 11/1/2002
 * 
 * This is a hack fix for bug # P6476
 * lc_user list will return duplicate entries for the features we have 
 * checked out after checkout() is called  
 * if it was called first before the checkout or if checkout
 * has been called more than once without calling checkin in the middle.
 * 
 * This func will compare the info in the config against all the entries in the
 * current userlist that we are building. If there is a match then we hit 
 * the bug so ignore this config
 * 
 * Parameters:
 * 		job		- the job that has the current userlist we are building
 * 		config	- the config we will use to contact the host and get the user list
 * 
 * Return:  (boolean)
 * 		1	- It's ok to query the host in the config.
 * 		0	- We found the bug. Skip this config.
 *********************************************************************/ 
static int is_not_bug_p6476(LM_HANDLE * job, CONFIG *config, int num_entries)
{
	/* iterate through the user list and check for matches */
	LM_USERS *	user;
	int 		index;
	
	for ( 	index = 0, user = job->userlist; 
			index < num_entries; 
			index++, user++)
	{
		/* Don't need to compare features. We wouldn't be here if 
		 * it didn't match */
		
		if ( config == user->ul_conf )
		{
			/* if the current config pointer matches this entry in the user list
			 * then we have not hit the bug. We just found a different user
			 * license. check the next user record */
			continue;
		}
		
		if (l_compare_version(job, user->version, config->version) != 0 )
			continue;	/* no match. this is not the bug. get next record */	

		/* everything matched to this point We found the bug.
		 * Skip this config. */
		return 0; /* false */		
	}
	return 1; /* true: ok to continue with this config */
}	
