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
 *	Module: $Id: l_connect.c,v 1.34.2.1 2003/06/19 00:31:59 sluu Exp $
 *
 *	Function: l_connect(job, master_list, daemon, transport)
 *		  l_connect_endpoint(job, endpoint, host)
 *		  l_get_endpoint(job, master_list, daemon, transport, endpoint)
 *
 *	Description: Connects to the specified daemon.
 *
 *	Parameters:	(LM_SERVER *) master_list - List of server nodes
 *			(char *) daemon - Daemon name
 *			Other data is in the configuration parameter file.
 *			(int) endpoint - transport type and port/node/socket
 *					 number
 *			(char *) host - host name
 *			(int) transport - LM_TCP, LM_SPX, or LM_UDP
 *
 *	Return:		(int) - -1 - Error (see job->lm_errno)
 *				>= 0 - File descriptor of socket.
 *
 *	M. Christiano
 *	2/15/88
 *
 *	Last changed:  12/8/98
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "l_socket.h"
#include "flex_utils.h"
#include <stdio.h>
#include <errno.h>

#ifdef USE_WINSOCK
#include <pcsock.h>
#else
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#endif
/*
 *	These three includes are for platforms that don't get
 *	misc. definitions from the files above (e.g.: HP, DG, PC, etc.)
 */
/*- Note: we should make an INCLUSIVE ifdef for these three */
#if !defined(PC) && !defined(VXWORKS)
#include <sys/ioctl.h>
#include <sys/file.h>
#include <netinet/in.h>
#else
#if defined(VXWORKS)
#include "iolib.h"
#endif
static void mt_reaper(LM_HANDLE *job);
#endif

#ifdef VMS
  extern char *our_daemon;
#endif

static setup_transport lm_args((char *resp, int allow_set_transport,
					int transport_reset, int old_transport,
					COMM_ENDPOINT *endpoint));
static int same_servers lm_args((LM_SERVER *, LM_SERVER *));
static int l_try_connect lm_args(( LM_HANDLE *, COMM_ENDPOINT *, char *,
					LM_SERVER *, char *, int));


#if !defined(htons) && !defined(PC) && !defined (OSF) && !defined (BSDI) && !defined(RHLINUX64)
extern u_short htons();
#endif


#ifndef NO_FLEXLM_CLIENT_API
API_ENTRY
int
l_connect(job, master_list, daemon, transport)
LM_HANDLE *job;		/* Current license job */
LM_SERVER *master_list;		/* Master list (if non-NULL) */
char *daemon;			/* Daemon name to connect to (if non-NULL) */
int transport;
{
  COMM_ENDPOINT endpoint;
  int ret;

/*
 *	check the daemon name and master list
 */
	if (master_list && (master_list->sflags & L_SFLAG_THIS_HOST)
						&& !*master_list->name)
	{
	  char hostname [MAX_HOSTNAME + 1];

		gethostname(hostname, MAX_HOSTNAME);
		strcpy(master_list->name, hostname);
		if (job->line->server)
			strcpy(job->line->server->name, hostname);
	}

	memset(&endpoint, 0, sizeof(endpoint));
	if ((daemon == (char *) NULL) || (master_list == (LM_SERVER *) NULL))
	{
		return(-1);
	}

	l_get_endpoint(job, master_list, daemon, transport, &endpoint);


/*
 *	If we have already connected to this daemon, we are done
 */
	if (job->daemon->socket != LM_BAD_SOCKET &&
	   same_servers(master_list, job->daemon->server) &&
	    !strcmp(daemon, job->daemon->daemon))
					return(job->daemon->socket);
	ret = l_connect_host_or_list(job, &endpoint, (char *)NULL,
			      master_list, daemon, 1);
        return ret;
}
#endif /* NO_FLEXLM_CLIENT_API */


/*
 *	l_get_endpoint() - Get the port number for a connection.
 */

API_ENTRY
l_get_endpoint(job, master_list, daemon, transport, endpoint)
LM_HANDLE *job;		/* Current license job */
LM_SERVER *master_list;
char *daemon;
int transport;
COMM_ENDPOINT *endpoint;
{
#ifdef VMS
  char *dpath, dopts[200];
#endif /* VMS */
  struct servent *serv;
  endpoint->transport = transport;

#ifdef VMS
/*
 *	If port@host, get the port number from master_list, else get it
 *	from the DAEMON line
 */

	if (master_list->port != -1)
	{
		endpoint->transport_addr.port = master_list->port;
	}
	else
	{
		dpath =
		  lc_daemon(job, daemon, dopts,
					(int *)&endpoint->transport_addr.port);
		if (dpath == (char *) NULL) endpoint->transport_addr.port = 0;
	}

#else
/*
 *	If the port number was specified in the license file, use it.
 */
#ifdef SUPPORT_IPX
	/*
	 *	We take the passed-in transport as the user preferred
	 *	transport protocol.  However, if the license file does not
	 *	specify	a transport address for that protocol, this rouine
	 *	will automatically switch endpoint to the protocol that is
	 *	valid.
	 */
	if ((transport == LM_SPX) && !master_list->spx_addr.sa_socket)
		endpoint->transport = transport = LM_TCP;

	if ((transport == LM_TCP) && ( master_list->commtype == LM_SPX ) &&
	    !getservbyname("FLEXlm", "tcp"))
		endpoint->transport = transport = LM_SPX;

	if (transport == LM_SPX)
	{
		endpoint->transport_addr.spx_addr = master_list->spx_addr;
		if (master_list->spx_addr.sa_socket)
			return master_list->spx_addr.sa_socket;
		else
		{
			LM_SET_ERRNO(job, LM_NOSERVICE, 242, net_errno);
			return -1;
		}
	}
#endif /* SUPPORT_IPX */

#if 0
	if (master_list->port == 0)
		/*printf("whoa");*/
#endif

	endpoint->transport_addr.port = master_list->port;

	if (endpoint->transport_addr.port == 0)
	{

	LM_SET_NET_ERRNO(0);
/* offical port: 744 */
		serv = getservbyname("FLEXlm", "tcp");
		if (serv == NULL)
		{
			LM_SET_ERRNO(job, LM_NOSERVICE, 26, net_errno);
			return(-1);
		}
		endpoint->transport_addr.port = serv->s_port;
	}
	else
	{
/*
*		We pass the port around in host order
*/
		endpoint->transport_addr.port =
		 htons((unsigned short)endpoint->transport_addr.port);
	}
#endif /* VMS */
	return(endpoint->transport_addr.port);
}

#ifndef NO_FLEXLM_CLIENT_API
API_ENTRY
l_connect_endpoint(job, endpoint, host)
LM_HANDLE *job;		/* Current license job */
COMM_ENDPOINT *endpoint;
char *host;
{
  int ret;
	ret = l_connect_host_or_list(job, endpoint, host,
					(LM_SERVER *)NULL, "", 1);
        return ret;
}
int API_ENTRY
l_connect_host_or_list(job, endpoint, host, master_list, daemon, find_master)
LM_HANDLE *job;		/* Current license job */
COMM_ENDPOINT *endpoint;
char *host;
LM_SERVER *master_list;
char *daemon;
int find_master;
{
	unsigned short port;
	int ret = -1;


	if ((endpoint->transport_addr.port & 0xffff) == 0xffff)
	{
		/* kmaclean 12/17/2002  Bug fix P6872
		 * Added && job->lm_errno != LM_BADHOST
		 * */
		for (port = (unsigned short)LMGRD_PORT_START;
				ret <0 && job->lm_errno != LM_HOSTDOWN &&
				job->lm_errno != LM_BADHOST &&
					port <= job->port_end;
				port++)
		{
			endpoint->transport_addr.port = ntohs(port);
			ret = l_try_connect(job, endpoint, host, master_list,
				daemon, find_master);
		}
	}
	else
		ret = l_try_connect(job, endpoint, host, master_list,
			daemon, find_master);

#ifdef PC
	l_conn_harvest(job);
#endif
	return ret;
}

static
int
l_try_connect(job, endpoint, host, master_list, daemon, find_master)
LM_HANDLE *job;		/* Current license job */
COMM_ENDPOINT *endpoint;
char *host;
LM_SERVER *master_list;
char *daemon;
int find_master;		/* if false, connect to vd for this
				   lmgrd, even if not master */
{
  int done = 0;
  char msg[LM_MSG_LEN+1], resp[LM_MSG_LEN+1];
  char serverhost[MAX_HOSTNAME+1];
  char savhost[MAX_HOSTNAME+1]; /* P4154 */
  LM_SOCKET s;		/* The socket file descriptor */
  int cur_comm_rev = COMM_NUMREV;
  LM_SERVER *orig_master_list = master_list;
  int got_try_another = 0;
  int transport = endpoint->transport;
  static char *project = (char *)-1;
  int retval = -1; /* assume error */
  COMM_ENDPOINT sav_endpoint;
  int this_host = 0;


/*
 *	If this socket is in use, return an error.
 */
	memcpy(&sav_endpoint, endpoint, sizeof(sav_endpoint));
	if (job->daemon->socket != LM_BAD_SOCKET)
	{
		if ((job->daemon->usecount > 0) || (lc_disconn(job, 0) > 0))
		{
			goto exit_connect;
		}
	}
	if (project == (char *)-1)
		project = getenv("LM_PROJECT");	/* overrun checked */
/*
 *	We haven't connected yet.  Do it now.
 */
	job->daemon->server = (LM_SERVER *) NULL;
/*
 *	Fill in the data for the connection message (reset our comm ver/rev)
 */
	job->daemon->our_comm_revision = COMM_NUMREV;
	job->daemon->our_comm_version = COMM_NUMVER;

	if (host)
	{
	}
	else
	{
		l_zcp(serverhost, master_list->name, MAX_HOSTNAME);
		host = serverhost;
		if (master_list->sflags & L_SFLAG_THIS_HOST)
			this_host = 1;
		else
			this_host = 0;
	}
	l_zcp(savhost, host, MAX_HOSTNAME);



/*
 *	UDP note: (5/93)
 *	UDP is only performed by clients to vendor daemons
 *	A TCP connection is always first made to lmgrd, and then it
 *	must get a TRY_ANOTHER msg.  This next connection is the
 *	UDP connection.
 */
	while (!done)
	{
	  int t = transport;

		resp[MSG_CMD] = 0;
		if (transport == LM_UDP &&
			(!getenv("DEBUG_CONNECT_TO_VD") && !got_try_another))	/* overrun checked */
		{
			endpoint->transport = t = LM_TCP;
		}
		l_conn_msg(job, daemon, msg, t, find_master);
#ifdef VMS
		our_daemon = daemon;
#endif
		s = l_basic_conn(job, msg, endpoint, host, resp);
		if (s == LM_BAD_SOCKET)
		{
		    if ((job->lm_errno == CANTCONNECT) ||
			(job->lm_errno == LM_HOSTDOWN) ||
			(job->lm_errno == LM_CANTREAD) ||
			(job->lm_errno == LM_BADHOST))
		    {
			if (got_try_another == 2)
			{
			  char buf[MAX_HOSTNAME + MAX_VENDOR_NAME + 4];

				sprintf(buf, "%s : %s", daemon, host);
				LM_SET_ERROR(job, LM_VENDOR_DOWN, 380, 0, buf,
						LM_ERRMASK_ALL);
			}
			else if (got_try_another == 1) /* P4154 */
			{
				got_try_another = 2;
				l_zcp(serverhost, savhost, MAX_HOSTNAME);
				continue;
			}
			else if (this_host)
			{
				LM_SET_ERRNO(job, LM_NOT_THIS_HOST, 378, 0);
			}
			if (master_list && master_list->next)
			{
				int ret;

				master_list = master_list->next;
				got_try_another = 0;
				l_zcp(serverhost, master_list->name,
						MAX_HOSTNAME);
				if (master_list->sflags & L_SFLAG_THIS_HOST)
				this_host = 1;
				else
				this_host = 0;
				ret = l_get_endpoint(job, master_list,daemon,transport,
						  endpoint );
				if (ret == -1)
				{
					goto exit_connect;
				}
				continue;
			}
			else if (memcmp(&sav_endpoint, endpoint,
					sizeof(sav_endpoint)))
			{
				memcpy(&sav_endpoint, endpoint,
					sizeof(sav_endpoint));
				continue;
			}
			else goto exit_connect;
			}
			else if (resp[MSG_CMD] == LM_TRY_ANOTHER && master_list)
			{
			int ret;
			got_try_another = 1;
			if (resp[MSG_TRY_HOST])
			{
				(void) strncpy(serverhost, &resp[MSG_TRY_HOST],
							MAX_SERVER_NAME);/* LONGNAMES */
				(void) strncpy(&serverhost[MAX_SERVER_NAME],
						&resp[MSG_TRY_HOST2],
						MAX_HOSTNAME - MAX_SERVER_NAME);	/* LONGNAMES */
			}
			ret = setup_transport(resp,
				    job->options->allow_set_transport,
				    job->options->transport_reset,
				    job->options->commtype,
				    endpoint);
			if (ret == -1)	/* Server in process of connecting */
			{
			  int rc;
				if ((master_list = master_list->next) == NULL)
				{
					LM_SET_ERRNO(job, LM_CANTCONNECT, 28, 0);
					break;
				}
				(void) strncpy(serverhost, master_list->name,
							MAX_HOSTNAME);
				rc = l_get_endpoint(job, master_list,
						daemon, transport, endpoint);
				if (rc == -1)
				{
					goto exit_connect;
				}
				if (master_list->sflags & L_SFLAG_THIS_HOST)
					this_host = 1;
				else
					this_host = 0;
			}
			else
				transport = endpoint->transport;
		    }
		    else if ((job->options->flags & LM_OPTFLAG_TRY_OLD_COMM) &&
				orig_master_list)
		    {
			master_list = orig_master_list;		/* Reset it */
			if (!got_try_another)
			{
				int ret;
				(void) strncpy(serverhost, master_list->name,
							MAX_HOSTNAME);
				ret = l_get_endpoint(job, master_list, daemon,
							transport, endpoint);
				if (ret == -1)
				{
					goto exit_connect;
				}
				if (master_list->sflags & L_SFLAG_THIS_HOST)
					this_host = 1;
				else
					this_host = 0;
			}

			cur_comm_rev--;
			/*- If rev4 didn't work, rev3 won't either.
			    we always skip 2 and try 1 immediately */
			if (cur_comm_rev == 3) cur_comm_rev = 1; /* Skip 3, 2 */
			if (cur_comm_rev >= 0)
			{
				job->daemon->our_comm_revision =
								cur_comm_rev;
				l_conn_msg(job, daemon, msg, LM_TCP, 1);
			}
			else
				break;		/* Done - doesn't work */
		    }
		    else
			break;
		}
		else
			done = 1;
	}
	if (resp[MSG_CMD] == LM_OK)
	{
/*
 *	First, set the socket to non-blocking, then send the HEARTBEAT
 *	message, then return the data
 */
#if defined(USE_WINSOCK) || defined(VXWORKS)
		unsigned long non_blocking_mode = 1;
		(void) network_control(s, FIONBIO, &non_blocking_mode );
#else
		network_control(s, F_SETFL, FNDELAY | fcntl(s, F_GETFL, 0));
#endif
		if (!(job->err_info.mask & LM_ERRMASK_FROM_SERVER))
			LM_SET_ERRNO(job, 0, 29, 0); /* rare case where this is ok */
		job->daemon->socket = s;
		l_clr_rcv_queue(job);	/* Clear out l_rcvmsg() receive queue */
		(job->daemon->serialno)++;
		if (resp[MSG_DATA] == 0)
		{
			job->daemon->comm_version =
					job->daemon->our_comm_version;
			job->daemon->comm_revision =
					job->daemon->our_comm_revision;
		}
		else
		{
		  int i = 0;
			l_decode_int(&resp[MSG_OK_COMM_VER],
					&(job->daemon->comm_version));
			l_decode_int(&resp[MSG_OK_COMM_REV],
					&(job->daemon->comm_revision));
			l_decode_int(&resp[MSG_OK_VER], &i);
			job->daemon->ver = i;
			l_decode_int(&resp[MSG_OK_REV], &i);
			job->daemon->rev = i;
			job->daemon->patch = resp[MSG_OK_PATCH];
		}

#if 0
		job->daemon->server = master_list;	/* Remember */
#else
/* find the LM_SERVER struct in the job */
		if (master_list)
		{
		  LM_SERVER_LIST *l;
		  LM_SERVER *s;
			for (l = job->conf_servers; l; l = l->next)
				if (!strcmp(l->s->name, master_list->name) &&
					(l->s->port == master_list->port))
					break;

			if (l)
				job->daemon->server = l->s;
			else /* add it to conf_servers */
			{
			  int cnt;
				/* count the number of servers */
				for (cnt = 0, s = master_list; s;
						s = s->next, cnt++)
					;
				l = (LM_SERVER_LIST *)
					l_malloc(job, sizeof(LM_SERVER_LIST ));
				l->s =
					(LM_SERVER *)l_malloc(job,
						sizeof(LM_SERVER) * cnt);
				for (s = master_list; s; s = s->next)
				{
/*
 *                              Copy only the things that we need,
 *                              and avoid things like hostids...
 */
					memcpy(l->s, s, sizeof(LM_SERVER));
					l->s->idptr = 0; /* we don't need it, and
							  causes malloc/free confusion
							  */
					if (s->next) l->s->next = l->s + 1;
				}
				l->next = job->conf_servers;
				job->conf_servers = l;
			}
		}
		else  job->daemon->server = 0;
#endif

		l_zcp(job->daemon->daemon, daemon, MAX_DAEMON_NAME);


		if (
#ifdef FIFO
			transport != LM_LOCAL &&
#endif /* FIFO */
			*daemon )
		{
/*
 *			If this is a vendor daemon, and we are
 *			checking clocks, do it now.
 */
			if (((
				job->options->capacity >= 0) &&
				((*daemon == '\0') || strcmp(daemon, "lmgrd")))
				|| project)
			{
			  char *rmsg, type;

				(void) memset(msg, '\0', LM_MSG_LEN+1);
				if (job->options->capacity >= 0)
				{
					l_encode_int(&msg[
						MSG_CLOCK_CAPACITY-MSG_DATA],
						(int)job->options->capacity);
				}
				if (project)
					l_zcp(&msg[MSG_CLOCK_PROJECT-MSG_DATA],
						project, MAX_PROJECT_LEN);
				l_sndmsg(job, LM_CLOCKSETTING, msg);
				l_rcvmsg(job, &type, &rmsg);
				if (type == LM_OK)
				{
					LM_SET_ERRNO(job, 0, 117, 0); /* ok */
				}
			}
		}
/*
 *		Start the whole heartbeat thing off (if not lmgrd)
 */
		if ((*daemon != '\0') && strcmp(daemon, "lmgrd") &&
			!(job->options->flags & LM_OPTFLAG_NO_HEARTBEAT))
		{
		  LM_ERR_INFO e;

			memset(&e, 0, sizeof(e));
			l_err_info_cp(job, &e, &job->err_info);

			l_heartbeat(job, (char *)0, msg);
			l_sndmsg(job, LM_HEARTBEAT, msg);
			l_err_info_cp(job, &job->err_info, &e);
			l_free_err_info(&e);
			/* ignore any error here */
		}
		retval = job->daemon->socket;
		goto exit_connect;
	}
	if (resp[MSG_CMD] == LM_NO_SUCH_FEATURE)
	{
		LM_SET_ERROR(job, LM_VENDOR_DOWN, 121, 0, host, LM_ERRMASK_ALL);
	}
	else if (job->lm_errno == 0 && resp[MSG_CMD] != LM_OK)
	{
		LM_SET_ERROR(job, LM_BADCOMM, 122, 0, host, LM_ERRMASK_ALL);
	}
exit_connect:
	return(retval);
}

/*
 *	setup_transport():
 *
 *	Cases:	1) default TCP
 *		2) Options file	specified
 *		3) Environment variable specified
 *		4) allow_set_transport == false, default TCP
 *		5) Set in application
 *		6) Vendor daemon only supports one protocol.
 *		7) Vendor daemon is pre-2.71.
 *
 *	Each higher number supercedes lower numbers.
 *
 *	Values for each case
 *
 *	Case	try_reason 	allow_set_transport 	transport_reset
 *	1	' '		1			0
 *	2	REASON_USER	1			0
 *	3	!=REASON_DAEMON	1			RESET_USER
 *	4	!=REASON_DAEMON 0			!=RESET_APPL
 *	5	!=RESET_DAEMON	ANY			RESET_APPL
 *	6	RESET_DAEMON	ANY			ANY
 *	7	?
 *
 */
static
setup_transport(resp, allow_set_transport, transport_reset, old_transport,
		endpoint)
char *resp;
int allow_set_transport;
int transport_reset;
int old_transport;
COMM_ENDPOINT *endpoint;
{
  int try_udp_port = ntohs( (unsigned short)atoi(&resp[MSG_TRY_UDP_PORT]) );
  int try_tcp_port = ntohs( (unsigned short)atoi(&resp[MSG_TRY_TCP_PORT]) );
#ifdef SUPPORT_IPX
  int try_spx_port = ntohs( (unsigned short)atoi(&resp[MSG_TRY_TCP_PORT]) );
#endif
  char try_reason = resp[MSG_TRY_TRANSPORT_REASON];
  int try_transport = LM_TCP; /* default */
  int *transport = &(endpoint->transport);

	switch (resp[MSG_TRY_TRANSPORT])
	{
	case LM_COMM_UDP: try_transport = LM_UDP; break;
	case LM_COMM_TCP: try_transport = LM_TCP; break;
#ifdef SUPPORT_IPX
	case LM_COMM_SPX: try_transport = LM_SPX; break;
#endif
#ifdef FIFO
	case LM_COMM_LOCAL: try_transport = LM_LOCAL; break;
#endif
	}
	*transport = -1;

	/*case #6*/
	if (try_reason == LM_COMM_REASON_DAEMON)
		*transport = try_transport;


	/*case #5*/
	else if (transport_reset == LM_RESET_BY_APPL)
		*transport = old_transport;

	/*case #4*/
	else if (!allow_set_transport)
		*transport = old_transport;

	/*case #3*/
	else if (transport_reset == LM_RESET_BY_USER)
		*transport = old_transport;

	/*case #2*/
	else if (try_reason == LM_COMM_REASON_USER)
		*transport = try_transport;

	/*case #1*/
	else *transport = old_transport;

#ifdef SUPPORT_IPX
	if (*transport == LM_SPX)
		return endpoint->transport_addr.spx_addr.sa_socket=
								try_spx_port;
#endif

	if (*transport == LM_UDP)
		return endpoint->transport_addr.port = try_udp_port;
	else
		return endpoint->transport_addr.port = try_tcp_port;
}


#if defined(_WINDOWS) || defined(WINNT) || defined(OS2)
/*
 *      l_get_port() - Get the port number for a connection.
 *
 *      Note: This routine used to be in v4.0.  After the IPX changes,
 *            it is replaced by l_get_endpoint().  This routine is here
 *            to provide DLL backward compatibility for v4.0 vendor daemon,
 *            which uses l_get_port() in the DLL.
 *            UNIX and VMS should not use or define l_get_port() after v4.0.
 *            Instead, l_get_endpoint() should be used.
 */

API_ENTRY
l_get_port(job, master_list, daemon, transport)
LM_HANDLE *job;         /* Current license job */
LM_SERVER *master_list;
char *daemon;
int transport;
{
  COMM_ENDPOINT endpoint;

  endpoint.transport = transport;
  return(l_get_endpoint(job, master_list, daemon, transport, &endpoint));
}

#endif /* defined(_WINDOWS) || defined(WINNT) */

int API_ENTRY
l_connect_by_conf(job, conf)
LM_HANDLE *job;
CONFIG *conf;
{

  LM_SERVER *server;

#ifndef PC
                errno = 0;
#else
                l__WSASetLastError(0);
#endif

	if (!conf || !*conf->code)
	{
		if (!job->line)
		{
			l_init_file(job);
/*
 *			only fail if job->line isn't initialized
 */
			if (!job->line) return job->lm_errno;
		}
		/* P3315 */
		for (conf = job->line; conf; conf = conf->next)
			if (l_keyword_eq(job, conf->daemon, job->vendor) &&
				(conf->server || job->daemon->server))
				break;
		if (!conf)
		{
			LM_SET_ERRNO(job, LM_NOFEATURE, 417, 0);
			return (LM_NOFEATURE);
		}
	}
	if (conf->server)
		server = conf->server;
	else if (job->daemon->server)
		server = job->daemon->server;
	else
	{
		LM_SET_ERRNO(job, LM_NOSERVER, 278, errno);
		return (LM_NOSERVER);
	}
	if (l_connect(job, server, conf->daemon,
						job->daemon->commtype) == -1)
	{
		if (job->lm_errno == 0)
		{
			LM_SET_ERRNO(job, LM_NOSERVER, 241, errno);
			return LM_NOSERVER;
		}
		else
			return job->lm_errno;
	}
	if (job->daemon->socket == LM_BAD_SOCKET)
		return job->lm_errno;
	return 0;
}

/*
 *	l_connect_by_conf_for_vsend, if necessary -- return 0 success, non-zero, lm_errno
 * This routime is present only to be used by lc_vsend(). This routine is identical in
 * functionality to l_connect_by_conf() with the exception that the routine will
 * loop though all items in the conf struct. This insures that all server references
 * in a FLEXLM_LICENSE_FILE registry entry is contacted as opposed to simply contacting
 * the first item in a list.
 */
int API_ENTRY
l_connect_by_conf_for_vsend(LM_HANDLE *job, CONFIG *conf)
{

  LM_SERVER *server;
#ifndef PC
                errno = 0;
#else
                l__WSASetLastError(0);
#endif

	if (!conf || !*conf->code)
	{
		if (!job->line)
		{
			l_init_file(job);
/*
 *			only fail if job->line isn't initialized
 */
			if (!job->line) return job->lm_errno;
		}
		/* P3315 */
		for (conf = job->line; conf; conf = conf->next)
		{
			if (l_keyword_eq(job, conf->daemon, job->vendor) && (conf->server || job->daemon->server))
			{
				/* try to connect */
				/* P6576 */
				job->lm_errno = LM_NOERROR;	/* reset errno for sanity sake */
				if (conf->server)
					server = conf->server;
				else if (job->daemon->server)
					server = job->daemon->server;
				else
				{
					LM_SET_ERRNO(job, LM_NOSERVER, 278, errno);
					return (LM_NOSERVER);
				}
				if (l_connect(job, server, conf->daemon, job->daemon->commtype) != -1)
					break;
			}
		}	/* end for loop */

		if (!conf)
		{
			LM_SET_ERRNO(job, LM_NOFEATURE, 417, 0);
			return (LM_NOFEATURE);
		}
	}
	/* catch any connect errors */
	/*
	if (job->lm_errno != 0)
			return job->lm_errno;
	*/

	if (job->daemon->socket == LM_BAD_SOCKET)
		return job->lm_errno;

	return 0;
}
/*
 *	same_servers
 *	return 1 if same, else 0
 */
static
int
same_servers (s1, s2)
LM_SERVER *s1;
LM_SERVER *s2;
{
	LM_SERVER *sp1, *sp2;

	/*-
	 *	bug P1268:  In the case of redundant servers, when you've
	 *	already connected, one of these lists may be only 2 servers.
	 *	2 servers matching is ok.  This is not a security issue.
	 */
	for (sp1 = s1, sp2 = s2; sp1 && sp2; sp1 = sp1->next, sp2 = sp2->next)
	{
		/* kmaclean 12/20/02
		 * added checks for name == NULL. */
		if (sp1->name == NULL || sp2->name == NULL ||
				strcmp(sp1->name, sp2->name) || sp1->port != sp2->port)
			break;

	}
	if ((sp1 || sp2)  && s1 != NULL)
	{
		for (sp1 = s1->next, sp2 = s2; sp1 && sp2;
					sp1 = sp1->next, sp2 = sp2->next)
		{
			if (sp1->name == NULL || sp2->name == NULL ||
					strcmp(sp1->name, sp2->name) || sp1->port != sp2->port)
				break;
		}
	}
	if ((sp1 || sp2) && s2)
	{
		for (sp1 = s1, sp2 = s2->next; sp1 && sp2;
					sp1 = sp1->next, sp2 = sp2->next)
		{
			if (sp1->name == NULL || sp2->name == NULL ||
					strcmp(sp1->name, sp2->name) ||
					sp1->port != sp2->port)
				break;
		}
	}
	if (sp1 || sp2)
		return 0;
	else
		return 1;
}

#endif /* NO_FLEXLM_CLIENT_API */
