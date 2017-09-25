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
 *	Module: $Id: ls_sconnect.c,v 1.11 2003/04/18 23:48:00 sluu Exp $
 *
 *	Function: ls_sconnect(node, port, exptime)
 *
 *	Description:	Connects to the named server.
 *
 *	Parameters:	(char *) node - Which node's server to connect to.
 *			(int) port - Which network port number to use
 *		(GLOBAL)(int) lm_job->options->conn_timeout - 
 *						How long to wait for connection
 *
 *	Return:		(int) - File descriptor (-1 error)
 *			(long *) exptime - Expiration time for connection
 *
 *	M. Christiano
 *	3/6/88
 *
 *	Last changed:  1/8/99
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_glob.h"
#include <sys/types.h>
#ifndef NO_UIO_H
#include <sys/uio.h>
#endif
#ifndef ANSI
#define const
#endif

#ifdef USE_WINSOCK
#include <pcsock.h>
#else
#include <sys/socket.h>
/*- #include <netinet/in.h>	 Included in lsserver.h */
#include <netdb.h>
#endif
#include <errno.h>
extern int ls_i_am_lmgrd;
extern char ls_our_hostname[];
static int x;			/* Dummy for ls_wakeup */
#ifdef THREAD_SAFE_TIME
static struct tm tst;
#endif
#if !defined(ALPHA) && !defined(OSF)  && !defined(USE_WINSOCK) && !defined(BSDI)
#ifndef htons			/*- Sun systems make this a macro! */
extern u_short htons();
#endif
#endif

extern LM_QUORUM quorum;	/* The LM_QUORUM in this server */
#ifdef RELEASE_VERSION
#define SLOG(x) 
#define _SLOG(x) 
#else
#define SLOG(x) if (quorum.debug) { DLOG(x); }
#define _SLOG(x) if (quorum.debug) { _DLOG(x); }
#endif /* RELEASE_VERSION */

LM_SOCKET
ls_sconnect(node, port, exptime)
char *node;
int port;
long *exptime;
{
  LM_SOCKET s;		/* The socket file descriptor */
  struct hostent *host = NULL;
  struct servent *serv;
  struct sockaddr_in sock;
  long ipaddr;
  int optval = 1;
  LM_SERVER *ls;
  CLIENT_ADDR ca;
  char msg[LM_MSG_LEN+1];

	(void) memset(&ca, '\0', sizeof(ca));
	ca.is_fd = 1;
	ca.transport = LM_TCP;

	SLOG(("sconnect "));
	LM_SET_NET_ERRNO(0);
	if (port == 0)
	{
		serv = getservbyname("license", "tcp");
		if (serv == NULL)
		{
			LM_SET_ERRNO(lm_job, LM_NOSERVICE, 356, net_errno);
			return(LM_BAD_SOCKET);
		}
	}
	if (!(ipaddr = l_ipaddr(node)))
	{
		host = gethostbyname(node);
		if (host == NULL)
		{
			LOG((
			lmtext("Host \"%s\" unknown in network database.\n"), 
									node));
			LOG_INFO((INFORM, 
				"The specified host name could not be found \
			in the network database on the local node.  This is \
			usually due to an incorrect hostname in the license \
			file."));
			lm_job->lm_errno = BADHOST;
			lm_job->u_errno = net_errno;
			return(LM_BAD_SOCKET);
		}
	}
/*
 *	Create the socket, and connect
 */
	if ( (s = socket(AF_INET, SOCK_STREAM, 0)) == LM_BAD_SOCKET)
	{
		lm_job->lm_errno = lm_job->u_errno = net_errno; /* Socket failure */
		return(LM_BAD_SOCKET);
	}
#if !defined (ALPHA) && !defined(DGUX)
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
		       (const char *)&optval, sizeof(int)) < 0)
				lm_job->lm_errno = lm_job->u_errno = net_errno;
#endif
#ifdef MOTO_88K
       	{
	  int optval = 1;
	  struct linger ling;

		ling.l_onoff = 0;
		ling.l_linger = 0;
		if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ling, 
						sizeof(struct linger)) < 0)
			perror("setsockopt");
	}
#endif
	bzero((char *)&sock, sizeof(sock));
	if (ipaddr) 
	{
		memcpy(&sock.sin_addr, &ipaddr, 4);
		sock.sin_family = AF_INET;
	}
	else 
	{
		bcopy(host->h_addr, (char *)&sock.sin_addr, host->h_length);
		sock.sin_family = host->h_addrtype;
	}
	if (port)
		sock.sin_port = htons((unsigned short) port);
	else
		sock.sin_port = serv->s_port;
/*
 *	Set a timer so that we don't wait for the full (45 sec)
 *	TCP/IP timeout for this connection.
 */
#ifndef PC /* The timer wakeup signal on NT does not break connect()! */ 
	ls_wakeup(lm_job->options->conn_timeout, &x);
#endif /* PC */

	LM_SET_NET_ERRNO(0);
#ifdef PC   	
	if (nt_connect(s, (struct sockaddr *) &sock, sizeof(sock)) != 0)
#else
	if (connect(s, (struct sockaddr *) &sock, sizeof(sock)) != 0)
#endif /* PC */
	{
		/*This is normally perfectly OK -- it simply means
		 * the other, redundant, server is not up yet.
		 */
		LM_SET_ERRNO(lm_job, LM_CANTCONNECT, 389, net_errno);
		DLOG(("connect to %d@%s failed with %d\n", port, node, net_errno));
		ls_sock_close(&s, "redundant connect");
		return s;
	}
/*
 *	We connected the socket, send the SHELLO message to
 *	prime the conversation.
 */
	_SLOG((" connect fd %d\n", s)); 
	(void) bzero(msg, LM_MSG_LEN+1);
	msg[MSG_HEL_VER] = COMM_VERSION;
	msg[MSG_HEL_VER+1] = COMM_REVISION;
	(void) strncpy(&msg[MSG_HEL_HOST], ls_our_hostname, 
						MAX_SERVER_NAME);/* LONGNAMES */
	(void) strncpy(&msg[MSG_HEL_DAEMON], lm_job->vendor, 
						MAX_DAEMON_NAME);/* LONGNAMES */
	ca.addr.fd = s;
	ls_server_send(&ca, LM_SHELLO, msg);
#ifdef THREAD_SAFE_TIME
	l_get_date(&x, &x, &x, exptime, &tst);
#else /* !THREAD_SAFE_TIME */
	l_get_date(&x, &x, &x, exptime);
#endif
/*
 *	Now send a quorum of the server nodes, in order
 */
	if (ls_i_am_lmgrd)
	{
	  int i;

	    for (i = 1, ls = ls_s_first(); ls && (i <= ls_s_qnum()); 
						ls = ls->next, i++)
	    {
		bzero(msg, LM_MSG_LEN + 1);
		(void) sprintf(&msg[MSG_ORDER_N], "%d", i);
		(void) strncpy(&msg[MSG_ORDER_HOST], ls->name, 
						MAX_SERVER_NAME);/* LONGNAMES */
		ls_server_send(&ca, LM_ORDER, msg);
	    }
	}
	SLOG(("end sconnect\n"));
	return(s);
}

#ifdef PC 
nt_connect( LM_SOCKET s, struct sockaddr *sock, int size )
{
	u_long non_blocking_mode;
	time_t start_time;
	int rcode,exit_code;
	SELECT_MASK fd_read, fd_write, fd_except;
	struct timeval sel_timeout;
			
	
	/*
	 *	On Windows and Windows NT platform, we will
	 *	temporarily switch to non-blocking mode connect()
	 *	so that we can control the timeout for connect().
	 */
	non_blocking_mode = 1;
	(void) network_control(s, FIONBIO, &non_blocking_mode );

	if ( connect(s, sock, size) == 0 )
		return 0;

	rcode = WSAGetLastError();

	if ( (rcode!=WSAEWOULDBLOCK)&&(rcode!=WSAEINPROGRESS) )
		return -1;

	/*
	 *	Socket is in waiting mode for connect().
	 *	Now use select() until connect() succeeds or
	 *	fails.
	 */
	start_time = time(NULL);
	sel_timeout.tv_sec = 2;		/* A fixed 2 second timeout. */
	sel_timeout.tv_usec = 0;
		
	MASK_CREATE( fd_write );
	MASK_CREATE( fd_read );
	MASK_CREATE( fd_except );
	MASK_INIT( fd_write, s  );
	MASK_INIT( fd_read, s );
	MASK_INIT( fd_except, s );
				
	if ( !l_select( 0, fd_read, fd_write, fd_except, &sel_timeout ))
	{
		/* Timeout */
		MASK_DESTROY(fd_read);
		MASK_DESTROY(fd_write);
		MASK_DESTROY(fd_except);
		return -1;
	}

	/*
	 *	switch back to blocking mode.
	 */
	non_blocking_mode = 0;
	(void) network_control(s, FIONBIO, &non_blocking_mode );
	
	if (IS_SET(fd_write, s) || IS_SET(fd_read, s))
		exit_code = 0;
	else
	{
		exit_code=-1;
	}
	MASK_DESTROY(fd_read);
	MASK_DESTROY(fd_write);
	MASK_DESTROY(fd_except);
	return exit_code;
}
#endif /* PC */
