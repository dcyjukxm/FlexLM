/******************************************************************************

	    COPYRIGHT (c) 1989, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lm_remove.c,v 1.13 2003/04/18 23:48:08 sluu Exp $
 *
 *	Function: lc_remove
 *
 *	Description: Remove a user/host/display from a specified feature.
 *
 *	Usage:	lc_remove(feature, user, host, display)
 *
 *	M. Christiano
 *	4/3/89
 *	extracted from utils/lmremove.c v2.4
 *
 *	Last changed:  9/9/98
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include "lm_attr.h"
#include <stdio.h>
#include <errno.h>

#ifdef ANSI
#include <string.h>
#else
#ifndef PC
char *strncpy();
#endif
#endif

#ifdef USE_WINSOCK
#include <pcsock.h>
#endif

static send_and_check();

int API_ENTRY
lc_remove(job, feature, user, host, display)
LM_HANDLE *job;		/* Current license job */
char *feature;
char *user;
char *host;
char *display;
{
#ifdef VMS
  char *p;
#endif
  char msg[LM_MSG_LEN+1];
  CONFIG *c;
  int d = LM_BAD_SOCKET;
  short	bRetBorrowEarly = 0;

	if (LM_API_ERR_CATCH) return job->lm_errno;

	if (!display) display="/dev/tty";

	(void) memset(msg, 0, LM_MSG_LEN+1);
	c = lc_get_config(job, feature);
	if (!c)
	{
		LM_API_RETURN(int, job->lm_errno)
	}
	if (job->daemon && (job->daemon->socket != LM_BAD_SOCKET))
		d = job->daemon->socket;
	if ((!c->conf_state))
		d = l_connect(job, c->server, c->daemon, job->options->commtype);
	if (d < 0)
	{
		/*LM_SET_ERRNO(job, LM_CANTCONNECT, 193, -1);*/
		LM_API_RETURN(int, job->lm_errno)
	}
#ifdef VMS
/*
 *	Turn the username, hostname, and displayname into uppercase
 */
	l_uppercase(user);
	l_uppercase(host);
	l_uppercase(display);
#endif
	msg[MSG_CMD] = LM_REMOVE; 
	(void)strncpy(&msg[MSG_REMOVE_USER], user, MAX_USER_NAME); /* LONGNAMES */
	msg[MSG_REMOVE_USER + MAX_USER_NAME] = '\0';
	(void)strncpy(&msg[MSG_REMOVE_FEAT], feature, MAX_FEATURE_LEN); 
	msg[MSG_REMOVE_FEAT + MAX_FEATURE_LEN] = '\0';
	(void)strncpy(&msg[MSG_REMOVE_HOST], host, MAX_SERVER_NAME); /* LONGNAMES */
	msg[MSG_REMOVE_HOST + MAX_SERVER_NAME] = '\0';
	(void)strncpy(&msg[MSG_REMOVE_DISP], display, MAX_DISPLAY_NAME); /* LONGNAMES */
	msg[MSG_REMOVE_DISP + MAX_DISPLAY_NAME] = '\0';/* LONGNAMES */
	msg[MSG_REMOVE_FORCE] = 0;
	if (send_and_check(job, msg, d))
		LM_API_RETURN(int, job->lm_errno)
	else
		LM_API_RETURN(int, 0)
}


int API_ENTRY
l_return_early(
	LM_HANDLE *	job,
	char *		feature,
	char *		vendor,
	char *		user,
	char *		host,
	char *		display)
{
#ifdef VMS
	char *p;
#endif
	char msg[LM_MSG_LEN+1];
	int d = LM_BAD_SOCKET;
	int iFound = 0;
	CONFIG *	pConf = NULL;
	CONFIG *	pPos = NULL;

	if (LM_API_ERR_CATCH)
		return job->lm_errno;

	if (!display)
		display="/dev/tty";

	/*
	 *	Check to make sure we have something/one to contact
	 *	Make sure db has been initialized
	 */
	if (job->line == (CONFIG *) NULL)
	{
		l_init_file(job);
		/*
		 *	Read data from registry and put into list.
		 */
		if(job->l_new_job)
			l_read_borrow(job, feature);
	}
	(void) memset(msg, 0, LM_MSG_LEN+1);
	/*
	 *	Setup message
	 */
#ifdef VMS
/*
 *	Turn the username, hostname, and displayname into uppercase
 */
	l_uppercase(user);
	l_uppercase(host);
	l_uppercase(display);
#endif
	msg[MSG_CMD] = LM_REMOVE; 
	(void)strncpy(&msg[MSG_REMOVE_USER], user, MAX_USER_NAME); /* LONGNAMES */
	msg[MSG_REMOVE_USER + MAX_USER_NAME] = '\0';
	(void)strncpy(&msg[MSG_REMOVE_FEAT], feature, MAX_FEATURE_LEN); 
	msg[MSG_REMOVE_FEAT + MAX_FEATURE_LEN] = '\0';
	(void)strncpy(&msg[MSG_REMOVE_HOST], host, MAX_SERVER_NAME); /* LONGNAMES */
	msg[MSG_REMOVE_HOST + MAX_SERVER_NAME] = '\0';
	(void)strncpy(&msg[MSG_REMOVE_DISP], display, MAX_DISPLAY_NAME); /* LONGNAMES */
	msg[MSG_REMOVE_DISP + MAX_DISPLAY_NAME] = '\0';/* LONGNAMES */
	msg[MSG_REMOVE_FORCE] = 1;

	/*
	 *	Check for existing connection
	 */
	if (job->daemon && (job->daemon->socket != LM_BAD_SOCKET))
	{
		d = job->daemon->socket;
		/*
		 *	Since connection already present, will only
		 *	attempt to return to this server.
		 */
		if(send_and_check(job, msg, job->daemon->socket))
			LM_API_RETURN(int, job->lm_errno)
		else
			LM_API_RETURN(int, 0)

	}
	else
	{
		while (pConf = l_next_conf_or_marker(job, feature,
						&pPos, 1, (char *)NULL))
		{
			if(l_connect(job, pConf->server, vendor, job->options->commtype) < 0)
				continue;
			else
			{
				iFound = 1;
				if(!send_and_check(job, msg, job->daemon->socket))
				{
					LM_API_RETURN(int, 0)
				}
				else
				{
					/*
					 *	an error occurred, close this connection and try another.
					 */
					lc_disconn(job, 0);
				}
			}
		}
		if(!iFound)
		{
			LM_SET_ERRNO(job, LM_CANTCONNECT, 617, -1);
		}
		LM_API_RETURN(int, job->lm_errno)
	}
}

int API_ENTRY
lc_removeh(job, feature, server, port, handle)
LM_HANDLE *job;		/* Current license job */
char *feature;
char *server;
int port;
char *handle;
{
  char msg[LM_MSG_LEN+1];
  CONFIG *c, *l_lookup_serv(), *pos = 0;
  int d = 0;

	
	if (LM_API_ERR_CATCH) return job->lm_errno;

	while (c = l_next_conf_or_marker(job, feature, &pos, 0, 0))
	{
		if (!strcmp(c->server->name, server) && 
			(c->server->port == port ) ||
			(c->server->port == -1 && 
			port >= LMGRD_PORT_START &&
			port <= job->port_end ))
		break;	
	}
	if (!c)
	{
		if (!job->lm_errno)
			LM_SET_ERRNO(job, LM_NOFEATURE, 411, net_errno);
			
		LM_API_RETURN(int, job->lm_errno)
	}
	if (job->daemon && (job->daemon->socket != LM_BAD_SOCKET))
		d = job->daemon->socket;
	if ((!c->conf_state))
		d = l_connect(job, c->server, c->daemon, job->options->commtype);
	if (d < 0)
	{
		LM_API_RETURN(int, job->lm_errno)
	}
	(void) memset(msg, '\0', LM_MSG_LEN+1);
	msg[MSG_CMD] = LM_REMOVEH;
	(void)strncpy(&msg[MSG_REMOVEH_FEAT], feature, MAX_FEATURE_LEN); 
	(void)strncpy(&msg[MSG_REMOVEH_HANDLE], handle, MAX_LONG_LEN); 
	if (send_and_check(job, msg, d))
		LM_API_RETURN(int, job->lm_errno)
	else
		LM_API_RETURN(int, 0)
}

static
send_and_check(job, msg, d)
LM_HANDLE *job;		/* Current license job */
char *msg;
int d;
{
  int i;
  int msgsize = l_msg_size(job->daemon->our_comm_revision);
  char type, *msgparam;

	l_msg_cksum(msg, 
		job->daemon->our_comm_revision,
		job->daemon->commtype);
	if (job->daemon->commtype == LM_UDP)
		l_write_sernum(msg, &job->daemon->udp_sernum);
	if(job->daemon && job->daemon->encryption)
	{
		l_str_crypt(msg, msgsize, job->daemon->encryption, 1);		
	}
	i = network_write(d, msg, msgsize);
	if (i < msgsize)
	{
		LM_SET_ERRNO(job, LM_CANTWRITE, 197, net_errno);
		return LM_CANTWRITE;
	}

	if (l_rcvmsg(job, &type, &msgparam) == 0)
	{
		; /* use errno from l_rcvmsg() */
		/*
		LM_SET_ERRNO(job, LM_CANTREAD, 198, net_errno);
		*/
		return job->lm_errno;
	}
	else if (type == LM_OK)
	{
		return 0;
	}
	else if (type == LM_NOT_ADMIN)
	{
		LM_SET_ERRNO(job, LM_NOTLICADMIN, 199, 0);
		return LM_NOTLICADMIN;
	}
	else if (type == LM_TOO_SOON)
	{
		LM_SET_ERRNO(job, LM_REMOVETOOSOON, 200, 0);
		return LM_REMOVETOOSOON;
	}
	else if (type == LM_QUEUED)
	{
		LM_SET_ERRNO(job, LM_REMOVE_LINGER, 398, 0);
		return LM_REMOVE_LINGER;
	}
	else if (type == LM_NO_SUCH_FEATURE)
	{
		LM_SET_ERRNO(job, LM_NOSERVSUPP, 201, 0);
		return LM_NOSERVSUPP;
	}
	else if (type == LM_NO_USER)
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 202, 0);
		return LM_BADPARAM;
	}
	else if (type == LM_NO_RETURN_EARLY)
	{
		LM_SET_ERRNO(job, LM_BORROW_RETURN_EARLY_ERR, 592, 0);
		return LM_BORROW_RETURN_EARLY_ERR;
	}
	else
	{
		LM_SET_ERRNO(job, LM_BADCOMM, 203, 0);
		return LM_BADPARAM;
	}
}
