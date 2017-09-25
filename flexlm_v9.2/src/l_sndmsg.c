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
 *	Module: $Id: l_sndmsg.c,v 1.8 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function: l_sndmsg(job, type, param)
 *
 *	Description: Sends a message to the server/client
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char) type - The message type
 *			(char *) param - The message parameter
 *
 *	Return:		(int) - NULL - errror, see job->lm_errno
 *			<> 0  - OK
 *
 *	M. Christiano
 *	2/15/88
 *
 *	Last changed:  12/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include <stdio.h>
#include <errno.h>
#include <time.h>

#ifdef USE_WINSOCK
#include <pcsock.h>
#endif

#ifdef RELEASE_VERSION
#define DEBUG(x)
#else
#define DEBUG(x)        if (l_sndmsg_debug) (void) printf x
#endif

API_ENTRY
l_sndmsg(job, type, param)
LM_HANDLE *job;		/* Current license job */
int type;	/* The message type (NOTE: C promotes this to an int) */
char *param;	/* The message parameter */
{

  char msg[LM_MSG_LEN+1];
  int size = l_msg_size(job->daemon->our_comm_revision);
#ifndef RELEASE_VERSION
  char * l_sndmsg_debug =  l_real_getenv("L_SNDMSG_DEBUG");
#endif
  static int test_udp_sernum = -1;
  int test_this_time = 0;
  int dont_write = 0;
  int write_tries = 1;
  char cmd;
  int wfd =
#ifdef FIFO
  (job->daemon->commtype == LM_LOCAL) ? job->localcomm->wfd :
#endif
							job->daemon->socket;

	if (wfd == LM_BAD_SOCKET)
	{
		LM_SET_ERRNO(job, LM_NOSOCKET, 96, 0);
		return(0);
	}
	else
	{
		DEBUG(("Sending %c message (%d) (time %d)\n", type, type,
					time(0)));
		memset(msg, 0, sizeof(msg));
		msg[MSG_CMD] = type;
		(void) bcopy(param, &msg[MSG_DATA], LM_MSG_LEN - MSG_DATA);
		l_msg_cksum(msg, job->daemon->our_comm_revision,
			job->daemon->commtype);
		if (job->daemon->commtype == LM_UDP)
			l_write_sernum(msg, &job->daemon->udp_sernum);
							/* Insert checksum */
		if (test_udp_sernum == -1)
		{
		    if (job->daemon->commtype == LM_TCP)
			test_udp_sernum = 0;
#ifdef SUPPORT_IPX
		    if (job->daemon->commtype == LM_SPX)
			test_udp_sernum = 0;
#endif /* SUPPORT_IPX */
		    else if (getenv("TEST_UDP_SERNUM"))	/* overrun checked */
			sscanf(getenv("TEST_UDP_SERNUM"), "%d",	/* overrun don't care */
			    &test_udp_sernum);
		    else
			test_udp_sernum = 0;

		    if (test_udp_sernum < 0)
		    {
				test_udp_sernum *= -1;
				if (getenv("DEBUG_UDP"))
					(void) printf("TEST_UDP_SERNUM\n");	/* overrun checked */
		    }
		    else if (test_udp_sernum )
		    {
			srand((unsigned int) time(0));
			if (getenv("DEBUG_UDP"))	/* overrun checked */
			    (void) printf("TEST_UDP_SERNUM random\n");
		    }
		}

		if (test_udp_sernum && ((rand() % test_udp_sernum) == 0))
		{
			static int tested_last_time = 0;
			if (tested_last_time)
			    tested_last_time = 0;
			else
			{
			    test_this_time = tested_last_time = 1;
			    if ((rand()%2) == 0)
				    dont_write = 1;
			}
		}
/*
 *		Encrypt it, if necessary
 */
		cmd = *msg;
		if (job->daemon->encryption)
		       (void) l_str_crypt(msg, size, job->daemon->encryption,
									1);
		if (test_this_time && dont_write)
		{
			if (getenv("DEBUG_UDP"))	/* overrun checked */
				(void) printf("not writing (%c/%d)\n", cmd, cmd);
		}
		else
		{
		  int write_cnt;

			if (job->daemon->commtype == LM_UDP)
				write_tries=3;
			for (write_cnt =0;
				write_cnt != size && write_tries > 0;
				write_tries--)
			{

				LM_SET_NET_ERRNO(0);
				DEBUG(("writing fd %d\n", wfd));
				write_cnt =
				  network_write(wfd,
					msg, size);
				DEBUG(("writing bytes %d(err %d)\n",
							write_cnt, net_errno));

#ifdef PC
				/*
				 *	This is to work around a PCNFS v5.1a
				 *	problem	where WinSock send() returns
				 *	WSAEWOULDBLOCK on a blocking socket.
				 */
				if ( write_cnt == -1 )
				{
					if ( net_errno == WSAEWOULDBLOCK )
					{
						write_tries = 3;
						Yield();
						continue;
					}
				}
#endif /* PC */

#if !defined(RELEASE_VERSION) && !defined(PC)
				if (l_real_getenv("DEBUG_WRITE"))
				{
					if (net_errno)
						perror("write");
					printf("Writing %d bytes: %c\n",
								size, *msg);
				}

#endif
			}

			if (write_cnt != size)
			{
				if (net_errno == NET_ECONNRESET) lc_disconn(job, 1);
				LM_SET_ERRNO(job, LM_CANTWRITE, 97, net_errno);
				return(0);
			}
			if (test_this_time)
			{
				network_write(wfd,
								msg, size);
				if (getenv("DEBUG_UDP"))	/* overrun checked */
				    (void) printf("writing twice (%c/%d)\n", cmd, cmd);
			}
		}
		if (type != LM_HEARTBEAT &&
			job->daemon->commtype == LM_UDP)
		{
			memcpy(job->last_udp_msg, msg, size);
		}
		return(1);
	}
}

void
l_resend_cmd(job)
LM_HANDLE *job;		/* Current license job */
{
  int size = l_msg_size(job->daemon->our_comm_revision);
#ifndef RELEASE_VERSION
	if (getenv("DEBUG_WRITE"))	/* overrun checked */
		printf("Resending %d bytes: %c\n", size,
					*job->last_udp_msg);
#endif
	(void) network_write(job->daemon->socket, job->last_udp_msg, size);
}
