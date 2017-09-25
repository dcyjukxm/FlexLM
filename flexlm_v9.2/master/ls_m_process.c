/*****************************************************************************

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
 *	Module: $Id: ls_m_process.c,v 1.31.2.3 2003/06/24 20:32:12 sluu Exp $
 *
 *	Function: ls_m_process(ready_mask, nrdy, select_mask, tcp_s, spx_s,
 *							daemon, master)
 *
 *	Description: Processes waiting descriptors for the master server
 *
 *	Parameters:	(SELECT_MASK) ready_mask - Descriptors that can be read
 *			(int) nrdy - The number of ready descriptors.
 *			(int) select_mask - The "master" select mask.
 *			(LM_SOCKET) tcp_s - The tcp socket to accept connections on.
 *			(LM_SOCKET) spx_s - The tcp socket to accept connection
 *			(DAEMON **) daemon - DAEMONs we have started.
 *			(char *) master - The master node name
 *
 *	Return:		(int) - <> 0 -> vendor daemon info updated
 *				== 0 -> no update
 *
 *	Side effects:	select_mask - Updated for new/broken connections.
 *			server data updated as servers come up or go down.
 *
 *	M. Christiano
 *	2/27/88
 *
 *	Last changed:  1/8/99
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsmaster.h"
#include "lssignal.h"
#include "ls_glob.h"
#include "lmselect.h"
#include "l_openf.h"
#include "ls_comm.h"		/* Server-server comm protocol */
#include "../machind/lsfeatur.h"
#include "../app/ls_aprot.h"
#include "l_m_prot.h"
#include "flexevent.h"
#include <errno.h>

#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
  extern char *sys_errlist[];
#endif

unsigned int lmdown_ipaddr; /* if 0, the shutdown is from a kill on this host
			                       *	If non-zero, the ipaddr of the
			                       *	of the user doing the lmdown
			                       */


#ifdef ANSI
#include <string.h>
#else
extern char *strncat();
#endif

#include <fcntl.h>
#ifdef PC
#include <stdlib.h>
#ifndef NLM
#include <process.h>
#endif
#include <winsock.h>
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#endif /* WINNT */
#ifndef sco
#include <sys/uio.h>
#endif

static SELECT_MASK last_select_mask;
static int first = 1;
static int lasterrno = 0;
static int howmanyerrors = 0;
static doit();			/* Process the incomming command */
static int decide_transport lm_args(( int daemon_recommended,
					int *daemon_reason,
					int client_requested,
					DAEMON *daemon));
static int ls_m_reread lm_args(( char *, DAEMON **, char *, CLIENT_DATA *,
						char *, int));
#ifndef NO_LMGRD
static
#endif
char * getdlist lm_args((lm_noargs));
static void ls_old_serv_vers lm_args((CLIENT_DATA *));
extern int ls_allow_lmdown;
extern int ls_allow_lmremove;
extern 	int ls_msg_cnt;
extern int ls_msg_cnt_this_minute;
extern char **argv_sav;
extern int argc_sav;
#ifndef NO_LMGRD
static
#endif
char * getlf lm_args((char *));

#ifndef RELEASE_VERSION
static int debug_m_process = -99;
#include "lcommdbg.h"
#endif
extern char ls_our_hostname[];
#ifndef NO_GETDOMAINNAME_CALL
int add_domain_name = 0;
#endif
static void ls_m_hello lm_args((char *, CLIENT_ADDR *, DAEMON **, char *,
							CLIENT_DATA *, char *));
#if defined( TRUE_BSD) || defined(SUNOS4)
extern char *getwd();
#define GETWD(x) getwd(x)
#else
extern char *getcwd();
#define GETWD(x) getcwd(x, LM_MAXPATHLEN-1)
#endif /* TRUE_BSD */
char *only_this_vendor;

#ifndef NO_LMGRD
static int debug = 0;
int unpriv_utils = 1;           /* Do lmdown/lmremove not require ADMIN priv */


int
ls_m_process(
	SELECT_MASK	ready_mask,
	int			nrdy,
	SELECT_MASK	select_mask,
	LM_SOCKET	tcp_s,
	LM_SOCKET	spx_s,
	DAEMON **	daemon,
	char *		master)
{
	char				resp[LM_MSG_LEN+1] = {'\0'};
	int					len = 0;
	unsigned int		fromlen = 0;
	struct sockaddr_in	from;
	LM_SERVER *			ls = NULL;
	int					ret = 0; /* must only be set by doit -- it's just a reread flag */
	CLIENT_ADDR			new_sock_ca, msg_ca;
	int					size = 0;
	int					size2 = 0;
	int					comm_revision = 0;
	CLIENT_DATA *		client = NULL;

	char				szBuffer[MAX_PATH * 2] = {'\0'};
	char *				ppszInStr[20] = {NULL};	/* Should be more than enough */


	memset(&from, '\0', sizeof(struct sockaddr_in));
	memset(&msg_ca, '\0', sizeof(msg_ca));
	memset(&new_sock_ca, '\0', sizeof(new_sock_ca));
	memset(resp, '\0', sizeof(resp));
	msg_ca.is_fd = new_sock_ca.is_fd = 1;
	msg_ca.transport = new_sock_ca.transport = LM_TCP;
	msg_ca.addr.fd = 0;


	if (first)
	{
		first = 0;
		if (getenv("LS_M_PROCESS"))
			debug = 1;
		MASK_CREATE(last_select_mask);
	}
	if (nrdy <= 0)  /* failed select */
	{
		/*- ls_m_process.firstpart went here 11/11/88 */
		if (net_errno != NET_EINTR)
		{
			if ((net_errno == NET_EBADF) || (net_errno==NET_ENOTSOCK))
			{
				int i;

				while ((i = ls_findbad(select_mask)) >= 0)
				{
					ls_badserv(i);
				}
			}
			if (howmanyerrors > 1000)	/* 1000 is plenty */
			{
				lasterrno = net_errno;
				howmanyerrors = 0;
				LOG((lmtext("INTERNAL ERROR in the UNIX select() call\n")));
				LOG((lmtext("Please report the following information to your vendor:\n")));
#ifndef USE_WINSOCK
				LOG((lmtext("%s: select, select_mask: %x, last_select_mask: %x "),
						SYS_ERRLIST(errno), select_mask[0],
						last_select_mask[0]));
				_LOG((lmtext("(ready mask: %x)\n"),	ready_mask[0]));
#endif /* USE_WINSOCK */
			}
			else if (net_errno != lasterrno)		/* reset */
			{
				lasterrno = net_errno;
				howmanyerrors = 0;
			}
			howmanyerrors++;
		}
		return ret;
	}

/*
 * 	successful select
 */
	MASK_COPY(last_select_mask, select_mask); /* For select debug */

/*
 *	Now, loop through all ready fds.  Note that a failure on a
 *	particular fd must do a 'continue' and not return out of this
 *	function, or other fds will be ignored
 */

	while (ANY_SET((int *)ready_mask))
	{
		ls_msg_cnt ++;
		ls_msg_cnt_this_minute ++;
		while (IS_SET(ready_mask, msg_ca.addr.fd) == 0)
			msg_ca.addr.fd++;
		MASK_CLEAR(ready_mask, msg_ca.addr.fd);	/* Clear this one */

		if (
#ifdef SUPPORT_IPX
			(msg_ca.addr.fd == spx_s) ||
#endif
			(msg_ca.addr.fd == tcp_s))
		{
/*
 *			We got a new connection pending... process it
 */
			fromlen = sizeof(from);

			new_sock_ca.addr.fd = accept(msg_ca.addr.fd,
				(struct sockaddr *)&from, &fromlen);

			if (new_sock_ca.addr.fd == LM_BAD_SOCKET)
			{
				DLOG(("accept: %s\n", SYS_ERRLIST(net_errno)));
#if 0
				/*
				 *	sluu: 6/24/03
				 *	There appears to be some timing related problem here because
				 *	when running as a service, we fall into this block of code.
				 *	Doing a WSAGetLastError() on this comes back with a "An 
				 *	operation was attempted on something that is not a socket."
				 *	error, but only sometimes.  For now, just don't log it to the
				 *	event log.
				 */
				sprintf(szBuffer, "%s", SYS_ERRLIST(net_errno));
				if(l_flexEventLogIsEnabled())
				{
					ppszInStr[0] = szBuffer;
					l_flexEventLogWrite(lm_job,
										FLEXEVENT_ERROR,
										CAT_FLEXLM_LMGRD,
										MSG_FLEXLM_LMGRD_BAD_SOCKET,
										1,
										ppszInStr,
										0,
										NULL);
				}
#endif
				continue;
			}
/*
 *			accept succeeded
 */
#ifndef RELEASE_VERSION
			{
				static char *dd = (char *)-1;
			  	if (dd == (char *)-1)
					dd = l_real_getenv("LS_DOWN_DEBUG");
				if (dd)
				{
					DLOG(("accept fd %d port %d\n",	new_sock_ca.addr.fd, ntohs(from.sin_port)));
				}
			}
#endif

			ls_c_init(&new_sock_ca, -1);
			MASK_SET(select_mask, new_sock_ca.addr.fd);
			continue;
		}
/*
 *		msg_ca.addr.fd != tcp_s
 *		We got a msg from existing connection
 *
 *		Lookup the Client in the client database
 */

		if ((client = ls_lookup_client(&msg_ca)) == NULL)
		{
			/*This should never happen!*/
			DLOG(("INTERNAL ERROR Received msg from "));
			_DLOG(("Uninitialized client-not processing %d",
					msg_ca.addr.fd));
			_DLOG((lmtext(" (%s %d)\n"),__FILE__,__LINE__));
			continue;
		}


		if (client->comm_revision >= 0)
			size = l_msg_size(client->comm_revision);
		else
			size = LM_MSG_LEN_MIN;

		size2 = size;
/*
 *	    	If we don't know the comm revision of the client,
 *	    	first read the minimum size message any compatible
 *	    	client could send.  If it is a HELLO message, we
 *	    	know how much more to read.
 */
		LM_SET_NET_ERRNO( 0);
		len = l_read_timeout(msg_ca.addr.fd, resp,
				(long)size, (long)LS_READ_TIMEOUT);
#ifndef RELEASE_VERSION
		if (debug_m_process == -99)
			debug_m_process = (int)l_real_getenv("DEBUG_M_PROCESS");
		if (debug_m_process)
			fprintf(stdout, "read fd %d bytes %d\n", msg_ca.addr.fd, len);

#endif
		if (len > 0) /* success read */
		{
			long secs_now;  /* Current time */
			int x;          /* For l_get_date */
#ifdef THREAD_SAFE_TIME
			struct tm tst;
#endif
/*
*			Update the "last time heard from" time for this server
*/
			for (ls = ls_s_first(); ls; ls = ls->next)
			{
				if (msg_ca.addr.fd == ls->fd1 ||
					msg_ca.addr.fd == ls->fd2)
				{
#ifdef THREAD_SAFE_TIME
					l_get_date(&x, &x,&x, &secs_now, &tst);
#else /* !THREAD_SAFE_TIME */
					l_get_date(&x, &x,&x, &secs_now);
#endif
					ls->exptime = secs_now;
				}
			}
		}
		if ((len > 0) && ((resp[MSG_CMD] == LM_HELLO) ||
			(resp[MSG_CMD] == LM_HELLO_THIS_VD) ||
			(resp[MSG_CMD] == LM_SHELLO) ||
			(resp[MSG_CMD] == LM_VHELLO)))
		{
			comm_revision = resp[MSG_HEL_VER+1] - '0';
			if (resp[MSG_HEL_VER+1] == '\0') comm_revision = 0;
/*
 *			Redundant Servers must be at same Communications
 *			Revision level -- if not, log it and exit --
 *			That's better than undecipherable errors!
 *			--Daniel 3/93
 */
			if (comm_revision > COMM_NUMREV)
			{
				LOG(("Client communications revision is %d (%s %s %s)\n",
					comm_revision, client->name,
					client->display, client->node));
					ls_old_serv_vers(client);
				return 0;
			}
			if (comm_revision < 0 ||
				comm_revision > COMM_NUMREV)
			{
				/* Force it */
				comm_revision = COMM_NUMREV;
			}

			size2 = l_msg_size(comm_revision);
			client->comm_revision = comm_revision;
		}
		else if (len > 0 && client->comm_revision == -1)
/*
*		    	Don't know, assume standard length
*/
		{
			client->comm_revision = COMM_NUMREV;
			size2 = LM_MSG_LEN;
		}

		if (size2 > size)
		{
			len += l_read_timeout(msg_ca.addr.fd,
				&resp[size], (long)(size2 - size),
				(long)LS_READ_TIMEOUT);
		}
		if (len <= 0) /* failed read */
		{
			if ((len == 0) || (net_errno == NET_EPIPE) ||
#ifdef apollo
						(net_errno == EIO) ||
#endif
					(net_errno == NET_ECONNRESET) ||
#ifdef PC
					(net_errno == NET_ECONNABORTED) ||
#endif
					(net_errno == NET_ESHUTDOWN) ||
					(net_errno == NET_EBADF))
			{
				int closed = 0;

				for (ls = ls_s_first(); ls; ls = ls->next)
				{
					if ((msg_ca.addr.fd == ls->fd1) ||
						(msg_ca.addr.fd == ls->fd2))
					{
						if(l_flexEventLogIsEnabled())
						{
							char	szOne[20] = {'\0'};
							char	szTwo[20] = {'\0'};
							char	szThree[20] = {'\0'};
							char	szFour[20] = {'\0'};
							char	szFive[20] = {'\0'};
							char	szSix[20] = {'\0'};

							sprintf(szOne, "%d", msg_ca.addr.fd == ls->fd1 ? 1 : 2);
							sprintf(szTwo, "%d", len);
							sprintf(szThree, "%d", net_errno);
							sprintf(szFour, "%d", msg_ca.addr.fd);
							sprintf(szFive, "%d", __LINE__);

							ppszInStr[0] = szOne;
							ppszInStr[1] = szTwo;
							ppszInStr[2] = szThree;
							ppszInStr[3] = szFour;
							ppszInStr[4] = __FILE__;
							ppszInStr[5] = szSix;

							l_flexEventLogWrite(lm_job,
												FLEXEVENT_ERROR,
												CAT_FLEXLM_LMGRD,
												MSG_FLEXLM_READLEN_ERROR,
												6,
												ppszInStr,
												0,
												NULL);
						}


						DLOG((lmtext(" (fd%d) readlen %d errno %d fd %d %s:%d \n"),
							msg_ca.addr.fd ==
							ls->fd1 ? 1 : 2,
							len, net_errno,
							msg_ca.addr.fd,
							__FILE__, __LINE__));
						ls_s_down(ls);
						closed = 1;
						break;
					}
				}
				if (!closed)
				{
					int tfd = msg_ca.addr.fd;

					if (debug)
					{
						if(l_flexEventLogIsEnabled())
						{
							char	szOne[20] = {'\0'};
							char	szTwo[20] = {'\0'};
							char	szThree[20] = {'\0'};
							char	szFour[20] = {'\0'};

							sprintf(szOne, "%d", msg_ca.addr.fd);
							sprintf(szTwo, "%d", len);
							sprintf(szThree, "%d", net_errno);
							sprintf(szFour, "%d", __LINE__);

							ppszInStr[0] = szOne;
							ppszInStr[1] = szTwo;
							ppszInStr[2] = szThree;
							ppszInStr[3] = __FILE__;
							ppszInStr[4] = szFour;
							l_flexEventLogWrite(lm_job,
												FLEXEVENT_ERROR,
												CAT_FLEXLM_NETWORK_COMM,
												MSG_FLEXLM_LMGRD_CLOSE_ERROR,
												5,
												ppszInStr,
												0,
												NULL);
						}


						DLOG((lmtext("fd %d len %d errno %d  %s:%d"),
							msg_ca.addr.fd,
							len, net_errno,
							__FILE__,
							__LINE__));
						for (ls = ls_s_first();	ls;	ls = ls->next)
						{
							_DLOG((lmtext(" fd1 %d fd2 %d"), ls->fd1, ls->fd2));
						}
						_DLOG(("\n"));
					}
					ls_down(&tfd, "fd");
				}
				if ((msg_ca.addr.fd != LM_BAD_SOCKET) && ls_lookup_client(&msg_ca))
					ls_delete_client(&msg_ca);
			}
			else
			{
				DLOG(("read: net_errno is %d, using descriptor %d\n",
					   net_errno,
             msg_ca.addr.fd));

				if(l_flexEventLogIsEnabled())
				{
					char	szOne[20] = {'\0'};
					char	szTwo[20] = {'\0'};

					sprintf(szOne, "%d", net_errno);
					sprintf(szTwo, "%d", msg_ca.addr.fd);

					ppszInStr[0] = szOne;
					ppszInStr[1] = szTwo;
					l_flexEventLogWrite(lm_job,
									FLEXEVENT_ERROR,
									CAT_FLEXLM_NETWORK_COMM,
									MSG_FLEXLM_LMGRD_READ_ERROR,
									2,
									ppszInStr,
									0,
									NULL);
				}
			}

			continue;
		}
/*
 *		successful read
 */
		if (l_cksum_ok(resp, client->comm_revision, LM_TCP))
		{
			ret = doit(resp, &msg_ca, daemon, master, select_mask, client);
		}
		else
		{
			char msg[LM_MSG_LEN+1] = {'\0'};

			DLOG(("BAD CHECKSUM: %s sending WHAT\n", resp));

			DLOG(( "client comm revision: %d, msg size: %d\n",
					 client->comm_revision,
           size2));

			if(l_flexEventLogIsEnabled())
			{
				char	szOne[20] = {'\0'};
				char	szTwo[20] = {'\0'};

				sprintf(szOne, "%d", client->comm_revision);
				sprintf(szTwo, "%d", size2);

				ppszInStr[0] = resp;
				ppszInStr[1] = szOne;
				ppszInStr[2] = szTwo;
				l_flexEventLogWrite(lm_job,
									FLEXEVENT_ERROR,
									CAT_FLEXLM_LMGRD_HEALTH,
									MSG_FLEXLM_LMGRD_BAD_CHECKSUM,
									3,
									ppszInStr,
									0,
									NULL);
			}

			l_encode_int(&msg[MSG_DATA], SERVBADCHECKSUM);

			ls_client_send(client, LM_WHAT, msg);

			/*
			 *	This is a definite hack but at this point in time, the
			 *	least risky thing to do.  If we get a bad checksum
			 *	and the message starts off with "GET" and has "HTTP" in it,
			 *	assume it is a web browser and drop the connection.  This fixes
			 *	bug P6417.
			 */
			if(strncmp("GET", resp, 3) == 0 && strstr(resp, "HTTP"))
			{
				network_close(client->addr.addr.fd);
				MASK_CLEAR(select_mask, client->addr.addr.fd);
				ls_delete_client(&client->addr);
			}
		}
	} /* while (ANY_SET(ready_mask))  */
	return(ret);
}

#ifdef VOID_SIGNAL
int ls_kill_chld();
#endif
extern int _ls_going_down;

/*
 *	doit() - Process the command that just came in
 */
static
int
doit(
	char *			cmd,
	CLIENT_ADDR *	ca,
	DAEMON **		daemon,
	char *			master,
	SELECT_MASK		select_mask,
	CLIENT_DATA *	client)
{
	char msg[LM_MSG_LEN+1] = {'\0'};
	char	szBuffer[MAX_PATH * 2] = {'\0'};
	char *	ppszInStr[20] = {NULL};
	int retval = 0;
	int finder = 1;

	if (daemon)
		finder = 0;		/* Not a "license finder */

#ifndef RELEASE_VERSION
	if (debug_m_process > 0 && debug_m_process != -99)
	{
		/* print out cmd */
	    int siz = sizeof(msgs)/sizeof(struct _msgs);
	    int i;
		for (i=0;i<siz;i++)
		{
			if (msgs[i].code == cmd[MSG_CMD])
			{
				DLOG(("lmgrd got msg type %s from %d\n",
					msgs[i].transl, ca->is_fd ?	ca->addr.fd : *((int *)&ca->addr.sin)));
				break;
			}
		}
	}
#endif

	bzero(msg, LM_MSG_LEN);
	switch(cmd[MSG_CMD])
	{

		case LM_CLOCKSETTING:	/* Shouldn't ever get this */
			ls_client_send(client, LM_OK, msg);
			break;

		case LM_DAEMON:
		{
			char *s = &cmd[MSG_SPP_NAME];
			DAEMON *daemons = *daemon;

			while (daemons)
			{
				if (!strcmp(daemons->name, s))
					break;
				daemons = daemons->next;
			}
			if (daemons)
			{
				daemons->m_tcp_port = atoi(&cmd[MSG_SPP_PORT]);
				DLOG(("Setting master port to %d\n", daemons->m_tcp_port));
				daemons->m_udp_port =
						atoi(&cmd[MSG_SPP_UDP_PORT]);
				/* Flag it as there */
				if (BADPID(daemons->pid))
					daemons->pid = 0;
			}
			break;
		}

		case LM_DUMP:
			switch (cmd[MSG_DATA])
			{
			case '\0':
			default:
				break;
			case 'D':	/* DLOG on */
				LOG(("DEBUG logging turned ON, was %s\n",
					dlog_on ? "ON" : "OFF"));
				dlog_on = 1;
				break;
			case 'd':	/* DLOG off */
				LOG(("DEBUG logging turned OFF, was %s\n",
					dlog_on ? "ON" : "OFF"));
				dlog_on = 0;
				break;
			case 'Q':
				if (!finder) ls_s_dump();
				break;
			case 'S':
				if (*daemon == 0)
				{
					LOG(("No DAEMON information.\n"));
				}
				else
				{
					DAEMON *s;
					for (s = *daemon; s; s = s->next)
					{
#ifdef PC
                        if (s->pid & 0xffff0000)
                        {
							LOG(("%s at tcp_port %d (%d) (pid: %X)\n",
                                s->name, s->tcp_port,
                                s->m_tcp_port,
                                s->pid));
                        }
                        else
                        {
							LOG(("%s at tcp_port %d (%d) (pid: %d)\n",
                                s->name, s->tcp_port,
                                s->m_tcp_port,
                                s->pid));
                        }
#else
                        LOG(("%s at tcp_port %d (%d) (pid: %d)\n",
                                    s->name, s->tcp_port,
                                    s->m_tcp_port,  s->pid));
#endif /* PC */
					}
				}
				break;
			case 'M':
				if (!finder)
					ls_m_data();
				break;
			}
			break;

		case LM_HEARTBEAT:	/* Drop this on the floor */
			break;

		case LM_VHELLO:
		    {
				char *msg_daemon = &cmd[MSG_VHEL_DAEMON];
				DAEMON *daemons = *daemon;

				while (daemons)
				{
					if (!strcmp(daemons->name, msg_daemon))
						break;
					daemons = daemons->next;
				}
				if (daemons)
				{
					l_decode_int(&cmd[MSG_VHEL_FLEX_VER], &daemons->flex_ver);
					l_decode_int(&cmd[MSG_VHEL_FLEX_REV], &daemons->flex_rev);
#ifdef SUPPORT_IPX
					if ( cmd[MSG_VHEL_TRANSPORT] == LM_COMM_SPX)
					{
						daemons->spx_port = daemons->m_spx_port = atoi(&cmd[MSG_VHEL_ADDR_PORT]);
					}
					else
#endif /* SUPPORT_IPX */
					if ( cmd[MSG_VHEL_TRANSPORT] == LM_COMM_UDP)
					{
						daemons->udp_port = daemons->m_udp_port =  atoi(&cmd[MSG_VHEL_ADDR_PORT]);
					}
					else
					{
						daemons->tcp_port = atoi(&cmd[MSG_VHEL_ADDR_PORT]);

						if (!daemons->m_tcp_port)
							daemons->m_tcp_port = daemons->tcp_port;

						LOG(("%s using TCP-port %d\n",
								daemons->name, daemons->tcp_port));

					}
					if (cmd[MSG_VHEL_REC_TRANSPORT] == LM_COMM_UDP)
						daemons->recommended_transport = LM_UDP;
					else if (cmd[MSG_VHEL_REC_TRANSPORT] == LM_COMM_TCP)
						daemons->recommended_transport = LM_TCP;
#ifdef SUPPORT_IPX
					else if (cmd[MSG_VHEL_REC_TRANSPORT]==LM_COMM_SPX)
						daemons->recommended_transport = LM_SPX;
#endif
					else
						daemons->recommended_transport = LM_TCP;

					if (cmd[MSG_VHEL_TRANSPORT_REASON] == LM_COMM_REASON_USER)
						daemons->transport_reason = LM_RESET_BY_USER;
					else if (cmd[MSG_VHEL_TRANSPORT_REASON] ==	LM_COMM_REASON_DAEMON)
						daemons->transport_reason = LM_RESET_BY_APPL;
					else
						daemons->transport_reason = 0;
				}
				else
				{
					LOG((lmtext("Got Message from Unknown Vendor Daemon: %s -- ignoring\n"),
						msg_daemon));
				}

				ls_client_send(client, LM_OK, msg);

		    }
		    break;

		case LM_HELLO:
		case LM_HELLO_THIS_VD:
		    ls_m_hello(cmd, ca, daemon, master, client, msg);
		    break;

#if 0
		case LM_HELLO_THIS_VD:
		    {
				DAEMON *d;

				for (d = *daemon; d ; d = d->next)
					if (!strcmp(d->name, &cmd[MSG_DATA]))
						break;
				if (d)
				{
					/* send back the local vd tcp port */
					l_encode_int(&msg[MSG_DATA],
								d->tcp_port);
					ls_client_send(client, LM_OK, msg);
				}
				else
					ls_client_send(client, LM_WHAT, msg);
		    }

		    break;

#endif

		case LM_REREAD:
		    retval = ls_m_reread(cmd, daemon, master, client, msg,
							finder);
		    break;


		case LM_SEND_LF:
		{
			char *p = (char *) NULL;

			if (finder)
			{
				p = ls_get_lfpath(&cmd[MSG_LF_FINDER_TYPE],
						  &cmd[MSG_LF_NAME],
						  &cmd[MSG_LF_HOST],
						  &cmd[MSG_LF_DISPLAY],
						  &cmd[MSG_LF_DAEMON]);
			}
			else
			{
				if (L_STREQ(&cmd[MSG_LF_FINDER_TYPE],
							LM_FINDER_GET_DLIST))
					p = getdlist();
				else
					p = getlf(&cmd[MSG_LF_FINDER_TYPE]);
			}
			if (p)
			{
				ls_client_send_str(client, p);
			}
			else
			{
				DLOG(("SEND_LF Failed, sending WHAT\n"));

        /* To Do: (DRT) Write message to event log?? */

				ls_client_send(client, LM_WHAT, msg);
			}
			if (p)
				free (p);

		}
		break;

		case LM_SHUTDOWN:
			{
				int local_ok = 1;
				extern int ls_local;
				extern int ls_force_shutdown;
				unsigned long ul = 0;

	/*
	 *		    Shut down our vendor daemons, and ourself.
	 */

				l_decode_32bit_packed(&cmd[MSG_DOWN_IPADDR], &ul);
				lmdown_ipaddr = (unsigned int) ul;
				ls_force_shutdown = cmd[MSG_DOWN_FLAGS] & LM_DOWN_FORCE;
				if (ls_force_shutdown)
				{
					LOG(("Forced shutdown requested\n"));
				}
				else
				{
					DLOG(("NO Forced shutdown requested\n"));

					if(l_flexEventLogIsEnabled())
					{
						l_flexEventLogWrite(lm_job,
											FLEXEVENT_INFO,
											CAT_FLEXLM_LMGRD_EVENT,
											MSG_FLEXLM_NO_FORCED_SHUTDOWN,
											0,
											NULL,
											0,
											NULL);
					}
				}
				if (ls_local && !ls_is_local())
				{
					local_ok = 0; /* failed */
					bzero(msg, LM_MSG_LEN+1);
					l_encode_int(&msg[MSG_DATA], LM_MUST_BE_LOCAL);
					ls_client_send(client, LM_NOT_ADMIN, msg);
				}


				if ((unpriv_utils || ((client->comm_revision > 0) &&
					lm_isadmin(&cmd[MSG_DOWN_NAME]))) &&
					ls_allow_lmdown && local_ok)
				{
					char vendor[MAX_VENDOR_NAME + 1] = {'\0'};

					l_zcp(vendor, &cmd[MSG_HEL_DAEMON], MAX_VENDOR_NAME);

					if (!*vendor)
						_ls_going_down = 1;	/* Let everyone know */
					else
						only_this_vendor = vendor;
		/*
		 *			We only need to delay 60s in multi-server configurations
		 */
					if (!finder && ls_s_qnum() > 1)
					{
						LOG(("Waiting 60 seconds to shutdown redundant servers\n"));
						lm_sleep(60);  /* Let all servers get it */
					}
					else
						lm_sleep(1);  /* Let the message get back */

					if (client->comm_revision == 0)
					{
						LOG((lmtext("Daemon SHUTDOWN requested\n")));
					}
					else
					{
						LOG((lmtext("SHUTDOWN request from %s at node %s"),
							 &cmd[MSG_DOWN_NAME], &cmd[MSG_DOWN_HOST]));
						if(l_flexEventLogIsEnabled())
						{
							ppszInStr[0] = &cmd[MSG_DOWN_NAME];
							ppszInStr[1] = &cmd[MSG_DOWN_HOST];

							l_flexEventLogWrite(lm_job,
												FLEXEVENT_INFO,
												CAT_FLEXLM_LMGRD,
												MSG_FLEXLM_LMGRD_SHUTDOWN_REQ,
												2,
												ppszInStr,
												0,
												NULL);
						}
						if (*vendor)
							_LOG((" for vendor \"%s\"", vendor));
						_LOG(("\n"));
					}
					only_this_vendor = 0;
					if (!finder)
					{
						_ls_going_down = 1;
						if (ls_kill_chld(SIGTERM) == LM_BORROW_DOWN)
						{
							l_encode_int(&msg[MSG_DATA],
								LM_BORROW_DOWN);
							ls_client_send(client, LM_NOT_ADMIN,
								msg);
							_ls_going_down = 0;
						}
						else
						{
							ls_client_send(client, LM_OK, msg);
							ls_go_down(SIGTERM);
						}
					}
				}
				else if (!ls_allow_lmdown)
				{
					LOG((lmtext("lmdown requests disabled\n")));
					ls_client_send(client, LM_NO_SUCH_FEATURE, msg);
				}
				else if (local_ok)
				{
					LOG((lmtext("UNAUTHORIZED shutdown request")));
					if (client->comm_revision > 0)
					{
						_LOG((lmtext(" from %s at node %s\n"),
							   &cmd[MSG_DOWN_NAME], &cmd[MSG_DOWN_HOST]));
					}
					else
					{
						_LOG((lmtext(" from v1.x lmdown (unknown user)\n")));
					}
					LOG((lmtext("(Requestor is not a license administrator)")));
					ls_client_send(client, LM_NOT_ADMIN, msg);
				}
			}
			lmdown_ipaddr = 0; /* reset for next time */
			break;

		case LM_WHAT:
/*
 *			This shouldn't come over here unsolicited, unless it
 *			is to check the socket.  Just let it drop on the floor.
 */
			break;

		default:
		{
			char *sp;
			DAEMON *daemons = *daemon;

			if (finder)
				break;
			sp = ls_hookup(cmd, ca, master, client, select_mask);

			if (sp)
			{
				while (daemons)
				{
					if (!strcmp(daemons->name, sp))
						break;
					daemons = daemons->next;
				}
				if (daemons)
				{
			    	if (!CHILDPID(daemons->pid))
					{
						ls_server_send(ca, LM_NO_SUCH_FEATURE,
										msg);
					}
					else
					{
						bzero(msg, LM_MSG_LEN+1);
						l_encode_int(&msg[MSG_TRY_TCP_PORT], daemons->tcp_port);
						ls_server_send(ca, LM_TRY_ANOTHER, msg);
					}
				}
				else
				{
					msg[MSG_DATA] = 0;
					ls_server_send(ca, LM_NO_CONFIG_FILE, msg);
				}
			}
			break;
		}
	}
	return(retval);
}

/*
 *	decide_transport -- determine which transport to tell client
 *				to use.
 */
static
int
decide_transport(
	int			daemon_recommended,
	int *		daemon_reason,
	int			client_requested,
	DAEMON *	daemon)
{
#ifdef SUPPORT_IPX
	if (client_requested == LM_SPX)
		return LM_SPX;
#endif

	if (daemon->m_udp_port<0)
	{
		*daemon_reason = LM_RESET_BY_APPL;
		return LM_TCP;
	}
	if (daemon->m_tcp_port<0)
	{
		*daemon_reason = LM_RESET_BY_APPL;
		return LM_UDP;
	}
	return daemon_recommended;
}


#ifdef SIGNAL_NOT_AVAILABLE
void
ls_send_reread(daemon)
DAEMON *daemon;
{
	int				d_socket = 0;
	int				c_rev = 0, comm_rev = 0;
	char			resp[LM_MSG_LEN+1] = {'\0'};
	char			msg[LM_MSG_LEN+1] = {'\0'};
	char			local_host[MAX_HOSTNAME] = {'\0'};
	int				scomm_rev = 0;
	int				i = 0;
	COMM_ENDPOINT	endpoint;

	c_rev = lm_job->daemon->our_comm_revision;
	lm_job->daemon->our_comm_revision = comm_rev = COMM_NUMREV;
	l_conn_msg(lm_job, daemon->name, msg, LM_TCP, 1);

	/* Connect to lmgrd */
	endpoint.transport = LM_TCP;
	endpoint.transport_addr.port = htons((unsigned short)daemon->m_tcp_port);
	gethostname( (char *)local_host, MAX_HOSTNAME );
	d_socket = l_basic_conn(lm_job, msg, &endpoint, local_host, resp);

	if (d_socket < 0)
	{
		LOG((lmtext("Can't send reread to %s: %s\n"), daemon->name,
				lmtext(lc_errstring(lm_job))));
		return;
	}

	scomm_rev = lm_job->daemon->our_comm_revision;

	if (resp[MSG_CMD] == LM_OK)
	{
		if (resp[MSG_DATA] == '\0')
			scomm_rev = 1;
		else
			l_decode_int(&resp[MSG_OK_COMM_REV], &scomm_rev);
	}

	memset(msg, '\0', LM_MSG_LEN+1);
	msg[MSG_CMD] = LM_REREAD;
	strncpy(&msg[MSG_HEL_NAME], lm_username(0), MAX_USER_NAME);/* LONGNAMES */
	strncpy(&msg[MSG_HEL_HOST], lm_hostname(0), MAX_SERVER_NAME);/* LONGNAMES */
	strncpy(&msg[MSG_HEL_DAEMON], daemon->name, MAX_DAEMON_NAME);/* LONGNAMES */
	l_msg_cksum(msg, comm_rev, LM_TCP);
	i = network_write(d_socket, msg, l_msg_size(comm_rev));
	if (i < l_msg_size(comm_rev))
	{
		LOG(( lmtext("lmreread: error sending REREAD message\n")));
	}
	else
	{
		char type, *msgdata = NULL;

		if (scomm_rev >= 3)
		{
			lm_job->daemon->socket = d_socket;
			if (l_rcvmsg(lm_job, &type, &msgdata))
			{
				if (type == LM_NOT_ADMIN)
				{
					LOG((lmtext("lmreread: you are not a license administrator\n")));
				}
				else if (type == LM_TRY_ANOTHER)
				{
					LOG((lmtext("lmreread: unknown daemon\n")));
				}
				else if (type != LM_OK)
				{
					LOG((lmtext("ERROR: unknown return \'%c\' (%d) from daemon\n"),
						msg[MSG_DATA], msg[MSG_DATA]));
				}
			}
			else
			{
				LOG((lmtext("Error reading REREAD response: %s\n"),
					lmtext(lc_errstring(lm_job))));
			}
		}
	}
	lm_disconn( 0 /* don't force close */ );
}
#endif /* SIGNAL_NOT_AVAILABLE */

/*
 *	Get this guy hooked up with the right server
 */
DAEMON *flexlmd = (DAEMON *)-1;

static
void
ls_m_hello(
	char *			cmd,
	CLIENT_ADDR *	ca,
	DAEMON **		daemon,
	char *			master,
	CLIENT_DATA *	client,
	char *			msg)
{
	char *		s = &cmd[MSG_HEL_DAEMON];
	int			transport = 0;
	DAEMON *	daemons = NULL;
	int			req_transport = 0;
	int			tcp_port = 0;

	if ( cmd[MSG_HEL_REQ_COMM] == LM_COMM_UDP )
		req_transport =  LM_UDP;
#ifdef SUPPORT_IPX
	else if ( cmd[MSG_HEL_REQ_COMM] == LM_COMM_SPX )
		req_transport =  LM_SPX;
#endif
	else
		req_transport =  LM_TCP;

	if (*s == 0)
	{		 /* SERVER connect for debug */
		l_encode_int(&msg[MSG_OK_VER],
				lm_job->code.flexlm_version);
		l_encode_int(&msg[MSG_OK_REV],
				lm_job->code.flexlm_revision);
		memcpy(&msg[MSG_OK_PATCH],
				lm_job->code.flexlm_patch, 1);
		msg[MSG_OK_COMM_VER] = COMM_VERSION;
		msg[MSG_OK_COMM_VER+1] = '\0';
		msg[MSG_OK_COMM_REV] = COMM_REVISION;
		msg[MSG_OK_COMM_REV+1] = '\0';
		l_zcp(&msg[MSG_OK_HOST], master, MAX_HOSTNAME);
		strcpy(&msg[MSG_OK_DAEMON], "lmgrd");
		ls_client_send(client, LM_OK, msg);
	}
	else
	{
		if (daemon)
			daemons = *daemon;
		else
			daemons = (DAEMON *) NULL;

		while (daemons)
		{
			if (!strcmp(daemons->name, s))
				break;
			daemons = daemons->next;
		}
		if (!daemons) /* look for flexlmd */
		{
			if (flexlmd == (DAEMON *)-1)
			{
				if (daemon)
					daemons = *daemon;
				else
					daemons = (DAEMON *) NULL;
				while (daemons)
				{
					if (!strcmp(daemons->name, "flexlmd"))
						break;
					daemons = daemons->next;
				}
				if (daemons)
					flexlmd = daemons;
				else
					flexlmd = (DAEMON *)0;
			}
			daemons = flexlmd;
		}
		if (daemons)
		{
			if (cmd[MSG_CMD] == LM_HELLO_THIS_VD)
			{
				tcp_port = daemons->tcp_port;
			}
			else
			{
				tcp_port = daemons->m_tcp_port;
			}
		}
/*
 *		now daemons == daemon we got HEL from or 0
 */
		if (daemons &&
		    (CHILDPID(daemons->pid)) &&
		    (daemons->m_tcp_port >=0||
		    daemons->m_udp_port>=0))
		{
			char name[MAX_HOSTNAME+2] = {'\0'};

			bzero(msg, LM_MSG_LEN+1);
			strncpy(name, master, MAX_HOSTNAME);
#ifndef NO_GETDOMAINNAME_CALL
#ifndef WINNT_TODO
			if (add_domain_name)
			{
				char xxx[MAX_SERVER_NAME+1] = {'\0'};/* LONGNAMES */
				getdomainname(xxx, MAX_SERVER_NAME);/* LONGNAMES */
				strcat(name, ".");
				strncat(name, xxx, MAX_SERVER_NAME);/*LONGNAMES */
			}
#endif /* WINNT_TODO */
#endif
			transport = decide_transport(
				daemons->recommended_transport,
				&daemons->transport_reason,
				req_transport,
				daemons);
			switch (transport)
			{
#ifdef SUPPORT_IPX
			case LM_SPX:
				msg[MSG_TRY_TRANSPORT] = LM_COMM_SPX;
				break;
#endif
			case LM_TCP:
				msg[MSG_TRY_TRANSPORT] = LM_COMM_TCP;
				break;
			case LM_UDP:
				msg[MSG_TRY_TRANSPORT] = LM_COMM_UDP;
				break;
#ifdef FIFO
			case LM_LOCAL:
				msg[MSG_TRY_TRANSPORT] = LM_COMM_UDP;
				break;
#endif /* FIFO */
			default:
				DLOG(("Internal error %s %d\n",
					__FILE__, __LINE__));
			}
#ifdef FIFO
			if (transport == LM_LOCAL)
			{
				l_encode_int(&msg[MSG_TRY_TCP_PORT],0);
				l_encode_int(&msg[MSG_TRY_UDP_PORT],0);
			}
			else
#endif /* FIFO */
			{
#ifdef SUPPORT_IPX
				if (transport==LM_SPX)
				/*
				 * Use the field of TCP_PORT
				 * SPX port.
				 */
					l_encode_int( &msg[MSG_TRY_TCP_PORT],
							daemons->m_spx_port);
				else
#endif
				{
					l_encode_int( &msg[MSG_TRY_TCP_PORT],
								tcp_port);
				}
				l_encode_int(&msg[MSG_TRY_UDP_PORT],
							daemons->m_udp_port);
			}
			if (daemons->transport_reason == LM_RESET_BY_USER)
			{
				msg[MSG_TRY_TRANSPORT_REASON] =	LM_COMM_REASON_USER;
			}
			else if (daemons->transport_reason== LM_RESET_BY_APPL)
			{
				msg[MSG_TRY_TRANSPORT_REASON] =	LM_COMM_REASON_DAEMON;
			}
			else
			{
				msg[MSG_TRY_TRANSPORT_REASON] = ' ';
			}

			strncpy(&msg[MSG_TRY_HOST], name, MAX_SERVER_NAME);
			strncpy(&msg[MSG_TRY_HOST2], &name[MAX_SERVER_NAME],
						MAX_HOSTNAME - MAX_SERVER_NAME);/* LONGNAMES */
			ls_client_send(client, LM_TRY_ANOTHER, msg);
		}
		else
		{
			msg[MSG_DATA] = 0;
			ls_client_send(client, LM_NO_SUCH_FEATURE, msg);
		}
	}
}

/*
 *		Re-read the license file; signal the old daemons and start
 *		any new ones.
 */
static
int
ls_m_reread(
	char *			cmd,
	DAEMON **		daemon,
	char *			master,
	CLIENT_DATA *	client,
	char *			msg,
	int				finder)
{
	DAEMON * new = NULL, * old = NULL, * newptr = NULL;
	int hadit = 0;
	char *s = &cmd[MSG_DOWN_DISPLAY]; /* it's really the desired vdaemon */
	int retval = 0;
	extern LM_SERVER *main_master_list;
	extern char **argv_sav;
	extern int argc_sav;
	extern int ls_local;

	if (ls_local)
	{

		char hname[500] = {'\0'};
		int cipaddr = 1, ipaddr = 2;

		*hname = 0;
		gethostname(hname, 500);
		ipaddr = l_get_ipaddr(hname, 0, 0, 0);
		l_decode_int(&cmd[MSG_DOWN_IPADDR], &cipaddr); /* P6265 */

		if (ipaddr != cipaddr)
		{
			LOG(("lmreread only authorized on host \"%s\"\n",
				*hname ? hname : "Can't get hostname", ipaddr));
			LOG(("Request made from host \"%s\"\n",
				&cmd[MSG_DOWN_HOST], cipaddr));
			bzero(msg, LM_MSG_LEN+1);
			l_encode_int(&msg[MSG_DATA], LM_MUST_BE_LOCAL);

			ls_client_send(client, LM_NOT_ADMIN, msg);
			return retval;
		}
	}
	if (!unpriv_utils && !((client->comm_revision > 0) &&
					lm_isadmin(&cmd[MSG_DOWN_NAME])))
	{
		LOG((lmtext("UNAUTHORIZED reread request")));
		if (client->comm_revision > 0)
		{
			_LOG((lmtext(" from %s at node %s\n"),
			       &cmd[MSG_DOWN_NAME], &cmd[MSG_DOWN_HOST]));
		}
		else
		{
			_LOG((lmtext(" from v1.x lmreread (unknown user)\n")));
		}
		LOG((lmtext("(Requestor is not a license administrator)")));
		ls_client_send(client, LM_NOT_ADMIN, msg);
		return retval;
	}

	if (finder)
	{
		execvp(argv_sav[0], argv_sav); /* exits */
	}
	l_flush_config(lm_job); /* Get rid of cached data */
	newptr = l_get_dlist(lm_job);
	if (*s)
	{
/*
*		Signal the one daemon specified
*/
		hadit = 0;
		for (old = *daemon; old; old = old->next)
		{
			if (L_STREQ(s, old->name))
			{
				hadit = 1;
				for (new = newptr; new; new = new->next)
				{
					if (!strcmp(new->name, old->name))
					{
						break;
					}
				}
				if (new && strcmp(old->path, new->path)  && old->pid)
				{
/*
 *			    	Same daemon, but with a new path
 */
					if (old->pid)
					{
#ifdef SIGNAL_NOT_AVAILABLE
						ls_send_shutdown( old );
#else
						if (CHILDPID(old->pid))
							kill(old->pid,
							SHUTDOWN_SIGNAL);
#endif /* SIGNAL_NOT_AVAILABLE */
					}
				}

/*
 *					Re-Start the daemon
 */
				if (new &&
					(strcmp(old->path, new->path) ||
						!old->pid))
				{
					ls_startup(new,
						ls_our_hostname, master);
					if (CHILDPID(new->pid))
					{
						if (ls_s_imaster())
							old->m_tcp_port = new->tcp_port;

						old->pid = new->pid;
						old->tcp_port = new->tcp_port;

						LOG((lmtext("REstarted %s"), new->name));
#ifndef PC
						_LOG((" (internet tcp_port %d)", new->tcp_port));
#endif
						_LOG(("\n"));
					}
					old->path = new->path;
				}
				else if (old->pid)
				{
					char *sav = new->path;
					char *optsav = new->options_path;
					DAEMON *savd = new->next;

					memcpy(new, old, sizeof (DAEMON));
					new->path = sav;
					new->options_path = optsav;
					new->next = savd;
					if ((old->pid != 0) && (old->pid != -1))
#ifdef SIGNAL_NOT_AVAILABLE
						ls_send_reread( old );
#else
						kill(old->pid, REREAD_SIGNAL);
#endif
				}
				break;	/* Done */
			}
		}
		if (hadit)
			ls_client_send(client, LM_OK, msg);
		else
		{
			ls_client_send(client, LM_TRY_ANOTHER,msg);
		}
	}
	else
	{
/*
*		Start or signal all the new daemons
*/
		for (new = newptr; new; new = new->next)
		{
			hadit = 0;
			for (old = *daemon; old; old = old->next)
			{
				if (old->pid &&
				    !strcmp(new->name, old->name))
				{
					hadit = 1;
					break;	/* Had it */
				}
			}
			if (hadit)
			{
				if (strcmp(old->path, new->path))
/*
 *			    	Same daemon, but with a new path
 */
				{
#ifdef SIGNAL_NOT_AVAILABLE
					ls_send_shutdown( old );
#else
					if (old->pid > 0)
						kill(old->pid, SHUTDOWN_SIGNAL);
#endif /* SIGNAL_NOT_AVAILABLE */

/*
 *				Re-Start the daemon
 */
					ls_startup(new,
						ls_our_hostname, master);
					if (CHILDPID(new->pid))
					{
						if (ls_s_imaster())
							new->m_tcp_port =
								new->tcp_port;
						/*else
							new->m_tcp_port = 0;*/

						LOG((lmtext("REstarted %s"), new->name));
#ifndef PC
						_LOG((" (internet tcp_port %d)", new->tcp_port));
#endif /* PC */
						_LOG(("\n"));
					}
				}
				else
				{
					char *sav = new->path;
					char *optsav = new->options_path;
					DAEMON *savd = new->next;
/*
 *					Signal the daemon
 */
					if ((old->pid != 0) && (old->pid != -1))
#ifdef SIGNAL_NOT_AVAILABLE
						ls_send_reread( old );
#else
						kill(old->pid, REREAD_SIGNAL);
#endif /* SIGNAL_NOT_AVAILABLE */
					memcpy(new, old, sizeof (DAEMON));
					new->path = sav;
					new->options_path = optsav;
					new->next = savd;
				}
			}
			else
			{
/*
 *				Start the daemon
 */
				ls_startup(new, ls_our_hostname, master);
				if (CHILDPID(new->pid))
				{
					if (ls_s_imaster())
						new->m_tcp_port = new->tcp_port;
					/*else
						new->m_tcp_port = 0;*/

					LOG((lmtext("Reread: Started %s"), new->name));
#ifdef PC
                    if (new->print_pid & 0xffff0000)
                    {
						_LOG((" (pid %X)", new->print_pid));
                    }
                    else
                    {
						_LOG((" (pid %d)", new->print_pid));
                    }
#else
					_LOG((" (internet tcp_port %d pid %d)\n", new->tcp_port, new->pid));
#endif
				}
			}
		}
/*
 *		Now, shutdown all daemons that aren't there anymore
 */
		for (old = *daemon; old; old = old->next)
		{
			int haveit = 0;

			for (new = newptr; new; new = new->next)
			{
				if (!strcmp(new->name, old->name))
				{
					haveit = 1;
					break;	/* Had it */
				}
			}
			if (!haveit && old->pid)
			{
/*
 *				Signal the daemon
 */
#ifdef SIGNAL_NOT_AVAILABLE
				ls_send_shutdown( old );
#else
				if (old->pid > 0)
					kill(old->pid, SHUTDOWN_SIGNAL);
#endif /* SIGNAL_NOT_AVAILABLE */
			}
		}
/*
 *		Update DAEMON ptr, and send new program info
 */
		l_free_daemon_list(lm_job, *daemon);
		*daemon = newptr;
		if (ls_s_imaster())
			retval = 1;
		ls_client_send(client, LM_OK, msg);
	}
	ls_m_init(argc_sav, argv_sav, &main_master_list, 1);
	ls_quorum(main_master_list, "", 1);

	return retval;
}

static
#endif /* NO_LMGRD */
char *
getdlist(lm_noargs)
{
	DAEMON * dlist = NULL, * dlp = NULL;
	char * cp = NULL, * ret = NULL;
	int len = 0;

	dlist = l_get_dlist(lm_job);

	if (!dlist)
		return (char *)0;

	for (len = 0, dlp = dlist; dlp; dlp = dlp->next)
	{
		len += strlen(dlp->name);
		len+=5;
	}
	ret = (char *)LS_MALLOC(len);

	for (cp = ret, dlp = dlist; dlp; dlp = dlp->next)
	{
		strcpy(cp, dlp->name);
		cp += (strlen(dlp->name));
		*cp = ' ';
		cp++;
	}
	*cp = '\0';

	l_free_daemon_list(lm_job, dlist);
	return ret;
}

static
void
ls_old_serv_vers(CLIENT_DATA * client)
{
	char tmpmsg[LM_MSG_LEN+1] = {'\0'};

	l_encode_int(&tmpmsg[MSG_DATA], LM_SERVOLDVER);
	DLOG(("Old server version, sending WHAT\n"));
	ls_client_send(client, LM_WHAT, tmpmsg);
}

#ifndef NO_LMGRD
static
#endif
char *
getlf(char * what)
{
	LICENSE_FILE * lf = NULL;
	extern char *ls_get_lfpath();
	char * buf = NULL;
	char * cp = NULL;
	int sav = lm_job->lfptr;
	char * str = NULL;
	char cwd[LM_MAXPATHLEN] = {'\0'};
	int dofile = !L_STREQ(what, LM_FINDER_GET_PATHS);

	GETWD(cwd);
	sprintf(cwd, "%s%c", cwd, PATHTERMINATOR);
	for (lm_job->lfptr = 0; lm_job->lic_files[lm_job->lfptr];
		lm_job->lfptr++)
	{
		str = 0;
		if (dofile)
		{
			if ((lf = l_open_file(lm_job, LFPTR_CURRENT)) &&
				lf->type == LF_STRING_PTR)
			{
				str = lf->ptr.str.s;
			}
		}
		else
			str = lm_job->lic_files[lm_job->lfptr];
		if (str)
		{
			cp = (char *)LS_MALLOC( (buf ? strlen(buf) : 0) + strlen(str) + strlen(cwd) + 5);

#ifdef PC
            if (!dofile && L_STREQ_N(str, ".\\", 2))
				str += 2;
            sprintf(cp, "%s%s%s%s", buf ? buf : "",
                    (dofile && buf) ? "\n" :
                    (!dofile && buf) ? ":" : "",
                    dofile ? "" :
                    (str[1] == ':') ? "" : cwd,
                    str);

#else
            if (!dofile && L_STREQ_N(str, "./", 2))
				str += 2;
            sprintf(cp, "%s%s%s%s", buf ? buf : "",
                    (dofile && buf) ? "\n" :
                    (!dofile && buf) ? ":" : "",
                    dofile ? "" :
                    (*str == PATHTERMINATOR) ? "" : cwd,
                    str);

#endif

			if (buf)
				free(buf);
			buf = cp;
		}
	}
	lm_job->lfptr = sav;
	return buf;
}

int
ls_is_local(void)
{

	char hname[500] = {'\0'};
	int ipaddr = 2;

	*hname = 0;
	gethostname(hname, 500);
	ipaddr = l_get_ipaddr(hname, 0, 0, 0);

	if (!lmdown_ipaddr || (ipaddr != lmdown_ipaddr))
	{
		DLOG(("%x != %x\n", lmdown_ipaddr, ipaddr));
		LOG(("lmdown only authorized on host \"%s\"\n",
			*hname ? hname : "Can't get hostname"));
		return 0;
	}
	return 1;
}


