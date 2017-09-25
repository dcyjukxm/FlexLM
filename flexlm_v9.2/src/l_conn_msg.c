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
 *	Module: $Id: l_conn_msg.c,v 1.4 2003/04/18 23:48:02 sluu Exp $
 *
 *	Function: l_conn_msg(job, daemon, msg)
 *
 *	Description: Creates the connection message.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) daemon - The daemon to connect to.
 *			(char *) message - Pointer to message area
 *
 *	Return:		Message filled in.
 *
 *	M. Christiano
 *	4/19/90
 *
 *	Last changed:  10/6/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#if defined( UNIX) && defined(ANSI)
#include <unistd.h>
#include <sys/types.h>
#endif
#ifdef PC
#include <process.h>
#endif

void l_fillin_platform lm_args((LM_HANDLE *, char *));	/*- Used only here and 
								l_file_ops */

void API_ENTRY
l_conn_msg(job, daemon, msg, transport, find_master)
LM_HANDLE *job;		/* Current license job */
char *daemon;			/* Daemon to connect to */
char *msg;			/* message - to be filled in */
int transport;			/* LM_TCP, LM_UDP, or LM_SPX */
int find_master;		/* if false, connect to the vd even if not
				   master */
{
  char *hostname;
  char *username;
  char *display;
  int comm_rev = job->daemon->our_comm_revision;

/*
 *	fill in the data for the connection message
 */
	hostname = lc_hostname(job, 1);
	username = lc_username(job, 1);
	display = lc_display(job, 1);
	(void) memset(msg, '\0', LM_MSG_LEN+1);
	if (find_master) msg[MSG_CMD] = LM_HELLO;
	else 		 msg[MSG_CMD] = LM_HELLO_THIS_VD;
	msg[MSG_HEL_VER] = '0' + job->daemon->our_comm_version;
	msg[MSG_HEL_VER+1] = '0' + job->daemon->our_comm_revision;
	(void) strncpy(&msg[MSG_HEL_NAME], username, MAX_USER_NAME);/* LONGNAMES */
	msg[MSG_HEL_NAME + MAX_USER_NAME] = '\0';
	(void) strncpy(&msg[MSG_HEL_HOST], hostname, MAX_SERVER_NAME);/* LONGNAMES */
	msg[MSG_HEL_HOST + MAX_SERVER_NAME] = '\0';
	(void) strncpy(&msg[MSG_HEL_DAEMON], daemon, MAX_DAEMON_NAME);/* LONGNAMES */
	msg[MSG_HEL_DAEMON + MAX_DAEMON_NAME] = '\0';
/*
 *	PC and VMS will have to punt here 
 */
	l_encode_long(&msg[MSG_HEL_PID], getpid());
	
	l_fillin_platform(job, msg);
	if (comm_rev >= 1)
	{
		(void) strncpy(&msg[MSG_HEL_DISPLAY], display,MAX_DISPLAY_NAME);/* LONGNAMES */
		msg[MSG_HEL_DISPLAY + MAX_DISPLAY_NAME] = '\0';
		if (comm_rev >= 3)
		{
/* 
 *			We check options struct, because the daemon struct 
 *			isn't yet set
 */
			switch (transport)
			{
			case LM_UDP:
				msg[MSG_HEL_REQ_COMM] = LM_COMM_UDP;
				l_encode_int(&msg[MSG_HEL_UDP_TIMEOUT],
					job->daemon->udp_timeout);
				break;
			case LM_TCP:
				msg[MSG_HEL_REQ_COMM] = LM_COMM_TCP;
/* 
 *				Note: maximum timeout for tcp is 255 minutes
 *				0 means No timeout
 *				Also, the message is sent in 3-minute
 *				increments.
 */
				l_encode_long_hex(&msg[MSG_HEL_TCP_TIMEOUT],
					(long) ((job->daemon->tcp_timeout/
						(LM_TCP_TIMEOUT_INCREMENT)) & 
						0xff));
				break;
#ifdef SUPPORT_IPX
			case LM_SPX:
				msg[MSG_HEL_REQ_COMM] = LM_COMM_SPX;
				break;
#endif /* SUPPORT_IPX */
				
#ifdef FIFO
			case LM_LOCAL:
				msg[MSG_HEL_REQ_COMM] = LM_COMM_LOCAL;
				/* strip off path */
				for (cp = job->localcomm->readname +
					strlen(job->localcomm->readname) - 1;
					cp > job->localcomm->readname; cp--)
				{
					if (*cp == '/')
					{
						cp++;
						break;
					}
				}
				strcpy(&msg[MSG_HEL_LOCAL], cp);
				/* strip of _r from end of name */
				msg[MSG_HEL_LOCAL + 
					(strlen(job->localcomm->readname) - 2)]
					= '\0';
				break;
#endif /* FIFO */
			}
			
		}
	}
	msg[MSG_HEL_FLEX_VER] = (unsigned char) job->code.flexlm_version;
	msg[MSG_HEL_FLEX_REV] = (unsigned char ) job->code.flexlm_revision;

	l_msg_cksum(msg, comm_rev, transport); /* Insert the checksum */
	if (transport == LM_UDP)
				l_write_sernum(msg, &job->daemon->udp_sernum);
}

void
l_fillin_platform(job, msg)
LM_HANDLE *job;
char *msg;
{
/*
 *	On unix, GPLATFORM is set by the gplatargs shell command
 *      On VMS, it's enumerated, since we can't seem to pass a simple  
 *              quoted string from a command line.
 */
	if (*job->options->platform_override)
		l_zcp(&msg[MSG_HEL_PLATFORM], 
			job->options->platform_override, MAX_PLATFORM_NAME);
	else
		l_zcp(&msg[MSG_HEL_PLATFORM], l_platform_name(),
							MAX_PLATFORM_NAME);

}
