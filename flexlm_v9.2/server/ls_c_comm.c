/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
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
 *	Module: $Id: ls_c_comm.c,v 1.9 2003/01/13 22:31:37 kmaclean Exp $
 *
 *	Function:	ls_client_send(), ls_server_send(), ls_client_receive()
 *
 *	Description: 	Sends/receives a message to another server or a client.
 *
 *	M. Christiano
 *	5/2/90
 *
 *	Last changed:  1/8/99
 *
 */

#include "lmachdep.h"
#include <errno.h>
#ifndef PC
#include <sys/time.h>
#endif /* PC */
#include "lmclient.h"
#include "l_prot.h"
#include "lmselect.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"

#ifndef RELEASE_VERSION
static long debug = -1;
#define DEBUG_INIT if (debug == -1) debug = (long)l_real_getenv("LS_SEND_DEBUG");
#define DEBUG(x) if (debug) printf x
#else
#define DEBUG_INIT
#define DEBUG(x)
#endif

static void real_send();
#if !defined( VMS) && !defined(MOTOSVR4)
#define USE_SELECT_DONT_LOOP
#endif

/*
 *	Send a message to a client (with possible encryption)
 */

void
ls_client_send(c, msgtype, msg)
CLIENT_DATA *c;
int msgtype;
char *msg;
{
  static unsigned char sernum;
	msg[0] = msgtype;
	l_msg_cksum(msg, c->comm_revision, c->addr.transport);
#ifdef SUPPORT_UDP	
	if (c->addr.transport == LM_UDP) 
		l_write_sernum(msg, &c->udp_sernum);
#endif /* SUPPORT_UDP */	
	ls_msg_dump("client SENDING", msg, &(c->addr), l_msg_size(c->comm_revision));
	if (c->encryption)
		(void) l_str_crypt(msg, l_msg_size(c->comm_revision), 
					c->encryption, c->flexlm_ver >= 5);
	real_send(&c->addr, msg, c->comm_revision, msgtype);
}

/*
 *	Send a message to another server (no possibility of encryption)
 */

void
ls_server_send(ca, msgtype, msg)
CLIENT_ADDR *ca;
int msgtype;
char *msg;
{
	msg[0] = msgtype; 
	l_msg_cksum(msg, COMM_NUMREV, LM_TCP);
	ls_msg_dump("server SENDING", msg, ca, l_msg_size(COMM_NUMREV));
	real_send(ca, msg, COMM_NUMREV, msgtype);
}

/*
 *	Send the message
 */


static 
void
real_send(ca, msg, comm_revision, msgtype)
CLIENT_ADDR *ca;
char *msg;
int comm_revision;
int msgtype;
{
  int rem, retry = 10, n;
  char *_msg = msg;
  int i;
  SELECT_MASK j;
  struct timeval timeout;

	DEBUG_INIT;
	if (comm_revision <= 0 || comm_revision > COMM_NUMREV)
		rem = l_msg_size(1); /*force it*/
	else
		rem = l_msg_size(comm_revision);

#ifdef USE_SELECT_DONT_LOOP
	timeout.tv_sec = 5;	/* 10 - 5 sec timeouts */
	timeout.tv_usec = 0;
	MASK_CREATE(j);
#else
	retry = 10;		/* Loop 10 times (was 1000!) - no select */
#endif

	while (rem > 0 && retry > 0) 
#ifdef USE_SELECT_DONT_LOOP
	{
		do
		{
			if ((ca->transport == LM_TCP)
#ifdef SUPPORT_IPX
			    || (ca->transport == LM_SPX)
#endif
			    ) 
			{
				MASK_INIT(j, ca->addr.fd);
			}
			else if (ca->transport == LM_UDP)
			{
				MASK_INIT(j, ls_udp_s);
			}
			else 
			{
			    DLOG((lmtext("Internal error: Got bad transport")));
			    _DLOG((lmtext(" for sockaddr %s@%d\n"), __FILE__, 
				__LINE__));
			}
			LM_SET_NET_ERRNO(0);
#ifdef USE_WINSOCK
			i = l_select(0, 0, j, 0,&timeout);
#else
			i = l_select(lm_nofile, (int *)0, (int *)j, (int *)0, 
							&timeout);
#endif
		} while ( (retry-- > 0) && (net_errno == NET_EINTR) );

		if (i > 0  && (ANY_SET((int *)j) || ca->transport )) 
						/*- Apollo select returns -1
							instead of 0 !!!  */
		{
			if ((ca->transport == LM_TCP)
#ifdef SUPPORT_IPX
			    || (ca->transport == LM_SPX)
#endif
			    )
			{
				n = network_write(ca->addr.fd, _msg, rem);
				DEBUG((
				"write fd %d, bytes %d, errno %d wrote %d\n", 
					ca->addr.fd, rem, errno, n));
			}
#ifdef SUPPORT_UDP
			else if (ca->transport == LM_UDP)
			{
			    n = ls_udp_send(ls_udp_s, &ca->addr.sin, _msg, rem);
			    if (msgtype != LM_HEARTBEAT)
				    (void)memcpy(ls_lookup_client(ca)->
					    udp_last_resp, _msg, LM_MSG_LEN+1);
			} 
#endif	/* SUPPORT_UDP */
			if (n > 0) 
			{ 
				rem -= n; 
				_msg += n;
			}
		}
	} 
	MASK_DESTROY(j);
#else /* USE_SELECT_DONT_LOOP */
	{
		
		if ((ca->transport == LM_TCP)
#ifdef SUPPORT_SPX
		    (ca->transport == LM_SPX)
#endif
		    )		    
		{
			n = network_write(ca->addr.fd, _msg, rem);
			DEBUG(( "write fd %d, bytes %d, errno %d wrote %d\n",
					ca->addr.fd, rem, errno, n));
		}
#ifdef SUPPORT_UDP
		else if (ca->transport == LM_UDP) 
		{
			if ((n = ls_udp_send(ls_udp_s, &ca->addr.sin, _msg, 
								rem)) != rem)
			{
		    		if (net_errno == ECONNREFUSED)
				{
					DLOG((lmtext(
				"Can't talk to UDP client -- client gone\n")));
				}
				else
				{
					DLOG((lmtext("udp_send error %d\n"), 
						net_errno));
				}
			}
		        if (msgtype != LM_HEARTBEAT)
			        (void)memcpy(ls_lookup_client(ca)->
					    udp_last_resp, _msg, LM_MSG_LEN+1);
		}
#endif	/* SUPPORT_UDP */
		else
		{
			DLOG((lmtext("INTERNAL ERROR: trying to send to")));
			_DLOG((lmtext("TCP sockaddr -- not supported yet\n")));
		}
		retry--;
		if (n > 0) 
		{ 
			rem -= n; 
			_msg += n;
			if (rem > 0)
				ls_pause(10);
		}
	}
	if ((retry < 1) && (rem > 0))
	{
		LOG((lmtext("Error %d sending mesage to client\n"),errno));
	}
#endif /* USE_SELECT_DONT_LOOP */
}

/*
 *	Receive a message from another server
 */

int
ls_serv_receive(ca, msgtype, msg)
CLIENT_ADDR *ca;
char *msgtype;
char **msg;
{
#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
  extern char *sys_errlist[];
#endif
  int _msglen = 0;
  static char _msg[LM_MSG_LEN+1];
  int i;

	(void) memset(_msg, 0, sizeof(_msg));
	*msgtype = LM_NO_MESSAGE;
	_msglen = l_read_timeout(ca->addr.fd, _msg, LM_MSG_LEN, 2 * 1000);
		

	if (_msglen <= 0)
	{
#ifdef PC
		DLOG((lmtext("Winsock read error: %d return %d\n"), WSAGetLastError(), _msglen));
#else
		DLOG((lmtext("read: %s\n"), SYS_ERRLIST(errno)));
#endif
	}
	else if (!l_cksum_ok(_msg, COMM_REVISION, LM_TCP))
	{ 
		DLOG(("BAD CHECKSUM: %s\n", _msg)); 
	}
	else
	{ 
		*msgtype = _msg[MSG_CMD]; 
		*msg = &_msg[MSG_DATA]; 
		{
			ls_msg_dump("RECEIVED", *msg, ca, LM_MSG_LEN);
		}
	}
	return _msglen;
}

/*
 *	Dump the data that a server is sending/receiving.  Set 
 *	ls_dump_send_data to a non-zero value in ls_vendor.c 
 *	(or ls_lmgrd.c) to get this output.
 */

void
ls_msg_dump(param, msg, ca, size)
char *param;
char *msg;
CLIENT_ADDR *ca;
int size;
{ 
  extern int ls_dump_send_data;

	if (ls_dump_send_data)
	{ 
		LOG((lmtext("%s (fd: %d) ["), param, ca->addr.fd));
		switch (*msg)
		{
		    case LM_HELLO:
		    {
			_LOG(("HELLO v%c.%c N:%s H:%s S:%s]\n", msg[MSG_HEL_VER],
				msg[MSG_HEL_VER+1], &msg[MSG_HEL_NAME],
				&msg[MSG_HEL_HOST], &msg[MSG_HEL_DAEMON])); 
			break;
		    }
		    case LM_SHELLO:
		    {	
			_LOG(("SHELLO v%c.%c %s %s]\n", msg[MSG_HEL_VER],
				msg[MSG_HEL_VER+1],
				&msg[MSG_HEL_HOST], &msg[MSG_HEL_DAEMON])); 
			break;
		    }
		    case LM_USERNAME:
		    {	
			_LOG(("USERNAME %s %s %s]\n", &msg[MSG_U_NAME],
				&msg[MSG_U_NUM], &msg[MSG_U_TIME])); 
			break;
		    }
		    case LM_TRY_ANOTHER:
		    {	
			_LOG(("TRY_ANOTHER %s %s]\n", &msg[MSG_TRY_HOST],
					&msg[MSG_TRY_TCP_PORT])); 
			break;
		    }
		    case LM_LF_DATA: 
		    {
			_LOG(("LF_DATA %25.25s]\n", &msg[MSG_LF_DATA])); 
			break; 
		    }
		    case LM_DAEMON:
		    {	
			_LOG(("DAEMON %s %s]\n", &msg[MSG_SPP_NAME],
					&msg[MSG_SPP_PORT])); 
			break;
		    }
		    case LM_HANDSHAKE: 	{ _LOG(("HANDSHAKE]\n")); break; }
		    case LM_DISOWN:	{ _LOG(("DISOWN]\n")); break; }
		    case LM_DUMP:	{ _LOG(("DUMP]\n")); break; }
		    case LM_IS_MASTER:	{ _LOG(("IS_MASTER]\n")); break; }
		    case LM_DETACH:	{ _LOG(("DETACH]\n")); break; }
		    case LM_HEARTBEAT:	{ _LOG(("HEARTBEAT]\n")); break; }
		    case LM_CHECKIN:	{ _LOG(("CHECKIN]\n")); break; }
		    case LM_LOG:	{ _LOG(("LOG]\n")); break; }
		    case LM_RESTART:	{ _LOG(("RESTART]\n")); break; }
		    case LM_LIST:	{ _LOG(("LIST]\n")); break; }
		    case LM_I_MASTER:	{ _LOG(("I_MASTER]\n")); break; }
		    case LM_NEWSERVER:	{ _LOG(("NEWSERVER]\n")); break; }
		    case LM_CHECKOUT:	{ _LOG(("CHECKOUT]\n")); break; }
		    case LM_PID:	{ _LOG(("PID]\n")); break; }
		    case LM_SHUTDOWN:	{ _LOG(("SHUTDOWN]\n")); break; }
		    case LM_MASTER_READY:{ _LOG(("MASTER_READY]\n")); break; }
		    case LM_TELLME:	{ _LOG(("TELLME]\n")); break; }
		    case LM_REMOVE:	{ _LOG(("REMOVE]\n")); break; }
		    case LM_ORDER:	{ _LOG(("ORDER]\n")); break; }
		    case LM_SWITCH:	{ _LOG(("SWITCH]\n")); break; }
		    case LM_DOTELL:	{ _LOG(("DOTELL]\n")); break; }
		    case LM_U_MASTER:	{ _LOG(("MASTER]\n")); break; }
		    case LM_DOREMOVEALL: { _LOG(("DOREMOVEALL]\n")); break; }
		    case LM_CLOCKSETTING:{ _LOG(("CLOCKSETTING]\n")); break; }
		    case LM_REREAD:	{ _LOG(("REREAD]\n")); break; }
		    case LM_SWITCH_REPORT:{ _LOG(("SWITCH_ERROR]\n")); break; }
		    case LM_SINCE:	{ _LOG(("SINCE]\n")); break; }
		    case LM_BUSY:	{ _LOG(("BUSY]\n")); break; }
		    case LM_NO_CONFIG_FILE: { _LOG(("NO_CONFIG_FILE]\n")); break; }
		    case LM_BUSY_NEW:	{ _LOG(("BUSY_NEW]\n")); break; }
		    case LM_FEATURE_AVAILABLE: { _LOG(("FEATURE_AVAILABLE]\n"));
						break; }
		    case LM_HEARTBEAT_RESP: { _LOG(("HEARTBEAT_RESP]\n")); 
						break; }
		    case LM_NUSERS:	{ _LOG(("NUSERS]\n")); break; }
		    case LM_OK: 	{ _LOG(("OK]\n")); break; }
		    case LM_QUEUED:	{ _LOG(("QUEUED]\n")); break; }
		    case LM_SHELLO_OK:	{ _LOG(("SHELLO_OK]\n")); break; }
		    case LM_WHAT:	{ _LOG(("WHAT]\n")); break; }
		    case LM_NO_SUCH_FEATURE: 
		    		{ _LOG(("NO_SUCH_FEATURE]\n")); break; }


		    default:
		    {	
			if (msg[MSG_DATA])
			{
				_LOG(("%c %s]\n", *msg, &msg[MSG_DATA]));
			}
			else
			{
				_LOG(("0x%x]\n", *msg));
			}
			break;
		    } 
		}
	} 
}

/*
 *	ls_client_send_str -- send arbitrary size string
 *	The msgtype is always LM_LF_DATA -- [this is for historical reasons,
 *		and the name should probably be changed to something more
 *		meaningful.]
 */
void
ls_client_send_str(c, str) 
CLIENT_DATA *c;
char *str;
{
  char msg[LM_MSG_LEN + 1];
  int este;
  long remain = strlen(str);


	while (remain > 0)
	{
		memset(msg, 0, sizeof (msg));
		este = MAX_LF_DATA;
		if (este > remain) este = remain;
		l_encode_long(&msg[MSG_LF_REMAIN], remain);
		(void) memcpy(&msg[MSG_LF_DATA], str, este);
		ls_client_send(c, LM_LF_DATA, msg);
		str += este;
		remain -= este;
	}
}
