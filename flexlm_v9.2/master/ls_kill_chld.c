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
 *	Module: $Id: ls_kill_chld.c,v 1.29 2003/06/18 21:52:32 sluu Exp $
 *
 *	Function: ls_kill_chld()
 *
 *	Description:	Signal handler for misc signals - kills application
 *					daemons, then we exit
 *
 *	Parameters:	(int) sig - The signal we got (-1 -> no messages)
 *
 *	Return:		None - All the application daemons are killed.
 *
 *	M. Christiano
 *	3/31/88
 *
 *	Last changed:  6/26/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "lsmaster.h"
#include "../machind/lsfeatur.h"
#include "../app/ls_aprot.h"
#include "l_m_prot.h"
#include "flexevent.h"
#include <errno.h>
#ifndef PC
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#endif

extern DAEMON *master_daemons;	/* The DAEMON structure we are controlling */
extern int _ls_going_down;
void ls_kill_one_daemon lm_args((DAEMON *));
int
ls_kill_chld(int sig)
{
 DAEMON *s;
 int cnt;
 extern char * only_this_vendor;
 int ret = 0;
 int from_lmdown = _ls_going_down;
#ifdef THREAD_SAFE_TIME
 struct tm tst;
#endif

/*
 *	Set "going down", so that the SIGCHLD handler will not try to
 *	restart the servers as soon as we kill them.
 */
	if (only_this_vendor)
	{
		/* This gets set again in ls_exit; that's OK */
		_ls_going_down = 0;	
	}
	if (sig == SIGTERM) 
	{
#ifdef THREAD_SAFE_TIME
		ls_logtime(ls_gettime(&tst));
#else /* THREAD_SAFE_TIME */
		ls_logtime(ls_gettime());
#endif
		LOG_INFO((INFORM, "The user requested that the license \
			daemon shut down itself and all the vendor daemons."));
	}
	else if (sig != -1)
	{
		LOG((lmtext("LICENSE DAEMON died due to signal %d"), sig));
#ifdef THREAD_SAFE_TIME
		ls_logtime(ls_gettime(&tst));
#else /* THREAD_SAFE_TIME */
		ls_logtime(ls_gettime());
#endif
		LOG_INFO((ERROR, "The license daemon logs the signal that \
			caused it to exit."));
	}
	if (master_daemons)
	{
	   if (!only_this_vendor)
		   LOG((lmtext(
		"lmgrd will now shut down all the vendor daemons\n\n")));
	   for (cnt = 0, s = master_daemons; s; s = s->next)
	   {
		if (only_this_vendor && !L_STREQ(s->name, only_this_vendor))
			continue;
		if (CHILDPID(s->pid))
		{
		  int rc;
			LOG((lmtext("Shutting down %s\n"), s->name));
			LOG_INFO((INFORM, "The license daemon is shutting down \
				the vendor daemon specified."));
		
			if (((rc = ls_send_shutdown( s )) != LM_BORROW_DOWN) && rc)
#ifndef PC
				ls_kill_one_daemon(s)
#endif
				;
			cnt++;
			if (rc == LM_BORROW_DOWN) 	ret = rc;
			else if (ret != LM_BORROW_DOWN && lm_job->lm_errno)
				ret = lm_job->lm_errno;
		}
	   }
	}
	if (!from_lmdown /*&& _ls_going_down && !only_this_vendor*/)
		ls_go_down(sig);

	if (!cnt && only_this_vendor) 
		LOG((lmtext("Can't find server for vendor \"%s\"\n"), 
			only_this_vendor));
	return ret;
}
#ifndef SIGNAL_NOT_AVAILABLE
void
ls_kill_one_daemon(s)
DAEMON *s;
{
	if (s->pid)
	{
		(void) kill(s->pid, SIGTRAP);
/* 
 *			This is a little tricky.
 *			When the child dies, SIGCHLD happens, the
 *			following select gets interrupted, and the
 *			s->pid gets set to 0 -- that's what we look for
 */
		{
		  struct timeval tval;

			tval.tv_sec = 3; /* timeout in 3 seconds */
			tval.tv_usec = 0;
			errno = 0;
			l_select(0,0,0,0,&tval);
		}
#ifdef PC
                if (s->pid & 0xffff0000)
                        DLOG(("still trying to kill daemon process pid %X\n",  
                                                                s->pid));
                else
                        DLOG(("still trying to kill daemon process pid %d\n",  
                                                                s->pid));
#else	 
                DLOG(("still trying to kill daemon process pid %d\n",  s->pid));
#endif /* PC */
		if (s->pid) kill(s->pid, SIGTRAP);
		/* we'll assume this one works */
	}
}
#endif /* SIGNAL_NOT_AVAILABLE */


#ifdef SUPPORT_IPX
static
get_my_ipx_addr( ipx_addr )
struct sockaddr_ipx *ipx_addr;
{
	LM_SOCKET ipx_s;
	int addrlen;
	
	if ((ipx_s=socket(AF_NS, SOCK_STREAM, NSPROTO_SPX)) == LM_BAD_SOCKET )
		return -1;
		

	memset(ipx_addr, 0, sizeof(ipx_addr));
	ipx_addr->sa_family = AF_NS;

	if (bind(ipx_s, (const struct sockaddr *) ipx_addr, sizeof(*ipx_addr)))
	{
		network_close(ipx_s);
		return -1;
	}

	addrlen = sizeof(ipx_addr);
	if (getsockname(ipx_s, (struct sockaddr *) ipx_addr, &addrlen))
	{
		network_close(ipx_s);
		return -1;
	}

	network_close(ipx_s);
	return 0;
}
#endif /* SUPPORT_IPX */

int 
ls_send_shutdown( target_dm )
DAEMON *target_dm;
{
  char msg[LM_MSG_LEN+1];
  char resp[LM_MSG_LEN+1];
  int d_socket;
  int msgsize = l_msg_size(lm_job->daemon->our_comm_revision);
  char type, *msgptr;
  char local_host[MAX_HOSTNAME + 1];
  COMM_ENDPOINT endpoint;
  extern ls_force_shutdown;
  int ret=0;
#ifdef PC
  unsigned int status = 0;
#endif
  int check_borrow = 1;
	char *	ppszInStr[20] = {NULL};
 
#ifdef UNIX
	if (target_dm->flex_ver < 8) return 1; /* P6200 was return 0 */
#endif /* UNIX */
	l_conn_msg(lm_job, target_dm->name, msg, LM_TCP, 0);
	gethostname( (char *)local_host, MAX_HOSTNAME );

	endpoint.transport = LM_TCP;
	memset(resp, 0, sizeof(resp));
	endpoint.transport_addr.port = (int) ntohs((unsigned short)target_dm->tcp_port);
	d_socket = l_basic_conn(lm_job, msg, &endpoint, local_host, resp);

#ifdef PC
	/*
	 * Use SPX/IPX when TCP failed
	 */
	if ( d_socket < 0 )
	{
		endpoint.transport = LM_SPX;
		if ( !get_my_ipx_addr(&endpoint.transport_addr.spx_addr) )
		{
			endpoint.transport_addr.spx_addr.sa_socket =
							target_dm->m_spx_port;
			d_socket = l_basic_conn(lm_job, msg, &endpoint,
							local_host,msg);
		}
	}
#endif /* PC */

	if ( d_socket < 0 )
	{			
		LOG((lmtext("Can't connect to license server.  Shutdown %s failed. \n"), target_dm->name));
                LOG(("\t%s\n", lc_errstring(lm_job)));
                ret=1;
		goto exit_ls_send_shutdown;
        }


try_again:
	bzero(msg, LM_MSG_LEN+1);
	msg[MSG_CMD] = LM_SHUTDOWN;
	msg[MSG_HEL_VER] = check_borrow; /* flag to find out if licenses are borrowed */
	strncpy(&msg[MSG_HEL_NAME], lc_username(lm_job, 0), MAX_USER_NAME);/* LONGNAMES */
	strncpy(&msg[MSG_HEL_HOST], lc_hostname(lm_job, 0), MAX_SERVER_NAME);/* LONGNAMES */
	l_msg_cksum(msg, lm_job->daemon->our_comm_revision, LM_TCP);
	if (network_write(d_socket, msg, msgsize) != msgsize)
	{
		LOG((lmtext( "Can't write to license server.  First try of shutdown %s failed.\n"),
		     target_dm->name));
		ret=1;
		goto exit_ls_send_shutdown;
	}

        if ((lm_job->daemon->our_comm_revision < 3))
        {
			LOG(("Shut down FLEXlm %s server on node %s\n", /* P6166 */
				target_dm->name, lc_hostname(lm_job, 0)));
			if(l_flexEventLogIsEnabled())
			{
				ppszInStr[0] = target_dm->name;
				ppszInStr[1] = lc_hostname(lm_job, 0);

				l_flexEventLogWrite(lm_job,
									FLEXEVENT_INFO,
									CAT_FLEXLM_LMGRD,
									MSG_FLEXLM_LMGRD_SHUTDOWN,
									2,
									ppszInStr,
									0,
									NULL);
			}
			goto exit_ls_send_shutdown;
        }


	lm_job->daemon->socket = d_socket;
	if (!l_rcvmsg(lm_job, &type, &msgptr))
	{
		LM_SET_ERRNO(lm_job, LM_CANTREAD, 354, 0);
		LOG((lmtext("Can't read from license server.  First try of shutdown %s failed. \n"),target_dm->name));
		ret=1;
		goto exit_ls_send_shutdown;
	}


	if (type == LM_OK)
	{
		LOG(("Shut down FLEXlm %s server on node %s\n", /* P6166 */
			target_dm->name, lc_hostname(lm_job, 0)));
		if(l_flexEventLogIsEnabled())
		{
			ppszInStr[0] = target_dm->name;
			ppszInStr[1] = lc_hostname(lm_job, 0);

			l_flexEventLogWrite(lm_job,
								FLEXEVENT_INFO,
								CAT_FLEXLM_LMGRD,
								MSG_FLEXLM_LMGRD_SHUTDOWN,
								2,
								ppszInStr,
								0,
								NULL);
		}
	}
	else if (type == LM_NOT_ADMIN)
	{
		LM_SET_ERRNO(lm_job, LM_NOTLICADMIN, 355, 0);
		LOG((lmtext("Unauthorized shutdown detected.  Shutdown %s failed.\n"), target_dm->name));
		ret =  LM_NOTLICADMIN;
		goto exit_ls_send_shutdown;
	}
	else if (type == LM_BUSY)
	{
		if (ls_is_local())
		{
			if (ls_force_shutdown)
			{
				check_borrow =0;
				DLOG(("forcing shutdown...\n"));
				goto try_again;
			}
			else
			{
				DLOG(("Not forced\n"));
			}

			LOG(("Redo lmdown with '-force' arg.\n"));
			LOG(("Required when borrowed licenses outstanding\n"));
			if(l_flexEventLogIsEnabled())
			{
				l_flexEventLogWrite(lm_job,
									FLEXEVENT_WARN,
									CAT_FLEXLM_LMGRD,
									MSG_FLEXLM_VENDOR_BORROW_OUTSTANDING,
									0,
									NULL,
									0,
									NULL);
			}
		}
		else
		{
			DLOG(("Not local\n"));
		}
		_ls_going_down = 0;
		LM_SET_ERRNO(lm_job, LM_BORROW_DOWN, 567, 0);
		LOG(("%s\n", lc_errstring(lm_job)));
		ret = LM_BORROW_DOWN;
		goto exit_ls_send_shutdown;
	}
	else
	{
		LOG((
	"Unknown response (%c) 0x%x.  First try of shutdown %s failed.\n", 
						type, type, target_dm->name));
	}
exit_ls_send_shutdown:

        lc_disconn(lm_job, 0 );

#ifdef PC

        if (ret == 1)
        {
                if (TerminateProcess((HANDLE) target_dm->pid , status ))
                {
					LOG(("Shut down FLEXlm %s server on node %s\n", /* P6166 */
						target_dm->name, lc_hostname(lm_job, 0)));
					if(l_flexEventLogIsEnabled())
					{
						ppszInStr[0] = target_dm->name;
						ppszInStr[1] = lc_hostname(lm_job, 0);

						l_flexEventLogWrite(lm_job,
											FLEXEVENT_INFO,
											CAT_FLEXLM_LMGRD,
											MSG_FLEXLM_LMGRD_SHUTDOWN,
											2,
											ppszInStr,
											0,
											NULL);
					}
                }
                else
                {
					LOG((lmtext( "Can't Shutdown license server.  Shutdown %s failed.\n"),
                        target_dm->name));
                }
        }

#endif
	return ret;
}
