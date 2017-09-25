/*****************************************************************************
 
 		Copyright (c) 1990, 2003 by Macrovision, Inc.
 		This software has been provided pursuant to a License Agreement
 		containing restrictions on its use.  This software contains
 		valuable trade secrets and proprietary information of
 		Macrovision Corporationc and is protected by law.  It may
 		not be copied or distributed in any form or medium, disclosed
 		to third parties, reverse engineered or used in any manner not
 		provided for in said License Agreement except with the prior
 		written authorization from Macrovision Corporation.
 ****************************************************************************/
/*
 *	Module: $Id: l_basic_conn.c,v 1.38 2003/04/18 23:48:01 sluu Exp $
 *	Function:	l_basic_conn(job, msg, endpoint, node, resp)
 *
 *	Description:	Connects to server at 'endpoint' on host 'node'
 *			transport is LM_TCP or LM_UDP or LM_SPX
 *
 *	M. Christiano
 *	4/19/90
 *
 *	Last changed:  11/19/02
 *
 */
/*-  this is a test -*/

#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "l_timers.h"
#include "flex_file.h"
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#ifndef PC
#include <sys/ioctl.h>
#ifdef USE_SYS_TIMES_H
#include <sys/times.h>
#else
#include <sys/time.h>
#endif 
#ifdef DGX86
#include <netinet/tcp.h>
#endif /* DGX86 */
#endif /* !PC */
#include <sys/types.h>

#ifndef PC
#include <netinet/in.h>
#endif

#ifndef NO_UIO_H
#include <sys/uio.h>
#endif
#include <errno.h>
#include "l_socket.h"
#include "l_prot.h"
#ifdef SUPPORT_IPX
#include <wsipx.h>
#endif
#ifdef PC
/*#define PC_MT_CONNECT */
#include "lmselect.h"
#endif

#ifdef LM_GENERIC_VD

static int send_seeds lm_args((LM_HANDLE *,char *,int ));
#endif /* LM_GENERIC_VD*/


#ifdef RELEASE_VERSION
#define DEBUG(x)
#else
#define DEBUG(x)	if (l_conn_debug) fprintf x
#endif

/*
 *	Timer handler
 */

static void put_timer_back lm_args((LM_HANDLE *));
static void install_timer lm_args((LM_HANDLE *));
static void pause_timer lm_args((LM_HANDLE *));
static void continue_timer lm_args((LM_HANDLE *));
static void timerproc lm_args((lm_noargs));

static LM_PFV set_timer;
#ifndef VOID_SIGNAL
typedef int (*LM_PFI)();
typedef LM_PFI (*PFPFI)();
#endif

static
void
timerproc()
{
#ifdef VMS
	sys$setef(TIMER_EF);
	sys$setef(READ_EF);
#endif
}

static int sigpipe_installed = 0;
#ifdef PC

int l_pc_connect(LM_HANDLE *job, int *s, struct sockaddr *, int size, char *node, int transport);

int ConnectReturn;
#endif /* win32s */

/*ARGSUSED*/
static void _timer(sig) int sig;{ timerproc(); }/* The timer signal handler */
static SIGFUNCP _user_sigpipe = (SIGFUNCP)0;
/*ARGSUSED*/
static void _lm_sigpipe(sig) int sig;
{
        /* P4038 */
  LM_HANDLE j;
  LM_OPTIONS o;

        memset(&j, 0, sizeof(j));
        memset(&o, 0, sizeof(o));
        j.options = &o;
        l_timer_signal(&j, SIGPIPE, (SIGFUNCP)_lm_sigpipe);
}
static struct sockaddr_in sock;
#ifdef SUPPORT_IPX
static struct sockaddr_ipx sock_ipx;
#endif
static char saved_hostname[MAX_HOSTNAME+1];
static int first = 1;
static int usetimers;
static int saved_user;
static SIGFUNCP _utimer;
static struct itimerval timeout, remain_timeout, _utime;
static int l_try_basic_conn lm_args(( LM_HANDLE *, char *, COMM_ENDPOINT *,
				char *, char *, char *));
LM_SOCKET l_make_tcp_socket lm_args((LM_HANDLE *));
LM_SOCKET l_make_udp_socket lm_args((LM_HANDLE *));
#ifdef SUPPORT_IPX
static LM_SOCKET
	l_make_spx_socket lm_args((LM_HANDLE *, char *));
#endif /* SUPPORT_IPX */

#ifndef RELEASE_VERSION
char * l_conn_debug = (char *)-1;
FILE *l_basic_conn_efp;
#endif /* RELEASE_VERSION */

int API_ENTRY
l_basic_conn(job, msg, endpoint, hostname, resp)
LM_HANDLE *job;
char *msg;
COMM_ENDPOINT *endpoint;
char *hostname;
char *resp;
{
  unsigned short port = 0;
  int ret = -1;
  int idx;

#ifndef RELEASE_VERSION
        if (l_conn_debug = (char *)-1)
        {
                l_basic_conn_efp = stdout;
                if (l_conn_debug= l_real_getenv("L_BASIC_CONN_DEBUG"))
                        if (*l_conn_debug=='2')
								l_basic_conn_efp = l_flexFopen(job, "bconn.out", "w");
        }
#endif



	if (((endpoint->transport_addr.port & 0xffff) == 0xffff)
		)
	{
                if ((endpoint->transport_addr.port & 0xffff) == 0xffff)
                        port = (unsigned short)LMGRD_PORT_START;
                else
                        port = (unsigned short)
                        htons ((unsigned short)(endpoint->transport_addr.port & 0xffff));
		for (idx = 0; ret <0 && job->lm_errno != LM_HOSTDOWN &&
				port <= (unsigned short)job->port_end;
				port++, idx++)
		{
                        endpoint->transport_addr.port = ntohs(port);
                        {
                                ret = l_try_basic_conn(job, msg, endpoint,
                                        hostname, resp, 0);
                        }
		}
	}
	else
		ret = l_try_basic_conn(job, msg, endpoint, hostname, resp, 0);


	return ret;
}

static
int
l_try_basic_conn(job, msg, endpoint, node, resp, thread)
LM_HANDLE *job;		/* Current license job */
char *msg;		/* Message to send to daemon */
COMM_ENDPOINT *endpoint; /* endpoint to connect to */
char *node;		/* Node name to connect to */
char *resp;		/* Response from server */
char *thread;
{
  int i;
  LM_SOCKET s = LM_BAD_SOCKET;
  int goodhost = 1;
  int msgsize = l_msg_size(job->daemon->our_comm_revision);
#ifdef USE_WINSOCK
  u_long	non_blocking_mode;
#endif
  int transport = endpoint->transport;
  int sent_seeds = 0;

	DEBUG((l_basic_conn_efp, "Try connect \n"));
	if (first == 1)
	{
		first = 0;
		saved_hostname[0] = '\0';
	}
	set_timer = job->options->setitimer;
	_utimer = 0;	/* User's timer handler */
	saved_user = 0;
	job->flags |= LM_FLAG_IN_CONNECT; /* used by timers to re: SA_RESTART */
/*
 *	If we have never set up a SIGPIPE handler in
 *	this process, do it now
 */
#if !defined (VMS) && !defined (PC)
	if (!sigpipe_installed)
	{
/*
 *		Set up the SIGPIPE handler
 */
		_user_sigpipe = l_timer_signal(job, SIGPIPE, _lm_sigpipe);
		if ((_user_sigpipe != SIG_IGN) && (_user_sigpipe !=SIG_DFL))
		{
/*
 *			There already was a SIGPIPE handler; put it back
 */
			(void) l_timer_signal(job, SIGPIPE, _user_sigpipe);
		}
		sigpipe_installed = 1;
	}
#endif

/*
 *	First, set up the timers
 */
	if (job->options->conn_timeout > 0)
	{
		memset(&timeout, 0, sizeof(timeout));
		timeout.it_value.tv_sec = job->options->conn_timeout;
		usetimers = 1;
	}
	else
	{
		usetimers = 0;
	}

/*
 *	Create the socket, and connect
 */
	while (1)
	{
		install_timer(job);	 /* Time the whole operation out */

		if (transport == LM_TCP)
		{
			if ((s = l_make_tcp_socket(job)) == LM_BAD_SOCKET)
				break;
		}
#ifdef SUPPORT_UDP
		else if (transport == LM_UDP)
		{
			if ((s = l_make_udp_socket(job)) == LM_BAD_SOCKET)
				break;
		}
#endif /* SUPPORT_UDP */


		if ( strcmp(saved_hostname, node))
		{
			struct sockaddr_in s_in;
			LM_SET_NET_ERRNO(0);
			memset(&s_in, 0, sizeof(s_in));

			if (l_get_ipaddr(node, 0, &s_in, getenv("FLEXLM_ANYHOSTNAME") ? 1 : 0))	/* overrun checked */
			{
				memcpy(&sock, &s_in, sizeof(s_in));
				goodhost = 1;
			}
			else
				goodhost = 0;
		}
		if (goodhost == 0)
		{

#ifdef WINNT
		  extern   int network_installed;
			if (!network_installed)
			{
				LM_SET_ERROR_THREAD(job,LM_NONETWORK,111,WSAENETDOWN, 0, LM_ERRMASK_ALL, thread);
			}
			else
#endif
			{
				/************************************************************
				 * P5138
				 * P6872
				 * kmaclean 1/16/03
				 * My previous fix for P6872 apparently broke other things
				 * including the testsuite bug test 5318.
				 * LM_BADHOST is set if the host name can not be resolved.
				 * However no where in the code do we check for this as we do 
				 * for LM_HOSTDOWN. Since these two errors mean the same thing
				 * as far as making decisions in the code and I'm afraid to 
				 * change the checks for LM_HOSTDOWN in other parts of the code,
				 * I've changed the error here to LM_HOSTDOWN.
				 ***********************************************************/												   
				/* LM_SET_ERROR_THREAD(job, LM_BADHOST, 7, net_errno, node, LM_ERRMASK_ALL, thread);*/
				
				LM_SET_ERROR_THREAD(job, LM_HOSTDOWN, 7, net_errno, node, LM_ERRMASK_ALL, thread);
			}
			network_close(s);
			s = LM_BAD_SOCKET;
			break;
		}
		(void) l_zcp(saved_hostname, node, MAX_HOSTNAME);
		sock.sin_port = endpoint->transport_addr.port;
		DEBUG((l_basic_conn_efp, "Trying connection to %s port %d\n", node,
					ntohs(sock.sin_port)));
		LM_SET_NET_ERRNO(0);
#ifdef USE_WINSOCK
		if (l_pc_connect(job, &s, (struct sockaddr *)&sock,	sizeof(sock), node, transport) == -1)
#else /* USE_WINSOCK */
		if (connect(s, (struct sockaddr *) &sock, sizeof(sock)))
#endif
		{

#ifdef UNIX
			if (errno == EINTR)
			{
				LM_SET_ERROR_THREAD(job, LM_HOSTDOWN, 482, errno, node, LM_ERRMASK_ALL, thread);
			}
			/* P2480 */
			else if (errno == EADDRINUSE)
			{
				put_timer_back(job);
				continue;
			}
			else
#else
			if (!job->lm_errno) /* windows */
#endif /* UNIX */
			{
				DEBUG((l_basic_conn_efp, "setting errno %d\n", net_errno));
				LM_SET_ERROR(job, LM_CANTCONNECT, 570,
					net_errno, node, LM_ERRMASK_ALL);
			}
#ifdef PC
			else DEBUG((l_basic_conn_efp, "setting errno %d\n", net_errno));
#else
			shutdown(s, 2);
#endif
			network_close(s);
			s = LM_BAD_SOCKET;
			break;
		}

/*
 *		Now we are connected, see if this is the server we want
 */
		DEBUG((l_basic_conn_efp, "Connected to %s\n", node));

		LM_SET_NET_ERRNO(0);

		i = network_write(s, msg, msgsize);
		if (i <= 0)
		{
			LM_SET_ERROR_THREAD(job, LM_CANTWRITE, 13, net_errno, node, LM_ERRMASK_ALL, thread);
			s = LM_BAD_SOCKET;

			if (net_errno == NET_ECONNRESET)
			{

				lc_disconn(job, 1);
			}
			break;
		}
		DEBUG((l_basic_conn_efp, "Sent: %s\n", msg));
		pause_timer(job);
		DEBUG((l_basic_conn_efp, "Selecting read for %d seconds\n",
					job->options->conn_timeout));
		i = l_select_one(s, 1, 1000 * job->options->conn_timeout);
		continue_timer(job);
		if (i <= 0)	/*- Apollo select returns -1
							instead of 0 !!!  */
		{
			LM_SET_ERROR_THREAD(job, LM_CANTREAD, 287, net_errno, node, LM_ERRMASK_ALL, thread);
			i = 0;
			if (transport == LM_UDP)
			{
				/*retry*/
				DEBUG((l_basic_conn_efp, "UDP: Reseding: %s\n", msg));
				i = network_write(s, msg, msgsize);
				pause_timer(job);
				i = l_select_one(s, 1,
				    1000 *job->options->conn_timeout);
				continue_timer(job);
			}
		}
		if (i > 0)
		{
			i = l_read_timeout(s, resp, msgsize,
					job->options->conn_timeout * 1000);
			if (i<0)
			{
				LM_SET_ERROR_THREAD(job, LM_CANTREAD, 289, net_errno, node, LM_ERRMASK_ALL, thread);
			}
#ifdef SUPPORT_UDP
/*
 *			Read the sernum, but don't worry if it's right
 */
			if ((transport == LM_UDP) &&
				!l_read_sernum(resp,
					&job->daemon->udp_sernum))
				/* force it */
				job->daemon->udp_sernum =
					((resp[MSG_CHECKSUM] >> 4) & 0x0f);
#endif /* SUPPORT_UDP */
		}
		if (i <= 0)
		{
			{
#if !defined(MOTO_88K) && !defined(USE_WINSOCK)
				(void) shutdown(s, 2);
#endif
				(void) network_close(s);
			}
			s = LM_BAD_SOCKET;
			break;
		}
		DEBUG((l_basic_conn_efp, "Received pid %d %d bytes: msgtype %c\n",
				abs(getpid())%1000, i, resp[MSG_CMD]));
		if (resp[MSG_DATA])
		{
			DEBUG((l_basic_conn_efp, "pid %d line %d\n", abs(getpid())%1000, __LINE__));
/*
 *			rare place where clear_error is OK
 *			We clear the error to indicate that we
 *			connected and exchanged messages
 */
			if (!(job->err_info.mask & LM_ERRMASK_FROM_SERVER))
			{
				/*- rare place where this is OK */
				{
					l_clear_error(job);
				}
			}
			DEBUG((l_basic_conn_efp, "..... %s\n", &resp[MSG_DATA]));
		}
		if (resp[MSG_CMD] != LM_OK)
		{
		  int lerrno = 0;
			DEBUG((l_basic_conn_efp, "pid %d line %d\n", abs(getpid())%1000, __LINE__));
			DEBUG((l_basic_conn_efp, "*resp is %c\n", *resp));
			if (*resp == LM_WHAT)
			{
				DEBUG((l_basic_conn_efp, "pid %d line %d\n", abs(getpid())%1000, __LINE__));
                                DEBUG((l_basic_conn_efp, "resp is WHAT \n", resp[MSG_CMD]));
				{
					lerrno = atoi(&resp[MSG_DATA]);
					DEBUG((l_basic_conn_efp, "lerrno is %d\n", lerrno));
					if ((lerrno < 0) &&
						(lerrno >= LM_LAST_ERRNO))
					{
		LM_SET_ERROR_THREAD(job, lerrno, 274, 0, node, LM_ERRMASK_ALL, thread);
					}
					else
					{
			DEBUG((l_basic_conn_efp, "BADCOMM\n"));
		LM_SET_ERROR_THREAD(job, LM_BADCOMM, 16, 0, node, LM_ERRMASK_ALL, thread);
					}
				}
			}
			if (!sent_seeds)
			{
				DEBUG((l_basic_conn_efp, "pid %d line %d\n", abs(getpid())%1000, __LINE__));
				{
#if !defined(MOTO_88K) && !defined(USE_WINSOCK)
					(void) shutdown(s, 2);
#endif
					(void) network_close(s);
				}
			DEBUG((l_basic_conn_efp, "BADSOCKET\n"));
				s = LM_BAD_SOCKET;
			}
		}
		else
		DEBUG((l_basic_conn_efp, "pid %d line %d\n", abs(getpid())%1000, __LINE__));
		break;
		/*NOTREACHED*/
	}
	job->flags &= ~LM_FLAG_IN_CONNECT;
	put_timer_back(job);
	DEBUG((l_basic_conn_efp, "returning pid %d %d min_errno %d\n",
				abs(getpid())%1000, s, job->err_info.min_errno));
	return(s);
}

static void
install_timer(job)
LM_HANDLE *job;		/* Current license job */
{
	if (usetimers && !saved_user)
	{
#ifndef PC
		_utimer = l_timer_signal(job, SIGALRM, _timer);
		(void) (*set_timer)(
#ifdef VMS
				    job,
#endif
					ITIMER_REAL, &timeout, &_utime);
		saved_user = 1;
#endif  /* ndef PC */
	}
}

static void
pause_timer(job)
LM_HANDLE *job;		/* Current license job */
{
	if (usetimers)
	{
#ifndef PC
		(void) l_timer_signal(job, SIGALRM, SIG_IGN);
		(void) (*set_timer)(
#ifdef VMS
				    job,
#endif
					ITIMER_REAL, &timeout, &remain_timeout);
#endif  /* ndef PC */
	}
}


static void
continue_timer(job)
LM_HANDLE *job;		/* Current license job */
{
	if (usetimers)
	{
#ifndef PC
		(void) l_timer_signal(job, SIGALRM, _timer);
		(void) (*set_timer)(
#ifdef VMS
				    job,
#endif
					ITIMER_REAL, &remain_timeout,
							(struct itimerval *) 0);
#endif
	}
}

static void
put_timer_back(job)
LM_HANDLE *job;		/* Current license job */
{
	if (usetimers)
	{
#ifndef PC
		if (_utimer == SIG_IGN || _utimer == SIG_DFL)
			(void) l_timer_signal(job, SIGALRM, SIG_IGN);
		else
			(void) l_timer_signal(job, SIGALRM, _utimer);
		(void) (*set_timer)(
#ifdef VMS
				    job,
#endif
					ITIMER_REAL, &_utime,
							(struct itimerval *) 0);
#endif
	}
}


LM_SOCKET
l_make_tcp_socket(job)
LM_HANDLE *job;		/* Current license job */
{
  int optval = 1;
  LM_SOCKET s;




	LM_SET_NET_ERRNO(0);
	if ( (s = socket(AF_INET, SOCK_STREAM, 0)) == LM_BAD_SOCKET)
	{
		DEBUG((l_basic_conn_efp, "sock call failed %d\n", net_errno));
		/* Socket failure */
		LM_SET_ERRNO(job, LM_SOCKETFAIL, 17, net_errno);
		return s;
	}
#if !defined(PC) && !defined(VXWORKS)
/*
 *		Prevent STOPped programs from hanging the port
 */
	fcntl(s, F_SETFD, 1); /* close-on-exec */
#endif /* PC */
	DEBUG((l_basic_conn_efp, "sock is %d\n", s));
	job->daemon->commtype = LM_TCP;

	return s;
}

#ifdef SUPPORT_UDP
LM_SOCKET
l_make_udp_socket(job)
LM_HANDLE *job;		/* Current license job */
{
  LM_SOCKET s;

	LM_SET_NET_ERRNO(0);
	if ( (s = socket(AF_INET, SOCK_DGRAM, 0)) == LM_BAD_SOCKET)
	{
		LM_SET_ERRNO(job, LM_SOCKETFAIL, 19, net_errno);
		return s;
	}
	job->daemon->commtype = LM_UDP;
#ifndef PC
	fcntl(s, F_SETFD, 1); /* close-on-exec */
#endif
	return s;
}
#endif /* SUPPORT_UDP */

#ifdef SUPPORT_IPX
static
LM_SOCKET
l_make_spx_socket(job, thread)
LM_HANDLE *job;		/* Current license job */
char *thread;
{
  LM_SOCKET s;

	LM_SET_NET_ERRNO(0);
	if ( (s = socket(AF_NS, SOCK_STREAM, NSPROTO_SPX)) == LM_BAD_SOCKET)
	{
		LM_SET_ERROR_THREAD(job, LM_SOCKETFAIL, 275, net_errno, 0, LM_ERRMASK_ALL, thread);
		return s;
	}
	job->daemon->commtype = LM_SPX;
	return s;
}
#endif /* SUPPORT_IPX */


