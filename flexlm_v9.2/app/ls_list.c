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
 *	Module: $Id: ls_list.c,v 1.12 2003/04/18 23:47:53 sluu Exp $
 *
 *	Function: ls_list(cmd, user)
 *
 *	Description: Processes the "LIST USERS" command.
 *
 *	Parameters:	(char *) cmd - The command from the client
 *			(CLIENT_DATA *) user - The client data for the client.
 *
 *	Return:		None - response is returned directly to the client.
 *
 *	M. Christiano
 *	2/27/88
 *
 *	Last changed:  11/16/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "ls_comm.h"	/* Server-server comm */
#include "lsfeatur.h"	/* Dynamic feature data */
#include "ls_aprot.h"	
#ifdef ANSI
#include <stdlib.h>
#endif /* ANSI */
extern int ls_show_vendor_def;	/* Send vendor_def data back to client */
static void ls_res_send();


void
ls_list(cmd, user)
char *cmd;
CLIENT_DATA *user;
{
  char msg[LM_MSG_LEN+1];	/* For ls_client_send */
  int n, lic;
  USERLIST *u, *queue, *temp;
  OPTIONS *o, *x;
  char data[LM_MSG_LEN+1];
  int num, i, j;
  int borrow;
  int tot;

	(void)memset(msg, 0, sizeof(msg)); /*- purify umr fix */

	if (user->comm_revision < 3)
	{
		cmd[MSG_LIST_CODE] = '\0'; /* force it */
	}
	else
		borrow = (cmd[MSG_LIST_BFLAG] != '\0');
/*
 *	If we are the top-level server, process this command directly.
 *	If not, we need to get the data from "topdog"
 */

	u = f_list(&cmd[MSG_LIST_FEATURE], &cmd[MSG_LIST_CODE], 
				&n, &lic, &tot, &queue, &o, 0, NULL);
	temp = queue;
	while (temp) { temp = temp->next; n++; }
	for (x = o; x; x = x->next)
		if (x->type == RESERVE || x->type == DYNAMIC_RESERVE || x->type == OPT_BORROW) n++;
	l_encode_int(&msg[MSG_NUSERS_N], n);
/*
*		Pre- COMM v1.1 clients will ignore this stuff
*/
	l_encode_long(&msg[MSG_NUSERS_TIME], ls_currtime);
	l_encode_int(&msg[MSG_NUSERS_TOT], tot);
	ls_client_send(user, LM_NUSERS, msg);
/*
*		First, send the "normal" users.
*/
	while (u)
	{
		num = f_count(u, &i, &j);
		num += i;
		ls_send_vendor_def(user, data, u->vendor_def);
		ls_send_user(user, data, u->name, u->node, u->display, 
				num, u->time, u->version, 
				u->linger, u->license_handle, u->flags);
		u = u->next;
	}
/*
*		Next, the queued users
*/
	while (queue)
	{
		ls_send_vendor_def(user, data, queue->vendor_def);
		ls_send_user(user, data, queue->name, queue->node,
				queue->display, -(queue->count), 
				queue->time, queue->version,
				queue->linger,
				queue->license_handle, queue->flags);
		queue = queue->next;
	}
/*                                   
*		Finally, give them the reservations.
*/
	for (x = o; x; x = x->next)
	{
		if (x->type == RESERVE || x->type == DYNAMIC_RESERVE || x->type == OPT_BORROW)
		{
			ls_res_send(user, data, x, borrow);
		}
	}
}

void
ls_send_user(user, buf, name, node, display, num, timeon, version,
	     linger, handle, flags)
CLIENT_DATA *user;
char *buf;
char *name;
char *node;
char *display;
int num;
long timeon;
char *version;
int linger;	/* Linger interval */
int handle;	/* License handle */
int flags;
{
	/*
	 *	This function needs to be modified to deal with long user/file/hostnames
	 *	LONGNAMES
	 */
	(void) bzero(buf, LM_MSG_LEN+1);
	if (user->comm_revision == 0)
	{
	  char x[MAX_USER_NAME + MAX_SERVER_NAME + 100];	/* LONGNAMES */

		(void) sprintf(x, "%s@%s", name, node);	/* OVERRUN */
		(void) strncpy(&buf[MSG_U_NAME_v1_0], x, MAX_USER_NAME + 
							MAX_SERVER_NAME + 1);	/* LONGNAMES */
		(void) sprintf(&buf[MSG_U_NUM_v1_0], "%d", num);
		(void) sprintf(&buf[MSG_U_TIME_v1_0], "%ld", timeon);
	}
	else
	{
		l_zcp(&buf[MSG_U_NAME], name, MAX_USER_NAME);/* LONGNAMES */
		l_zcp(&buf[MSG_U_NODE], node, MAX_SERVER_NAME);/* LONGNAMES */
		l_zcp(&buf[MSG_U_DISP], display, MAX_DISPLAY_NAME);/* LONGNAMES */
		l_encode_int(&buf[MSG_U_NUM], num);
/*
 *		For historic reasons, we have 3 bytes we can encode into
 *		this field.  AS of v8 we're only using 1 bit.  But someday
 *		I'd like room for more, especially since MSG_U_ can't
 *		be expanded.
 */
		buf[MSG_U_FLAGS] = (flags >> 16) && 0xff;
		buf[MSG_U_FLAGS + 1] = (flags >> 8) && 0xff;
		buf[MSG_U_FLAGS + 2] = flags & 0xff;
		l_encode_long(&buf[MSG_U_TIME], timeon);
		l_zcp(&buf[MSG_U_VER], version, MAX_VER_LEN);

		if (user->comm_revision >= 3)
		{
			l_encode_int(&buf[MSG_U_HANDLE], handle);
			l_encode_int(&buf[MSG_U_LINGER], linger);
		}
	}
	ls_client_send(user, LM_USERNAME, buf);
}

void
ls_send_vendor_def(user, buf, data)
CLIENT_DATA *user;
char *buf;
char *data;
{
	(void) bzero(buf, LM_MSG_LEN+1);
	if (ls_show_vendor_def && user->comm_revision >= 3)
	{
		(void) strncpy(&buf[MSG_DATA], data, MAX_VENDOR_CHECKOUT_DATA);
		ls_client_send(user, LM_USERNAME2, buf);
	}
}

static
void
ls_res_send(user, buf, opt, borrow)
CLIENT_DATA *user;
char *buf;
OPTIONS *opt;
int borrow;
{
  int type = opt->type2;
  char nambuf[50];
  char *name = opt->name;

	(void) bzero(buf, LM_MSG_LEN+1);
	if (user->comm_revision == 0)
	{
		if (type == RES_USER_v1_0) 
			(void) strcpy(&buf[MSG_U_NAME_v1_0], "U:");
		else 
			(void) strcpy(&buf[MSG_U_NAME_v1_0], "H:");
		(void) strncpy(&buf[MSG_U_NAME_v1_0] + 2, name ? name : "",
					MAX_USER_NAME + MAX_SERVER_NAME - 1);/* LONGNAMES */
		(void) strcpy(&buf[MSG_U_NUM_v1_0], "1");
		(void) strcpy(&buf[MSG_U_TIME_v1_0], "0");
	}
	else
	{
	  char x[MAX_USER_NAME + MAX_SERVER_NAME];/* LONGNAMES */

		if ((type == OPT_INTERNET ) || 
			((type == OPT_HOST) && !opt->name))
		{
			l_addr_to_inet(&opt->inet_addr[0], opt->addr_type, x);
			name = x;
		}

		if (opt->type == OPT_BORROW)   
		{
			if (borrow)		/* This client can handle it */
				buf[MSG_U_NAME] = 'B';
			else
				buf[MSG_U_NAME] = 'H'; /* Fake BORROW as
							  HOST reservation */
		}
		else if (type == OPT_USER)     buf[MSG_U_NAME] = 'U';
		else if (type == OPT_HOST)     buf[MSG_U_NAME] = 'H';
		else if (type == OPT_DISPLAY)  buf[MSG_U_NAME] = 'D';
		else if (type == OPT_GROUP)    buf[MSG_U_NAME] = 'G';
		else if (type == OPT_HOST_GROUP) buf[MSG_U_NAME] = 'K';
		else if (type == OPT_INTERNET) buf[MSG_U_NAME] = 'I';
		else if (type == OPT_PROJECT)     buf[MSG_U_NAME] = 'P';
		else if (type == OPT_BUNDLE)
		{
			/*
			 *	Since a FLEXlm packet is only 147 bytes, and there ISN'T a place where we can specify
			 *	vendor info when SUITE_DUP_GROUP has a V, just add an additional designation here to
			 *	specify that the BUNDLE has V set.  Don't need to actually send what V is set to as
			 *	that info is up to ISV and shouldn't be visible to the user anyway.
			 */
			if(opt->pszVendor)
				buf[MSG_U_NAME] = 'V';
			else
				buf[MSG_U_NAME] = 'L';
		}
		if(type == OPT_BUNDLE)
		{
			int len = _U_LEN;
			if(opt->pszUser)
				strncpy(&buf[MSG_U_NAME] + 1, opt->pszUser, MAX_USER_NAME);/* LONGNAMES */
			if(opt->pszHost)
				strncpy(&buf[MSG_U_NODE], opt->pszHost, MAX_SERVER_NAME);/* LONGNAMES */
			if(opt->pszDisplay)
				strncpy(&buf[MSG_U_DISP], opt->pszDisplay, MAX_DISPLAY_NAME);/* LONGNAMES */
		}
		else
		{
			(void) strncpy(&buf[MSG_U_NAME] + 1, name,
						MAX_USER_NAME + MAX_SERVER_NAME - 1);/* LONGNAMES */
		}
		(void) strcpy(&buf[MSG_U_NUM], "1");
		(void) strcpy(&buf[MSG_U_TIME], "0");
	}
	ls_client_send(user, LM_USERNAME, buf);
}
