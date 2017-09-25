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
 *	Module: $Id: l_m_prot.h,v 1.8 2003/01/13 22:26:49 kmaclean Exp $
 *
 *	Description: Prototypes for master directory functions
 *
 *	D. Birns
 *	3/94
 *
 *	Last changed:  10/18/95
 *
 */

#ifndef _L_M_PROT_H
#define _L_M_PROT_H

int	ls_bind_socket		lm_args((LM_SOCKET *tcp_s,LM_SOCKET *spx_s,
					 LM_SERVER *master));
#ifdef  SIGNAL_NOT_AVAILABLE
void	ls_chld_died		lm_args((int sig, int exit_code));
#else
#ifdef VOID_SIGNAL
void
#endif /* VOID_SIGNAL */
	ls_chld_died		lm_args((int sig));
#endif /* SIGNAL_NOT_AVAILABLE */ 
void	ls_daemons		lm_args((int fd, DAEMON *daemons));
void	ls_finder		lm_args((char *config, SELECT_MASK *));
char	*ls_get_lfpath		lm_args((char *, char *, char *, char *,
								char *));
int 	ls_is_local		(void);
int 	ls_kill_chld		(int sig);
void	ls_m_data		lm_args((lm_noargs));
int	ls_m_init		lm_args((int argc, char *argv[],
					 LM_SERVER **list, int reread_flag));
					 
void	ls_m_main		lm_args(( DAEMON **daemons,
						SELECT_MASK *select_mask));
int	ls_m_process		lm_args((SELECT_MASK ready_mask,
					 int nrdy, SELECT_MASK select_mask,
					 LM_SOCKET tcp_s, LM_SOCKET spx_s,
					 DAEMON **daemon, char *master));
void	ls_send_reread		lm_args((DAEMON *daemon));
int	ls_send_shutdown	lm_args((DAEMON *target_dm));
void	ls_startup		lm_args((DAEMON *daemon, char *hostname, 
					 char *master_name));
void 	ls_statfile		lm_args((lm_noargs));
void 	ls_statfile_argv0	lm_args((char *));
void 	ls_statfile_rm		lm_args((lm_noargs));
void	ls_timestamp		lm_args((lm_noargs));

#ifdef WINNT
void	main_service_thread(int argc, char *argv[]);   
#endif /* WINNT */


#endif /* _L_M_PROT_H */
