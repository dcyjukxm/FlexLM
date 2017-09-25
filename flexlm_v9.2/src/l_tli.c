/******************************************************************************

	    COPYRIGHT (c) 1993, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_tli.c,v 1.6 2003/01/13 22:41:56 kmaclean Exp $
 *
 *	Function: flm_accept(), 
 *		  flm_bind(), 
 *		  flm_connect(), 
 *		  flm_getservbyname(),
 *		  flm_getsockname(),
 *		  flm_listen(), 
 *		  flm_recv(),
 *		  flm_recvfrom(),
 *		  flm_setsockopt(),
 *		  flm_sendto(),
 *		  flm_socket(), 
 *		  flm_shutdown(),
 *		  flm_net_close(), 
 *		  flm_net_read(), 
 *		  flm_net_write()
 *
 *	Description: 	tli interface routines for socket library calls.
 *
 *	Parameters:
 *
 *	Return:
 *
 *	M. Christiano
 *	1/17/93
 *
 *	Last changed:  3/10/98
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <errno.h>

#ifdef SUNOS5
#include <tiuser.h>
#include <stropts.h>
#include <fcntl.h>
#include <stdio.h>
extern int t_errno;
#include <errno.h>

#define flm_accept accept
#define flm_bind bind
#define flm_connect connect
#define flm_getservbyname getservbyname
#define flm_getsockname getsockname
#define flm_listen listen
#define flm_recv recv
#define flm_recvfrom recvfrom
#define flm_sendto sendto
#define flm_setsockopt setsockopt
#define flm_shutdown shutdown
#define flm_socket socket

static int debugon = 0;
static int infoon = 0;
#ifdef DEBUG
#undef DEBUG
#endif
#define INFO(x) if (infoon) { _logtime(); printf x ; }
#define DEBUG(x) if (debugon) { _logtime(); printf x ; }
#define TDEBUG(x) if (debugon) { _logtime(); t_error x ; }


/*-
 * 	t_call struct used by listen/accept
 *
 * TODO: This assumes we are only going to listen on one fd!
 */

static struct t_call *call;
static struct sockaddr sockname;
static int socknamelen = 0;

static
_logtime()
{
  time_t ltime;
#ifdef THREAD_SAFE_TIME
	struct tm *localtime_r(const time_t *clock, struct tm * res), *t;
	struct tm tst;
#else /* !THREAD_SAFE_TIME */
	struct tm *localtime(), *t;
#endif

	ltime = time(0);
#ifdef THREAD_SAFE_TIME
	localtime_r(&ltime, &tst);
	t = &tst;
#else /* !THREAD_SAFE_TIME */
	t = localtime(&ltime);
#endif
	printf("%2d/%02d %2d:%02d:%02d (TLI_DEBUG) ", t->tm_mon+1, t->tm_mday,
				t->tm_hour, t->tm_min, t->tm_sec);
}

/*---------------------------------------------------------------------------*/
/*
 *	socket library emulation
 */

/*
 * TODO: Fill in name/namelen
 *	 Get device to open from table instead of hardcoding "/dev/tcp"
 */

flm_accept(s, name, namelen)
int s;
struct sockaddr *name;
int *namelen;
{
  int status;
  int fd;

	INFO(("accept()\n"));
	status = t_listen(s, call);
	if (status != 0)
	{
		/* if (errno != EINTR) */ TDEBUG(("t_listen() in accept"));
		return(-1);
	}
	fd = t_open("/dev/tcp", O_RDWR, (struct t_info *) NULL);
	if (fd == -1)
	{
		TDEBUG(("t_open() in accept"));
		return(-1);
	}
	status = t_bind(fd, (struct t_bind *) NULL, (struct t_bind *) NULL);
	if (status == -1)
	{
		TDEBUG(("t_bind() in accept"));
		return(-1);
	}
	status = t_accept(s, fd, call);
	if (status == -1)
	{
		TDEBUG(("t_accept() in accept"));
		if (t_errno == TLOOK)	/* Must be disconnect */
		{
			if (t_rcvdis(s, (struct t_discon *) NULL) == -1)
			{
				TDEBUG(("t_rcvdis() in accept"));
				return(-1);
			}
			if (t_close(fd) == -1)
			{
				TDEBUG(("t_close() in accept"));
				return(-1);
			}
			return(-1);
		}
		return(-1);
	}
	*namelen = call->addr.len;
	(void) memcpy(name, call->addr.buf, *namelen);
	status = ioctl(fd, I_PUSH, "tirdwr");
	if (status)
	{
		DEBUG(("PUSH ioctl in accept"));
		return(-1);
	}
	return(fd);
}

flm_bind(s, name, namelen)
int s;				/* Socket FD number */
struct sockaddr *name;
int namelen;
{
  struct t_bind *tbind;
  int status;

	INFO(("bind()\n"));
	tbind = (struct t_bind *) t_alloc(s, T_BIND, T_ALL);
	if (tbind ==  (struct t_bind *) NULL)
	{
		TDEBUG(("t_alloc() in bind"));
		return(-1);
	}
	tbind->qlen = 10;
	tbind->addr.len = namelen;
	(void) memcpy(tbind->addr.buf, name, namelen);
	status = t_bind(s, tbind, tbind);
	if (status < 0)
	{
		TDEBUG(("t_bind() in bind"));
		return(status);
	}
/*
 *	Save the address for subsequent getsockname
 */
	socknamelen = tbind->addr.len;
	if (socknamelen > sizeof (struct sockaddr))
	{
		DEBUG(("Oops: t_bind returns address size %d, sockaddr size %d\n",
					socknamelen, sizeof(struct sockaddr)));
		socknamelen = sizeof(struct sockaddr);
	}
	(void) memcpy(&sockname, tbind->addr.buf, socknamelen);
	return(status);
}

flm_connect(s, name, namelen)
int s;
struct sockaddr *name;
int namelen;
{
  struct t_call *sndcall;
  int status;

	INFO(("connect(%d, ..., %d)\n", s, namelen));
	status = t_bind(s, (struct t_bind *) NULL, (struct t_bind *) NULL);
	if (status == -1) 
	{
		TDEBUG(("t_bind() in connect"));
		return(status);
	}
	sndcall = (struct t_call *) t_alloc(s, T_CALL, T_ADDR);
	if (sndcall == (struct t_call *) NULL) 
	{
		TDEBUG(("t_alloc() in connect"));
		return(-1);
	}
	sndcall->addr.len = namelen;
	(void) memcpy(sndcall->addr.buf, name, namelen);
	status = t_connect(s, sndcall, (struct t_call *) NULL);
	if (status == -1) 
	{
		TDEBUG(("t_connect() in connect"));
		if (t_errno == TLOOK)
		{
		  int look;

			look = t_look(s);
			switch (look)
			{
			  case T_LISTEN:
				DEBUG(("T_LISTEN received\n"));
				break;
			  case T_CONNECT:
				DEBUG(("T_CONNECT received\n"));
				break;
			  case T_DATA:
				DEBUG(("T_DATA received\n"));
				break;
			  case T_EXDATA:
				DEBUG(("T_EXDATA received\n"));
				break;
			  case T_DISCONNECT:
				DEBUG(("T_DISCONNECT received\n"));
				break;
			  case T_ORDREL:
				DEBUG(("T_ORDREL received\n"));
				break;
			  case T_UDERR:
				DEBUG(("T_UDERR received\n"));
				break;
			  default:
				DEBUG(("Unknown t_look() code %d received\n",
								look));
				break;
			}
		}
		return(status);
	}
	status = ioctl(s, I_PUSH, "tirdwr");
	if (status)
	{
		return(status);
	}
	return(status);
}

struct servent *
flm_getservbyname(name, proto)
char *name;
char *proto;
{
	return((struct servent *) NULL);
}

flm_getsockname(s, name, namelen)
int s;
struct sockaddr *name;
int *namelen;
{
	*namelen = socknamelen;
	if (socknamelen > 0) (void) memcpy(name, &sockname, socknamelen);
	return(socknamelen != 0);
}

flm_listen(s, backlog)
int s;
int backlog;
{
	INFO(("listen()\n"));
	call = (struct t_call *) t_alloc(s, T_CALL, T_ALL);
	if (call == (struct t_call *) NULL)
	{
		TDEBUG(("t_alloc() in listen"));
	}
	return(call == (struct t_call *) NULL);
}

#ifndef SUN64
flm_recv(s, msg, len, flags)
int s;
char *msg;
int len;
int flags;
{
	INFO(("recv(%d, %x ..., %d, %d)\n", s, msg[0], len, flags));
	DEBUG(("recv() not implemented\n"));
	return(-1);
}

/*
 *	recvfrom() - Not used in tcp/ip
 */

flm_recvfrom(s, msg, len, flags, from, fromlen)
int s;
char *msg;
int len;
int flags;
struct sockaddr *from;
int *fromlen;
{
	INFO(("recvfrom()\n"));
	DEBUG(("recvfrom() not implemented\n"));
	return(0);
}

/*
 *	sendto() - Not used in tcp/ip
 */
flm_sendto(s, msg, len, flags, to, tolen)
int s;
const char *msg;
int len;
int flags;
struct sockaddr *to;
int tolen;
{
	INFO(("sendto()\n"));
	DEBUG(("sendto() not implemented\n"));
	return(0);
}
#endif /* SUN64 */

flm_setsockopt(s, level, opt, optval, optlen)
int s;
int level;
int opt;
const char *optval;
int optlen;
{
	INFO(("setsockopt(%d, %d, %d, %x, %d)\n", s, level, opt, optval,
								optlen));
	DEBUG(("setsockopt(, , %d, ,) not implemented\n", opt));
	return(0);
}

flm_shutdown(s, how)
int s;
int how;
{
  int i;

	INFO(("shutdown(%d)\n", s));
	i = t_sndrel(s);
	if (i)
	{
		TDEBUG(("t_sndrel in shutdown"));
	}
/*
 *	Close it here, since we won't get a chance to intercept the
 *	real close call.
 */
	i = t_close(s);
	if (i)
	{
		TDEBUG(("t_close() in net_close"));
	}
	return(i);
}


/*
 *	socket() emulation - just open the protocol driver.
 *	let either bind() or connect() do the actual work.
 */
flm_socket(af, type, protocol)
int af;
int type;
int protocol;
{
  short int d;
  char *device;

	infoon = getenv("FLEXLM_TLI_INFO");
	debugon = getenv("FLEXLM_TLI_DEBUG");
	INFO(("socket()\n"));
	if (type == SOCK_STREAM) device = "/dev/tcp";
	else if (type == SOCK_DGRAM) device = "/dev/udp";
	else device = "/dev/null";
	d = t_open(device, O_RDWR, (struct t_info *) NULL);
	return(d);
}

#endif
