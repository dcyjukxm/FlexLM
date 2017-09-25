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
 *	Module: $Id: ls_sprot.h,v 1.15 2003/01/13 22:31:36 kmaclean Exp $
 *
 *	D. Birns
 *	4/22/93
 *
 *	Last changed:  9/2/97
 *
 */

#ifndef _LS_SERVPROT_H_
#define _LS_SERVPROT_H_
#include "lmselect.h"
#include "lmclient.h"
#include "lsserver.h"
#include "ls_log.h"
#ifdef WINNT
#include <stdlib.h>
#include <string.h>
#endif /* WINNT */
#ifndef _LS_REPLOG_H
typedef unsigned int DICTKEY;
#endif
/*
 *	prototypes for CLIENT_DB functions
 */
void		f_nousers	lm_args((lm_noargs));
void 		ls_delete_client lm_args((CLIENT_ADDR *ca));
#ifdef ANSI
void 		ls_all_clients (int(*callback)(CLIENT_DATA *cd, void *data),
								void *data);
#else
void 		ls_all_clients();
#endif
CLIENT_ADDR * 	ls_addr_by_handle lm_args((HANDLE_T handle));
CLIENT_DATA * 	ls_add_client 	lm_args((CLIENT_ADDR *ca));
void 		ls_append_repfile lm_args((char *));
#ifdef NO_REDUNDANT_SERVER
#define		ls_badserv(fd)
#else
void		ls_badserv 	lm_args((int fd));
#endif /* NO_REDUNDANT_SERVER */
int 		ls_ca_cmp 	lm_args((CLIENT_ADDR *ca1, CLIENT_ADDR *ca2));
void 		ls_ca_copy 	lm_args((CLIENT_ADDR *ca1, CLIENT_ADDR *ca2));
CLIENT_DATA * 	ls_c_init 	lm_args(( CLIENT_ADDR *ca, int comm_revision));
void 		ls_client_add_hostid lm_args((CLIENT_DATA *, HOSTID *));
CLIENT_DATA * 	ls_client_by_handle lm_args((HANDLE_T handle));
void		ls_client_send 	lm_args(( CLIENT_DATA *c, int msgtype, 
				char *msg));
void 		ls_client_send_str lm_args((CLIENT_DATA *c, char *str));
int		ls_close_repfile lm_args((lm_noargs));
HANDLE_T 	ls_create_handle lm_args((CLIENT_DATA *client));
void 		ls_decode_ca 	lm_args((char *from, CLIENT_ADDR *ca));
void 		ls_delete_handle lm_args((HANDLE_T handle));
unsigned int	ls_dict		lm_args((char *));
void		ls_dict_flush	lm_args((lm_noargs));
void		ls_dict_init	lm_args(());
void		ls_dict_stat	lm_args((char *));
void		ls_dlog_head 	lm_args((lm_noargs));
void 		ls_down 	lm_args(( LM_SOCKET *fd, char *msg));
void 		ls_encode_ca 	lm_args((char *where, CLIENT_ADDR *ca));
#ifdef VOID_SIGNAL
void
#else
int
#endif
		ls_exit		lm_args((int));
int 		ls_findbad 	lm_args((SELECT_MASK select_mask));
void 		ls_flush_replog lm_args((lm_noargs));
#ifdef THREAD_SAFE_TIME
struct tm *		ls_gettime  lm_args((struct tm * ptst));
#else /* THREAD_SAFE_TIME */
struct tm * 	ls_gettime 	lm_args((lm_noargs));
#endif 
void 		ls_go_down 	lm_args((int));
HANDLE_T 	ls_handle_by_addr lm_args((CLIENT_ADDR *addr));
HANDLE_T 	ls_handle_by_client lm_args((CLIENT_DATA *client));
CLIENT_DATA * 	ls_hash_client 	lm_args((CLIENT_ADDR *ca));
char * 		ls_hookup 	lm_args((char *cmd, CLIENT_ADDR *input_ca, 
					char *master, CLIENT_DATA *user, 
							/* A particular user */ 
					SELECT_MASK select_mask));
char * 		ls_i_master 	lm_args((LM_SERVER *ls, char *hostname));
int		ls_last_msg	lm_args((lm_noargs));
int 		ls_lf		lm_args((char *));
CONFIG * 	ls_lickey_to_conf lm_args((char *, char *));
/*
 * 	There doesn't seem to be any portable way to declare this in
 *	ansi style
 */
#ifdef ANSI
void            ls_log_asc_printf (char *fmt, ...);
#else						
void            ls_log_asc_printf ();
#endif /* ANSI */
void		ls_log_close_ascii lm_args((lm_noargs));
void		ls_log_close_report lm_args((int log_tail));
DICTKEY		ls_log_client lm_args(( CLIENT_DATA *, int));
#ifdef ANSI
void		ls_log_comment	lm_args((int where, char *fmt, ...));
void		ls_log_error	lm_args((char *fmt, ...));
#endif /* ANSI */
void		ls_log_cpu_usage lm_args((CLIENT_DATA *));
char * 		ls_log_daemon_name lm_args((lm_noargs));
void 		ls_log_endline 	lm_args((int where));
void 		ls_log_feature 	lm_args((int where, char *, LS_POOLED *,int,
					 int overdraft, int nres, 
					 char * version));
void 		ls_log_head 	lm_args((lm_noargs));
void 		ls_log_open_ascii lm_args((char *));
int		ls_log_open_report  lm_args((char *, int, CLIENT_DATA *));
void 		ls_log_prefix 	lm_args((int where, int key));
void		ls_log_reopen_ascii lm_args((char *filename));
int		ls_log_reopen_report lm_args((char *, CLIENT_DATA *, int));
char *		ls_log_report_name lm_args((lm_noargs));
char *		ls_log_ascii_name lm_args((lm_noargs));
void 		ls_log_res_usage lm_args((int where, long handle, int what,
					   char *licgroup, int restype, 
					   int nlic));
void 		ls_log_switchto	lm_args(( char *, CLIENT_DATA *));
void 		ls_log_switchfrom lm_args((char *));
void		ls_log_timestamp lm_args((int where));
void 		ls_log_usage 	lm_args((int where, long handle, long brhandle,
					  int what, int why, char *user, 
					  char *node, char *display, 
					  char *vendor_def, char *feature,
					  char *lickey,
					  LS_POOLED *pooled, 
					  char * version, int nlicreq, 
					  int nlicact, int reftime, 
					  CLIENT_DATA *who, int group, 
					  unsigned int flags, 
					int borrow_seconds));
void		ls_logtime	    lm_args((struct tm *t));
CLIENT_DATA * 	ls_lookup_client lm_args((CLIENT_ADDR *ca));
void 		ls_lost 	lm_args((int sig));
char * 		ls_malloc 	lm_args((unsigned size, int line, char *file));
void		ls_mask_clear	lm_args((int fd));
int		ls_mast_rdy 	lm_args(( LM_SERVER *ls, char *hostname));
void		ls_mk_flexlm_dir lm_args((lm_noargs));
void		ls_msg_dump 	lm_args(( char *, char *, CLIENT_ADDR *, int ));
CLIENT_DATA * 	ls_new_client_data lm_args((lm_noargs));
void    ls_oneless              lm_args((lm_noargs));
int 		ls_on_host 	lm_args(( char *which));
void		ls_pause	lm_args((int msec));
int 		ls_pick_mast	lm_args((lm_noargs));
void		ls_print_clients lm_args((lm_noargs));
void 		ls_process	lm_args((SELECT_MASK ready_mask,
					int nrdy, SELECT_MASK select_mask,
					LM_SOCKET *tcp_s, LM_SOCKET *spx_s, 
					 char *master_name, 
					int num_reserve));
void		ls_q_check	    lm_args((lm_noargs));
#ifdef NO_REDUNDANT_SERVER
#define		ls_quorum(master_list,master_name) 1
#else
int		ls_quorum 	lm_args(( LM_SERVER *master_list, 
					  char *master_name, int));
#endif /* NO_REDUNDANT_SERVER */
int		ls_readready 	lm_args(( CLIENT_ADDR *ca, int interval));
void		ls_recycle_control lm_args((lm_noargs));
void		ls_release_control lm_args((lm_noargs));
void 		ls_replog_cksum lm_args((char *));
void 		ls_replog_msg_vol lm_args((int));
void		ls_replog_cpu_usage lm_args((CLIENT_DATA *));
void 		ls_replog_delete_client lm_args(( CLIENT_DATA *));
void		ls_replog_end 	lm_args((lm_noargs));
void 		ls_replog_error lm_args((char *));
void 		ls_replog_flush	lm_args(( lm_noargs));
void 		ls_replog_res_usage lm_args(( char *, int, int, int, int));
void 		ls_replog_timestamp	lm_args(( lm_noargs));
void 		ls_replog_usage lm_args((long handle, long brhandle,
					  int what, int why, char *user, 
					  char *node, char *display, 
					  char *vendor_def, char *feature,
					  char *lickey,
					  LS_POOLED *pooled, 
					  char * version, int nlicreq, 
					  int nlicact, int reftime, 
					  CLIENT_DATA *who, int group,
					  unsigned int userflags, 
					  int linger_seconds));

void 		ls_resend_last_resp lm_args((CLIENT_DATA *));
LM_SOCKET	ls_sconnect 	lm_args(( char *node, int port, long *exptime));
void 		ls_send 	lm_args(( int server_num, int type, 
					char *data));
char * 		ls_serv_conn 	lm_args(( CLIENT_ADDR *ca, char *msg, 
					  CLIENT_DATA *client));
int 		ls_serv_time 	lm_args(( SELECT_MASK select_mask));
int 		ls_serv_receive lm_args(( CLIENT_ADDR *ca, char *msgtype, 
			    char **msg));
LM_SOCKET       ls_socket       lm_args(( char *, COMM_ENDPOINT_PTR, int  ));
void 		ls_sock_close 	lm_args(( LM_SOCKET *fd, char *msg));
void		ls_store_socket lm_args((LM_SOCKET, LM_SOCKET, LM_SOCKET));
void		ls_s_down	lm_args((LM_SERVER *ls));
#ifdef NO_REDUNDANT_SERVER
#define		ls_s_dump()
#else
void 		ls_s_dump();
#endif /* NO_REDUNDANT_SERVER */
LM_SERVER *	ls_s_find 	lm_args(( LM_SOCKET fd1));
LM_SERVER *	ls_s_first();
int		ls_s_havemaster();
#ifdef NO_REDUNDANT_SERVER
#define		ls_s_havequorum() 1			
#define		ls_s_imaster()	1			
#define		ls_s_init()				
#else
int		ls_s_havequorum();
int		ls_s_imaster();
void		ls_s_init();
#endif /* NO_REDUNDANT_SERVER */
char *		ls_s_master();
int		ls_s_masterup();
void		ls_s_mready	lm_args(( char *cmd, CLIENT_DATA *user));
void		ls_s_order	lm_args(( char *cmd));
void		ls_s_pconn	lm_args(( CLIENT_ADDR *ca));
int		ls_s_qnum();
void		ls_s_reset();
void		ls_s_setmaster	lm_args(( int i));
void		ls_s_sconn	lm_args(( LM_SERVER *ls, CLIENT_ADDR *ca));
#ifdef NO_REDUNDANT_SERVER
#define		ls_s_shut_if_client(client)	0
#else
int		ls_s_shut_if_client lm_args(( CLIENT_DATA *client));
#endif /* NO_REDUNDANT_SERVER */
void		ls_server_send 	lm_args(( CLIENT_ADDR *, int , char *));
void		ls_statfile_rm 	lm_args((lm_noargs));
int		ls_udp_read	lm_args((char *buf, int size,CLIENT_ADDR *ca));
int		ls_udp_send	lm_args((LM_SOCKET s, struct sockaddr_in *sock,
							 char *buf, int size));
void		ls_unlock	lm_args((int ));
int 		ls_user		lm_args(( CLIENT_DATA *));
int		ls_wakeup	lm_args((int secs, int *flag));
void 		ls_wpipe 	lm_args(( int *, char * ));
#ifdef WINNT 
int		nt_connect( LM_SOCKET s, struct sockaddr *sock, int size );
#endif
#ifdef SUNOS4
extern time_t time();
#endif
#endif /* _LS_SERVPROT_H_ */
