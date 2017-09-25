#ifndef _LM_MASTER_H_
#define _LM_MASTER_H_

/******************************************************************************

	    COPYRIGHT (c) 1988, 1996 by Macrovision Corporation.
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
 *	Module: $Id: lsmaster.h,v 1.6 2002/09/30 20:43:59 jwong Exp $
 *
 *	Description: Local definitions for master daemon
 *
 *	M. Christiano
 *	2/22/88
 *
 *	Last changed:  10/23/96
 *
 */

/*
 *	DAEMONS that we have started
 */


typedef struct _daemon {
			char name[MAX_DAEMON_NAME + 1];
			char *path;	/* Path to this DAEMON executable */
			int tcp_port;	/* TCP Internet Port number (local)*/
			int udp_port;	/* UDP Internet Port number (local)*/
			int m_tcp_port;	/* TCP Port number (on master) */
			int m_udp_port;	/* UDP Port number (on master) */
			int pid;	/* pid of this daemon */
			int print_pid; /* this is different on PC */
			int recommended_transport; /*LM_TCP or LM_UDP*/
			int transport_reason; /*LM_RESET_BY_USER/APPL*/
			long time_started; /* last time started */
			int num_recent_restarts;
			struct _daemon *next;
#ifdef SUPPORT_IPX
			int spx_port;	/* SPX Port number (local)*/ 		
			int m_spx_port;	/* SPX Port number (on master) */
#endif /* SUPPORT_IPX */
			int file_tcp_port;	/* port on DAEMON line */
			char *options_path; /* path to end-user options file */
			int flex_ver;
			int flex_rev;
			
			
		  } DAEMON;

/*
 * if (imaster) then tcp_port == m_tcp_port, and same for udp_port
 * If another node is master, then they will differ.
 * It takes an LM_DAEMON message from another lmgrd to tell lmgrd that
 *    about these (master) ports. 
 *					-- daniel 4/93 
 */
void 		l_print_daemon	lm_args(( DAEMON *, char *));
struct _daemon * API_ENTRY l_cur_dlist lm_args((LM_HANDLE *job));
lm_extern DAEMON * API_ENTRY l_get_dlist lm_args((LM_HANDLE *job));
lm_extern void API_ENTRY l_free_daemon_list lm_args((LM_HANDLE_PTR job,
                                                      DAEMON *sp));

DAEMON * API_ENTRY l_get_dlist_from_server lm_args((LM_HANDLE *));
#endif /*  _LM_MASTER_H_  */
