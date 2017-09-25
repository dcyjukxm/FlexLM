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
 *	Module: $Id: ls_socket.c,v 1.12.4.1 2003/06/25 20:53:37 sluu Exp $
 *
 *	Function: ls_socket(hostname, endpoint, dynamic)
 *
 *	Description: Create the socket, and listen for incomming connections
 *
 *	Parameters:	(char *) hostname - The host we are running on.
 *					    if LOCAL, the fifo pathname.
 *			(COMM_ENDPOINT *) endpoint - communication endpoint
 *			(int) dynamic - Flag to indicate that the socket
 *				should be bound to a "floating" port #
 *				if LOCAL, true means use tmpname().
 *
 *	Return:		(int) - The socket file descriptor.
 *			The port number is filled in.
 *
 *	M. Christiano
 *	2/27/88
 *
 *
 *	Last changed:  9/9/98
 */


#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "flexevent.h"
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
extern char *sys_errlist[];
#endif
#ifndef NO_UIO_H
#include <sys/uio.h>
#endif
#ifdef USE_WINSOCK
#include <pcsock.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
/*- #include <netinet/in.h> 	 Included in lsserver.h */
#ifdef DGX86
#include <netinet/tcp.h>
#endif /* DGX86 */
#include <netdb.h>
#endif
#include <errno.h>

static char *myname = "license manager";
extern int ls_i_am_lmgrd;
static LM_SOCKET
#ifdef SUPPORT_FIFO
	ls_make_fifo lm_args((char *, int, int)),
#endif /* SUPPORT_FIFO */
	ls_make_tcp_socket(),
#ifdef SUPPORT_IPX
	ls_make_spx_socket(),
#endif
	ls_make_udp_socket();

static int bind_to_default_port lm_args((LM_SOCKET , struct sockaddr_in *,
							unsigned short *));
static int bind_to_named_port 	lm_args((LM_SOCKET, struct sockaddr_in *,
						char *,
						unsigned short *,
						int transport));

#if !defined (htons) && !defined(i386) && !defined(OSF)	&& !defined(PC)
			/*- Sun systems make this a macro! */
extern u_short htons();
#endif

LM_SOCKET
ls_socket(hostname, endpoint, dynamic )
char *hostname; /* if LM_LOCAL, this is the fifo pathname */
COMM_ENDPOINT *endpoint;
int dynamic;
{
  char errmsg[100];
  struct hostent *host;
  struct servent ss, *serv = &ss;
  struct sockaddr_in sock;
  LM_SOCKET s;
  int x;
  int transport = endpoint->transport;
#ifdef SUPPORT_IPX
  struct sockaddr_ipx sock_ipx;
  unsigned short  spx_port;
#endif /* SUPPORT_IPX */
  unsigned short *port;
#ifdef VMS
  long ipaddr;
#endif /* VMS */
  int lmgrd_default_port = 0;

#ifdef SUPPORT_IPX
  	if (endpoint->transport == LM_SPX)
	{
		/*
		 * Since sa_socket is short, we need an intermediate spx_port
		 * of type int to take it address to assign to port.
		 */
		spx_port = (int) endpoint->transport_addr.spx_addr.sa_socket;
  		port = &spx_port;
	}
	else
#endif
		port = &(endpoint->transport_addr.port);


#ifdef FIFO
	if (transport != LM_LOCAL)
#endif
		if (dynamic || *port)
		{
			ss.s_name = "FLEXlm";
			ss.s_aliases = NULL;
			ss.s_port = (unsigned short) *port;
			if (transport == LM_TCP)
				ss.s_proto = "tcp";
#ifdef SUPPORT_IPX
			if (transport == LM_SPX)
				ss.s_proto = "spx";
#endif
			else
				ss.s_proto = "udp";
		}
		else
		{
			if (transport == LM_TCP)
				serv = getservbyname("FLEXlm", "tcp");
			else
				serv = getservbyname("FLEXlm", "udp");
#ifndef SUPPORT_IPX
			if (serv == NULL)
			{
				ss.s_port = ntohs((unsigned short)
							LMGRD_PORT_START);
				lmgrd_default_port = 1;
			}
			*port = (int) serv->s_port;
#endif /* SUPPORT_IPX */
		}

	switch(transport)
	{
	case LM_TCP:
		s = ls_make_tcp_socket();
		break;
	case LM_UDP:
		s = ls_make_udp_socket();
		break;
#ifdef SUPPORT_IPX
	case LM_SPX:
		s = ls_make_spx_socket();
		break;
#endif
#ifdef SUPPORT_FIFO
	case LM_LOCAL:
		/* everything is done here */
		return ls_make_fifo(hostname, *port, dynamic);
#endif /* SUPPORT_FIFO */
	}

#ifdef SUPPORT_IPX
	if ( transport == LM_SPX )
	{
		sock_ipx = endpoint->transport_addr.spx_addr;
		if (dynamic ) sock_ipx.sa_socket = 0;
	}
	else
#endif
	{

		bzero((char *)&sock, sizeof(sock));

#ifndef NLM	/* NLM Vendor daemon may be started with no argument */

		if (!(
#ifdef VMS
			ipaddr =
#endif /* VMS */
			l_ipaddr(hostname)))
		{
			host = gethostbyname(hostname);
			if (host == NULL)
			{
				if(getenv("FLEXLM_ANYHOSTNAME"))
				{
					host = gethostbyname("localhost");
				}
				if(host == NULL)
				{
					LOG((lmtext("Unknown host: %s\n"), hostname));
					LOG_INFO((INFORM, "A host specified in the license file is \
					not available in the local network database."));
					ls_go_down(EXIT_BADCONFIG);
				}
			}
		}
#endif /* NLM */

#ifdef VMS	/*- This code was used for all platforms prior to v2.1 */
		if (ipaddr)
		{
			memcpy(&sock.sin_addr, &ipaddr, 4);
			sock.sin_family = AF_INET; /* IS THIS RIGHT FOR VMS?? */
		}
		else
		{
			bcopy(host->h_addr, (char *)&sock.sin_addr,
							host->h_length);
			sock.sin_family = host->h_addrtype;
		}
#else
		sock.sin_addr.s_addr = INADDR_ANY;	/* Listen on any
							interface on
							this machine */
		sock.sin_family = AF_INET;
#endif
		if (serv) sock.sin_port = serv->s_port;

		if ((sock.sin_port & 0xffff) == 0xffff)
				sock.sin_port = 0;
	}

/*
 *	Bind the socket.  If we are the top-level server, try 75 times,
 *	sleeping 5 seconds between retrys.  This is so that we can be sure
 *	of not running into the network timeout of 8 * 45 seconds, since
 *	this address might be lingering from another server that died.
 *	Log the fact that we are sleeping every 30 seconds.
 */

	if (lmgrd_default_port)
		x = bind_to_default_port(s, &sock, port);
	else
	{
#ifdef SUPPORT_IPX
		x = bind_to_named_port(s, &sock, (char *)&sock_ipx, port,
						transport);
#else
		x = bind_to_named_port(s, &sock, 0, port,0);
#endif
	}
	if (x != 0)
	{
		return -1;
	}
	if (dynamic )
	{
	  unsigned int len = sizeof(sock);
/*
 *		Find out what port number the system assigned.
 */
#ifdef SUPPORT_IPX
	  if (transport == LM_SPX)
	  {
		len = sizeof(sock_ipx);
		if (getsockname(s, (struct sockaddr *) &sock_ipx, &len) != 0)
			perror("getsockname");
		endpoint->transport_addr.spx_addr.sa_socket=sock_ipx.sa_socket;
	  }
	  else
#endif
	  {
		if (getsockname(s, (struct sockaddr *) &sock, (int *)&len) != 0)
			perror("getsockname");
		/* The port # we got */
		endpoint->transport_addr.port = (int)sock.sin_port;
	  }
	}
#ifndef ACCEPT_BUG
	if ((transport == LM_TCP)
#ifdef SUPPORT_IPX
	    || (transport == LM_SPX)
#endif
	    )

		if (listen(s, LISTEN_BACKLOG) < 0)
		{
			(void) sprintf(errmsg, "%s: listen", myname);
			perror(errmsg);
			ls_go_down(EXIT_COMM);
		}
#endif

#ifndef RELEASE_VERSION
	if (l_real_getenv("LS_DOWN_DEBUG")) DLOG(("bind fd %d\n", s));
#endif
	return(s);
}

#ifdef SUPPORT_FIFO
/*
 *	ls_make_fifo() - does a mknod, but does not open, because it
 *			would block. always returns -1;
 */
static
LM_SOCKET
ls_make_fifo(path, port, dynamic)
char *path;
int port;
int dynamic;
{
  char filename[LM_MAXPATHLEN];

	sprintf(filename, "%s/%d", LM_FLEXLM_DIR, port);
	(void)unlink(filename);
	if (mknod(filename, S_IFIFO | 0777, 0))
	{
		LOG((lmtext("fifo mknod failed, errno %d, exiting"),
			errno));
		ls_go_down(EXIT_BADCALL);
	}
	lm_job->localcomm = (LM_LOCAL_DATA *)LS_MALLOC(sizeof (LM_LOCAL_DATA));
	lm_job->localcomm->readname = (char *)LS_MALLOC(strlen(filename) + 1);
	strcpy(lm_job->localcomm->readname, filename);
	return -1;
}
#endif /* SUPPORT_FIFO */

/*
 *	ls_make_tcp_socket() - returns socket handle of type LM_SOCKET
 */

static
LM_SOCKET
ls_make_tcp_socket()
{
  LM_SOCKET s;
  char errmsg[100];
  int optval = 1;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == LM_BAD_SOCKET)
	{
		(void) sprintf(errmsg, "%s: socket", myname);
		perror(errmsg);
#ifdef SUPPORT_IPX
		return (LM_BAD_SOCKET);
#else
		ls_go_down(EXIT_COMM);
#endif
	}
#if !defined(ALPHA) && !defined(PC)
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(int)) < 0)
					perror("setsockopt KEEPALIVE");
#endif
#if defined (MOTO_88K) | defined(MOTOSVR4)
	{
	  char optval = 1;
	  struct linger ling;

		ling.l_onoff = 0;
		ling.l_linger = 0;
		if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ling,
						sizeof(struct linger)) < 0)
			perror("setsockopt");
	}
#endif
#ifdef DGX86
	{
	  int sol_tcp;
	  struct protoent *proto;
		if (proto = getprotobyname("tcp"))
		{
			sol_tcp = proto->p_proto;
			setsockopt(s, sol_tcp, TCP_NODELAY, (char *)&optval,
								sizeof(optval));
		}
	}
#endif /* DGX86 */
	return(s);

}

static
LM_SOCKET
ls_make_udp_socket()
{
  LM_SOCKET s;
  char errmsg[100];

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == LM_BAD_SOCKET )
	{
		(void) sprintf(errmsg, "%s: socket", myname);
		perror(errmsg);
#ifndef SUPPORT_IPX
		ls_go_down(EXIT_COMM);
#endif
	}
	return(s);
}

#ifdef SUPPORT_IPX
static
LM_SOCKET
ls_make_spx_socket()
{
  LM_SOCKET s;
  char errmsg[100];

	if ((s = socket(AF_NS, SOCK_STREAM, NSPROTO_SPX)) == LM_BAD_SOCKET )
	{
		(void) sprintf(errmsg, "%s: socket", myname);
		perror(errmsg);
	}
	return(s);
}

#endif /* SUPPORT_IPX */

/**********************************************************************
 * Bind the socket to the port specified by the user in the license file
 * or as part of the port@host syntax.
 *
 * Also called while searching for an available default port.
 *
 * Parameters:
 * 			  s	 	 		   - The socket returned by the socket() call
 * 			  *sock			   - The sockaddr to bind with
 * 			  *sock_ipx		   - IPX socket. IPX is not supported any longer
 * 			  *port            - The port number. Only used for log messages if there is an error
 * 			  transport        - IPX or TCP. ALthough we don't support IPX any more
 *
 * Return:
 * 		  The value from bind
 *********************************************************************/
static int bind_to_named_port( LM_SOCKET s,
							   struct sockaddr_in *sock,
							   char *sock_ipx,
							   unsigned short *port,
	   	   					   int transport)
{
  	int x;
  	int count = 0;
  	time_t start_time = time(0);
	char *	ppszInStr[20] = {NULL};
	char	buffer[MAX_PATH] = {'\0'};

	while (1)
	{
#ifdef SUPPORT_IPX
		if ( transport == LM_SPX )
			x = bind(s, (struct sockaddr *) sock_ipx, sizeof(struct sockaddr_ipx ));
		else
#endif
		x = bind(s, (struct sockaddr *) sock, sizeof(*sock));
		if (x != 0 && ls_i_am_lmgrd && net_errno == NET_EADDRINUSE &&
	 			!((ntohs((unsigned short)*port) >= LMGRD_PORT_START) &&
				  (ntohs((unsigned short)*port) <= lm_job->port_end)) &&
		   		count++ <= 75)
		{
			/* kmaclean 10/25/02
			 * Removed the following conditions from the above if ()
			 * They caused us to not retry if the named port was in the
			 * default range. This func is never called while searching the
			 * default ports. It's only called if a port was specified by the
			 * user
	 		//	!(
			//	(ntohs((unsigned short)*port) >= LMGRD_PORT_START) &&
			//	(ntohs((unsigned short)*port) <= lm_job->port_end)) &&
			 *
			 * */
			if (count == 6)
			{
			    LOG((lmtext("The TCP port number in the license, %d, is already in use.\n"),
							ntohs((short) *port)));

				LOG((lmtext("Possible causes: \n")));
				LOG((lmtext("   1) lmgrd is already running for this license.\n")));
				LOG((lmtext("   2) The OS has not \"cleared\" this port since lmgrd died.\n")));
				LOG((lmtext("   3) Another process is using this port number (unlikely).\n")));
				LOG((lmtext("Solutions:\n")));
				LOG((lmtext("   1) Make sure lmgrd and all vendor daemons for this \n")));
				LOG((lmtext("      license are not running.\n")));
				LOG((lmtext("   2) You may have to wait for the OS to clear this port.\n")));
#ifdef SUN
				LOG((lmtext("   3) On some Solaris systems, the port doesn't clear for 5\n")));
				LOG((lmtext("      minutes. This can be fixed by running (as root):\n")));
				LOG((lmtext("      ndd -set /dev/tcp tcp_close_wait_interval 2400\n")));
#endif /* SUN */
	   	  	  	LOG((lmtext("Retrying for about 5 more minutes\n")));
				if(l_flexEventLogIsEnabled())
				{
					sprintf(buffer, "%d", ntohs((short)(*port)));
					ppszInStr[0] = buffer;

					l_flexEventLogWrite(NULL,
										FLEXEVENT_WARN,
										CAT_FLEXLM_LMGRD,
										MSG_FLEXLM_LMGRD_PORT_NOT_AVAILABLE,
										1,
										ppszInStr,
										0,
										NULL);
				}

			}
			else if ((time(0) - start_time) > (60*5))
			{
				LOG((lmtext("Exiting ... \n")));
				exit(EXIT_PORT_IN_USE);
			}
			else if ((count % 6) == 0)
			{
				LOG((lmtext("Still trying... \n")));
			}
			lm_sleep(3);
		}
#ifdef CANT_RE_BIND
		/*-
		 *		Apollo and Motorola have a real problem with TCP compatibility
		 */
		else if (x != 0 && ls_i_am_lmgrd && net_errno == NET_EINVAL)
		{
			LOG((lmtext("Socket bind failed, please re-run lmgrd\n")));
			LOG_INFO((INFORM, "On some systems, if the socket \
				exists when the first bind system call is \
				issued, no further bind attempt will succeed \
				until the program is re-run."));
			ls_go_down(EXIT_PORT_IN_USE);
		}
#endif
		else
			 break;
	}
	return x;
}

/**********************************************************************
 * Try to bind the socket to each of the default ports in sequence
 * until success.
 *
 * return:
 * 		  the value from bind.
 *********************************************************************/
static int bind_to_default_port(LM_SOCKET s,
	   	   						struct sockaddr_in *sock,
								unsigned short *port)
{
	unsigned short i;
	int x;
	for (i = (unsigned short)LMGRD_PORT_START;
			i <= (unsigned short)lm_job->port_end; i++)
	{
		sock->sin_port = *port = ntohs(i);
		if (x = bind(s, (struct sockaddr *) sock, sizeof(*sock)))
#ifdef UNIX
			LOG(("Can't use port %d (%s)\n", i,
						SYS_ERRLIST(net_errno)));
#else
			LOG(("Can't use port %d (error: %d)\n", i, net_errno));
#endif
	}
	return x;
}
