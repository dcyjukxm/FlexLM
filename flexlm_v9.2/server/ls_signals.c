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
 *	Module: $Id: ls_signals.c,v 1.16 2003/01/13 22:31:39 kmaclean Exp $
 *
 *	Functions:	ls_control(), ls_recycle_control(),
 *			ls_release_control(), ls_exit(), ls_go_down()
 *
 *	Description:	Process CONTROL_SIGNAL and exit signals
 *			Clean up the socket (ls_go_down())
 *
 *	Parameters:	None
 *
 *	Return:		None.
 *			State of the server is changed.
 *
 *	Notes:		CONTROL_SIGNAL is the CONTROL token that is
 *			passed among the servers to decide who will listen
 *			to the socket for new connections from clients.
 *			If the bottom-level server passes the token to
 *			the top level, and has not freed any file
 *			descriptors before it re-arrives, then a new
 *			child server is spawned.
 *
 *	M. Christiano
 *	3/6/88
 *
 *	Last changed:  2/18/97
 *
 */

#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lssignal.h"
#include <errno.h>
#include <sys/types.h>
extern int ls_i_am_lmgrd;	    /* Flag to distinguish lmgrd from 
							vendor daemons */

#define DEBUG_COREDUMPS		     /* Define to get core dumps on faults */
#ifdef WINNT
#include "l_service.h"
#endif
/*
 *	Receive the control token.
 */


/*
 *	ls_exit() - Clean up the server
 */
static LM_SOCKET _socket_tcp = LM_BAD_SOCKET;	/* Our socket for accepts*/
static LM_SOCKET _socket_udp = LM_BAD_SOCKET;	/* Our socket for accepts*/
#ifdef SUPPORT_IPX
static LM_SOCKET _socket_spx = LM_BAD_SOCKET;	/* Our socket for accepts*/
#endif /* SUPPORT_IPX */
int _ls_going_down = 0;

#ifdef VOID_SIGNAL
void
#endif
ls_exit(sig)
int sig;
{

	_ls_going_down = sig;
	if (sig == 0) _ls_going_down = 1;

}

void
ls_go_down(sig)
int sig;
{
int kill_failed = 0;

	_ls_going_down = 1;
	f_nousers();			/* Remove all clients */

	if (sig != SHUTDOWN_SIGNAL && sig < BASE_EXIT && sig > 0 && 
				!(ls_i_am_lmgrd && sig == SIGTERM))
	{
		LOG((lmtext("EXITING DUE TO SIGNAL %d\n"), sig));
		LOG_INFO((INFORM, "The daemon is listing the signal that \
				caused it to exit."));
	}
	else if (sig == SHUTDOWN_SIGNAL )
	{
		LOG((lmtext("daemon shutdown requested - shutting down\n")));
		LOG_INFO((INFORM, "The daemon logs the fact that a shutdown \
			was requested by the user."));
	}
/*
 *	Close files
 */
#ifndef NO_LMGRD		
	if (ls_i_am_lmgrd) 
	{
/*
 *		lmgrd - status file
 */
		ls_statfile_rm();
	}
	else
#endif /* NO_LMGRD */
	{
/*
 *		vendor daemon - report log
 */
		ls_log_close_report(1);
	}
/*
 *	Closing the sockets was moved to AFTER closing the report log
 *	since report log can be written to a socket
 */
	ls_down(&_socket_tcp, "on exit");	
	ls_down(&_socket_udp, "on exit");
#ifdef SUPPORT_IPX
	ls_down(&_socket_spx, "on exit");
#endif /* SUPPORT_IPX */
#ifndef NO_REDUNDANT_SERVER
	{
	  LM_SERVER *ls;
		for (ls = ls_s_first(); ls; ls = ls->next)
		{
			ls->state = 0; /* so ls_s_down won't complain */
			ls_s_down(ls);
		}
	}
#endif /* NO_REDUNDANT_SERVER */
	
	ls_log_close_ascii();
	if (sig != EXIT_SERVERRUNNING)
		ls_unlock(1);		/* Get rid of the lock file */
#if defined( _ALPHA_ ) && defined( WINNT )
	Sleep(2000);
#endif

#if 0
	if (sig == SIGSEGV || sig == SIGBUS || sig == SIGILL)
	{
/*
 *		Force the coredump - use SIGSYS
 */
		(void) signal(SIGSYS, SIG_DFL);
		(void) kill(getpid(), SIGSYS);
	}
	else
#endif
	{
#ifndef SIGNAL_NOT_AVAILABLE	
		if (kill_failed)
			(void) kill(0, SHUTDOWN_SIGNAL);  /* Kill the child */
#endif

		/*lc_free_job(lm_job);*/
		
#ifdef WINNT
		if (is_lmgrd_a_service)
		{
			// tricky situation. Need to know where the shutdown request
			// came from.

            if (iProcess_Service_Message == 1)
                return; /* don't exit yet! Let service 
                           callback shutdown the lmgrd. */
		}

#endif

#if defined(VMS) 
		exit(0);
#else
		if (sig == SHUTDOWN_SIGNAL)
			exit(EXIT_REQUESTED);
		else
			exit(sig);
#endif
	}
	
}

void
ls_store_socket(s_tcp, s_udp, s_spx)	
LM_SOCKET s_tcp;	
LM_SOCKET s_udp;	
LM_SOCKET s_spx;	
{
	_socket_tcp = s_tcp;	
	_socket_udp = s_udp;
#ifdef SUPPORT_IPX
	_socket_spx = s_spx;
#endif /* SUPPORT_IPX */
}
