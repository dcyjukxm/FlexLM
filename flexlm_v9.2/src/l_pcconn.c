/******************************************************************************

	    COPYRIGHT (c) 2001, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of
	Macrovision Corporation and is protected by law.  It may
	not be copied or distributed in any form or medium, disclosed
	to third parties, reverse engineered or used in any manner not
	provided for in said License Agreement except with the prior
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/******************************************************************************
 *
 *
 *	NOTE:	The purchase of FLEXlm source does not give the purchaser
 *
 *		the right to run FLEXlm on any platform of his choice.
 *
 *		Modification of this, or any other file with the intent
 *
 *		to run on an unlicensed platform is a violation of your
 *
 *		license agreement with Macrovision Corporation.
 *
 *
 *****************************************************************************/
/*
 *	Module: $Id: l_pcconn.c,v 1.22 2003/05/12 16:38:00 sluu Exp $
 *
 *	Description: 	Multi-threaded PC connect
 *
 *	D. Birns
 *	2/01
 *
 */
/*#define STANDALONE*/
#ifndef STANDALONE
#include "lmclient.h"
#include "l_prot.h"
#include "lmachdep.h"
#include "lmselect.h"
#endif /* !STANDALONE */
#ifdef RELEASE_VERSION
#define DEBUG(x)
#else
#endif

#if defined ( _MSC_VER)
#include <windows.h>
#define ERRNO WSAGetLastError()
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/errno.h>
extern int errno;
#define ERRNO errno;
#endif /* PC */

#include <stdio.h>
#include <time.h>
#include <string.h>
#define STR "Client side hello!"
/*int h_errno; */
WSADATA wsadata;
WORD WinsockVerRequested;

#ifdef STANDALONE 	/* for testing purposes */
extern int gportnum ;
#define LMGRD_PORT_START gportnum
#define LMGRD_PORT_END gportnum + 9

#define DEBUG(x) printf("%d:", t % 9999); printf x
#define LM_HANDLE long
#define l_malloc(x, y) calloc(y, 1)
static struct connarg *c;
#define L_CONN_DATA c
#define PCC_TIMEOUT 10
#define L_DEFAULT_PORTS 10
#define L_THREADS threads
HANDLE *threads;
static time_t t;
#define LM_SET_ERRNO(a, b, c, d)
#define LM_SET_ERROR(a, b, c, d, e, x)
#define LM_SOCKET unsigned int
typedef int  *SELECT_MASK;
#define MASK_INIT(j, fd)        { FD_ZERO((fd_set *)j); FD_SET(fd,(fd_set *) j); }
#define MASK_ZERO(set)          FD_ZERO((fd_set *)set)
#define l_select select
#define network_close closesocket
#define net_errno WSAGetLastError()
#define LM_BAD_SOCKET (~0)

#else /* IN FLEXLM...*/

#ifndef RELEASE_VERSION
extern char * l_conn_debug;
extern FILE *l_basic_conn_efp;
#define DEBUG(x)	if (l_conn_debug) fprintf x
#endif

#define L_CONN_DATA  ((struct connarg *)job->connargs)
#define PCC_TIMEOUT job->options->conn_timeout
#define L_THREADS job->threads

#endif /* STANDALONE*/
LM_SOCKET l_make_tcp_socket (LM_HANDLE *);
LM_SOCKET l_make_udp_socket (LM_HANDLE *);
#define PCC_WAIT_TIMEOUT (PCC_TIMEOUT * 10)
#define SELECT_TIMEOUT				100000	/* timeout for default port connect, in microseconds */
#define SOCKET_STATE_UNINITIALIZED	0		
#define SOCKET_STATE_INITIALIZED	1
#define SOCKET_STATE_TIMED_OUT		2
#define L_PCCONN_GETSTAT			-1
#define L_PCCONN_CLOSE_HANDLE		-2
#define FLEXLM_TIMEOUT				"FLEXLM_TIMEOUT"	/* env var used to override default */

static int pc_connect_timeout(
	LM_HANDLE *			job,
	LM_SOCKET			s,
	char *				node,
	struct sockaddr *	sock,
	int					size);

void l_conn_harvest(LM_HANDLE *job);


/*
 *	Purpose:	Connect to license server listening at port 27000 - 27009
 *
 *	Input:		job - FLEXlm job handle
 *				pSocket - pointer to LM_SOCKET that be set to the socket that was
 *						  succussfully able to connect.
 *				saddr - system we're trying to connect to.
 *	Returns:	0 on success, else -1.
 */
static
int
sDoConnectDefault(
	LM_HANDLE *			job,
	LM_SOCKET *			pSocket,
	struct sockaddr *	saddr)
{
	int						ret = -1;
	int						i = 0;
	int						err = 0;
	int						mode = 1;	/* non blocking */
	int						connected = SOCKET_ERROR;
	int						port = 0;
	long					temp = 0;
	struct sockaddr_in		sin;
	struct timeval			timeout;
	fd_set					wfds;

	if( (job == NULL) || (pSocket == NULL) || (saddr == NULL) )
		goto done;

	memset(&sin, 0, sizeof(struct sockaddr_in));
	memcpy(&sin, saddr, sizeof(struct sockaddr_in));
	port = ntohs(sin.sin_port);
	FD_ZERO(&wfds);

	timeout.tv_sec = 0;
	timeout.tv_usec = SELECT_TIMEOUT;

	if(l_getenv(job, FLEXLM_TIMEOUT))	/* overrun checked */
	{
		temp = atol(l_getenv(job, FLEXLM_TIMEOUT));	/* overrun checked */
		if(temp)
			timeout.tv_usec = temp;
	}

	/*
	 *	Create sockets needed
	 */
	if(job->sockState == SOCKET_STATE_UNINITIALIZED)
	{
		job->sockState = SOCKET_STATE_INITIALIZED;
		memset(job->sockets, -1, sizeof(job->sockets));

		for(i = port - LMGRD_PORT_START; i < L_DEFAULT_PORTS; i++)
		{
			job->sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
			if(job->sockets[i] == SOCKET_ERROR)
				goto done;
			/*
			 *	Set to non blocking
			 */
			err = ioctlsocket(job->sockets[i], FIONBIO, &mode);
			if(err == SOCKET_ERROR)
				goto done;
		}

		/*
		 *	Try to connect to the server
		 */
		for(i = 0; i < L_DEFAULT_PORTS; i++)
		{
			if(job->sockets[i] == SOCKET_ERROR)
				continue;
			sin.sin_port = (short)htons((short)(LMGRD_PORT_START + i));
			err = connect(job->sockets[i], (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
			if(err == SOCKET_ERROR)
			{
				if(WSAGetLastError() == WSAEWOULDBLOCK)
				{
					/*
					 *	Add this socket to fd_set
					 */
					FD_SET(job->sockets[i], &wfds);
				}
				else
				{
					/*
					 *	Just close this socket
					 */
					closesocket(job->sockets[i]);
					job->sockets[i] = SOCKET_ERROR;
				}
			}
			else
			{
				/*
				 *	Connected, just add it to the list anyway
				 */
				FD_SET(job->sockets[i], &wfds);
			}
		}
	}
	else
	{
		/*
		 *	Setup fd_set struct
		 */
		for(i = 0; i < L_DEFAULT_PORTS; i++)
		{
			if(job->sockets[i] != SOCKET_ERROR)
				FD_SET(job->sockets[i], &wfds);
		}
	}

	if(job->sockState != SOCKET_STATE_TIMED_OUT)
	{
		i = select(0, NULL, &wfds, NULL, &timeout);
		if(i > 0)
		{
			for(i = 0; i < L_DEFAULT_PORTS; i++)
			{
				if(job->sockets[i] != SOCKET_ERROR &&
					FD_ISSET(job->sockets[i], &wfds))
				{
					mode = 0;
					(void)ioctlsocket(job->sockets[i], FIONBIO, &mode);
					*pSocket = job->sockets[i];
					/*
					 *	whoever receives this socket will close it so we can
					 *	just set it to SOCKET_ERROR
					 */
					job->sockets[i] = SOCKET_ERROR;
					ret = 0;
					break;
				}
			}
		}
		else if(i == 0)
		{
#if 0
			/*
			 *	Connection on all sockets timed out
			 */
			job->sockState = SOCKET_STATE_TIMED_OUT;
			job->timeout_used = SELECT_TIMEOUT;

			/*
			 *	Cleanup after ourselves
			 */
			l_conn_harvest(job);
#endif
		}
		else
		{
			/*
			 *	Error occurred
			 */
			l_conn_harvest(job);
		}
	}

done:
	return ret;
}


/*
 *	l_pc_connect -- same args as l_connect()
 *	If not a default port, call pc_connect()
 *	If a default port, create 10 sockets, set to non blocking,
 *	then try connecting, wait up to SELECT_TIMEOUT seconds for connection.
 *	No longer create 10 threads, etc......
 */

int
l_pc_connect(
	LM_HANDLE *			job,
	LM_SOCKET *			s,
	struct sockaddr *	saddr,
	int					size,
	char *				node,
	int					transport)
{
	struct sockaddr_in *sin = (struct sockaddr_in*)saddr;
	int portnum = ntohs(sin->sin_port);
	int i = 0;
	int ret = -1;
	int gotit = 0;
	int max = L_DEFAULT_PORTS;
#ifdef STANDALONE
	t = time(0);
#endif
	static char *connect_no_thread = (char *)-1;

/*
 *	We don't use any socket that's passed in
 */
	DEBUG((l_basic_conn_efp, "l_pcconnect_threaded \n"));
	if (*s != LM_BAD_SOCKET)
		network_close(*s);
	*s = LM_BAD_SOCKET;
	if (connect_no_thread == (char *)-1)
	{
		connect_no_thread = getenv("FLEXLM_NO_MT_CONNECT");	/* overrun checked */
		if (connect_no_thread)
			 job->flags |= LM_FLAG_CONNECT_NO_THREAD;
	}



	if (((ntohs(sin->sin_port) < LMGRD_PORT_START) ||
				(ntohs(sin->sin_port) > job->port_end)) ||
		 job->flags & LM_FLAG_CONNECT_NO_THREAD)
	{
/*
 *	Non-default port -- do a simple non-threaded connect
 */
		int non_blocking_mode = 1;

		if ((*s =
			transport == LM_TCP ?
			l_make_tcp_socket(job) : transport == LM_UDP ?
			l_make_udp_socket(job) : -1
			) == LM_BAD_SOCKET)
		{
			DEBUG((l_basic_conn_efp, "socket failed %d\n", net_errno));
			LM_SET_ERRNO(job, LM_SOCKETFAIL, 10, net_errno);
			return -1;
		}
		ioctlsocket(*s, FIONBIO, &non_blocking_mode );
		non_blocking_mode = 0;

		DEBUG((l_basic_conn_efp, "calling pc_connect_timeout\n" ));
		if ((ret = pc_connect_timeout(job, *s, node,
				(struct sockaddr *)sin, size)) == -1)
		{
			DEBUG((l_basic_conn_efp, "connect failed %d sock %d, node %s\n", net_errno, *s, node));
			LM_SET_ERROR(job, LM_CANTCONNECT, 578,
					net_errno, node, LM_ERRMASK_ALL);
			network_close(*s);
			*s = LM_BAD_SOCKET;
		}
		else
			ioctlsocket(*s, FIONBIO, &non_blocking_mode );

		return ret;
	}
/*
 *	Default port
 */

	ret = sDoConnectDefault(job, s, saddr);
	DEBUG((l_basic_conn_efp, "returning %d\n", ret));

	return ret;
}

void
l_conn_harvest(LM_HANDLE *job)
{
	int	i = 0;

	if (job->flags & LM_FLAG_CONNECT_NO_HARVEST)
		return;

	job->sockState = SOCKET_STATE_UNINITIALIZED;

	for(i = 0; i < L_DEFAULT_PORTS; i++)
	{
		if(job->sockets[i] != SOCKET_ERROR)
		{
			closesocket(job->sockets[i]);
			job->sockets[i] = SOCKET_ERROR;
		}
	}
}

/**************************/

static
int
pc_connect_timeout(
	LM_HANDLE *			job,
	LM_SOCKET			s,
	char *				node,
	struct sockaddr *	sock,
	int					size)
{
	fd_set  w32_fd_read, w32_fd_write, w32_fd_except;

	if ( connect(s, sock, size ) )
	{
		time_t start_time;
		SELECT_MASK fd_read, fd_write, fd_except;
		struct timeval sel_timeout;
		int e = net_errno;

		fd_read=(int *)&w32_fd_read;
		fd_write=(int *) &w32_fd_write;
		fd_except=(int *) &w32_fd_except;
		if ( (e!=WSAEWOULDBLOCK)&&(e!=WSAEINPROGRESS) )
		{
			network_close(s);
			return -1;
		}

		/*
		 *			Socket is in waiting mode for connect().
		 *			Now use select() until connect() succeeds or
		 *			fails.
		 */
		start_time = GetTickCount();
		sel_timeout.tv_sec = PCC_TIMEOUT;
		sel_timeout.tv_usec = 0;

		while (1)
		{
			MASK_INIT( fd_write,s);
			MASK_ZERO( fd_read );
			MASK_ZERO( fd_except );
			MASK_INIT( fd_except,s);

			l_select( 0, fd_read,
				fd_write,
				fd_except,
				&sel_timeout );


			if ( FD_ISSET(s, fd_except ) )
			{
				/*
				 *	Connect() major error.
				 */
				LM_SET_ERROR(job,
				LM_CANTCONNECT, 10,
				NET_ECONNREFUSED, node,
				LM_ERRMASK_ALL);

				network_close(s);
				return -1;
			}
			if (FD_ISSET(s, fd_write))
				return 0;

			if ( GetTickCount() - start_time >
				     PCC_TIMEOUT * 975 )
			{
				/*
				 *	Connect() timeout!
				 */
				network_close(s);

				LM_SET_ERROR(job, LM_HOSTDOWN,
				491, errno, node,
				LM_ERRMASK_ALL);
				return -1;
			}

		}
		if ( s == LM_BAD_SOCKET )
			return -1;
	}

	return 0;
}


