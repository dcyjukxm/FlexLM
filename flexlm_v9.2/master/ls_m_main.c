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
 *	Module: $Id: ls_m_main.c,v 1.21 2003/04/18 23:47:58 sluu Exp $
 *
 *	Function: ls_m_main(tcp_s, spx_s, daemons, select_mask)
 *
 *	Description: Main loop of the top_level "master" server
 *
 *	Parameters:	(int) tcp_s - The tcp socket to accept connections on.
 *			(int) spx_s - The spx socket to accept connections on. 
 *			(DAEMON **) daemons - List of DAEMONs we started
 *			(SELECT_MASK *) select_mask - Masks for select()
 *
 *	Return:		None -- returns when the # of servers drops
 *				below a quorum.
 *
 *	M. Christiano
 *	2/28/88
 *
 *	Last changed:  7/14/98
 *
 */
#ifdef OS2
#define INCL_DOSPROCESS
#define INCL_ERRORS
#endif
#include "lmachdep.h"
#include "flex_file.h"
#include <stdio.h>
#include <errno.h>
#ifdef PC
#include <windows.h>
#else /* PC */
#include <sys/time.h>
#include <sys/wait.h>
#endif /* WINNT */
#include <sys/types.h>
#include <fcntl.h>
#include "lmclient.h"
#include "l_prot.h"
#include "lsmaster.h"
#include "lsserver.h"
#include "../machind/lsfeatur.h" 
#include "../app/ls_aprot.h"
#include "l_m_prot.h"
#include "lmselect.h"
#include "ls_sprot.h"
#include "ls_glob.h"

extern LM_SOCKET tcp_s;
extern LM_SOCKET spx_s;
#ifndef RELEASE_VERSION
static char * debug = (char *)-1;
#define DEBUG(x) if (debug) printf x
#define DEBUG_INIT  if (debug == (char *)-1) debug = l_real_getenv("LS_MAIN")
#else
#define DEBUG(x)
#define DEBUG_INIT  
#endif

#define SOCKETCHECK_DEFAULT 1		/* Check socket every 1 * 60 seconds */
long ls_currtime;

static void vendor_start();
static void send_daemons();
void ls_kill_one_daemon lm_args((DAEMON *));
#ifdef SIGNAL_NOT_AVAILABLE
static void check_chld_died(DAEMON **daemons);
#endif /* SINGAL_NOT_AVAILABLE */

int havequorum;			/* For ls_chld_died */
static int still_waiting;	/* Still waiting for the connections
				   we started to finish or time out */
static int havemaster;		/* ... or a master */
static int master_ready;	/* Master is "READY" */
static int first = 1;
static int count = 0;
static SELECT_MASK ready_mask;
extern int _ls_going_down;
extern time_t ls_test_hang;
extern time_t ls_test_exit;
extern char ls_our_hostname[];
extern int ls_timer_expired;
static char msg[LM_MSG_LEN+1];		/* Socket degunking message */
int ls_max_msgs_per_minute;
int ls_msg_cnt;
int ls_msg_cnt_this_minute;
int sent_daemons = 1;
/*char *l_key_callback = 0;
char *l_key_gen_callback = 0;*/

extern LM_QUORUM quorum;
#define q (&quorum)

void
ls_m_main(daemons, select_mask)
DAEMON **daemons;		/* The DAEMONs we started */
SELECT_MASK *select_mask;	/* What to select on */
{
  int nrdy;
  LM_SERVER *ls;
  int waitmaster;
  int resend, num;
  DAEMON *start;
#ifdef PC
  int last_fired_at = 0;
#endif

	havemaster = 0;	
	master_ready = 0;		/* Master is "READY" */
	havequorum = 0;

	DEBUG_INIT;
	if (first)
	{
		first = 0;
		MASK_CREATE(*select_mask);
		MASK_CREATE(ready_mask);
		msg[0] = LM_WHAT;  /* This message keeps us up-to-date on the 
				      status of the socket.  It is 
				      sent once per _timer() invocation 
				      to other daemons */
		l_msg_cksum(msg, COMM_NUMREV, LM_TCP);
	}
/*
 *	Fifo fd will be -1 at this point.  We do a blocking open call
 *	for the first client
 */
	MASK_ZERO(*select_mask);
#ifdef FIFO
	if (lm_job->options->commtype != LM_LOCAL)
#endif /* FIFO */
	if (tcp_s != LM_BAD_SOCKET)
		MASK_SET(*select_mask, tcp_s);	/* Always select the socket */
#ifdef SUPPORT_IPX
	if (spx_s != LM_BAD_SOCKET)
		MASK_SET(*select_mask, spx_s);	/* Always select the socket */
#endif
	while (1)
	{
		ls_currtime = time(0);
#ifndef RELEASE_VERSION
		ls_q_check();	/* Debug checks on the quorum struct */
#endif

#ifdef SIGNAL_NOT_AVAILABLE
		/*	Due to the lack of SIGCHLD signal on NT,
		 *	we have to check vendor daemon status from the
		 *	foreground.
		 */
		check_chld_died(daemons);
#endif /* SIGNAL_NOT_AVAILABLE */
		
		if (_ls_going_down)
		{
			ls_go_down(_ls_going_down);
		}
                if (ls_test_exit && (ls_currtime > ls_test_exit))
                {
                        ls_test_exit = 0;
                        DLOG(("exiting\n"));
			ls_go_down(_ls_going_down);
                }
                if (ls_test_hang && (ls_currtime > ls_test_hang))
                {
                        ls_test_hang = 0;
                        DLOG(("hanging for 4 minutes\n"));
                        l_select_one(0, -1, 1000*60*4);
                        DLOG(("back\n"));
                }
/*
 *		If the timer went off, check the health of the other servers
 */
#ifdef PC
                /*
                 *      The fact that vendor daemon once started never returns
                 *      contro to NT's window manager makes the timer routine
                 *      never called.   We need to do our own time expiration
                 *      checking here.
                 */
                if ((ls_currtime - last_fired_at) > TIMERSECS )
#else
                if (ls_timer_expired)
#endif /* PC */
                {  
                        long now;
                        int x;
#ifdef THREAD_SAFE_TIME
			struct tm tst;
#endif
 
			ls_log_msg_cnt();
#ifdef PC
                        last_fired_at = ls_currtime;
#endif /* PC */

			ls_timer_expired = 0;
#ifndef apollo		/* This code used to kill apollo daemons when it was */
			/* in the _timer routine - perhaps OK now? */
			count--;
			if (count <= 0)
			{
			  
#ifdef THREAD_SAFE_TIME
			    l_get_date(&x, &x, &x, &now, &tst);
#else /* !THREAD_SAFE_TIME */
			    l_get_date(&x, &x, &x, &now);
#endif
			    now -= MASTER_ALIVE;	/* Subtract 2 mins */
			    count = SOCKETCHECK_DEFAULT;
			    for (ls = ls_s_first(); ls; ls = ls->next)
			    {
				if ((ls->sflags & L_SFLAG_US) || (ls->fd1 == LM_BAD_SOCKET)) continue;
				if ((now >= ls->exptime) ||
				    (network_write(ls->fd1, msg, LM_MSG_LEN) 
								< LM_MSG_LEN))
				{
				  CLIENT_ADDR ca;
					memset(&ca, 0, sizeof(ca));
					ca.is_fd = 1; 
					ca.transport = LM_TCP;
					if ((ca.addr.fd = ls->fd2) != 
								LM_BAD_SOCKET)
						ls_delete_client(&ca);
					if ((ca.addr.fd = ls->fd1) != 
								LM_BAD_SOCKET)
						ls_delete_client(&ca);
					ls_s_down(ls);
				}
			    }
			}
#endif
		}
/*
 *		First, see if the log file needs to be timestamped
 *		and time out any connections that haven't made it.
 */
		ls_timestamp();
		ls_statfile();
		still_waiting = ls_serv_time(*select_mask);
/*
 *		Build the masks for connections to other servers
 */
		for (ls = ls_s_first(); ls; ls = ls->next)
		{
			if (ls->fd1 != LM_BAD_SOCKET)
			{
				MASK_SET(*select_mask, ls->fd1);
			}
			if (ls->fd2 != LM_BAD_SOCKET)
			{
				MASK_SET(*select_mask, ls->fd2);
			}
		}
/*
 *              Continue until we get a quorum, then after we have one,
 *              as long as the master is alive and we still have a quorum.
 *		First, find out if we are connecting, or running
 */
		if (!havequorum)
		{
			if (!still_waiting && ls_s_havequorum() &&
					(q->alpha_order || (q->n_order <= 0)))
			{
				havequorum = 1;
				waitmaster = ls_pick_mast();
				if (waitmaster < 0) havemaster = 1;
			}
		}
		else
		{
			if ((!ls_s_havequorum())   ||     /* Lost quorum */
			    (havemaster && 
				q->list[q->master].fd1 == LM_BAD_SOCKET &&
				!(q->list[q->master].sflags & L_SFLAG_US))) 
					/* Lost master */
			{
#ifdef PC
			  DWORD	exit_code;
#else /* PC */
#ifdef WAIT_STATUS_INT
			  int st;
#else
			  union wait st;
#endif /* WAIT_STATUS_INT */
#endif /* WINNT */
				/* Kill all children */

				havequorum = havemaster = 0;
				if (!ls_s_havequorum())
				{
					LOG((lmtext("lmgrd lost quorum.  lmgrd will now only\n")));
					LOG((lmtext("process connection requests from other lmgrd processes\n")));
					LOG_INFO((INFORM, "The daemon lost \
						quorum , so will process only \
						connection requests from other \
						daemons."));
				}
				else
				{
					LOG((lmtext("Lost connection to master\n")));
					LOG((lmtext("lmgrd will now attempt to select a new master\n")));
					LOG_INFO((INFORM, "The daemon lost the\
						connection to the master \
						daemon; this is equivalent \
						to losing quorum."));
				}
				LOG((lmtext("lmgrd will now shut down all vendor daemons\n\n")));
				ls_s_setmaster(-1); master_ready = 0;
				for (start = *daemons; start; start=start->next)
				{
					if (CHILDPID(start->pid))
					{
#ifdef SIGNAL_NOT_AVAILABLE
					    ls_send_shutdown( start );
#else
					     ls_kill_one_daemon(start);
#endif /* SIGNAL_NOT_AVAILABLE */
					    LOG((lmtext("SHUTting down %s\n"),
								start->name));
					    LOG_INFO((INFORM, "The license \
							daemon is shutting \
							down the vendor \
							daemon %s."));
					}
#ifdef WINNT
					/*
					 *	Wait until the child process
					 *	is really dead.
					 */
					while ( GetExitCodeProcess(
						(HANDLE)start->pid,&exit_code))
					{
						if (exit_code == STILL_ACTIVE)
							continue;
						else
							break;
					}
#endif /* WINNT */
					if (start->file_tcp_port <= 0)
					{
						start->tcp_port = 0;
					}
					start->pid = 0;
				}
#ifndef PC
/* JLL: Should this be WAIT_STATUS_INT */
#ifdef CRAY_NV1
				while (wait(&st) > 0)
#else /* CRAY_NV1 */
				while (wait((union wait *)&st) > 0)
#endif /* CRAY_NV1 */
					;	/* Wait until they die */
#endif /* PC */
				break;
			}
/*
 *			If we were waiting for a particular node to come
 *			up, see if it is up now.
 */
			if (!havemaster)
			{
				if (!ls_s_havequorum() ||
					q->list[waitmaster].state == 0 ||
				    q->list[waitmaster].state & C_CONNECTED)
				{
				    waitmaster = ls_pick_mast();
				    if (waitmaster < 0)
					havemaster = 1;
				}
			}
		}
/*
 *		Finally, wait until the master is "ready" and has
 *		sent the "MASTER_READY" message.
 */
		if (havequorum && havemaster && !master_ready)
		{
			if (q->list[q->master].state & C_MASTER_READY)
			{
				master_ready = 1;
				vendor_start(*daemons);
			}
		}

#ifdef SUPPORT_FIFO
		if (lm_job->options->commtype == LM_LOCAL && tcp_s == -1)
		{
			DEBUG(("blocking opening fifo %s\n",
				lm_job->localcomm->readname));
			tcp_s = l_flexOpen(lm_job->localcomm->readname, O_RDONLY);
			DEBUG(("opened fifo fd:%d (err:%d)\n",tcp_s, errno));
			if (tcp_s != -1)
				MASK_SET(*select_mask, tcp_s);	
			else
				continue;
		}
#endif /* SUPPORT_FIFO */
		MASK_COPY(ready_mask, *select_mask);
		if (ANY_SET((int *)ready_mask) == 0)
		{
			LOG((lmtext("license daemon: lost all connections\n")));
			LOG((lmtext("This is an internal error.  lmgrd will now\n")));
			LOG((lmtext("attempt to re-establish contact with other daemons.\n")));
			LOG_INFO((ERROR, "The license daemon lost the \
				connections to all clients and other \
				daemons.  Connections will be \
				re-established."));
			(void) fflush(stderr);
			break;
		}

		num = q->count;
		LM_SET_NET_ERRNO(0);
#ifdef PC
		{
		  struct timeval quarter_minute;
                        quarter_minute.tv_sec = 15;
                        quarter_minute.tv_usec = 0;		
                        nrdy = l_select(0, ready_mask,  0, 0, 
				(struct timeval *) &quarter_minute );
                        /*
                         *	Do not conitnue on to ls_process() if
                         *	select() timeout, i.e., nrdy == 0.
                         */
                        if ( !nrdy ) continue;
		}
#else		

		nrdy = l_select(lm_nofile, (int *)ready_mask, 0, 0, 0);
#endif /* PC */

		resend = ls_m_process(ready_mask, nrdy, *select_mask, tcp_s,
					        spx_s, daemons, ls_s_master());
/*
 *		If a new server has come up, re-send the list
 *		of DAEMON port numbers out.
 */
		if (resend || ((q->master >=0) && !sent_daemons && 
				(q->list[q->master].sflags & L_SFLAG_US)) |
			((q->count > num) && havemaster && 
				(q->list[q->master].sflags & L_SFLAG_US)))
	        {
			send_daemons(*daemons);
		}
	}
} 

/*
 *	vendor_start() - Do the processing to start up
 *			 vendor daemons once a master is picked.
 */
static
void
vendor_start(daemons)
DAEMON *daemons;	/* The DAEMONs we started */
{
  DAEMON *start;

/* 
 *	We don't need to know this if there is only 1 server node
 */
	if (ls_s_qnum() > 1)	
	{
		LOG((lmtext("CONNECTED, master is %s\n"), ls_s_master()));
		LOG_INFO((INFORM, "The license daemon logs this message when \
			a quorum is up and everyone has selected a master."));
	}
/*
 *     	Start up the vendor servers, mapping their ports dynamically
 */

	LOG((lmtext("Starting vendor daemons ... \n")));
	for (start = daemons; start; start = start->next)
	{
		if (start->tcp_port == -1) start->tcp_port = 0;
		ls_startup(start, ls_our_hostname, ls_s_master());
		if (CHILDPID(start->pid))
		{
			LOG((lmtext("Started %s"), start->name));
#ifdef PC
                        if (start->print_pid & 0xffff0000)
                        {
                                _LOG((" (pid %X)", start->print_pid));
                        }
                        else
                        {
                                _LOG((" (pid %d)", start->print_pid));
                        }
#else
			_LOG((" (internet tcp_port %d pid %d)", start->tcp_port, start->pid));
#endif
			_LOG(("\n"));
			LOG_INFO((INFORM, "The license daemon logs this message\
				whenever it starts a new vendor daemon."));
			if (ls_s_imaster())
				start->m_tcp_port = start->tcp_port;
			else if (!start->m_tcp_port)
				start->m_tcp_port = 0;
				
		}
	}
        sent_daemons = 0;
#if 0
	if (ls_s_imaster())
		send_daemons(daemons);
#endif
}

/*
 *	send_daemons() - Send the list of daemons to all the
 *			other servers
 */

static
void
send_daemons(daemons)
DAEMON *daemons;	/* The DAEMONs we started */
{
  LM_SERVER *ls;
  DAEMON *d;
/*
 *	Send the list of DAEMON port numbers to all the
 *	other servers
 */
/*
 *      First, make sure we have the port numbers!
 */
        for (d = daemons; d; d = d->next)
                if (CHILDPID(d->pid) && (!d->tcp_port))
                {
                        return; /* we're not ready yet! */
                }

	for (ls = ls_s_first(); ls; ls = ls->next)
	{
		if (!(ls->sflags & L_SFLAG_US) && (ls->state & C_CONNECTED) && 
			ls->fd1 != LM_BAD_SOCKET)
			ls_daemons(ls->fd1, daemons);
	}
	sent_daemons = 1;
}

void
ls_m_data()
{
	LOG((lmtext("ls_m_data: havemaster: %d, havequorum: %d\n"),
				havemaster, havequorum));
	LOG((lmtext("ls_m_data: still_waiting: %d, master_ready: %d\n"),
				still_waiting, master_ready));
	LOG((lmtext("ls_m_data: q->n_order: %d\n"), q->n_order));
}

#ifdef SIGNAL_NOT_AVAILABLE

/*
 * check_chld_died() loop through all vendor daemons to check if they have 
 * exited.  On unix, this is done through SIGCHLD signal.  On NT and OS2,
 * where such thing is not available, we have to check it in the foreground.
 */
static 
void 
check_chld_died(DAEMON **daemons)
{
	DAEMON *start;
	DWORD	exit_code;
	
        for (start = *daemons; start; start=start->next)
	{
		if (CHILDPID(start->pid))
		{
#ifdef OS2
			{
				RESULTCODES rcodes;
				PID pid_died;

				if ( DosWaitChild(DCWA_PROCESS, DCWW_NOWAIT,
				      &rcodes, &pid_died, start->pid) ==
				     ERROR_CHILD_NOT_COMPLETE )
					continue;
				else
				{
                                        exit_code = rcodes.codeResult;
                                        ls_chld_died(start->pid, exit_code);
                                }
			}
#else 		
			if ( GetExitCodeProcess((HANDLE)start->pid,&exit_code))
			{
				if (exit_code == STILL_ACTIVE)
					continue;
				else
				{
					ls_chld_died(start->pid, exit_code);
			        
                                }
			}
#endif /* OS2 */			
		}
	}
}
#endif /* SIGNAL_NOT_AVAILABLE */
