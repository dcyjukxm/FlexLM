/******************************************************************************

	    COPYRIGHT (c) 1996, 2000 by Macrovision Corporation.
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
 *	Module: $Id: lmstrip.c,v 1.23 2003/05/05 15:58:04 sluu Exp $
 *
 *	Function:	Strip names from a binary
 *
 *	Description:    Run this program on executables license by FLEXlm.
 *			It strips important strings from the program.
 *	
 *			CAUTION:  DO NOT RUN this if a shared library
 *			makes calls to any FLEXlm functions.  If you do,
 *			the program won't run, so this won't be hard to 
 *			detect.
 *
 *	Arguments:
 *			lmstrip file [ -l ] [ -n | -N ] [ strings ... ]
 *			file:   to be stripped -- replaced
 *			-l:	list internal and external strings that 
 *				get stripped
 *			-e:	don't use external table.
 *			-n:	don't use internal and external table.
 *			-v:	verbose
 *			-z:	replace strings with zeroes.
 *			strings: Added to list to be stripped.
 *
 *	Return:
 *
 *	D. Birns
 *	4/18/96
 *
 *	Last changed:  11/2/98
 *
 */
#include "lmclient.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>


#include <stdio.h>
#define MAXNAMES 10000
#ifdef PC
#define FOPEN_READ "rb"
#define FOPEN_WRITE "wb"
#else
#define FOPEN_READ "r"
#define FOPEN_WRITE "w"
#endif

typedef struct namelist {
	char *name;
	char *randname;
} NAMELIST;
NAMELIST user_names[MAXNAMES];
NAMELIST external_names[] =  {
	"f_add", 0,
	"f_checklinger", 0,
	"f_count", 0,
	"f_dequeue", 0,
	"f_drop", 0,
	"f_drop_client", 0,
	"f_dump", 0,
	"f_featcount", 0,
	"f_get_feat", 0,
	"f_get_feat_next", 0,
	"f_list", 0,
	"f_lookup", 0,
	"f_nousers", 0,
	"f_pooled", 0,
	"f_queue", 0,
	"f_remove", 0,
	"f_remove_all", 0,
	"f_remove_all_old", 0,
	"f_remove_children", 0,
	"f_user_remove", 0,
	"f_user_remove_handle", 0,
	"la_lmgrds", 0,
	"la_init", 0,
	"l_add_key_filter", 0,
	"lc_add_key_filter", 0,
	"lc_alarm", 0,
	"lc_auth_data", 0,
	"l_auth_data", 0,
    "lc_baddate", 0,
	"l_bios_id", 0,
	"lc_check", 0,
	"l_check", 0,
	"l_check_conf", 0,
	"lc_check_key", 0,
	"l_check_key", 0,
	"lc_checkin", 0,
	"l_checkin", 0,
	"lc_checkout", 0,
	"l_checkout", 0,
	"lc_chk_conf", 0,
	"l_chk_conf", 0,
	"lc_ck_feats", 0,
	"l_ck_feats", 0,
	"lc_cleanup", 0,
	"l_cleanup", 0,
	"l_composite_id", 0,
	"lc_convert", 0,
	"l_convert", 0,
	"lc_copy_hostid", 0,
	"l_copy_hostid", 0,
	"l_cpu_id", 0,
	"lc_disconn", 0,
	"l_disconn", 0,
	"l_disk_geometry_id", 0,
	"lc_display", 0,
	"l_display", 0,
	"lc_err_info", 0,
	"l_err_info", 0,
	"lc_errstring", 0,
	"l_errstring", 0,
	"lc_errtext", 0,
	"l_errtext", 0,
	"lc_expire_days", 0,
	"l_expire_days", 0,
	"lc_feat_list", 0,
	"l_feat_list", 0,
	"lc_feat_set", 0,
	"l_feat_set", 0,
	"lc_first_job", 0,
	"l_first_job", 0,
	"lc_free_hostid", 0,
	"l_free_hostid", 0,
	"lc_free_job", 0,
	"l_free_job", 0,
	"lc_free_lmgrd_stat", 0,
	"l_free_lmgrd_stat", 0,
	"lc_free_mem", 0,
	"l_free_mem", 0,
	"lc_get_attr", 0,
	"l_get_attr", 0,
	"lc_get_config", 0,
	"l_get_config", 0,
	"lc_get_errno", 0,
	"l_get_errno", 0,
	"lc_get_feats", 0,
	"l_get_feats", 0,
	"lc_get_registry", 0,
	"l_get_registry", 0,
	"lc_gethostid", 0,
	"l_gethostid", 0,
	"lc_getid_type", 0,
	"l_getid_type", 0,
	"l_good_lic_key", 0,
	"lc_heartbeat", 0,
	"l_heartbeat", 0,
	"lc_hostid", 0,
	"l_hostid", 0,
	"lc_hostname", 0,
	"l_hostname", 0,
	"lc_hosttype", 0,
	"l_hosttype", 0,
	"lc_idle", 0,
	"l_idle", 0,
	"lc_init", 0,
	"l_init", 0,
	"l_free_keylist", 0,
	"lc_lic_where", 0,
	"l_lic_where", 0,
	"l_init_simple_composite_id", 0,
	"lc_init_simple_composite", 0,
	"lc_install_license", 0,
	"l_install_license", 0,
	"lc_install_license_path", 0,
	"l_install_license_path", 0,
	"lc_is_admin", 0,
	"l_is_admin", 0,
	"lc_log", 0,
	"l_log", 0,
	"lc_master_list", 0,
	"l_master_list", 0,
	"lc_new_job", 0,
	"l_new_job", 0,
	"lc_next_conf", 0,
	"l_next_conf", 0,
	"lc_next_job", 0,
	"l_next_job", 0,
	"lc_perror", 0,
	"l_perror", 0,
	"lc_set_attr", 0,
	"l_set_attr", 0,
	"lc_set_errno", 0,
	"l_set_errno", 0,
	"lc_set_registry", 0,
	"l_set_registry", 0,
	"lc_status", 0,
	"l_status", 0,
	"lc_test_conf", 0,
	"l_test_conf", 0,
#if 0 /*P1518 */
	"lc_timer", 0,
	"l_timer_heart", 0,	/*	Not added for same reason as P1518.	*/
#endif
	"l_timer", 0,
	"lc_userlist", 0,
	"l_userlist", 0,
	"lc_username", 0,
	"l_username", 0,
	"lc_vsend", 0,
	"l_vsend", 0,
	"l_x77_buf", 0,
	"l_xorname", 0,
	"lp_checkin", 0,
	"lp_checkout", 0,
	"lp_errstring", 0,
	"lp_heartbeat", 0,
	"lp_perror", 0,
	"lp_pwarn", 0,
	"lp_warning", 0,
	"l_new_hostid", 0,
	"l_extract_date", 0,
	"l_cur_dlist", 0,
	"l_free_daemon_list", 0,
	"l_get_dlist", 0,
	"l_get_dlist_lfp", 0,
	"l_master_list_from_job", 0,
	"l_print_conf_dec", 0,
	"l_print_config", 0,
	"l_text", 0,
	"la_free_daemon", 0,
	"la_parse_daemon", 0,
	"lc_daemon", 0,
	"lc_disalarm", 0,
	"lc_get_redir", 0,
	"lc_isadmin", 0,
	"lc_nap", 0,
	"lc_print_config", 0,
	"lc_sleep", 0,
	"lc_xstrcmp", 0,
	"ls_add_client", 0,
	"ls_addr_by_handle", 0,
	"ls_all_clients", 0,
	"ls_app_init", 0,
	"ls_append_repfile", 0,
	"ls_attr_setup", 0,
	"ls_badserv", 0,
	"ls_borrow_check", 0,
	"ls_build_opt", 0,
	"ls_c_init", 0,
	"ls_ca_copy", 0,
	"ls_checkhost", 0,
	"ls_checkin", 0,
	"ls_checklock", 0,
	"ls_checkout", 0,
	"ls_checkroot", 0,
	"ls_ck_feats", 0,
#if 0
	"ls_ck_udp_clients", 0,
#endif
	"ls_client_add_hostid", 0,
	"ls_client_by_handle", 0,
	"ls_client_send", 0,
	"ls_client_send_str", 0,
	"ls_close_repfile", 0,
	"ls_closeout_this_tcp_client", 0,
	"ls_closeout_this_udp_client", 0,
#if 0
	"ls_control", 0,
#endif
	"ls_create_handle", 0,
	"ls_daemon", 0,
	"ls_decode_ca", 0,
	"ls_delete_client", 0,
	"ls_delete_handle", 0,
	"ls_dict", 0,
	"ls_dict_flush", 0,
	"ls_dict_init", 0,
	"ls_dict_lookup_str", 0,
	"ls_dict_stat", 0,
	"ls_disown", 0,
	"ls_docmd", 0,
	"ls_down", 0,
	"ls_encode_ca", 0,
	"ls_exclude", 0,
	"ls_exit", 0,
	"ls_feat_checkout", 0,
	"ls_feat_dump", 0,
	"ls_feat_info", 0,
	"ls_feat_prune", 0,
	"ls_feat_start_validate", 0,
	"ls_findbad", 0,
	"ls_flush_replog", 0,
	"ls_get_attr", 0,
	"ls_get_info", 0,
	"ls_get_opts", 0,
	"ls_gettime", 0,
	"ls_go_down", 0,
	"ls_handle_by_addr", 0,
	"ls_handle_by_client", 0,
	"ls_hookup", 0,
	"ls_host", 0,
	"ls_i_fhostid", 0,
	"ls_i_master", 0,
	"ls_include", 0,
	"ls_ingroup", 0,
	"ls_init_feat", 0,
	"ls_is_duplicate", 0,
	"ls_last_msg", 0,
	"ls_lf", 0,
	"ls_lickey_to_conf", 0,
	"ls_list", 0,
	"ls_log_asc_printf", 0,
	"ls_log_ascii_name", 0,
	"ls_log_client", 0,
	"ls_log_close_ascii", 0,
	"ls_log_close_report", 0,
	"ls_log_comment", 0,
	"ls_log_cpu_usage", 0,
	"ls_log_daemon_name", 0,
	"ls_log_endline", 0,
	"ls_log_error", 0,
	"ls_log_msg_cnt", 0,
	"ls_log_open_ascii", 0,
	"ls_log_open_report", 0,
	"ls_log_prefix", 0,
	"ls_log_reopen_ascii", 0,
	"ls_log_reopen_report", 0,
	"ls_log_report_name", 0,
	"ls_log_res_usage", 0,
	"ls_log_switchfrom", 0,
	"ls_log_switchto", 0,
	"ls_log_timestamp", 0,
	"ls_log_usage", 0,
	"ls_logtime", 0,
	"ls_lookup_client", 0,
	"ls_lost", 0,
	"ls_main", 0,
	"ls_malloc", 0,
	"ls_mask_clear", 0,
	"ls_mast_rdy", 0,
	"ls_max_exceeded", 0,
	"ls_mk_flexlm_dir", 0,
	"ls_msg_dump", 0,
	"ls_on_host", 0,
	"ls_oneless", 0,
	"ls_pause", 0,
	"ls_pool", 0,
	"ls_pooled_check", 0,
	"ls_print_clients", 0,
	"ls_print_feats", 0,
	"ls_process", 0,
	"ls_q_check", 0,
	"ls_quorum", 0,
	"ls_readready", 0,
	"ls_receive", 0,
	"ls_recycle_control", 0,
	"ls_release_control", 0,
	"ls_replog_cksum", 0,
	"ls_replog_comment", 0,
	"ls_replog_cpu_usage", 0,
	"ls_replog_delete_client", 0,
	"ls_replog_end", 0,
	"ls_replog_error", 0,
	"ls_replog_msg_vol", 0,
	"ls_replog_res_usage", 0,
	"ls_replog_timestamp", 0,
	"ls_replog_usage", 0,
	"ls_reread", 0,
#if 0
	"ls_reread_sig", 0,
#endif
	"ls_resend_last_resp", 0,
	"ls_s_down", 0,
	"ls_s_dump", 0,
	"ls_s_fd2_down", 0,
	"ls_s_find", 0,
	"ls_s_first", 0,
	"ls_s_havemaster", 0,
	"ls_s_havequorum", 0,
	"ls_s_imaster", 0,
	"ls_s_init", 0,
	"ls_s_master", 0,
	"ls_s_masterup", 0,
	"ls_s_mready", 0,
	"ls_s_order", 0,
	"ls_s_pconn", 0,
	"ls_s_qnum", 0,
	"ls_s_reset", 0,
	"ls_s_sconn", 0,
	"ls_s_setmaster", 0,
	"ls_s_shut_if_client", 0,
	"ls_sconnect", 0,
	"ls_send", 0,
	"ls_send_user", 0,
	"ls_send_vendor_def", 0,
	"ls_sendup", 0,
	"ls_serv_conn", 0,
	"ls_serv_receive", 0,
	"ls_serv_time", 0,
	"ls_server_send", 0,
	"ls_setlock", 0,
	"ls_shakehand", 0,
#if 0
	"ls_shutdown", 0,
	"ls_sigchld", 0,
#endif
	"ls_since", 0,
	"ls_sock_close", 0,
	"ls_socket", 0,
	"ls_statfile_rm", 0,
	"ls_store_master_list", 0,
	"ls_store_socket", 0,
	"ls_time_line", 0,
	"ls_udp_read", 0,
	"ls_udp_send", 0,
	"ls_unlock", 0,
	"ls_user", 0,
	"ls_user_based", 0,
	"ls_user_lookup", 0,
	"ls_user_or_hostbased", 0,
	"ls_vd_info", 0,
	"ls_wakeup", 0,
	"ls_wpipe", 0,
	0, 0};
NAMELIST internal_names[] = {
#if 0 /* P2392 -- global non-functions cause problems on some OSs */
	"_lm_errlist", 0,
	"_lm_nerr", 0,
#endif
	"key_filter", 0,
	"leapyear", 0,
	"l_addr_to_inet", 0,
	"l_allfeat", 0,
	"l_any_set", 0,
	"l_asc_date", 0,
	"l_asc_hostid", 0,
	"l_asc_hostid_len", 0,
	"l_asc_id_one", 0,
	"l_auth_data", 0,
	"l_baddate", 0,
	"l_basic_conn", 0,
	"l_bin_date", 0,
	"l_block_sigalarm", 0,
	"l_br", 0,
	"l_bufg_64", 0, 
	"l_buf_64", 0, 
	"l_c", 0,
	"l_check", 0,
	"l_check_fmt_ver", 0,
	"l_check_fmt_ver_hostids", 0,
	"l_checkoff", 0,
	"l_chkdir", 0,
	"l_ckout_crypt", 0,
	"l_ckout_string_key", 0,
	"l_cksum", 0,
	"l_cksum_ok", 0,
	"l_clear_error", 0,
	"l_clr_rcv_queue", 0,
	"l_compare_version", 0,
	"l_conf_copy", 0,
	"l_config", 0,
	"l_conn_msg", 0,
	"l_connect", 0,
	"l_connect_by_conf", 0,
	"l_connect_endpoint", 0,
	"l_connect_host_or_list", 0,
	"l_crypt_private", 0,
	"l_date", 0,
	"l_date_compare", 0,
	"l_date_to_time", 0,
	"l_days_ascdate", 0,
	"l_decode_16bit_packed", 0,
	"l_decode_32bit_packed", 0,
	"l_debug_datastr", 0,
	"l_decimal_format", 0,
	"l_decode_int", 0,
	"l_decode_long", 0,
	"l_decode_long_hex", 0,
	"l_do_redir", 0,
	"l_dump_check_tables", 0,
	"l_enable_pkg_feats", 0,
	"l_encode_16bit_packed", 0,
	"l_encode_32bit_packed", 0,
	"l_encode_int", 0,
	"l_encode_long", 0,
	"l_encode_long_hex", 0,
	"l_err_info_cp", 0,
	"l_ether_id", 0,
	"l_ether_str_to_num", 0,
	"l_exitcall", 0,
	"l_extract_date", 0,
	"l_feat_find", 0,
	"l_feat_find_first_empty", 0,
	"l_featon", 0,
	"l_file_check", 0,
	"l_file_checkin", 0,
	"l_file_checkout", 0,
	"l_file_checkq", 0,
	"l_file_log", 0,
	"l_file_remove", 0,
	"l_file_removeh", 0,
	"l_file_sdata", 0,
	"l_file_userlist", 0,
	"l_files_in_dir", 0,
	"l_fillin_platform", 0,
	"l_find_msg", 0,
	"l_finder", 0,
	"l_finder_port", 0,
	"l_flush_config", 0,
	"l_free", 0,
	"l_free_conf", 0,
	"l_free_err_info", 0,
    	"l_free_ids", 0, 
	"l_free_job_conf", 0,
	"l_free_job_featdata", 0,
	"l_free_job_lf_pointers", 0,
	"l_free_job_license", 0,
	"l_free_job_servers", 0,
	"l_free_one_hostid", 0,
	"l_free_server", 0,
	"l_get_attr", 0,
	"l_get_cksum", 0,
	"l_get_conf_from_server", 0,
	"l_get_date", 0,
	"l_get_dec_daemon", 0,
	"l_get_dlist_from_server", 0,
	"l_get_endpoint", 0,
	"l_get_featlist_from_server", 0,
	"l_gethostid", 0,
	"l_get_id", 0,
	"l_get_lfile", 0,
	"l_get_one_id", 0,
    	"l_get_registry", 0, 
	"l_getadr", 0,
	"l_getattr", 0,
	"l_getattr_init", 0,
	"l_getdomain", 0,
	"l_getenv", 0,
	"l_getenv_clear", 0,
	"l_getexp", 0,
	"l_gethostid", 0,
	"l_getid_hw", 0,
	"l_getid_type", 0,
	"l_getlastfile", 0,
	"l_getnextfile", 0,
	"l_good_bin_date", 0,
	"l_handshake", 0,
	"l_has_floating_lic", 0,
	"l_has_nodelock_lic", 0,
	"l_hbs", 0,
	"l_heartbeat", 0,
	"l_host", 0,
	"l_host_one", 0,
	"l_hostid_cmp", 0,
	"l_hostid_cmp_exact", 0,
	"l_hostid_cmp_one", 0,
	"l_hostid_copy", 0,
	"l_icf", 0,
	"l_id_types_match", 0,
	"l_inet_cmp", 0,
	"l_inet_to_addr", 0,
	"l_init", 0,
	"l_init_file", 0,
	"l_install_license", 0,
	"l_int_month", 0,
	"l_ipaddr", 0,
	"l_key", 0,
	"l_keyword_eq", 0,
	"l_keyword_eq_n", 0,
	"l_la_init", 0,
	"l_lf_get_token", 0,
	"l_lf_put_token", 0,
	"l_lfclose", 0,
	"l_lfgets", 0,
	"l_lfseek", 0,
	"l_lmgrd_running", 0,
	"l_lmgrds", 0,
	"l_local_verify_conf", 0,
	"l_lowercase", 0,
	"l_make_tcp_socket", 0,
	"l_make_udp_socket", 0,
	"l_malloc", 0,
	"l_malloc_errstring", 0,
	"l_master_list_lfp", 0,
	"l_mem_free", 0,
	"l_mem_malloc", 0,
	"l_midnight", 0,
	"l_modify", 0,
	"l_more_featdata", 0,
	"l_movelong", 0,
	"l_msg_cksum", 0,
	"l_msg_size", 0,
	"l_msgrdy", 0,
	"l_n36_buf", 0,
	"l_n36_buff", 0,
	"l_new_hostid", 0,
	"l_next_conf_or_marker", 0,
	"l_nap", 0,
	"l_now", 0,
	"l_ok", 0,
	"l_onebyte", 0,
	"l_open_file", 0,
	"l_pack", 0,
	"l_pack_print", 0,
	"l_pack_unprint", 0,
	"l_parse_attr", 0,
	"l_parse_commtype", 0,
	"l_parse_component", 0,
	"l_parse_daemon", 0,
	"l_parse_decimal", 0,
	"l_parse_feature_line", 0,
	"l_parse_package", 0,
	"l_parse_server_line", 0,
	"l_pause", 0,
	"l_platform_name", 0,
	"l_post_pkg", 0,
	"l_queue_msg", 0,
	"l_queue_remove", 0,
	"l_rcvheart", 0,
	"l_rcvmsg", 0,
	"l_rcvmsg_real", 0,
	"l_rcvmsg_str", 0,
	"l_rcvmsg_timeout", 0,
	"l_rcvmsg_type", 0,
	"l_rcvmsg_type_timeout", 0,
	"l_rcvmsg_wrapper", 0,
	"l_read_lfile", 0,
	"l_read_sernum", 0,
	"l_read_timeout", 0,
	"l_real_getenv", 0,
	"l_reconnect", 0,
	"l_resend_cmd", 0,
	"l_reset_job_servers", 0,
	"l_select", 0,
	"l_select_one", 0,
	"l_send_hostid", 0,
	"l_sernum_to_32", 0,
	"l_serv_msg", 0,
	"l_set_error", 0,
	"l_set_error_path", 0,
	"l_set_license_path", 0,
	"l_set_user_init3", 0,
	"l_sg", 0,
	"l_sleep_cb", 0,
	"l_sndmsg", 0,
	"l_sort_configs", 0,
	"l_start_ok", 0,
	"l_str_crypt", 0,
	"l_str_dcrypt", 0,
	"l_string_key", 0,
	"l_strip_os_ver", 0,
	"l_sum", 0,
	"l_supersede", 0,
	"l_svk", 0,
	"l_timer_add", 0,
	"l_timer_change", 0,
	"l_timer_delete", 0,
	"l_timer_increment", 0,
	"l_timer_job_done", 0,
	"l_timer_msecs2time", 0,
	"l_timer_now", 0,
	"l_timer_reset", 0,
	"l_timer_route_or_set", 0,
	"l_timer_router", 0,
	"l_timer_set", 0,
	"l_timer_signal", 0,
	"l_timer_time2msecs", 0,
	"l_timer_to_tab", 0,
	"l_try_basic_conn", 0,
	"l_try_connect", 0,
	"l_unpack", 0,
	"l_update_license_file", 0,
	"l_update_license_path", 0,
	"l_update_mask", 0,
	"l_upgrade", 0,
	"l_upgrade_feat", 0,
	"l_uppercase", 0,
	"l_valid_job", 0,
	"l_valid_version", 0,
	"l_vendor_license_file", 0,
    	"l_vol_id", 0,
	"l_write_sernum", 0,
	"l_zcp", 0,
	"l_zinit", 0,
#ifdef PC /* P2392 -- global non-functions cause problems on some OSs */
	"lm_bpi", 0,
	"lm_max_masks", 0,
	"lm_nofile", 0,
#endif
	"lfgets_tokens", 0,
	"lfptr_eof", 0,
	"line_continuation", 0,
	"lm_start", 0,
	"lm_start_real", 0,
	"lm_debug_hostid", 0,
	"timer_proc@16", 0,
	"lc_timer_int", 0,
	"make_ver_str", 0,
	"FlexMsgBox", 0,
	"_lm_nerr", 0, 
	"l_get_port", 0,
	"pc_connect",0,
	"pc_connect",0,
	"outfilter",0,
	"user_crypt_filter", 0,
	"user_crypt_filter_gen", 0,
	"uninstall_signal", 0,
	"pselect", 0,
	"StripString", 0,
	0, 0};

int reading_mapfile;
int writing_mapfile;

int verbose = 0;
int randm = 1;

char *mapfile = "lmstrip.map";
void check lm_args((NAMELIST *, char *));
void setup lm_args((char *));
void strip lm_args((char *, int n, int, NAMELIST *));
int varprefix lm_args((char *));
int var1stchar lm_args((int));
int var2ndchar lm_args((int));
void err_exit lm_args((char *));
static void write_mapfile lm_args(( lm_noargs));
static void read_mapfile lm_args(( lm_noargs));
void strip_file lm_args((char *));
#define L_MAXWORD 16000
char matchword[L_MAXWORD];
char *mw_ptr;
char *files[1000];
int fidx;
int use_internal = 1;
int use_external = 1;
FILE *mapfp;





int
main(argc, argv)
char **argv;
{
 int numnames = 0;
 int i;
 NAMELIST *np;
 struct stat st;

	srand(time(0));
	for (i = 1; i<argc; i++)
	{
		if (*argv[i] == '-')
		{
			if (!strcmp(argv[i], "-l"))
			{
				printf("Internal names:\n");
				for (np = internal_names; np->name; np++)
					printf("\t%s\n", np->name);
				printf("\nExternal names:\n");
				for (np = external_names;np->name; np++)
					printf("\t%s\n", np->name);
				exit(0);
			}
			else if (!strcmp(argv[i], "-e"))
				use_external = 0;
			else if (!strcmp(argv[i], "-n"))
			{
				use_internal = use_external = 0;
			}
			else if (!strcmp(argv[i], "-v"))
				verbose = 1;
			else if (!strcmp(argv[i], "-z"))
				randm = 0;
			else if (!strcmp(argv[i], "-r"))
			{
				printf(
                                "Warning, -r is default, turn off with -z\n");
                        }
			else if (!strcmp(argv[i], "-m"))
			{
				writing_mapfile = 1;
				continue;
			}
			else if (!strcmp(argv[i], "-mapfile"))
			{
				i++;
				if (argc == i) err_exit(argv[i-1]);
				mapfile = argv[i];
				continue;
			}
			else if (!strcmp(argv[i], "-f"))
			{
				i++;
				if (argc == i) err_exit(argv[i-1]);
				files[fidx++] = argv[i];
				continue;
			}
			else if (!strcmp(argv[i], "-h") || 
					!strcmp(argv[i], "-?") ||
					!strcmp(argv[i], "-help"))
				err_exit("Help");

			else err_exit("Invalid parameter");
		}
		else if (!files[0])
		{ 
			files[0] = argv[i];
			fidx++;
		}
		else
		{
			user_names[numnames].name = argv[i];
			numnames++;
		}
	}
	if (!files[0])
	{
		err_exit("No file specified");
	}
	if (!stat(mapfile, &st))
	{
		writing_mapfile = 1;
		read_mapfile();
	}
	for (i = 0; i < fidx; i++)
	{
		strip_file(files[i]);
	}
	if (writing_mapfile) write_mapfile();
	exit(0);
	return 0;
}

void
strip_file(file)
char *file;
{ 
 int c;
 FILE *fp;
 char *buf;
 struct stat st;
 int cnt = 0;


	if (stat(file, &st))
		err_exit(file);
	
	if (!(buf = (char *)malloc(st.st_size)))
		err_exit("malloc failed");
		
	if (!(fp = fopen(file, FOPEN_READ)))
		err_exit(file);
	
	while ((c = getc(fp)) != EOF)
	{
		buf[cnt] = c;
		setup(&buf[cnt]);
		if (mw_ptr && !*mw_ptr) /* we have a null-terminated word */
		{
			/*printf("checking %s %d\n", matchword);*/
			check(user_names, &buf[cnt]);
			if (use_internal && mw_ptr) 
				check(internal_names, &buf[cnt]);
			if (use_external && mw_ptr) 
				check(external_names, &buf[cnt]);
			mw_ptr = 0;
		}
		cnt++;
	}
	fclose(fp);
	if (!(fp = fopen(file, FOPEN_WRITE)))
	{
		err_exit(file);
	}
	fwrite(buf, st.st_size, 1, fp);
	fclose(fp);
#ifndef PC
	chmod (file, st.st_mode & 04777);
#endif
}
void
setup(curpos)
char *curpos;
{
  char c = *curpos;

	if ( !mw_ptr && varprefix(curpos) && var1stchar(c))
	{
/* 
 *		Start of a word
 */
		/*printf("start %s prev:%d\n", curpos, curpos[-1]);*/
		mw_ptr = matchword; 
		*mw_ptr++ = c;
		*mw_ptr = (char) -1;
	}
	else if (mw_ptr && var2ndchar(c))
	{
/*
 *		Next character 
 */
		*mw_ptr++ = c;
		*mw_ptr = (char)-1;
	}
	else if (!c && mw_ptr) /* we've got a null-terminated word */
	{
		*mw_ptr = c;
	}
	else
	{
		mw_ptr = 0;
		return;
	}
}

void
check(np, curpos)
NAMELIST *np;
char *curpos;
{
	int i;
	char *cp = matchword;
	if (strlen(cp) < 2) return;
	if (!isalpha(*cp) && *cp != '_') cp++; 
	for (i = 0;np[i].name; i++)  /* check each word */
	{
		if (!strcmp(cp, np[i].name) || (*cp == '_' && 
			!strcmp(cp+1, np[i].name)))
		{
			strip(curpos - 1, strlen(np[i].name), i, np);
			mw_ptr = 0;
			break;
		}
	}
}
void
strip(buf, n, idx, np)
char *buf;
int n;
int idx;
NAMELIST *np;
{
 int i;
 int len;
 static char *letters = "abcdefghijklmnopqrstuvwxyz";
 static char *letters_num = "0123456789abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (!randm)
	{
		for (i=0;i<n;i++) *(buf - i) = 0; 
	}
	else
	{
			
			/* replace up to 6 characters with randm letters */
		if (!np[idx].randname)
		{

			len = strlen(np[idx].name);
			if (len > 6) len = 6;
			np[idx].randname = (char *)malloc(7);
			np[idx].randname[0] = letters[rand()%26];
			for (i = 1; i < len; i++)
				np[idx].randname[i] = letters_num[rand()%63];
			np[idx].randname[len] = 0; /* null terminate */
			
		}
		buf += -n + 1;
		sprintf(buf, np[idx].randname);
	}
}
void
err_exit(str)
char *str;
{
	fprintf(stderr, "%s:\n", str);
#ifndef _MSC_VER
	if (errno > 0) 
                perror("  ");
#endif
	fprintf(stderr, 
		"Usage: lmstrip file [-f f2 ... -f fn] [-l] [-e|-n] [-z] [-m [-mapfile mapfile]] [strings...]\n");
	fprintf(stderr, 
		"\t-l\tList internal and external names to be stripped\n");
	fprintf(stderr, 
		"\t-e\tDon't strip external names\n");
	fprintf(stderr, 
		"\t-n\tDon't strip internal and external names\n");
	fprintf(stderr, 
		"\t-z\tReplaces strings with zeroes instead of random names\n");
	fprintf(stderr, 
		"\t-m\tUse mapfile from previous lmstrip (lmstrip.map)\n");
	fprintf(stderr, 
		"\t-mapfile\tSet mapfile name (instead of lmstrip.map)\n");
	fprintf(stderr, 
		"\tstrings\tStrip these strings from the executable\n");
	fprintf(stderr, 
		"\n\tUse -e if lc_xxx calls are made from shared library\n");
	fprintf(stderr, 
		"\nIf mapfile exists, it's used, otherwise created.\n");
	exit(-1);
}
static
void
read_mapfile()
{
  int i;
  char s1[256];
  char s2[256];
  char s[513];
  NAMELIST *np;

	if (!(mapfp = fopen(mapfile, "r")))
		err_exit(mapfile);
	reading_mapfile = 1;
	printf("Using mapfile: %s\n", mapfile);
	fflush(stdout);
	np = user_names;
	*s = 0;
	fgets(s, 255, mapfp);	/* overrun checked */
	if (strcmp(s, "USER NAMES\n"))
		err_exit("Invalid mapfile");
	i = 0;
	while (fgets(s, 512, mapfp))	/* overrun checked */
	{
		if (!strcmp(s, "EXTERNAL NAMES\n"))
			break;
		if (sscanf(s, "%s %s", s1, s2) != 2)	/* overrun checked */
			err_exit("Corrupted mapfile");
		np[i].name = malloc(strlen(s1) + 1);
		strcpy(np[i].name, s1);
		np[i].randname = malloc(strlen(s2) + 1);
		strcpy(np[i].name, s2);

	}
	if (strcmp(s, "EXTERNAL NAMES\n"))
		err_exit("Truncated mapfile");
	np = external_names;
	i = 0;
	while (fgets(s, 512, mapfp))	/* overrun checked */
	{
		if (!strcmp(s, "INTERNAL NAMES\n"))
			break;
		if (sscanf(s, "%s %s", s1, s2) != 2)	/* overrun checked */
			err_exit("Corrupted mapfile");
		for (np = external_names; np->name; np++)
			if (!strcmp(s1, np->name))
				break;
		if (!np->name) err_exit("Corrupted mapfile");
		np->randname = malloc(strlen(s2) + 1);
		strcpy(np->randname, s2);

	}
	if (strcmp(s, "INTERNAL NAMES\n"))
		err_exit("Truncated mapfile");
	np = external_names;
	i = 0;
	while (fgets(s, 512, mapfp))	/* overrun checked */
	{
		if (sscanf(s, "%s %s", s1, s2) != 2)	/* overrun checked */
			err_exit("Corrupted mapfile");
		for (np = internal_names; np->name; np++)
			if (!strcmp(s1, np->name))
				break;
		if (!np->name) err_exit("Corrupted mapfile");
		np->randname = malloc(strlen(s2) + 1);
		strcpy(np->randname, s2);
	}
	fclose(mapfp);
}

static
void
write_mapfile()
{
  NAMELIST *np;

	if (reading_mapfile)
		printf("Updating mapfile: %s\n", mapfile);
	else
		printf("Creating mapfile: %s\n", mapfile);
	if (!(mapfp = fopen(mapfile, "w")))
		err_exit(mapfile);
	np = user_names;
	fprintf(mapfp, "USER NAMES\n");
	for (np = user_names; np->name; np++)
		if (np->randname)
			fprintf(mapfp, "%-40s%s\n", np->name, np->randname);
	fprintf(mapfp, "EXTERNAL NAMES\n");
	for (np = external_names; np->name; np++)
		if (np->randname)
			fprintf(mapfp, "%-40s%s\n", np->name, np->randname);
	fprintf(mapfp, "INTERNAL NAMES\n");
	for (np = internal_names; np->name; np++)
		if (np->randname)
			fprintf(mapfp, "%-40s%s\n", np->name, np->randname);
	fclose(mapfp);
}

int
varprefix(curpos)
char *curpos;
{
	if (!curpos[-1] || (!curpos[-2] && !isalnum(curpos[-1])))
		return 1;
	return 0;
}
int
var1stchar(c)
int c;
{
	if (isalpha(c) || (c == '_')) return 1;
	return 0;
}
int
var2ndchar(c)
int c;
{
/*
 *	Add @ for PC and $ for VMS
 */
	if (var1stchar(c) || isdigit(c) || c == '@' || c == '$')
		return 1;
	return 0;
}
