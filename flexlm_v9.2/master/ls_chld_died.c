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
 *	Module: $Id: ls_chld_died.c,v 1.14 2003/01/13 22:26:49 kmaclean Exp $
 *
 *	Function: ls_chld_died()
 *
 *	Description:	Signal handler for SIGCHLD - restarts application 
 *							daemons
 *
 *	Parameters:	None
 *
 *	Return:		None - The particular application daemon is
 *				restarted.
 *
 *	M. Christiano
 *	4/1/88
 *
 *	Last changed:  9/9/98
 *
 */


#include "lmachdep.h"
#ifndef PC
#include <sys/time.h>
#endif /* PC */
#include <sys/types.h>
#if !defined(MOTO_88K) && !defined(PC) 
#include <sys/resource.h>
#endif
#ifndef sco
#include <sys/uio.h>
#endif
#ifdef USE_WINSOCK
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/wait.h>
#endif
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_sprot.h"
#include "ls_glob.h"
#include "lsmaster.h"
#include "../machind/lsfeatur.h" 
#include "../app/ls_aprot.h" 
#include "l_m_prot.h" 

extern DAEMON *master_daemons;	/* The DAEMON structure we are controlling */
extern int _ls_going_down;
extern int havequorum;
extern char ls_our_hostname[];

#ifdef SIGNAL_NOT_AVAILABLE
void
ls_chld_died(sig, exit_code)
int sig;	/* actually pid */
int exit_code;  /* child process exit code */
{
#else /* SIGNAL_NOT_AVAILABLE */
#ifdef VOID_SIGNAL
void
#endif
ls_chld_died(sig)
int sig;
{
#ifdef WAIT_STATUS_INT
 int status;
#else
 union wait status;
#endif /* WAIT_STATUS_INT */
#endif /* SIGNAL_NOT_AVAILABLE */
 
 int pid;
 DAEMON *s;
 LM_SERVER *ls;
 unsigned char ret; /*This is what unix systems return -- not int*/
 char *master;
 int havemaster;
 int i;

/*
 *	First, find out if a child REALLY died, or if this is just noise
 */
#ifdef SIGNAL_NOT_AVAILABLE
	pid = sig; 
	{
		ret = exit_code;		
#else
 
/* JONL: I think this should be WAIT_STATUS_INT */
#ifdef CRAY_NV1
	pid = wait3(&status, WNOHANG, (struct rusage *) 0);	
#else /* CRAY_NV1 */
	pid = wait3((union wait *)&status, WNOHANG, (struct rusage *) 0);	
#endif /* CRAY_NV1 */
	(void) signal(SIGCHLD, ls_chld_died);
	if (CHILDPID(pid))
	{
/*
 *		Re-start the child that died.
 */
		ret = (
#ifdef WAIT_STATUS_INT
			status
#else
			status.w_status 
#endif
					& 0xff00) >> 8;
#endif /* SIGNAL_NOT_AVAILABLE */
		
#if 0
		DLOG(("Child pid (%d) died with status %d\n", pid, ret));	
#endif
		for (s = master_daemons; s; s = s->next)
		{
		  int restart = 1;	/* "Restart daemon" flag */

		    if (pid == s->pid)
		    {
			if (ret == EXIT_NOFEATURES)
			{
			  LOG((lmtext("%s daemon found no features.  Please correct\n"),
						s->name));
			  LOG((lmtext("license file and re-start daemons\n")));
			  LOG(("\n"));
			  LOG((lmtext("This may be due to the fact that you are using\n")));
			  LOG((lmtext("a different license file from the one you expect.\n")));
			  LOG((lmtext("Check to make sure that:\n")));
			  if (lm_job->lic_files[0])
			  {
				  LOG(("%s ", lm_job->lic_files[0]));
				  for (i = 1 ; i < lm_job->lm_numlf; i++)
					  _LOG(("%s ", lm_job->lic_files[i]));
			  } 
			  else LOG(("no files found"));
			  _LOG(("\n"));
				
			  LOG((lmtext("is the license file you want to use.\n")));
			  LOG(("\n"));
			  LOG_INFO((CONFIG, "The specified daemon found no \
				features to serve.  Make sure the correct \
				license file is being used by this daemon."));
			  s->pid = 0;
			  restart = 0;
			}
			else if (ret == EXIT_SERVERRUNNING)
			{
			  LOG((lmtext("MULTIPLE \"%s\" servers running.\n"), s->name));
#ifndef PC
			  LOG((lmtext("Please kill, and run lmreread\n")));
			  LOG(("\n"));
			  LOG((lmtext("This error probably results from either:\n")));
			  LOG((lmtext("  1. Another copy of lmgrd running\n")));
			  LOG((lmtext("  2. A prior lmgrd was killed with \"kill -9\"\n")));
			  LOG((lmtext("      (which would leave the vendor daemon running)\n")));
			  LOG((lmtext("To correct this, do a \"ps -ax | grep %s\"\n"),
							s->name));
			  LOG((lmtext("  (or equivalent \"ps\" command)\n")));
			  LOG((lmtext("and kill the \"%s\" process\n"), s->name));
			  LOG(("\n"));
#else
			  LOG(("Using Task Manager, stop %s and run lmreread\n", s->name));
			  LOG(("\n"));
			  LOG((lmtext("This error probably results from:\n")));
			  LOG((lmtext("  1. Another copy of lmgrd running\n")));
			  LOG((lmtext("  2. Another copy of the vendor daemon is running.\n")));
			  LOG((lmtext("  3. The locking mechanism(s) were removed by an\n")));
			  LOG((lmtext("       outside program.\n")));
			  LOG(("\n"));
#endif /* PC */

			  LOG_INFO((INFORM, "The license daemon has detected \
				that multiple copies of the specified vendor \
				daemon are running.  The user should kill all \
				running copies of this vendor daemon and then \
				run lmreread to re-start the vendor daemon."));
			  s->pid = 0;
			  restart = 0;
			}
			else if (ret == EXIT_REQUESTED)
			{
			  s->pid = 0;
			  restart = 0;		/* Don't do anything */
			}
			else if (ret == EXIT_NOMASTER)
			{
				/* 
				 *  Code removed here in v2.4 to check
				 *  for loss of lmgrd quorum 
				 */
			}
			else if (ret == EXIT_BADCONFIG ||
				 ret == EXIT_BADDAEMON ||
				 ret == EXIT_BORROW_ERROR ||
				 ret == EXIT_CANTEXEC ||
				 ret == EXIT_MALLOC ||
				 ret == EXIT_EXPIRED ||
				 ret == EXIT_FEATSET_ERROR ||
				 ret == EXIT_NO_LICENSE_FILE ||
				 ret == EXIT_WRONGHOST ||
				 (s->num_recent_restarts > 10)
				 )
			{	 /* Don't restart in any of these cases */
			  s->pid = 0;
			  restart = 0;
			  LOG((lmtext("Please correct problem and restart daemons\n")));
			  LOG_INFO((INFORM, "The license daemon has reported \
				an abnormal condition with one of the vendor \
				daemons.  Correct the original problem and \
				re-run the license daemon at a convenient \
				time."));
			}
			else
			{
				LOG((lmtext("Vendor daemon died with status %d\n"),
								ret));
				LOG((lmtext("Since this is an unknown status, lmgrd will\n")));
				LOG((lmtext("attempt to re-start the vendor daemon.\n")));
			}

/*
 *			If we haven't ruled out a restart, do it now.
 */
			master = ls_s_master();
			if (*master) havemaster = 1; else havemaster = 0;
			if (restart && havemaster && havequorum && 
								!_ls_going_down)
			{
#ifdef DEBUG_NORESTART
			    LOG((lmtext("Would REStart %s (old tcp port: %d)\n"),
						s->name, s->tcp_port));
#else
#if 1
			    if (s->file_tcp_port <= 0) s->tcp_port = 0;
#endif
			    ls_startup(s, ls_our_hostname, 
								ls_s_master());
			    if (CHILDPID(s->pid))
			    {
			 
#ifdef PC
                                if (s->pid & 0xffff0000)
                                {
                                        LOG((lmtext("REStarted %s (pid %X)\n"),
                                        s->name, s->print_pid));
                                }
                                else
                                {
                                        LOG((lmtext("REStarted %s (pid %d)\n"),
                                        s->name, s->print_pid));
                                }
#else						
			        LOG((lmtext(
                        "REStarted %s (internet tcp_port %d pid %d)\n"),
                                        s->name, s->tcp_port, s->pid));
#endif
			        LOG_INFO((INFORM, "The license daemon has \
				restarted the specified vendor daemon at \
				the internet port number specified."));
				if (ls_s_imaster() && ls_s_havequorum())
				{
				    s->m_tcp_port = s->tcp_port;
				    for (ls = ls_s_first(); ls; ls = ls->next)
				    {
					if (!(ls->sflags & L_SFLAG_US)
						&& (ls->fd1 != LM_BAD_SOCKET)
						) 
					  ls_daemons(ls->fd1, master_daemons);
				    }
				}
				else
				    s->m_tcp_port = 0;
			    }
#endif
			}
			else
				s->pid = 0; /* not restarted */
		    }
		}
	}
}


