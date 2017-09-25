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
 *	Module: $Id: l_rcvmsg.c,v 1.20 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function: l_rcvmsg(job, type, param)
 *		  l_rcvmsg_type(job, thistype, param)
 *		  l_msgrdy(job, thistype)
 *		  l_clr_rcv_queue(job)
 *
 *	Description:	l_rcvmsg - Receives a message from the server
 *			l_rcvmsg_type - Receives typed message from server
 *			l_msgrdy - Tells if a message of a particular type can
 *					be read.
 *			l_clr_rcv_queue - Clears out message queue
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(char *) type - The message type
 *			(char) thistype - Particular type of msg to look for
 *			(char **) param - The message parameter
 *
 *	Return:		(int) - <> 0 - OK, msg received
 *				NULL - Problem receiving msg
 *
 *	Notes:	"param" is assigned to a static string - it will not
 *		be saved across calls to rcvmsg.
 *
 *		Whenever we receive a message, there should be a
 *		heartbeat response message in front of it (or just behind).
 *		This is	insured by sending a heartbeat message after each
 *		read.  l_check will read a heartbeat response message
 *		and issue a new heartbeat message.
 *
 *	M. Christiano
 *	2/15/88
 *
 *	Last changed:  1/9/99
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lm_comm.h"
#include "l_time.h"
#include <stdio.h>
#include <errno.h>
#include <time.h>
#ifndef PC
#include <sys/types.h>
#include <sys/socket.h>
#endif
#ifndef NO_UIO_H
#include <sys/uio.h>
#endif

#ifdef USE_WINSOCK
#include <pcsock.h>
#endif


#ifndef RELEASE_VERSION
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)l_real_getenv(c);\
	}

#define DEBUG(x) if (debug) fprintf x
#else
#define DEBUG_INIT
#define DEBUG(x)
#endif

#ifndef NO_FLEXLM_CLIENT_API
static l_rcvmsg_real lm_args((LM_HANDLE *, int));
static l_rcvmsg_wrapper lm_args((LM_HANDLE *, int));
#endif /* NO_FLEXLM_CLIENT_API */
static char l_rcvmsg_type_timeout lm_args((LM_HANDLE_PTR, int,
						LM_CHAR_PTR_PTR, int));


#ifdef VMS
#define LOOPS1 1		/* The VMS select does a 1 second timeout */
				/*  for anything less than a second, so we */
				/*  don't want to loop 300 times! */
#else
#define LOOPS1 250
#endif
#define LOOPS2 50
/*
 *	the references to this have been changed to este
 *	(spanish for this ) to avoid any potential problems with
 *	C++ compilers
 */


#ifndef NO_FLEXLM_CLIENT_API
/*
 *	l_clr_rcv_queue() - Clears the receive queue
 */
void
l_clr_rcv_queue(job)
LM_HANDLE *job;		/* Current license job */
{
  MSGQUEUE *l, *p = job->msgqueue;

	while (p)
	{
		l = p;
		p = p->next;
		l_mem_free((void *)l, (void **)&(job->msgq_free));
	}
	job->msgqueue = (MSGQUEUE *) NULL;
}

static
MSGQUEUE *
l_find_msg(job, type, prev)
LM_HANDLE *job;		/* Current license job */
char type;
MSGQUEUE **prev;
{
  MSGQUEUE *p;

	p = job->msgqueue;
	*prev = (MSGQUEUE *) NULL;
	while (p)
	{
		if (p->msg[MSG_CMD] == type)
			break;

		*prev = p;
		p = p->next;
	}
	return(p);
}

static void l_queue_remove(job, este, prev)
LM_HANDLE *job;		/* Current license job */
MSGQUEUE *este, *prev;
{
	if (prev == (MSGQUEUE *) NULL)
	{
		job->msgqueue = este->next;
	}
	else
	{
		prev->next = este->next;
	}
}

static void
l_queue_msg(job)
LM_HANDLE *job;		/* Current license job */
{
  MSGQUEUE *p, *este, *last;


#ifdef DEBUG_L_RCVMSG
	(void) printf("queued %d ('%c') msg\n", &job->cur_msg->msg[MSG_CMD],
						&job->cur_msg->msg[MSG_CMD]);
#endif
	este = (MSGQUEUE *)l_mem_malloc(job, sizeof (MSGQUEUE), (void **)&job->msgq_free);
	if (este == (MSGQUEUE *) NULL)
	{
		LM_SET_ERRNO(job, LM_CANTMALLOC, 84, 0);
	}
	else
	{
		(void) bcopy((char *) &job->cur_msg, (char *) este,
							sizeof(MSGQUEUE));
		este->next = (MSGQUEUE *) NULL;
		last = job->msgqueue;
		if (job->msgqueue == (MSGQUEUE *) NULL)
		{
			job->msgqueue = este;
		}
		else
		{
			for (p = job->msgqueue; p; last = p, p = p->next) /* NULL */;
			last->next = este;
		}
	}
}

/*
 *	l_rcvmsg_type() - Receive a message of a particular type
 */
char
l_rcvmsg_type(job, thistype, param)
LM_HANDLE *job;		/* Current license job */
int thistype; /* gets promoted to int by compiler */
char **param;
{
	return l_rcvmsg_type_timeout(job, thistype, param,
					L_RCVMSG_DEFAULT_TIMEOUT);
}

static
char
l_rcvmsg_type_timeout(job, thistype, param, timeout)
LM_HANDLE *job;		/* Current license job */
int thistype; /* gets promoted to int by compiler */
char **param;
int timeout;
{
 MSGQUEUE *t, *p = (MSGQUEUE *) NULL, *prev = (MSGQUEUE *) NULL;
 char type = '\0';

#ifdef DEBUG_L_RCVMSG
	(void) printf("l_rcvmsg_type(%d...): ", thistype);
#endif
	for (t = job->msgqueue; t; prev = t, t = t->next)
	{
		if (((thistype == 0) && (t->msg[MSG_CMD] != LM_HEARTBEAT_RESP))
		     || (thistype == t->msg[MSG_CMD])
		     || ((thistype==LM_HEARTBEAT_RESP)&&
						(t->msg[MSG_CMD]==LM_WHAT)))
		{
			p = t;
#ifdef DEBUG_L_RCVMSG
			(void) printf("got it from queue\n");
#endif
			l_queue_remove(job, p, prev);
			break;
		}
	}
	if (p == (MSGQUEUE *) NULL)
	{
		while (1)
		{
			if (l_rcvmsg_wrapper(job, timeout))
			{
				if (((thistype == 0) &&
				     (job->cur_msg.msg[MSG_CMD] != LM_HEARTBEAT_RESP) &&
				     (job->cur_msg.msg[MSG_CMD] != LM_FEATURE_AVAILABLE || 	/*	Don't lose queue messages	*/
				       timeout == L_RCVMSG_BLOCK))							/*  Needed for LM_CO_WAIT	*/
				   || (thistype == job->cur_msg.msg[MSG_CMD])
				   || ((thistype==LM_HEARTBEAT_RESP)&&
						(job->cur_msg.msg[MSG_CMD]==LM_WHAT)))
				{
#ifdef DEBUG_L_RCVMSG
					(void) printf("received %d ('%c')\n",
							job->cur_msg.msg[MSG_CMD],
							job->cur_msg.msg[MSG_CMD]);
#endif
					p = &job->cur_msg;
					break;
				}
				else
					l_queue_msg(job);
			}
			else
				break;
		}
	}
	if (p)
	{
		type = p->msg[MSG_CMD];
		(void) bcopy(p->msg, job->savemsg, LM_MSG_LEN);
		*param = &job->savemsg[MSG_DATA];
		if (p != &job->cur_msg) l_mem_free((void *)p,
						(void **)&job->msgq_free);
	}
	return(type);
}

/*
 *	l_rcvmsg() - Receive a message, any message except heartbeat response
 */

API_ENTRY
l_rcvmsg(job, type, param)
LM_HANDLE *job;		/* Current license job */
char *type;	/* The message type */
char **param;	/* The message parameter */
{
	return l_rcvmsg_timeout(job, type, param, L_RCVMSG_DEFAULT_TIMEOUT);
}

API_ENTRY
l_rcvmsg_timeout(job, type, param, timeout)
LM_HANDLE *job;		/* Current license job */
char *type;	/* The message type */
char **param;	/* The message parameter */
int timeout;
{
	DEBUG_INIT

	*type = l_rcvmsg_type_timeout(job, 0, param, timeout);
	DEBUG((stdout, "Received %c msg (%d)\n", *type, *type));
	return(*type);
}
/*
 *	l_rcvheart() - Receive a heartbeat response only.  Prime the
 *			next heartbeat response by sending a heartbeat.
 */

l_rcvheart(job)
LM_HANDLE *job;		/* Current license job */
{
  char *x;
  int ret = 0;
  char outmsg[LM_MSG_LEN+1];

#ifdef DEBUG_L_RCVMSG
	(void) printf("l_rcvheart()\n");
#endif
	if (l_rcvmsg_type(job, LM_HEARTBEAT_RESP, &x))
	{
#ifdef DEBUG_L_RCVMSG
		(void) printf("l_rcvheart() sending heartbeat response\n");
#endif
		if (l_heartbeat(job, x, outmsg))
		{
			(void)l_sndmsg(job, LM_HEARTBEAT, outmsg);
					/* Prime the next heartbeat response */
			ret = 1;
		}
		else
			LM_SET_ERROR(job, LM_BADCOMM, 272, 0, job->daemon->daemon, LM_ERRMASK_ALL);
	}
	return(ret);
}


static
l_rcvmsg_real(job, timeout_override)
LM_HANDLE *job;		/* Current license job */
int timeout_override;
{
  char msg[LM_MSG_LEN+1];
  int got;		/* Remaining bytes on this xfer */
  int ret = 0;		/* "Did we receive a data msg" return value */
  int msgsize = l_msg_size(job->daemon->our_comm_revision);
  long timeout;
  int rfd =
#ifdef FIFO
	  job->daemon->commtype == LM_LOCAL ? job->localcomm->rfd :
#endif
						job->daemon->socket;

        if (rfd == LM_BAD_SOCKET)
        {
		LM_SET_ERRNO(job, LM_NOSOCKET, 85, 0);
                return(0);
        }
	memset(msg, 0, sizeof(msg));
/*
 *	if CONN_TIMEOUT is not reasonable, use 7 seconds
 */
	if (timeout_override == L_RCVMSG_DEFAULT_TIMEOUT)
		timeout =  (job->options->conn_timeout > 0) ?
				job->options->conn_timeout : 7;
	else
	{
		timeout = timeout_override;
	}
	if (timeout != L_RCVMSG_BLOCK) timeout *= 1000;
	job->flags |= LM_FLAG_INRECEIVE;
/*
 *		Read the socket, until we get all we need.
 */
	got = l_read_timeout(rfd, msg, msgsize, timeout);
 	if (got < msgsize)
	{
		if (net_errno == NET_ECONNRESET)
		{
			LM_SET_ERRNO(job, LM_CANTREAD, 86, net_errno);
			job->flags &= ~LM_FLAG_INRECEIVE;
			lc_disconn(job, 1);
			return(0);
		}
		LM_SET_ERRNO(job, LM_CANTREAD, 87, net_errno);
		job->flags &= ~LM_FLAG_INRECEIVE;
		return(0);
	}
	else
	{
/*
*			First, decrypt it, then checksum it
*/
		if (job->daemon->encryption)
			(void) l_str_dcrypt(msg, msgsize,
					job->daemon->encryption, 1);

		if (!l_cksum_ok(msg,
			job->daemon->our_comm_revision,
			job->daemon->commtype))
		{

			if (job->daemon->encryption)
				(void) l_str_dcrypt(msg, msgsize,
					job->daemon->encryption, 1);
			if (!l_cksum_ok(msg,
				job->daemon->our_comm_revision,
				job->daemon->commtype))
			{
				LM_SET_ERRNO(job, LM_BADCOMM, 88, 0);
				if (got > 0)
				{
					job->flags &= ~LM_FLAG_INRECEIVE;
					return(0);
				}
			}
		}
/*
*			LM_HEARTBEAT is used by the daemon
*			to check the health of the socket - ignore here
*/
		if (msg[MSG_CMD] != LM_HEARTBEAT)
		{
			(void) bcopy(msg, job->cur_msg.msg, LM_MSG_LEN);
			ret = 1;
		}
	}
	job->flags &= ~LM_FLAG_INRECEIVE;
	return(ret);
}
#endif /* NO_FLEXLM_CLIENT_API */

/*
 *	l_read_timeout -- like read, but makes any number of read
 *			  attempts, until the timeout (seconds)
 *			  max timeout is 99 seconds (99000 msecs)
 *			  return -1 -- error
 *			  otherwise, bytes read
 */
API_ENTRY
l_read_timeout(fd, buf, len, timeout)
int fd;
char *buf;
long len;
long timeout; /* milliseconds -- max of 99 seconds (99,000 msecs) ,
		or it's L_RCVMSG_BLOCK */
{
  int remain = (int)len;
  long remain_time = timeout;
  char *ptr = buf;
  int size;
  long start, elapsed_time;
  struct timeval tv;
  struct timezone tz; /*not used*/
  int rc;
#ifdef PC
  int got_select_zero = 0;
#endif
#define LM_TIME ((tv.tv_sec % 100) * 1000) + (tv.tv_usec / 1000)

#ifdef getenv
#undef getenv
#endif
	DEBUG_INIT
	DEBUG((stdout, "l_read_timeout fd %d len %d timeout %d\n", fd, len, timeout));
	if (timeout > 99000)
		remain_time = timeout = 99000;
	else if (timeout == L_RCVMSG_BLOCK)
		remain_time = L_SELECT_BLOCK;
    (void)l_gettimeofday(&tv, &tz);
	start = LM_TIME;
	while (remain > 0 && (remain_time > 0 || remain_time == L_SELECT_BLOCK))
	{
		rc = 1;  /* assume we do the read, in case we don't select */

		LM_SET_NET_ERRNO(0);
		DEBUG((stdout, "selecting %d\n", time(0)));
		if (remain_time > 0 || remain_time == L_SELECT_BLOCK)
		{
			rc = l_select_one(fd, 1, (int)remain_time);
		}
		if ((rc <= 0) && (net_errno != NET_EINTR)
                                && (net_errno != EAGAIN))
		{
#ifdef PC
            if (rc == 0)
            {
                if (got_select_zero < 2) /*retry twice*/
                {
                    got_select_zero++;
                    DEBUG((stdout, "trying again rc = %d net_errno = %d remain_time = %d\n", rc, net_errno, remain_time));
                    continue; /* try again??? */
                }
            }
#endif
			DEBUG((stdout, "break rc = %d net_errno = %d remain_time = %d\n", rc, net_errno, remain_time));
			break;
		}

		DEBUG((stdout, "reading fd %d\n", fd));
		LM_SET_NET_ERRNO(0);

		size = network_read(fd, ptr, remain);
		DEBUG((stdout, "network_read %d bytes errno:%d\n",size,net_errno));
		if (size > 0)
		{
			int i = 0;
			remain -= size;
			ptr += size;
			sscanf(&buf[MSG_DATA], "%10d", &i);	/* overrun checked */
			if ((*buf == LM_WHAT) && (i == LM_SERVOLDVER))
				break;
		}
		else if (net_errno == EPIPE)
			/*printf("got -1 and EPIPE")*/;
#ifdef NLM
		else /* for nlm dont do the net test */
#else
		else if (net_errno != NET_EINTR && net_errno != EAGAIN)
#endif
		/* removed "if (size == 0)" so that size<0 is considered */
		{
			break;
		}
		if (remain <= 0)
			break;
		if (net_errno == NET_ECONNRESET)
			return -1;
		LM_SET_NET_ERRNO(0);

        (void)l_gettimeofday(&tv, &tz);
		elapsed_time = LM_TIME - start;
/*
 *		since LM_TIME is like a clock time (it starts over every
 *		100 seconds), if elapsed_time is  negative, add 100000 to
 *		it, and it will be correct.
 */
		if (elapsed_time < 0)
			elapsed_time += 100000;
		if (remain_time != L_SELECT_BLOCK)
			remain_time = timeout - elapsed_time;
	}
#ifdef UNIX
	if (!errno && (len == remain)) /* this is an OS bug, solaris 2.4 */
		errno = ECONNRESET;
#endif
	if (size == -1 && net_errno)
		return -1;
	return ((int)len - remain);
}

#ifndef NO_FLEXLM_CLIENT_API

/*
 *	Tell if a message is ready to be read
 */

l_msgrdy(job, thistype)
LM_HANDLE *job;		/* Current license job */
int thistype; 	/* C promotes the char arg to an int */
{
  int ret = 0;
  MSGQUEUE *este, *prev;
  int fd =
#ifdef FIFO
	(job->daemon->commtype == LM_LOCAL) ? job->localcomm->rfd :
#endif
						job->daemon->socket;

	este = l_find_msg(job, thistype, &prev);
	if (este)
	{
		ret = 1;
	}
	else
	{
		while (l_select_one(fd, 1, L_SELECT_NONBLOCK))
		{
			if (l_rcvmsg_wrapper(job, L_RCVMSG_DEFAULT_TIMEOUT))
			{
				if (job->cur_msg.msg[MSG_CMD] == thistype)
				{
					ret = 1;
				}
				l_queue_msg(job);
			}
			else
				break;
		}
	}
	return (ret);
}

/*
 *	l_rcvmsg_wrapper
 */
static
int
l_rcvmsg_wrapper(job, timeout)
LM_HANDLE *job;
int timeout;
{
  int rc;
  int ntries;
  int i, serial_ok = 1;

	if (job->daemon->commtype == LM_UDP)
		ntries = 2;
	else
		ntries = 1;

	for (i=0;i<ntries;i++)
	{
		rc = l_rcvmsg_real(job, timeout);

		if (job->daemon->commtype == LM_UDP)
		{
			if (!rc)
			{
/*
 *				failure, probably timed out --
 *				resend and try again
 */
				l_resend_cmd(job);
				rc = l_rcvmsg_real(job, timeout);
			}

			if (job->cur_msg.msg[MSG_CMD] == LM_HEARTBEAT)
				break; /* we're done */
/*
 *			Check if we got a message twice
 */
			if (rc && (serial_ok = l_read_sernum(job->cur_msg.msg,
						&job->daemon->udp_sernum)))
			{
				break; /* we're done */
			}
/*
 *			Check if the message is bad
 */
			if (job->cur_msg.msg[MSG_CMD] == LM_WHAT)
			{
				LM_SET_ERRNO(job, LM_BADCOMM, 89, 0);
				/* return -- don't try it again */
				return(rc);
			}
			rc = 0;
		}
	}
	if (!serial_ok)
	{
		LM_SET_ERRNO(job, LM_BADCOMM, 90, 0);
	}
/*
 *	Check if the message is bad
 */
	return rc;
}
/*
 *	l_rcvmsg_str -- receive a string from server
 *	return string, or 0 on error
 *	NOTE:  This string is malloc'd by l_rcvmsg_str --
 *	The calling function must free this string.
 */
LM_CHAR_PTR API_ENTRY
l_rcvmsg_str(job)
LM_HANDLE *job;
{
  char msgtype;
  long remain, este;
  char *p;
  char *rmsg;
  char *ret;

	if (l_rcvmsg(job, &msgtype, &rmsg) != LM_LF_DATA)
	{
		return (char *)0;
	}
/*
 *	Get the remaining size, malloc it, and start storing the data in it.
 */

	l_decode_long(&rmsg[MSG_LF_REMAIN-MSG_DATA] , &remain);
	ret = p = (char *)l_malloc(job, (size_t)(remain+1));
	while (remain)
	{
		este = MAX_LF_DATA;
		if (remain < este)
		{
			este = remain;
		}
		(void) memcpy(p, &rmsg[MSG_LF_DATA-MSG_DATA], (size_t)este);
		p += este;
		remain -= este;
		if (remain)
		{
			if (l_rcvmsg(job, &msgtype, &rmsg) != LM_LF_DATA)
			{
				LM_SET_ERRNO(job, LM_BADCOMM, 47, 0);
				free(ret);
				return (char *)0;
			}
		}
	}
	*p = '\0';
	return ret;
}

#endif /* NO_FLEXLM_CLIENT_API */

