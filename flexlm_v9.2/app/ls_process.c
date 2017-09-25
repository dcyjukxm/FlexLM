/****************************************************************************

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
 *	Module: $Id: ls_process.c,v 1.29 2003/06/11 00:29:44 sluu Exp $
 *
 *	Function: ls_process(ready_mask, nrdy, select_mask, tcp_s, spx_s,
 *						masterconnectionsame, num_reserve)
 *
 *	Description: Processes waiting descriptors for a server process.
 *
 *	Parameters:	(SELECT_MASK) ready_mask - Descriptors that can be read
 *			(int) nrdy - The number of ready descriptors.
 *			(SELECT_MASK) select_mask - The "master" select mask.
 *			(LM_SOCKET) tcp_s - The tcp socket to accept connections
 *			(LM_SOCKET) spx_s - The spx socket to accept connections
 *			(char *) masterconnectionsame - Host where the master is running
 *			(int) num_reserve - Number of descriptors required
 *						for new server connections.
 *
 *	Return:		(SELECT_MASK) select_mask - Updated for new/broken
 *								connections.
 *
 *	M. Christiano
 *	2/27/88
 *
 *	Last changed:  11/12/98
 *
 */

#include <errno.h>
#include <fcntl.h>
#include "lmachdep.h"
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
extern char *sys_errlist[];
#endif
#include "lmclient.h"
#include "l_prot.h"
#include "lmselect.h"
#include "lsserver.h"
#include "lsfeatur.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "ls_comm.h"		/* Server-server comm protocol */
#include "ls_aprot.h"
#include "l_timers.h"
#include "flexevent.h"
#ifdef LM_GENERIC_VD
#include "lg_genvd.h"
#endif /* GENERIC */
#ifndef NO_UIO_H
#include <sys/uio.h>
#endif
#include <sys/file.h>
#ifdef ANSI
#include <stdlib.h>
#ifdef UNIX
#include <unistd.h>
#endif /* UNIX */
#endif /* ANSI */

#ifdef USE_WINSOCK
#include <winsock.h>
#else
#include <sys/socket.h>
#endif



/*-
 *	Use FORKDEBUG for apollo, since we can open more file
 *	descriptors than we can select on!!!
 */
#if defined (RELEASE_VERSION)
#define DEBUG(x)
#ifdef apollo
#endif
#else	/* RELEASE_VERSION */
#define DEBUG(x) if (debug) printf x
static long debug = -1;
static int first = 1;
#endif	/* RELEASE_VERSION */
#define MAXBADCHILD 5		/* Allow 5 bad reads from the child */



long max_connections = 0;
extern long ls_connections;

#ifdef LM_GENERIC_VD
GEN_FEATURE_LIST *gen_fl;
#endif /* GENERIC */

static int lasterrno = -1;
#define ERR_REPORT_CNT 500
static int howmanyconnectionsext = ERR_REPORT_CNT;
static int howmany_errno = 0;
#ifdef SUPPORT_UDP
static void ls_process_udp lm_args((SELECT_MASK select_mask));
#endif
static void ls_process_known_tcp lm_args((CLIENT_ADDR *ca,
			SELECT_MASK ready_mask, SELECT_MASK select_mask));
static void ls_process_client_msg lm_args((CLIENT_DATA *client, char *msg,
		int len, SELECT_MASK select_mask));
static void ls_bad_checksum lm_args(( char *msg, CLIENT_DATA *client));
static void ls_old_serv_vers lm_args((CLIENT_DATA *));
#if SCOMM_MSGSIZE >= LM_MSG_LEN
#define COMM_MSGLEN SCOMM_MSGSIZE+1
#else
#define COMM_MSGLEN LM_MSG_LEN;
#endif
extern FEATURE_LIST *ls_flist;
extern 	int ls_msg_cnt;
extern int ls_msg_cnt_this_minute;
extern void (*ls_child_exited)();

void
ls_process(ready_mask, nrdy, select_mask, tcp_s, spx_s, masterconnectionsame, num_reserve)
SELECT_MASK ready_mask;
int nrdy;
SELECT_MASK select_mask;
LM_SOCKET *tcp_s;
LM_SOCKET *spx_s;
char *masterconnectionsame;
int num_reserve;
{
  LM_SOCKET fd;
  int i;
  unsigned int fromlen;
  CLIENT_ADDR new_tcp_ca, old_tcp_ca;
  CLIENT_DATA *client;
  struct sockaddr_in from;

#ifndef RELEASE_VERSION

#if !defined( USE_WINSOCK ) || defined( OS2 )
	if (debug == -1) debug = (long)l_real_getenv("LS_PROCESS");
#endif

	if (first)
	{
	  char *str;
	  int num;

		first = 0;
#ifdef UNIX
		max_connections = lm_nofile - (4 + lm_job->lm_numlf);
#else
#ifdef WINNT
		max_connections= 4096 ;
#endif
#ifdef NLM
		max_connections = 512 ;
#endif
#endif
		str = l_real_getenv("FLEXLM_FORKDEBUG");
		if (str != (char *) NULL)
		{
			if (!strcmp(str, "NO")) max_connections = 0;
			else
			{
				num = atoi(str);
				if (num > 0) max_connections = num;
			}
		}
	}
#endif	/* RELEASE_VERSION */


/*
 *	Set up here
 */
	fd = 0;
	(void)memset((char *)&new_tcp_ca, '\0', sizeof(new_tcp_ca));
	(void)memset((char *)&old_tcp_ca, '\0', sizeof(old_tcp_ca));
	new_tcp_ca.is_fd = 1;
	new_tcp_ca.transport = LM_TCP;
	new_tcp_ca.addr.fd = LM_BAD_SOCKET;
	old_tcp_ca.is_fd = 1;
	old_tcp_ca.transport = LM_TCP;
	old_tcp_ca.addr.fd = LM_BAD_SOCKET;

	if (nrdy <= 0)
	{
#ifdef WINNT
                if (net_errno == WSAENOTSOCK) return;
#endif /* WINNT */
				/*- ls_process.firstpart went here 11/7/88 */
		if (net_errno != NET_EINTR)
		{
			if (net_errno == NET_EBADF)
			{

				while ((i = ls_findbad(select_mask)) >= 0)
				{
				        ls_badserv(i);
				}
			}
			else if (net_errno != lasterrno)
			{
				lasterrno = net_errno;
				howmany_errno = 0;
				howmanyconnectionsext = ERR_REPORT_CNT;
				if (net_errno != NET_EAGAIN)
				{
					LOG(("select:  error 0x%x", net_errno));
#ifndef USE_WINSOCK
					_LOG((" (select mask: %x %x)\n",
					       select_mask[0], select_mask[1]));
#endif
                                        _LOG(("\n"));
					LOG_INFO((ERROR,
						    "An error in the select \
						   system call was detected."));
				}
			}
			else if (++howmany_errno > howmanyconnectionsext)
			{
				LOG((lmtext("select (%d errors): %s\n"),
					howmanyconnectionsext, SYS_ERRLIST(errno)));
				howmany_errno = 0;
				if (howmanyconnectionsext < 10000)
					howmanyconnectionsext *= 2;
			}
			else if (net_errno == NET_EAGAIN)
			{
				ls_pause(10);	/* Wait 10 msec. */
			}
		}
	}
	else
	{
	    while (ANY_SET((int *)ready_mask))
	    {
		ls_msg_cnt++;
		ls_msg_cnt_this_minute++;

		while ((IS_SET(ready_mask, fd)) == 0)
			fd++;
#ifndef PC
		DEBUG (("ls_process: ready_mask: 0x%x, nrdy: %d fd: %d\n",
				*ready_mask, nrdy, fd));
#endif

		MASK_CLEAR(ready_mask, fd);	/* Clear this one */
		if ((fd == *tcp_s) || (fd == *spx_s))
		{
			if (fd == *spx_s) new_tcp_ca.transport = LM_SPX;

			/*
			*		    We got a new tcp connection pending... process it
			*/
			fromlen = sizeof(from);

			{
				if (fd == *spx_s)
					new_tcp_ca.addr.fd = accept(*spx_s,
					(struct sockaddr *)&from, (int *)&fromlen);
				else
					new_tcp_ca.addr.fd = accept(*tcp_s,
					(struct sockaddr *)&from, (int *)&fromlen);
			}
			if ((new_tcp_ca.addr.fd == LM_BAD_SOCKET) ||
				((max_connections > 0) &&
			  (ls_connections >= (max_connections - num_reserve))))
			{
				if (new_tcp_ca.addr.fd != LM_BAD_SOCKET)
				{
					LM_SET_ERRNO(lm_job, LM_SERVER_MAXED_OUT, 457, 0);
					DLOG(("Maxed out!???"));
				}
#ifndef VMS
				LOG(("This server can handle no more concurrent clients\n"));
				LOG(("since it is out of file descriptors.\n"));
				LOG(("We recommend splitting your licenses among more\n"));
				LOG(("servers, and using a license-file list, so client\n"));
				LOG(("clients can checkout from more servers.\n"));
#ifdef SUNOS4
				LOG(("This system is SunOS4, and has a particularly low\n"));
				LOG(("number of file descriptors available (256).\n"));
				LOG(("Moving to Solaris, or other platforms, will \n"));
				LOG(("significantly increase the number of clients that\n"));
				LOG(("can be supported on one system.\n"));
#endif
				LOG(("You can also use UDP to get around this limitation,\n"));
				LOG(("although more license-servers will improve response.\n"));
				LOG(("Please see www.macrovision.com or your vendor.\n"));
				LOG(("for more information.\n"));
#else
				LOG((
				"Set logical name LM_VMS_MAX_LINKS to a larger number\n"));
#endif /* VMS */
			}
			if (fd != LM_BAD_SOCKET)
			{
/*
 *				Set the new socket to non-blocking
 */
#ifdef USE_WINSOCK
				u_long non_blocking_mode = 1;
				network_control(new_tcp_ca.addr.fd, FIONBIO,
					&non_blocking_mode );
#else
				network_control(new_tcp_ca.addr.fd, F_SETFL,
						FNDELAY | network_control(
				new_tcp_ca.addr.fd, F_GETFL, 0));
#endif /* USE_WINSOCK */
#ifndef RELEASE_VERSION
				if (l_real_getenv("LS_DOWN_DEBUG"))
					DLOG(("accept fd %d port %d\n",
					new_tcp_ca.addr.fd, ntohs(from.sin_port)));
#endif
				client = ls_c_init(&new_tcp_ca, -1);
#ifdef VMS
				client->inet_addr[0] =
				client->inet_addr[1] =
				client->inet_addr[2] =
				client->inet_addr[3] = 0;
#else	/* VMS */
				if (from.sin_family == AF_INET)
				{
				  long addr;
					struct sockaddr_in *x = (struct sockaddr_in *) &from;

					addr = ntohl(x->sin_addr.s_addr);
					client->inet_addr[0] =
						(short) ((addr >> 24) & 0xff);
					client->inet_addr[1] =
						(short) ((addr >> 16) & 0xff);
					client->inet_addr[2] =
						(short) ((addr >> 8) & 0xff);
					client->inet_addr[3] =
						(short) ((addr) & 0xff);
				}
#endif	/* !VMS */
				MASK_SET(select_mask, new_tcp_ca.addr.fd);
			}
		}
#ifdef SUPPORT_UDP
		else if (fd == ls_udp_s) /*Got a UDP connection -- process it*/
		{
			ls_process_udp(select_mask);
		}
#endif /* SUPPORT_UDP */

		else
		{

/*
 *			This is a message from a known tcp or fifo client
 */
			old_tcp_ca.addr.fd = fd;
			ls_process_known_tcp(&old_tcp_ca,
				ready_mask, select_mask);
		}
	    } /* while (ANY_SET(ready_mask))  */
	}
}

#ifdef SUPPORT_UDP

static void
ls_process_udp(select_mask)
SELECT_MASK select_mask;
{
  char resp[COMM_MSGLEN];
  CLIENT_DATA *client;
  CLIENT_ADDR new_udp_ca;
  int bytes_read;

	(void)memset((char *)&new_udp_ca, '\0', sizeof(new_udp_ca));
	new_udp_ca.is_fd = 0; new_udp_ca.transport = LM_UDP;
	memset(resp, 0, COMM_MSGLEN);

/*
 *	Unlike TCP -- we always read the max size, since we only
 *	get distinct UDP messages -- this way we avoid reading the
 *	socket again!
 */
	if ((bytes_read =
		ls_udp_read(resp, sizeof(resp), &new_udp_ca)) < 0)
	{
		LOG((lmtext("Error reading from UDP socket: errno %d\n"), net_errno));
		return;
	}
	if (!(client = ls_lookup_client(&new_udp_ca)))
	{
/*
 *	Must be a new UDP client:
 */
		client = ls_c_init(&new_udp_ca, -1);
		if (new_udp_ca.addr.sin.sin_family == AF_INET)
		{
		  unsigned int addr=
		      (unsigned int) ntohl(new_udp_ca.addr.sin.sin_addr.s_addr);

			client->inet_addr[0] = (short) ((addr >> 24) & 0xff);
			client->inet_addr[1] = (short) ((addr >> 16) & 0xff);
			client->inet_addr[2] = (short) ((addr >> 8) & 0xff);
			client->inet_addr[3] = (short) ((addr) & 0xff);
		}
		else
		{
			DLOG(("Client Address type not AF_INET (%d)\n",
					new_udp_ca.addr.sin.sin_family));
		}
	}
	ls_process_client_msg(client, resp, bytes_read, select_mask);
}

#endif	/* SUPPORT_UDP */

static void
ls_process_known_tcp(ca, ready_mask, select_mask)
CLIENT_ADDR *ca;
SELECT_MASK ready_mask;
SELECT_MASK select_mask;
{

  char resp[COMM_MSGLEN];
  int size, len;
  CLIENT_DATA *client;

	if (client = ls_lookup_client(ca))
	{
		memset(resp, 0, COMM_MSGLEN);
		if (client->comm_revision == -1 ||
		    client->comm_revision > COMM_NUMREV)
						size = LM_MSG_LEN_MIN;
		else size = l_msg_size(client->comm_revision);

	} else size = COMM_MSGLEN;
/*
 *	If we don't know the comm revision of the client,
 *	first read the minimum size message any compatible
 *	client could send.  If it is a HELLO message, we
 *	know how much more to read.
 */
	len = l_read_timeout(ca->addr.fd, resp, size, LS_READ_TIMEOUT);
	if (!client && len)
	{
		DLOG(("Dropped message -- can't find client\n"));
		DLOG(("len is %d msg is %x%x%x%x\n", len,
					resp[0],
					resp[1],
					resp[2],
					resp[3]));
		return;
	}

	if (len <= 0)
	{
		ls_closeout_this_tcp_client(client, len, 1);
	}
	else
	{
		ls_process_client_msg(client, resp, len, select_mask);
	}
}
static void
ls_process_client_msg(
	CLIENT_DATA *	client,
	char *			msg,
	int				len,
	SELECT_MASK		select_mask)
{
  extern int ls_flexlmd;
  extern long ls_currtime;

	client->lastcomm = ls_currtime;
	if ((client->comm_revision < 0) &&
	    (client->encryption == 0) &&
	    (len > 0) &&
	    ( (msg[MSG_CMD] == LM_HELLO) || (msg[MSG_CMD] == LM_HELLO_THIS_VD)
			     || (msg[MSG_CMD] == LM_SHELLO)) )
	{
	  int r = msg[MSG_HEL_VER+1] - '0';
	  int size2;

		if (r < 0 || r > COMM_NUMREV)
		{
	      DLOG(("WARNING: Invalid client communications revision: %d\n",
							r));
	      DLOG(("%c message, COMM revision: %d (%c), version: %d (%c)\n",
				   msg[MSG_CMD],
				   msg[MSG_HEL_VER+1],
				   msg[MSG_HEL_VER+1],
				   msg[MSG_HEL_VER],
				   msg[MSG_HEL_VER]));
		  if(l_flexEventLogIsEnabled())
		  {
			  /*
			   *	sluu: I know these names aren't descriptive, just need to get this to work first...
			   */
			  char		szOne[20] = {'\0'};
			  char		szTwo[20] = {'\0'};
			  char		szThree[20] = {'\0'};
			  char		szFour[20] = {'\0'};
			  char		szFive[20] = {'\0'};
			  char		szSix[20] = {'\0'};

			  char *	ppszInStr[6] = {NULL};

			  sprintf(szOne, "%d", r);
			  sprintf(szTwo, "%c", msg[MSG_CMD]);
			  sprintf(szThree, "%d", msg[MSG_HEL_VER+1]);
			  sprintf(szFour, "%c", msg[MSG_HEL_VER+1]);
			  sprintf(szFive, "%d", msg[MSG_HEL_VER]);
			  sprintf(szSix, "%c", msg[MSG_HEL_VER]);
			  

			  ppszInStr[0] = szOne;
			  ppszInStr[1] = szTwo;
			  ppszInStr[2] = szThree;
			  ppszInStr[3] = szFour;
			  ppszInStr[4] = szFive;
			  ppszInStr[5] = szSix;
			  l_flexEventLogWrite(lm_job,
									FLEXEVENT_ERROR,
									CAT_FLEXLM_LMGRD_HEALTH,
									MSG_FLEXLM_CLIENT_COMM_VERSION_ERROR,
									6,
									ppszInStr,
									0,
									NULL);
		  }
			r = COMM_NUMREV + 1;	/* Point @min */
		}
		size2 = l_msg_size(r);
		client->comm_revision = r;
		if (r > COMM_NUMREV)
		{
			LOG(("Client communications revision is %d (%s %s %s\n",
					r, client->name, client->display,
							client->node));
			ls_old_serv_vers(client);
			return;
		}
		client->comm_version = msg[MSG_HEL_VER]-'0';
		if (client->addr.is_fd && size2 > len)
			len += l_read_timeout(client->addr.addr.fd, &msg[len],
				    size2 - len, LS_READ_TIMEOUT);
		len = size2;	/* Save for ls_msg_dump() */
	}
	if (client->encryption)
		(void) l_str_dcrypt(msg, LM_MSG_LEN,
			client->encryption, client->flexlm_ver >= 5);

	ls_msg_dump("RECEIVED", msg, &client->addr, len);
	if (!l_cksum_ok(msg, client->comm_revision, client->addr.transport))
	{

/*		For UDP clients only -- we may have missed the handshake
 *		message -- so we uncrypt it and see if we can read it
 */
		if (client->addr.transport == LM_UDP && client->encryption)
		{
			(void) l_str_dcrypt(msg, LM_MSG_LEN,
				client->encryption, client->flexlm_ver >= 5);;
			if (!l_cksum_ok(msg, client->comm_revision,
							client->addr.transport))
			{
				ls_bad_checksum(msg, client);
				return;
			}
		}
		else
		{
			ls_bad_checksum(msg, client);
			return;
		}
	}

#ifdef SUPPORT_UDP
	if (client->addr.transport == LM_UDP && !l_read_sernum(msg,
							&client->udp_sernum))
	{
		LOG((lmtext("Warning: UDP communications error, resending...\n")));
		ls_resend_last_resp(client);
		return;
	}
#endif
#ifdef LM_GENERIC_VD
	if (ls_flexlmd)
	{
		ls_flist = lg_flist(client, msg);
		if (ls_flist && (ls_flist == gen_fl->fl))
			return; /* we have to wait for seeds from this client */
	}
#endif
	ls_docmd(msg, client, select_mask);	/* LONGNAMES */
} /* if */

static
void
ls_old_serv_vers(client)
CLIENT_DATA *client;
{
  char tmpmsg[LM_MSG_LEN+1];

	memset(tmpmsg, 0, sizeof(tmpmsg));

	l_encode_int(&tmpmsg[MSG_DATA], LM_SERVOLDVER);
	ls_client_send(client, LM_WHAT, tmpmsg);
}
static void
ls_bad_checksum(msg, client)
char *msg;
CLIENT_DATA *client;
{
  char tmpmsg[LM_MSG_LEN+1];

	memset(tmpmsg, 0, sizeof(tmpmsg));
	switch (client->addr.transport)
	{
	case LM_TCP: /* fall through */
#ifdef FIFO
	case LM_LOCAL:
#endif
		DLOG(("checksum is %u, s/b: %u\n",
		     (unsigned char) msg[MSG_CHECKSUM],
		     l_get_cksum(msg, client->comm_revision,
		    client->addr.transport)));
		break;
	case LM_UDP:
		break;
	default:
		DLOG(("unknown commtype %s line %d\n", __FILE__, __LINE__));
	}

	l_encode_int(&tmpmsg[MSG_DATA], SERVBADCHECKSUM);
	ls_client_send(client, LM_WHAT, (char *)tmpmsg);
}


void
ls_closeout_this_tcp_client(client, len, remove_client)
CLIENT_DATA *client;
int len;
int remove_client;
{
  int e = net_errno;

	if (ls_child_exited) (*ls_child_exited)(client);

	if (len == 0 || e == NET_EPIPE ||
/*-
 *		EIO used to be #ifdef apollo,
 *		ENETUNREACH only happens on apollo
 *
 */		e == NET_EIO ||
#if !defined(VMS) && !defined(PC)
		e == ENETUNREACH ||
#endif
#if defined(PC)
		e == WSAESHUTDOWN ||
		e == WSAECONNABORTED || /* P5950 */
#endif
		e == NET_ECONNRESET ||
		e == NET_ETIMEDOUT)
	{
		/*ls_down sets client's fd to -1, so we sav it here*/
/*
 *		If this is another server, shut it down, otherwise, find the
 *		client.
 */
		if (!ls_s_shut_if_client(client))
		{
		   int tfd = client->addr.addr.fd;
/*
 *			If we are not in control, see if
 *			it is appropriate to regain control
 *			at this time, and if so, do it.
 */
			ls_down((LM_SOCKET *)&tfd, "client");
			client->use_vendor_def = 0;
			f_remove_all(client, 1, LL_REASON_CLIENT_CRASH, 0, NULL);
		}
		if (remove_client)
			ls_delete_client(&client->addr);
	}
	else
	{
		DLOG(("ls_process: Read <= 0\n"));
		DLOG(("read: %d %s, len %d", net_errno, SYS_ERRLIST(net_errno), len));
		_DLOG((" using descriptor %d\n", client->addr.addr.fd));
	}
}

#ifdef SUPPORT_UDP
void
ls_closeout_this_udp_client(client)
CLIENT_DATA *client;
{
/*
 *	NOTE:  this only sets the lastcomm field to DELETE
 *	The deletion must be done elsewhere
 */
	if (client->lastcomm != LS_DELETE_THIS_CLIENT) /*already done*/
	{
		if (ls_child_exited) (*ls_child_exited)(client);

		f_remove_all(client, 1, LL_REASON_CLIENT_CRASH, 0, NULL);
	}
}
#endif /* SUPPORT_UDP */
void
ls_oneless()
{
}

