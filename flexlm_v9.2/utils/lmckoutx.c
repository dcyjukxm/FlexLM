/******************************************************************************

	    COPYRIGHT (c) 1996, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lmckoutx.c,v 1.9 2003/01/13 21:58:47 kmaclean Exp $
 *
 *	Function:	stub functions for a node-locked checkout that is as 
 *			small as possible
 *
 *	Return:		All functions take no args and return nothing
 *
 *	D. Birns
 *	7/16/96
 *
 *	Last changed:  12/20/98
 *
 */

/* l_any_set */
l_any_set() {}

/* l_basic_conn */
l_basic_conn() {}

/* l_check */
l_check_conf() {}
l_checkoff() {}
l_dump_check_tables() {}
l_featon() {}
l_upgrade_feat() {}
lc_auth_data() {}
l_auth_data() {}
lc_check() {}
lc_idle() {}
lc_status() {}
lc_timer() {}

/* l_cksum_ok */
l_cksum_ok() {}
l_get_cksum() {}


/* l_conn_msg */
l_conn_msg() {}
l_fillin_platform() {}

/* l_connect */
l_connect() {}
l_connect_by_conf() {}
l_connect_endpoint() {}
l_connect_host_or_list() {}

/* l_rcvmsg */
l_clr_rcv_queue() {}
l_heartbeat() {}
l_msgrdy() {}
l_rcvheart() {}
l_rcvmsg() {}
l_rcvmsg_str() {}
l_rcvmsg_timeout() {}
l_rcvmsg_type() {}
l_read_timeout() {}

/* l_timers */
l_timer_add() {}
l_timer_change() {}
l_timer_delete() {}
l_timer_job_done() {}
l_timer_signal() {}
lc_alarm() {}
lc_disalarm() {}
lc_nap() {}
lc_sleep() {}



/* lm_disconn */
lc_disconn() {}

/* l_sndmsg */
l_sndmsg() {}
l_resend_cmd() {}

/* l_finder */
l_finder() {}
l_finder_port() {}

/* l_serv_msg.o */
l_get_conf_from_server () {}
l_get_dlist_from_server () {}
l_get_featlist_from_server () {}
l_serv_msg () {}

/* l_handshake */
l_handshake() {}

/* lm_free_job */
l_free_list() {}
l_free_conf () {}
l_free_job_conf () {}
l_free_job_lf_pointers () {}
l_free_job_license () {}
l_free_job_servers() {}
l_free_server () {}
lc_free_hostid () {}
lc_free_job () {}
l_free_job_meters() {}

/* l_inet */
l_addr_to_inet() {}
l_inet_to_addr() {}
l_inet_cmp() {}

/* l_supersede */
l_supersede() {}

/* l_package */
dump_config() {}
dump_configs() {}
l_parse_component() {}
l_parse_package() {}
l_post_pkg() {}

/* lm_gethostid */
lc_gethostid () {}
l_gethostid () {}

/* l_fdata */
l_free_job_featdata() {}
l_more_featdata() {}

/* l_modify */
l_modify () {}

/*char *l_key_callback;
char *l_key_gen_callback;*/

