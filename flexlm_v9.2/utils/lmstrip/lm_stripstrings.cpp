/************************************************************
 	Copyright (c) 2003 by Macrovision, Corporation.
 	This software has been provided pursuant to a License Agreement
 	containing restrictions on its use.  This software contains
 	valuable trade secrets and proprietary information of
 	Macrovision Corporation and is protected by law.  It may
 	not be copied or distributed in any form or medium, disclosed
 	to third parties, reverse engineered or used in any manner not
 	provided for in said License Agreement except with the prior
 	written authorization from Macrovision Corporation.
 ***********************************************************/
/* $Id: lm_stripstrings.cpp,v 1.7.2.6 2003/06/30 23:55:20 sluu Exp $
***********************************************************/
/**   @file lmstripstrings.c
 *    @brief contains the symbol strings to be stripped by lmstrip
 *    @version $Revision: 1.7.2.6 $
 *
  ************************************************************/

#include "lm_strip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct namelist {
	char *	name;
	char *	randname;
} NAMELIST;

static NAMELIST *findNP(const L_STRIP_OPTIONS * options, const char *searchForString, int *offsetIntoSearchForString);
static NAMELIST * check(NAMELIST *np, const char *theString, int *offsetIntoTheString);
static void randomize(L_STRIP_OPTIONS * options,NAMELIST *np);
static int writeNameList(FILE * fp, NAMELIST * np, char * title);
static int err_exit(char *str, int line);
static unsigned int getUsec(void);
static NAMELIST * checkNPBoth(NAMELIST *npList, const char * theString);
static int getFirstOffsetAndLen(const char * theString, int * len, int * mangled);

#define USER_LIST_INCREMENT	20	/* add 20 entries at a time to the user_names */
static int		user_names_size = 0;
static int		user_names_entries = 0;	
static NAMELIST *user_names = NULL;

static NAMELIST external_names[] =  {
	"f_add"				            , 0,
	"f_checklinger"		            , 0,
	"f_count"			            , 0,
	"f_dequeue"			            , 0,
	"f_drop"			            , 0,
	"f_drop_client"		            , 0,
	"f_dump"			            , 0,
	"f_featcount"		            , 0,
	"f_get_feat"		            , 0,
	"f_get_feat_next"	            , 0,
	"f_list"			            , 0,
	"f_lookup"			            , 0,
	"f_nousers"			            , 0,
	"f_pooled"			            , 0,
	"f_queue"			            , 0,
	"f_remove"						, 0,
	"f_remove_all"					, 0,
	"f_remove_all_old"				, 0,
	"f_remove_children"				, 0,
	"f_user_remove"					, 0,
	"f_user_remove_handle"			, 0,
	"la_free_daemon"				, 0,
	"la_init"						, 0,
	"la_lmgrds"						, 0,
	"la_parse_daemon"				, 0,
	"lc_add_key_filter"				, 0,
	"lc_alarm"						, 0,
	"lc_auth_data"					, 0,
	"lc_baddate"					, 0,
	"lc_borrow_return"				, 0,
	"lc_check"						, 0,
	"lc_check_key"					, 0,
	"lc_checkin"					, 0,
	"lc_checkout"					, 0,
	"lc_chk_conf"					, 0,
	"lc_ck_feats"					, 0,
	"lc_cleanup"					, 0,
	"lc_cleanup_internal"			, 0,
	"lc_convert"					, 0,
	"lc_copy_hostid"				, 0,
	"lc_crypt"						, 0,
	"lc_cryptstr"					, 0,
	"lc_daemon"						, 0,
	"lc_disalarm"					, 0,
	"lc_disconn"					, 0,
	"lc_display"					, 0,
	"lc_err_info"					, 0,
	"lc_errstring"					, 0,
	"lc_errtext"					, 0,
	"lc_expire_days"				, 0,
	"lc_feat_list"					, 0,
	"lc_feat_list_lfp"				, 0,
	"lc_feat_set"					, 0,
	"lc_feat_set_lfp"				, 0,
	"lc_first_job"					, 0,
	"lc_free_hostid"				, 0,
	"lc_free_job"					, 0,
	"lc_free_lmgrd_stat"			, 0,
	"lc_free_mem"					, 0,
	"lc_get_attr"					, 0,
	"lc_get_config"					, 0,
	"lc_get_errno"					, 0,
	"lc_get_feats"					, 0,
	"lc_get_redir"					, 0,
	"lc_get_registry"				, 0,
	"lc_gethostid"					, 0,
	"lc_getid_type"					, 0,
	"lc_heartbeat"					, 0,
	"lc_hostid"						, 0,
	"lc_hostname"					, 0,
	"lc_hosttype"					, 0,
	"lc_idle"						, 0,
	"lc_init"						, 0,
	"lc_init_simple_composite"		, 0,
	"lc_install_license"			, 0,
	"lc_install_license_path"		, 0,
	"lc_is_admin"					, 0,
	"lc_isadmin"					, 0,
	"lc_lic_where"					, 0,
	"lc_license_dump"				, 0,
	"lc_log"						, 0,
	"lc_master_list"				, 0,
	"lc_nap"						, 0,
	"lc_new_job"					, 0,
	"lc_next_conf"					, 0,
	"lc_next_job"					, 0,
	"lc_perror"						, 0,
	"lc_print_config"				, 0,
	"lc_redir_ver"					, 0,
	"lc_remove"						, 0,
	"lc_removeh"					, 0,
	"lc_set_attr"					, 0,
	"lc_set_errno"					, 0,
	"lc_set_registry"				, 0,
	"lc_shutdown"					, 0,
	"lc_sleep"						, 0,
	"lc_status"						, 0,
	"lc_test_conf"					, 0,
	"lc_timer"						, 0,
	"lc_timer_int"					, 0,
	"lc_userlist"					, 0,
	"lc_username"					, 0,
	"lc_vsend"						, 0,
	"lc_xstrcmp"					, 0,
	"lp_checkin"					, 0,
	"lp_checkout"					, 0,
	"lp_code"						, 0,
	"lp_errstring"					, 0,
	"lp_heartbeat"					, 0,
	"lp_key2_ptr"					, 0,
	"lp_perror"						, 0,
	"lp_pwarn"						, 0,
	"lp_warning"					, 0,
	"ls_add_client"					, 0,
	"ls_addr_by_handle"				, 0,
	"ls_all_clients"				, 0,
	"ls_app_init"					, 0,
	"ls_append_repfile"				, 0,
	"ls_attr_setup"					, 0,
	"ls_badserv"					, 0,
	"ls_borrow_check"				, 0,
	"ls_build_opt"					, 0,
	"ls_c_init"						, 0,
	"ls_ca_copy"					, 0,
	"ls_checkhost"					, 0,
	"ls_checkin"					, 0,
	"ls_checklock"					, 0,
	"ls_checkout"					, 0,
	"ls_checkroot"					, 0,
	"ls_ck_feats"					, 0,
	"ls_client_add_hostid"			, 0,
	"ls_client_by_handle"			, 0,
	"ls_client_send"				, 0,
	"ls_client_send_str"			, 0,
	"ls_close_repfile"				, 0,
	"ls_closeout_this_tcp_client"	, 0,
	"ls_closeout_this_udp_client"	, 0,
	"ls_create_handle"				, 0,
	"ls_daemon"						, 0,
	"ls_decode_ca"					, 0,
	"ls_delete_client"				, 0,
	"ls_delete_handle"				, 0,
	"ls_dict"						, 0,
	"ls_dict_flush"					, 0,
	"ls_dict_init"					, 0,
	"ls_dict_lookup_str"			, 0,
	"ls_dict_stat"					, 0,
	"ls_disown"						, 0,
	"ls_docmd"						, 0,
	"ls_down"						, 0,
	"ls_encode_ca"					, 0,
	"ls_exclude"					, 0,
	"ls_exit"						, 0,
	"ls_feat_checkout"				, 0,
	"ls_feat_dump"					, 0,
	"ls_feat_info"					, 0,
	"ls_feat_prune"					, 0,
	"ls_feat_start_validate"		, 0,
	"ls_findbad"					, 0,
	"ls_flush_replog"				, 0,
	"ls_get_attr"					, 0,
	"ls_get_info"					, 0,
	"ls_get_opts"					, 0,
	"ls_gettime"					, 0,
	"ls_go_down"					, 0,
	"ls_handle_by_addr"				, 0,
	"ls_handle_by_client"			, 0,
	"ls_hookup"						, 0,
	"ls_host"						, 0,
	"ls_i_fhostid"					, 0,
	"ls_i_master"					, 0,
	"ls_include"					, 0,
	"ls_ingroup"					, 0,
	"ls_init_feat"					, 0,
	"ls_is_duplicate"				, 0,
	"ls_last_msg"					, 0,
	"ls_lf"							, 0,
	"ls_lickey_to_conf"				, 0,
	"ls_list"						, 0,
	"ls_log_asc_printf"				, 0,
	"ls_log_ascii_name"				, 0,
	"ls_log_client"					, 0,
	"ls_log_close_ascii"			, 0,
	"ls_log_close_report"			, 0,
	"ls_log_comment"				, 0,
	"ls_log_cpu_usage"				, 0,
	"ls_log_daemon_name"			, 0,
	"ls_log_endline"				, 0,
	"ls_log_error"					, 0,
	"ls_log_msg_cnt"				, 0,
	"ls_log_open_ascii"				, 0,
	"ls_log_open_report"			, 0,
	"ls_log_prefix"					, 0,
	"ls_log_reopen_ascii"			, 0,
	"ls_log_reopen_report"			, 0,
	"ls_log_report_name"			, 0,
	"ls_log_res_usage"				, 0,
	"ls_log_switchfrom"				, 0,
	"ls_log_switchto"				, 0,
	"ls_log_timestamp"				, 0,
	"ls_log_usage"					, 0,
	"ls_logtime"					, 0,
	"ls_lookup_client"				, 0,
	"ls_lost"						, 0,
	"ls_main"						, 0,
	"ls_malloc"						, 0,
	"ls_mask_clear"					, 0,
	"ls_mast_rdy"					, 0,
	"ls_max_exceeded"				, 0,
	"ls_mk_flexlm_dir"				, 0,
	"ls_msg_dump"					, 0,
	"ls_on_host"					, 0,
	"ls_oneless"					, 0,
	"ls_pause"						, 0,
	"ls_pool"						, 0,
	"ls_pooled_check"				, 0,
	"ls_print_clients"				, 0,
	"ls_print_feats"				, 0,
	"ls_process"					, 0,
	"ls_q_check"					, 0,
	"ls_quorum"						, 0,
	"ls_readready"					, 0,
	"ls_receive"					, 0,
	"ls_recycle_control"			, 0,
	"ls_release_control"			, 0,
	"ls_replog_cksum"				, 0,
	"ls_replog_comment"				, 0,
	"ls_replog_cpu_usage"			, 0,
	"ls_replog_delete_client"		, 0,
	"ls_replog_end"					, 0,
	"ls_replog_error"				, 0,
	"ls_replog_msg_vol"				, 0,
	"ls_replog_res_usage"			, 0,
	"ls_replog_timestamp"			, 0,
	"ls_replog_usage"				, 0,
	"ls_reread"						, 0,
	"ls_resend_last_resp"			, 0,
	"ls_s_down"						, 0,
	"ls_s_dump"						, 0,
	"ls_s_fd2_down"					, 0,
	"ls_s_find"						, 0,
	"ls_s_first"					, 0,
	"ls_s_havemaster"				, 0,
	"ls_s_havequorum"				, 0,
	"ls_s_imaster"					, 0,
	"ls_s_init"						, 0,
	"ls_s_master"					, 0,
	"ls_s_masterup"					, 0,
	"ls_s_mready"					, 0,
	"ls_s_order"					, 0,
	"ls_s_pconn"					, 0,
	"ls_s_qnum"						, 0,
	"ls_s_reset"					, 0,
	"ls_s_sconn"					, 0,
	"ls_s_setmaster"				, 0,
	"ls_s_shut_if_client"			, 0,
	"ls_sconnect"					, 0,
	"ls_send"						, 0,
	"ls_send_user"					, 0,
	"ls_send_vendor_def"			, 0,
	"ls_sendup"						, 0,
	"ls_serv_conn"					, 0,
	"ls_serv_receive"				, 0,
	"ls_serv_time"					, 0,
	"ls_server_send"				, 0,
	"ls_setlock"					, 0,
	"ls_shakehand"					, 0,
	"ls_since"						, 0,
	"ls_sock_close"					, 0,
	"ls_socket"						, 0,
	"ls_statfile_rm"				, 0,
	"ls_store_master_list"			, 0,
	"ls_store_socket"				, 0,
	"ls_time_line"					, 0,
	"ls_udp_read"					, 0,
	"ls_udp_send"					, 0,
	"ls_unlock"						, 0,
	"ls_user"						, 0,
	"ls_user_based"					, 0,
	"ls_user_lookup"				, 0,
	"ls_user_or_hostbased"			, 0,
	"ls_vd_info"					, 0,
	"ls_wakeup"						, 0,
	"ls_wpipe"						, 0,
	0, 0};
static NAMELIST internal_names[] = {
	"AuthFlag"							, 0,
	"CleanUp"							, 0,
	"DemoFuncs$S15420"					, 0,
	"EtherCount"						, 0,
	"FeatureName$S15405"				, 0,
	"FlexLmJobInfo$S15430"				, 0,
	"FlexLockDemoVendorCodes$S15450"	, 0,
	"FlexLockJobInfo$S15440"			, 0,
	"FlexMsgBox"						, 0,
	"GetAccessAPIVersion"				, 0,
	"GlobalStrkey"						, 0,
	"KillThread_NT"						, 0,
	"KillTimer_NT"						, 0,
	"LoadDriver"						, 0,
	"Lp_key2_ptr"						, 0,
	"NTDriverDiscover"					, 0,
	"OverdriveOff"						, 0,
	"OverdriveOn"						, 0,
	"Passthru"							, 0,
	"RomDta"							, 0,
	"SemCreated"						, 0,
	"SetTimer_NT"						, 0,
	"ShaddowFlexLockDemoVendorCodes$S15451", 0,
	"StripString"						, 0,
	"TimerThreadProc"					, 0,
	"Win32s"							, 0,
	"Win95"								, 0,
	"WinNT"								, 0,
	"WinsockVersion"					, 0,
	"_lm_debug"							, 0,
	"_lm_nerr"							, 0, 
	"advanced_winsock_select@20"		, 0,
	"atexit_function"					, 0,
	"check_globes"						, 0,
	"check_new_adapter"					, 0,
	"copy_conf_data"					, 0,
	"dump_config"						, 0,
	"dump_configs"						, 0,
	"fl_available"						, 0,
	"fl_flexapi"						, 0,
	"fl_freedyndata"					, 0,
	"fl_global_variable_api_entry"		, 0,
	"fl_install"						, 0,
	"fl_transact"						, 0,
	"flapi_available"					, 0,
	"flapi_install"						, 0,
	"flapi_use_available"				, 0,
	"flex_WSACleanup@0"					, 0,
	"flex_WSAGetLastError@0"			, 0,
	"flex_WSAStartup@8"					, 0,
	"flex___WSAFDIsSet@8"				, 0,
	"flex_bind@12"						, 0,
	"flex_closesocket@4"				, 0,
	"flex_connect@12"					, 0,
	"flex_gethostbyaddr@12"				, 0,
	"flex_gethostbyname@4"				, 0,
	"flex_gethostname@8"				, 0,
	"flex_getservbyname@8"				, 0,
	"flex_getsockname@12"				, 0,
	"flex_getsockopt@20"				, 0,
	"flex_htonl@4"						, 0,
	"flex_htons@4"						, 0,
	"flex_ioctlsocket@12"				, 0,
	"flex_ntohl@4"						, 0,
	"flex_ntohs@4"						, 0,
	"flex_recv@16"						, 0,
	"flex_select@20"					, 0,
	"flex_send@16"						, 0,
	"flex_shutdown@8"					, 0,
	"flex_socket@12"					, 0,
	"free_job_conf_servers"				, 0,
	"free_job_daemon"					, 0,
	"free_job_key_filters"				, 0,
	"free_job_lic_files"				, 0,
	"free_job_msgq"						, 0,
	"free_job_options"					, 0,
	"free_job_packages"					, 0,
	"free_job_redirect"					, 0,
	"free_job_vendorids"				, 0,
	"garbagebag"						, 0,
	"gen_pkey_headers"					, 0,
	"get_netbios_ether_addr1"			, 0,
	"get_pc_ether_addr1"				, 0,
	"getenv1"							, 0,
	"gndtest"							, 0,
	"gpn"								, 0,
	"hDriver"							, 0,
	"hPassThruSem"						, 0,
	"hSA16CompatSem"					, 0,
	"hSemaphore"						, 0,
	"hThread"							, 0,
	"hTimerThread"						, 0,
	"icmp_hInst"						, 0,
	"ignore_local_license"				, 0,
	"is_confg_in_list"					, 0,
	"key_filter"						, 0,
	"l_CopyPackageInfoToComponent"		, 0,
	"l_IsComponentOfPackage"			, 0,
	"l__EnumProtocols@12"				, 0,
	"l__GetAddressByName@40"			, 0,
	"l__IcmpCloseHandle@4"				, 0,
	"l__IcmpCreateFile@0"				, 0,
	"l__IcmpSendEcho@32"				, 0,
	"l__WSACleanup@0"					, 0,
	"l__WSACreateEvent@0"				, 0,
	"l__WSAEnumNetworkEvents@12"		, 0,
	"l__WSAEventSelect@12"				, 0,
	"l__WSAFDIsSet@8"					, 0,
	"l__WSAGetLastError@0"				, 0,
	"l__WSALookupServiceBegin@12"		, 0,
	"l__WSALookupServiceEnd@4"			, 0,
	"l__WSALookupServiceNext@16"		, 0,
	"l__WSASetLastError@4"				, 0,
	"l__WSAStartup@8"					, 0,
	"l__WSAWaitForMultipleEvents@20"	, 0,
	"l____WSAFDIsSet@8"					, 0,
	"l__accept@12"						, 0,
	"l__bind@12"						, 0,
	"l__closesocket@4"					, 0,
	"l__connect@12"						, 0,
	"l__gethostbyaddr@12"				, 0,
	"l__gethostbyname@4"				, 0,
	"l__gethostname@8"					, 0,
	"l__getservbyname@8"				, 0,
	"l__getsockname@12"					, 0,
	"l__getsockopt@20"					, 0,
	"l__htonl@4"						, 0,
	"l__htons@4"						, 0,
	"l__ioctlsocket@12"					, 0,
	"l__listen@8"						, 0,
	"l__ntohl@4"						, 0,
	"l__ntohs@4"						, 0,
	"l__recv@16"						, 0,
	"l__recvfrom@24"					, 0,
	"l__select@20"						, 0,
	"l__send@16"						, 0,
	"l__sendto@24"						, 0,
	"l__setsockopt@20"					, 0,
	"l__shutdown@8"						, 0,
	"l__socket@12"						, 0,
	"l_add"								, 0,
	"l_add_key_filter"					, 0,
	"l_addi"							, 0,
	"l_addr_to_inet"					, 0,
	"l_allfeat"							, 0,
	"l_any_set"							, 0,
	"l_appfile"							, 0,
	"l_asc_date"						, 0,
	"l_asc_hostid"						, 0,
	"l_asc_hostid_len"					, 0,
	"l_asc_id_one"						, 0,
	"l_atox"							, 0,
	"l_auth_data"						, 0,
	"l_baddate"							, 0,
	"l_basic_conn"						, 0,
	"l_basic_conn_efp"					, 0,
	"l_bin_date"						, 0,
	"l_bios_id"							, 0,
	"l_block_sigalarm"					, 0,
	"l_borrow"							, 0,
	"l_borrow_decrypt"					, 0,
	"l_borrow_dptr"						, 0,
	"l_borrow_in_seconds"				, 0,
	"l_borrow_stat"						, 0,
	"l_borrow_string_to_time"			, 0,
	"l_borrowname"						, 0,
	"l_br"								, 0,
	"l_buf_64"							, 0, 
	"l_bufg_64"							, 0, 
	"l_c"								, 0,
	"l_check"							, 0,
	"l_check_aladdin"					, 0,
	"l_check_conf"						, 0,
	"l_check_dallas"					, 0,
	"l_check_debug"						, 0,
	"l_check_fmt_ver"					, 0,
	"l_check_fmt_ver_hostids"			, 0,
	"l_check_key"						, 0,
	"l_check_rainbow"					, 0,
	"l_checkin"							, 0,
	"l_checkoff"						, 0,
	"l_checkout"						, 0,
	"l_chk_conf"						, 0,
	"l_chkdir"							, 0,
	"l_ck_feats"						, 0,
	"l_ckout_borrow"					, 0,
	"l_ckout_crypt"						, 0,
	"l_ckout_ok"						, 0,
	"l_ckout_string_key"				, 0,
	"l_cksum"							, 0,
	"l_cksum_ok"						, 0,
	"l_cleanup"							, 0,
	"l_clear_error"						, 0,
	"l_clr_rcv_queue"					, 0,
	"l_compare_version"					, 0,
	"l_composite_id"					, 0,
	"l_conf_copy"						, 0,
	"l_config"							, 0,
	"l_conn_debug"						, 0,
	"l_conn_harvest"					, 0,
	"l_conn_msg"						, 0,
	"l_connect"							, 0,
	"l_connect_by_conf"					, 0,
	"l_connect_by_conf_for_vsend"		, 0,
	"l_connect_endpoint"				, 0,
	"l_connect_host_or_list"			, 0,
	"l_convert"							, 0,
	"l_convertStringMBToUTF8"			, 0,
	"l_convertStringUTF8ToMB"			, 0,
	"l_convertStringUTF8ToWC"			, 0,
	"l_convertStringWCToUTF8"			, 0,
	"l_copy_hostid"						, 0,
	"l_cpu_id"							, 0,
	"l_createdialog"					, 0,
	"l_crypt_private"					, 0,
	"l_cur_dlist"						, 0,
	"l_date"							, 0,
	"l_date_compare"					, 0,
	"l_date_to_time"					, 0,
	"l_days_ascdate"					, 0,
	"l_dbg_feat"						, 0,
	"l_debug_datastr"					, 0,
	"l_decimal_format"					, 0,
	"l_decode_16bit_packed"				, 0,
	"l_decode_32bit_packed"				, 0,
	"l_decode_int"						, 0,
	"l_decode_long"						, 0,
	"l_decode_long_hex"					, 0,
	"l_demuddle"						, 0,
	"l_dialogbox"						, 0,
	"l_disconn"							, 0,
	"l_disk_geometry_id"				, 0,
	"l_display"							, 0,
	"l_do_ping_operation"				, 0,
	"l_do_redir"						, 0,
	"l_ds_access"						, 0,
	"l_ds_databyte"						, 0,
	"l_ds_dowcheck"						, 0,
	"l_ds_first"						, 0,
	"l_ds_keyclose"						, 0,
	"l_ds_keyopen"						, 0,
	"l_ds_next"							, 0,
	"l_ds_romdata"						, 0,
	"l_ds_setup"						, 0,
	"l_dump_check_tables"				, 0,
	"l_eintelid"						, 0,
	"l_enable_pkg_feats"				, 0,
	"l_encode_16bit_packed"				, 0,
	"l_encode_32bit_packed"				, 0,
	"l_encode_int"						, 0,
	"l_encode_long"						, 0,
	"l_encode_long_hex"					, 0,
	"l_err_info"						, 0,
	"l_err_info_cp"						, 0,
	"l_errstring"						, 0,
	"l_errtext"							, 0,
	"l_ether_id"						, 0,
	"l_ether_str_to_num"				, 0,
	"l_exitcall"						, 0,
	"l_expdate_days"					, 0,
	"l_expire_days"						, 0,
	"l_extract_date"					, 0,
	"l_feat_find"						, 0,
	"l_feat_find_first_empty"			, 0,
	"l_feat_list"						, 0,
	"l_feat_list_curr"					, 0,
	"l_feat_set"						, 0,
	"l_featon"							, 0,
	"l_feature_is_borrowed"				, 0,
	"l_file_check"						, 0,
	"l_file_checkin"					, 0,
	"l_file_checkout"					, 0,
	"l_file_checkq"						, 0,
	"l_file_log"						, 0,
	"l_file_remove"						, 0,
	"l_file_removeh"					, 0,
	"l_file_sdata"						, 0,
	"l_file_userlist"					, 0,
	"l_files_in_dir"					, 0,
	"l_fillin_platform"					, 0,
	"l_filter_gen"						, 0,
	"l_find_msg"						, 0,
	"l_finder"							, 0,
	"l_finder_port"						, 0,
	"l_findrsrcdata"					, 0,
	"l_first_job"						, 0,
	"l_flexAccess"						, 0,
	"l_flexClose"						, 0,
	"l_flexEventLogAddString"			, 0,
	"l_flexEventLogCleanup"				, 0,
	"l_flexEventLogFlushStrings"		, 0,
	"l_flexEventLogInit"				, 0,
	"l_flexEventLogIsEnabled"			, 0,
	"l_flexEventLogWrite"				, 0,
	"l_flexFclose"						, 0,
	"l_flexFopen"						, 0,
	"l_flexFreopen"						, 0,
	"l_flexOpen"						, 0,
	"l_flexRemove"						, 0,
	"l_flexRename"						, 0,
	"l_flexStat"						, 0,
	"l_flexUnlink"						, 0,
	"l_flexid6"							, 0,
	"l_flexid7"							, 0,
	"l_flush_config"					, 0,
	"l_free"							, 0,
	"l_freeUTF8Cmdline"					, 0,
	"l_free_conf"						, 0,
	"l_free_conf_data"					, 0,
	"l_free_conf_no_data"				, 0,
	"l_free_daemon_list"				, 0,
	"l_free_err_info"					, 0,
	"l_free_hostid"						, 0,
	"l_free_ids"						, 0, 
	"l_free_job"						, 0,
	"l_free_job_conf"					, 0,
	"l_free_job_featdata"				, 0,
	"l_free_job_lf_pointers"			, 0,
	"l_free_job_license"				, 0,
	"l_free_job_servers"				, 0,
	"l_free_keylist"					, 0,
	"l_free_list"						, 0,
	"l_free_lmgrd_stat"					, 0,
	"l_free_mem"						, 0,
	"l_free_one_daemon"					, 0,
	"l_free_one_hostid"					, 0,
	"l_free_server"						, 0,
	"l_gcdecode"						, 0,
	"l_gcspim"							, 0,
	"l_gcspimx"							, 0,
	"l_gen_pkey_headers"				, 0,
	"l_genkeys"							, 0,
	"l_genrand"							, 0,
	"l_genseed"							, 0,
	"l_getEnvUTF8"						, 0,
	"l_getEnvironmentVariable"			, 0,
	"l_getHostname"						, 0,
	"l_getOSFamily"						, 0,
	"l_getUTF8Cmdline"					, 0,
	"l_getUserName"						, 0,
	"l_get_attr"						, 0,
	"l_get_cksum"						, 0,
	"l_get_conf_from_server"			, 0,
	"l_get_config"						, 0,
	"l_get_date"						, 0,
	"l_get_dec_daemon"					, 0,
	"l_get_dlist"						, 0,
	"l_get_dlist_from_server"			, 0,
	"l_get_dlist_lfp"					, 0,
	"l_get_endpoint"					, 0,
	"l_get_errno"						, 0,
	"l_get_featlist_from_server"		, 0,
	"l_get_feats"						, 0,
	"l_get_id"							, 0,
	"l_get_ipaddr"						, 0,
	"l_get_lfile"						, 0,
	"l_get_one_id"						, 0,
	"l_get_port"						, 0,
	"l_get_registry"					, 0, 
	"l_get_resource"					, 0,
	"l_get_ver"							, 0,
	"l_getadr"							, 0,
	"l_getattr"							, 0,
	"l_getattr_init"					, 0,
	"l_getdomain"						, 0,
	"l_getenv"							, 0,
	"l_getenv_clear"					, 0,
	"l_getexp"							, 0,
	"l_gethostid"						, 0,
	"l_getid_hw"						, 0,
	"l_getid_type"						, 0,
	"l_getlastfile"						, 0,
	"l_getnextfile"						, 0,
	"l_getntaddr"						, 0,
	"l_gettimeofday"					, 0,
	"l_good_bin_date"					, 0,
	"l_good_lic_key"					, 0,
	"l_handshake"						, 0,
	"l_has_floating_lic"				, 0,
	"l_has_nodelock_lic"				, 0,
	"l_hbs"								, 0,
	"l_heartbeat"						, 0,
	"l_host"							, 0,
	"l_host_one"						, 0,
	"l_hostid"							, 0,
	"l_hostid_cmp"						, 0,
	"l_hostid_cmp_exact"				, 0,
	"l_hostid_cmp_one"					, 0,
	"l_hostid_copy"						, 0,
	"l_hostname"						, 0,
	"l_hosttype"						, 0,
	"l_icf"								, 0,
	"l_id_types_match"					, 0,
	"l_idle"							, 0,
	"l_inet_cmp"						, 0,
	"l_inet_to_addr"					, 0,
	"l_init"							, 0,
	"l_init_file"						, 0,
	"l_init_simple_composite_id"		, 0,
	"l_install_conf"					, 0,
	"l_install_feature_text"			, 0,
	"l_install_license"					, 0,
	"l_install_license_path"			, 0,
	"l_int_month"						, 0,
	"l_intel_all"						, 0,
	"l_intelid"							, 0,
	"l_ipaddr"							, 0,
	"l_is_admin"						, 0,
	"l_is_inet"							, 0,
	"l_isalnum"							, 0,
	"l_isalpha"							, 0,
	"l_isascii"							, 0,
	"l_iscntrl"							, 0,
	"l_isdigit"							, 0,
	"l_isgraph"							, 0,
	"l_islower"							, 0,
	"l_isprint"							, 0,
	"l_ispunct"							, 0,
	"l_isspace"							, 0,
	"l_isupper"							, 0,
	"l_isxdigit"						, 0,
	"l_itox"							, 0,
	"l_key"								, 0,
	"l_keyword_eq"						, 0,
	"l_keyword_eq_n"					, 0,
	"l_la_init"							, 0,
	"l_lf_get_token"					, 0,
	"l_lf_put_token"					, 0,
	"l_lfclose"							, 0,
	"l_lfgets"							, 0,
	"l_lfseek"							, 0,
	"l_lic_where"						, 0,
	"l_lmgrd_running"					, 0,
	"l_lmgrds"							, 0,
	"l_local_verify_conf"				, 0,
	"l_log"								, 0,
	"l_lookup"							, 0,
	"l_lookup_serv"						, 0,
	"l_lowercase"						, 0,
	"l_main_tokens"						, 0,
	"l_make_regs_testsuite"				, 0,
	"l_make_tcp_socket"					, 0,
	"l_make_udp_socket"					, 0,
	"l_malloc"							, 0,
	"l_malloc_errstring"				, 0,
	"l_master_list"						, 0,
	"l_master_list_from_job"			, 0,
	"l_master_list_lfp"					, 0,
	"l_mem_free"						, 0,
	"l_mem_malloc"						, 0,
	"l_midnight"						, 0,
	"l_modify"							, 0,
	"l_month_int"						, 0,
	"l_more_featdata"					, 0,
	"l_move_in_hostid"					, 0,
	"l_movelong"						, 0,
	"l_msg_cksum"						, 0,
	"l_msg_size"						, 0,
	"l_msgrdy"							, 0,
	"l_mt_free"							, 0,
	"l_mt_lock"							, 0,
	"l_mt_unlock"						, 0,
	"l_n36_buf"							, 0,
	"l_n36_buff"						, 0,
	"l_nap"								, 0,
	"l_new_hostid"						, 0,
	"l_new_job"							, 0,
	"l_next_conf"						, 0,
	"l_next_conf_or_marker"				, 0,
	"l_next_job"						, 0,
	"l_now"								, 0,
	"l_ntadd"							, 0,
	"l_oMD5"							, 0,
	"l_ok"								, 0,
	"l_onebyte"							, 0,
	"l_open_file"						, 0,
	"l_our_encrypt"						, 0,
	"l_pack"							, 0,
	"l_pack_print"						, 0,
	"l_pack_unprint"					, 0,
	"l_parse_attr"						, 0,
	"l_parse_commtype"					, 0,
	"l_parse_component"					, 0,
	"l_parse_daemon"					, 0,
	"l_parse_decimal"					, 0,
	"l_parse_featset_line"				, 0,
	"l_parse_feature_line"				, 0,
	"l_parse_info_borrow"				, 0,
	"l_parse_package"					, 0,
	"l_parse_server_line"				, 0,
	"l_pause"							, 0,
	"l_pc_connect"						, 0,
	"l_pc_connect_threaded"				, 0,
	"l_perf"							, 0,
	"l_perror"							, 0,
	"l_platform_name"					, 0,
	"l_post_pkg"						, 0,
	"l_prikey_sign"						, 0,
	"l_print_conf_dec"					, 0,
	"l_print_config"					, 0,
	"l_print_config_asc"				, 0,
	"l_print_daemon"					, 0,
	"l_print_server"					, 0,
	"l_prompt_lf"						, 0,
	"l_pubkey_err"						, 0,
	"l_pubkey_verify"					, 0,
	"l_putenv"							, 0,
	"l_puts_rand"						, 0,
	"l_puts_rand1"						, 0,
	"l_queue_msg"						, 0,
	"l_queue_remove"					, 0,
	"l_rcfilename"						, 0,
	"l_rcvheart"						, 0,
	"l_rcvmsg"							, 0,
	"l_rcvmsg_real"						, 0,
	"l_rcvmsg_str"						, 0,
	"l_rcvmsg_timeout"					, 0,
	"l_rcvmsg_type"						, 0,
	"l_rcvmsg_type_timeout"				, 0,
	"l_rcvmsg_wrapper"					, 0,
	"l_read_borrow"						, 0,
	"l_read_lfile"						, 0,
	"l_read_sernum"						, 0,
	"l_read_timeout"					, 0,
	"l_real_getenv"						, 0,
	"l_reasonable_seed"					, 0,
	"l_reconnect"						, 0,
	"l_regQueryValue"					, 0,
	"l_regSetValue"						, 0,
	"l_remove_dup"						, 0,
	"l_remove_dups"						, 0,
	"l_replog"							, 0,
	"l_rerd"							, 0,
	"l_res_ok"							, 0,
	"l_resend_cmd"						, 0,
	"l_reset_env"						, 0,
	"l_reset_job_servers"				, 0,
	"l_return_early"					, 0,
	"l_sap"								, 0,
	"l_sap2"							, 0,
	"l_select"							, 0,
	"l_select_one"						, 0,
	"l_send_hostid"						, 0,
	"l_sernum_to_32"					, 0,
	"l_serv_msg"						, 0,
	"l_servcheck"						, 0,
	"l_set_attr"						, 0,
	"l_set_error"						, 0,
	"l_set_error_path"					, 0,
	"l_set_license_path"				, 0,
	"l_set_registry"					, 0,
	"l_set_user_init3"					, 0,
	"l_sg"								, 0,
	"l_shuffle"							, 0,
	"l_shutdown"						, 0,
	"l_shutdown_one"					, 0,
	"l_shutdown_one_file"				, 0,
	"l_shutdown_vendor"					, 0,
	"l_sleep_cb"						, 0,
	"l_sndmsg"							, 0,
	"l_sort_configs"					, 0,
	"l_start_ok"						, 0,
	"l_status"							, 0,
	"l_str_crypt"						, 0,
	"l_str_dcrypt"						, 0,
	"l_string_key"						, 0,
	"l_strip_os_ver"					, 0,
	"l_sum"								, 0,
	"l_supersede"						, 0,
	"l_svk"								, 0,
	"l_swap"							, 0,
	"l_test_conf"						, 0,
	"l_text"							, 0,
	"l_timer"							, 0,
	"l_timer_add"						, 0,
	"l_timer_change"					, 0,
	"l_timer_delete"					, 0,
	"l_timer_heart"						, 0,
	"l_timer_increment"					, 0,
	"l_timer_job_done"					, 0,
	"l_timer_msecs2time"				, 0,
	"l_timer_mt_add"					, 0,
	"l_timer_mt_change"					, 0,
	"l_timer_mt_delete"					, 0,
	"l_timer_now"						, 0,
	"l_timer_reset"						, 0,
	"l_timer_route_or_set"				, 0,
	"l_timer_router"					, 0,
	"l_timer_set"						, 0,
	"l_timer_signal"					, 0,
	"l_timer_time2msecs"				, 0,
	"l_timer_to_tab"					, 0,
	"l_tolower"							, 0,
	"l_toupper"							, 0,
	"l_try_basic_conn"					, 0,
	"l_try_connect"						, 0,
	"l_unpack"							, 0,
	"l_update_license_file"				, 0,
	"l_update_license_path"				, 0,
	"l_update_lm_license"				, 0,
	"l_update_mask"						, 0,
	"l_upgrade"							, 0,
	"l_upgrade_feat"					, 0,
	"l_uppercase"						, 0,
	"l_user_init3"						, 0,
	"l_userlist"						, 0,
	"l_username"						, 0,
	"l_valid_job"						, 0,
	"l_valid_version"					, 0,
	"l_validdate"						, 0,
	"l_vendor_license_file"				, 0,
	"l_vol_id"							, 0,
	"l_vsend"							, 0,
	"l_win95"							, 0,
	"l_winsock_error1"					, 0,
	"l_winsock_error2"					, 0,
	"l_write_sernum"					, 0,
	"l_x77_buf"							, 0,
	"l_xorname"							, 0,
	"l_xtoi"							, 0,
	"l_zcp"								, 0,
	"l_zinit"							, 0,
	"la_install_license"				, 0,
	"la_shutdown_vendor"				, 0,
	"lcm_executefulfill"				, 0,
	"lcm_fulfillavailable"				, 0,
	"leapyear"							, 0,
	"lfgets_tokens"						, 0,
	"lfptr_eof"							, 0,
	"line_continuation"					, 0,
	"lm_borrow"							, 0,
	"lm_bpi"							, 0,
	"lm_debug_hostid"					, 0,
	"lm_debug_printf"					, 0,
	"lm_debug_puts"						, 0,
	"lm_max_masks"						, 0,
	"lm_no_netbios"						, 0,
	"lm_nofile"							, 0,
	"lm_start"							, 0,
	"lm_start_real"						, 0,
	"lm_timers_in_use"					, 0,
	"loaded"							, 0,
	"lpDriverBuff"						, 0,
	"make_ver_str"						, 0,
	"move_in_hostid"					, 0,
	"move_in_one_hostid"				, 0,
	"move_to_end_of_idptr_list"			, 0,
	"movein_date"						, 0,
	"normal_volume"						, 0,
	"nowaitkeyopen"						, 0,
	"old_ether_add"						, 0,
	"our_encrypt"						, 0,
	"our_encrypt2"						, 0,
	"our_std_error"						, 0,
	"outfilter"							, 0,
	"patch_volume"						, 0,
	"pc_connect"						, 0,
	"pselect"							, 0,
	"real_crypt"						, 0,
	"recvcount"							, 0,
	"recverror"							, 0,
	"sHandle"							, 0,
	"sb_arc4Begin"						, 0,
	"sb_arc4Decrypt"					, 0,
	"sb_arc4Encrypt"					, 0,
	"sb_arc4End"						, 0,
	"sb_arc4GenerateKey"				, 0,
	"sb_changePwdKey"					, 0,
	"sb_dataSize"						, 0,
	"sb_decryptVerify"					, 0,
	"sb_decryptVerifyBegin"				, 0,
	"sb_decryptVerifyEnd"				, 0,
	"sb_decryptVerifyPwdBegin"			, 0,
	"sb_desBegin"						, 0,
	"sb_desDecrypt"						, 0,
	"sb_desEncrypt"						, 0,
	"sb_desEnd"							, 0,
	"sb_desGenerateKeys"				, 0,
	"sb_desSetIV"						, 0,
	"sb_dhGenerateValues"				, 0,
	"sb_dhPwdGenerateValues"			, 0,
	"sb_dhPwdRawSharedSecret"			, 0,
	"sb_dhPwdSharedSecret"				, 0,
	"sb_dhPwdSharedSecretWithAddInfo"	, 0,
	"sb_dhRawSharedSecret"				, 0,
	"sb_dhSharedSecret"					, 0,
	"sb_dhSharedSecretWithAddInfo"		, 0,
	"sb_ecaesBRDecrypt"					, 0,
	"sb_ecaesBREncrypt"					, 0,
	"sb_ecaesBRPwdDecrypt"				, 0,
	"sb_ecaesDecrypt"					, 0,
	"sb_ecaesEncrypt"					, 0,
	"sb_ecaesPwdDecrypt"				, 0,
	"sb_ecaesPwdUnwrap"					, 0,
	"sb_ecaesUnwrap"					, 0,
	"sb_ecaesWrap"						, 0,
	"sb_ecdsaGenCert"					, 0,
	"sb_ecdsaNoHashPwdSign"				, 0,
	"sb_ecdsaNoHashSign"				, 0,
	"sb_ecdsaNoHashVerify"				, 0,
	"sb_ecdsaParseCert"					, 0,
	"sb_ecdsaPwdGenCert"				, 0,
	"sb_ecdsaPwdSignEnd"				, 0,
	"sb_ecdsaSign"						, 0,
	"sb_ecdsaSignBegin"					, 0,
	"sb_ecdsaSignEnd"					, 0,
	"sb_ecdsaVerify"					, 0,
	"sb_ecdsaVerifyBegin"				, 0,
	"sb_ecdsaVerifyCert"				, 0,
	"sb_ecdsaVerifyEnd"					, 0,
	"sb_ecesBRDecrypt"					, 0,
	"sb_ecesBRDecryptBegin"				, 0,
	"sb_ecesBRDecryptEnd"				, 0,
	"sb_ecesBREncrypt"					, 0,
	"sb_ecesBREncryptBegin"				, 0,
	"sb_ecesBREncryptEnd"				, 0,
	"sb_ecesBRPwdDecryptBegin"			, 0,
	"sb_ecesDecrypt"					, 0,
	"sb_ecesDecryptBegin"				, 0,
	"sb_ecesDecryptEnd"					, 0,
	"sb_ecesEncrypt"					, 0,
	"sb_ecesEncryptBegin"				, 0,
	"sb_ecesEncryptEnd"					, 0,
	"sb_ecesPwdDecryptBegin"			, 0,
	"sb_ecesPwdUnwrap"					, 0,
	"sb_ecesUnwrap"						, 0,
	"sb_ecesWrap"						, 0,
	"sb_ecnrGenCert"					, 0,
	"sb_ecnrNoHashPwdSign"				, 0,
	"sb_ecnrNoHashSign"					, 0,
	"sb_ecnrNoHashVerify"				, 0,
	"sb_ecnrParseCert"					, 0,
	"sb_ecnrPwdGenCert"					, 0,
	"sb_ecnrPwdSignEnd"					, 0,
	"sb_ecnrSign"						, 0,
	"sb_ecnrSignBegin"					, 0,
	"sb_ecnrSignEnd"					, 0,
	"sb_ecnrVerify"						, 0,
	"sb_ecnrVerifyBegin"				, 0,
	"sb_ecnrVerifyCert"					, 0,
	"sb_ecnrVerifyEnd"					, 0,
	"sb_encryptSign"					, 0,
	"sb_encryptSignBegin"				, 0,
	"sb_encryptSignEnd"					, 0,
	"sb_encryptSignPwdEnd"				, 0,
	"sb_end"							, 0,
	"sb_errorMessage"					, 0,
	"sb_expandPublicKey"				, 0,
	"sb_fipsRngOptionalInput"			, 0,
	"sb_genKeyPair"						, 0,
	"sb_genPublicKey"					, 0,
	"sb_genPwdKeyPair"					, 0,
	"sb_getInfo"						, 0,
	"sb_getOID"							, 0,
	"sb_getSessionKeyLength"			, 0,
	"sb_heapSize"						, 0,
	"sb_initialize"						, 0,
	"sb_initializeCurve"				, 0,
	"sb_md2Begin"						, 0,
	"sb_md2End"							, 0,
	"sb_md2Hash"						, 0,
	"sb_md5Begin"						, 0,
	"sb_md5End"							, 0,
	"sb_md5Hash"						, 0,
	"sb_mqvGenerateValues"				, 0,
	"sb_mqvPwdRawSharedSecret"			, 0,
	"sb_mqvPwdSharedSecret"				, 0,
	"sb_mqvPwdSharedSecretWithAddInfo"	, 0,
	"sb_mqvRawSharedSecret"				, 0,
	"sb_mqvSharedSecret"				, 0,
	"sb_mqvSharedSecretWithAddInfo"		, 0,
	"sb_new2oldPublicKey"				, 0,
	"sb_old2newPublicKey"				, 0,
	"sb_os2uint"						, 0,
	"sb_rngFIPS186Private"				, 0,
	"sb_rngFIPS186Session"				, 0,
	"sb_rngInit"						, 0,
	"sb_rngSeedFIPS186"					, 0,
	"sb_rngSize"						, 0,
	"sb_sha1Begin"						, 0,
	"sb_sha1End"						, 0,
	"sb_sha1Hash"						, 0,
	"sb_shadowCombine"					, 0,
	"sb_shadowGenerate"					, 0,
	"sb_uint2os"						, 0,
	"timer_proc@16"						, 0,
	"uninstall_signal"					, 0,
	"user_crypt_filter"					, 0,
	"user_crypt_filter_gen"				, 0,
	"winsock_WSACleanup"				, 0,
	"winsock_WSAGetLastError"			, 0,
	"winsock_WSAStartup"				, 0,
	"winsock___WSAFDIsSet"				, 0,
	"winsock_accept"					, 0,
	"winsock_bind"						, 0,
	"winsock_closesocket"				, 0,
	"winsock_connect"					, 0,
	"winsock_enum_protocols"			, 0,
	"winsock_getaddressbyname"			, 0,
	"winsock_gethostbyaddr"				, 0,
	"winsock_gethostbyname"				, 0,
	"winsock_gethostname"				, 0,
	"winsock_getservbyname"				, 0,
	"winsock_getsockname"				, 0,
	"winsock_getsockopt"				, 0,
	"winsock_hInst"						, 0,
	"winsock_htonl"						, 0,
	"winsock_htons"						, 0,
	"winsock_ioctlsocket"				, 0,
	"winsock_listen"					, 0,
	"winsock_ntohl"						, 0,
	"winsock_ntohs"						, 0,
	"winsock_real_init"					, 0,
	"winsock_recv"						, 0,
	"winsock_recvfrom"					, 0,
	"winsock_select"					, 0,
	"winsock_send"						, 0,
	"winsock_sendto"					, 0,
	"winsock_setsockopt"				, 0,
	"winsock_shutdown"					, 0,
	"winsock_socket"					, 0,
	"winsock_wsacreateevent"			, 0,
	"winsock_wsaenumnetworkevents"		, 0,
	"winsock_wsaeventselect"			, 0,
	"winsock_wsalookupservicebegin"		, 0,
	"winsock_wsalookupserviceend"		, 0,
	"winsock_wsalookupservicenext"		, 0,
	"winsock_wsaset_last_error"			, 0,
	"winsock_wsawaitformultipleevents"	, 0,
	"winsockver2"						, 0,

	/* 6/24/03  kmaclean
	 * Added these symbols at the last minute. I have not sorted them into the
	 * list yet because I want to make sure stripping them does not break
	 *  anything */
	"Adapter"					, 0,
	"COM_LC_CRYPTSTR@16"		, 0,
	"COM_LC_HOSTID@8"			, 0,
	"ConnectReturn"				, 0,
	"DOWHandle"					, 0,
	"DriverBuff"				, 0,
	"EtherAddrs"				, 0,
	"EtherTime"					, 0,
	"FLEXLM_type"				, 0,
	"GetWindowsVersion"			, 0,
	"H4DecSingle"				, 0,
	"H4Decode"					, 0,
	"H4Encode"					, 0,
	"IDNum"						, 0,
	"LC_CRYPTSTR@24"			, 0,
	"LC_HOSTID@12"				, 0,
	"LP_CHECKIN@4"				, 0,
	"LP_CHECKOUT@24"			, 0,
	"LP_ERRSTRING@12"			, 0,
	"LP_HEARTBEAT@12"			, 0,
	"LP_SETUP@28"				, 0,
	"LSEnumProviders"			, 0,
	"LSFreeHandle"				, 0,
	"LSGetMessage"				, 0,
	"LSQuery"					, 0,
	"LSRelease"					, 0,
	"LSRequest"					, 0,
	"LSUpdate"					, 0,
	"L_S_ISDIR"					, 0,
	"LptNum"					, 0,
	"MainThreadId"				, 0,
	"NumTrans"					, 0,
	"OverdriveOff32"			, 0,
	"OverdriveOn32"				, 0,
	"Pass1"						, 0,
	"Pass2"						, 0,
	"SHAFinal"					, 0,
	"SHAInit"					, 0,
	"SHAUpdate"					, 0,
	"SeedCode"					, 0,
	"Service"					, 0,
	"Spx"						, 0,
	"TimeOut"					, 0,
	"TimerInterval"				, 0,
	"TimerProc"					, 0,
	"WinsockVerRequested"		, 0,
	"access32"					, 0,
	"applines"					, 0,
	"applinesp"					, 0,
	"appoutbuf"					, 0,
	"appoutcp"					, 0,
	"appxorlines"				, 0,
	"appxorlinesp"				, 0,
	"appxoroutbuf"				, 0,
	"appxoroutcp"				, 0,
	"client_fp"					, 0,
	"closedir_PC"				, 0,
	"databyte32"				, 0,
	"devicesPresent"			, 0,
	"dongleArray"				, 0,
	"dowcheck32"				, 0,
	"dwWaitResult"				, 0,
	"endianTest"				, 0,
	"first32"					, 0,
	"hInst"						, 0,
	"hInstance"					, 0,
	"hTimerEvent"				, 0,
	"hasp"						, 0,
	"iDevCount"					, 0,
	"iDeviceCount"				, 0,
	"iDevicesFound"				, 0,
	"iLoop"						, 0,
	"icmp_installed"			, 0,
	"isTSOK"					, 0,
	"keyclose32"				, 0,
	"keyopen32"					, 0,
	"licgen_fp"					, 0,
	"liclines"					, 0,
	"liclinesp"					, 0,
	"licoutbuf"					, 0,
	"licoutcp"					, 0,
	"lineflags"					, 0,
	"lis_getunitid"				, 0,
	"lu_getdtablesize"			, 0,
	"mask"						, 0,
	"mch"						, 0,
	"myICMPhandle"				, 0,
	"names"						, 0,
	"network_installed"			, 0,
	"next32"					, 0,
	"nt2_connect"				, 0,
	"num_statics"				, 0,
	"numnames"					, 0,
	"opendir_PC"				, 0,
	"p1"						, 0,
	"p2"						, 0,
	"p3"						, 0,
	"p4"						, 0,
	"phase2"					, 0,
	"quiet"						, 0,
	"readdir_PC"				, 0,
	"return_connect_error"		, 0,
	"romdata32"					, 0,
	"romid"						, 0,
	"sHandle2"					, 0,
	"sHandle3"					, 0,
	"sHandle4"					, 0,
	"sHandle5"					, 0,
	"seed"						, 0,
	"setup32"					, 0,
	"staticp"					, 0,
	"statics"					, 0,
	"str0"						, 0,
	"str2"						, 0,
	"str2_ph2"					, 0,
	"str3"						, 0,
	"str4"						, 0,
	"str5"						, 0,
	"szPingBuffer"				, 0,
	"varlines"					, 0,
	"varlinesp"					, 0,
	"varoutbuf"					, 0,
	"varoutcp"					, 0,
	"which_failed"				, 0,
	"wsadata"					, 0,
	"xor_vals"					, 0,
	"xorlines"					, 0,
	"xorlinesp"					, 0,
	"xoroutbuf"					, 0,
	"xoroutcp"					, 0,

	0, 0};

/***********************************************************************/
/** @brief find a string in the list of symbols we want to replace.
 *
 * @param searchForString the string to find
 *
 * @return 
 * 		- 1 Found the string
 * 		- 0 Not found
 ************************************************************************/
int findString(const L_STRIP_OPTIONS * options,const char *searchForString, REPLACEMENT_STRING *newString)
{
	NAMELIST * np = findNP(options, searchForString, &(newString->offset));
	if ( np != NULL )
	{
		newString->string = np->randname;
		return 1;
	}
	else
		return 0;
}                        
/***********************************************************************/
/** @brief find a NAMELIST 	entry matching the string
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
static NAMELIST *findNP(const L_STRIP_OPTIONS * options, const char *searchForString, int * offsetIntoSearchForString)
{
	NAMELIST * np = NULL;		


	if (! options->noUser)
		np = check(user_names, searchForString,offsetIntoSearchForString);
	if (np == NULL && ! options->noInternal)
		 np = check(internal_names, searchForString,offsetIntoSearchForString);
	if (np == NULL && ! options->noExternal )
		np = check(external_names, searchForString,offsetIntoSearchForString);
	return np;

}
/***********************************************************************/
/** @brief  Look for special chars in a symbol to determine if it 
 * 	is mangled or has a leading '_'
 *
 * @param
 *
 * @return  the offset into theString where the compare should start
 ************************************************************************/
static int getFirstOffsetAndLen(const char * theString, int * len, int * mangled)
{
	int tmpLen = 0;
	char *p = (char *)theString;
	int index = 0;

	*mangled = 0;

	if ( *p == '?' )
	{
		/* on windows this means it is a mangled name */
		*mangled = 1;
		while ( *p && *p == '?' )
		{
			index++;
			p++;
		}
		/* need to caculate the length of the actual symbol name 
		 * excluding the mangled part */
		for ( tmpLen = 0 ; *p && *p != '@'; p++, tmpLen++)
			   ;
	}
	else if (*p == '_') 
	{
		index++;
		p++; 
		tmpLen = strlen(p);
	}
	else 
	{
		
		tmpLen = strlen(p);
	}	
	*len = tmpLen;
	return index;
}	
/***********************************************************************/
/** @brief check to see if a string is in the list.
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
static NAMELIST * check(NAMELIST *np, const char *theString, int * offsetIntoTheString)
{
	int i;
	int len = 0;
	int mangled = 0;
	int index;

	if (np == NULL || theString == NULL || strlen(theString) < 2) 
		return NULL;
	index = getFirstOffsetAndLen(theString,&len, &mangled);
	*offsetIntoTheString = index;
	for (i = 0; np[i].name; i++)  /* check each word */
	{
		if (np[i].name != NULL && strcmp(&theString[index], np[i].name) == 0)
		{
			return &np[i];
		}
	}
	return NULL;
}
/***********************************************************************/
/** @brief find a NAMELIST 	entry matching the string
 *
 * @param
 *
 * @return
 * 		- 1 the string was found
 * 		- 0 not found
 ************************************************************************/
int findNPBoth(const L_STRIP_OPTIONS * options, const char * searchString)
{
	NAMELIST * np = NULL;		

	if (! options->noUser)
		np = checkNPBoth(user_names, searchString);
	if (np == NULL && ! options->noInternal)
		 np = checkNPBoth(internal_names, searchString);
	if (np == NULL && ! options->noExternal )
		np = checkNPBoth(external_names, searchString);
	return (np == NULL) ? 0 : 1;

}
/***********************************************************************/
/** @brief check to see if a string is in the list.
 *
 * @param
 *
 * @return
 *
 ************************************************************************/
static NAMELIST * checkNPBoth(NAMELIST *npList, const char * theString)
{
	int i = 0;                                
   int		len;
   int		mangled;
   int 		index;

	if (npList == NULL || theString == NULL) 
		return NULL;
	index = getFirstOffsetAndLen(theString,&len, &mangled);

	for (i = 0; npList[i].name; i++)  /* check each word */
	{
		if ((npList[i].name != NULL && strcmp(&theString[index], npList[i].name) == 0) || 
			(npList[i].randname != NULL && strcmp(&theString[index], npList[i].randname) == 0))
		{
			return &npList[i];
		}
	}
	return NULL;
}
/***********************************************************************/
/** @brief Make random strings for each string in the list
 *
 * @param options  the options to use. 
 ************************************************************************/
void randomizeAll(L_STRIP_OPTIONS * options)
{
	if ( options->random || options->zeros )
	{
		srand(time(NULL));
		randomize(options, internal_names);
		randomize(options, external_names);
		randomize(options, user_names);
	}
}	
/***********************************************************************/
/** @brief Make random strings for each string in the list
 *
 * @param options  the options flags
 * @param np	pointer to the NAMELIST list to be randomized
 ************************************************************************/
static void randomize(L_STRIP_OPTIONS * options,NAMELIST *np)
{
	int i;
	int len;
	static char *letters = "abcdefghijklmnopqrstuvwxyz";
	static char *letters_num = "0123456789abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static char *emptyString = "";

	if ( np == NULL )
		return ;
	if (options->zeros)
	{
		for (i=0 ; np[i].name != NULL ; i++) 
			if ( np[i].randname == NULL )
				np[i].randname = emptyString;
	}
	else
	{
		for (i=0 ; np[i].name != NULL ; i++) 
		{
			if ( np[i].randname == NULL && strlen(np[i].name) > 0)
			{
				int x;
			
				len = strlen(np[i].name);
				np[i].randname = (char *)malloc(len + 1);
				np[i].randname[0] = letters[rand()%26];
				for (x = 1; x < len; x++)
					np[i].randname[x] = letters_num[rand()%63];
				np[i].randname[len] = '\0'; /* null terminate */
			}
		}
	}
}
/***********************************************************************/
/** @brief Add a new symbol to the list to be stripped
 *
 * @param	symbol  the symbol to be added
 *
 * @return
 * 		- 1 if there were no errors
 * 		- 0 if error
 ************************************************************************/
int addSymbol(L_STRIP_OPTIONS *options,char *symbol,char * randName)
{
	NAMELIST *	found;
	int			offset;
	
	if ( options->verbose && randName == NULL)
		fprintf(stdout,"Adding symbol to list to be stripped: %s\n",symbol);
	if ( randName != NULL && strlen(randName) != strlen(symbol) )
	{
		fprintf(stderr, 
			"Random names must be the same length as original symbol name\n"
			"  '%s'  '%s'\n", symbol, randName);
		return 0;
			
	}
	found = findNP(options,symbol, &offset);
	if ( found != NULL)
	{
		/* it's already in the list */
		if ( randName == NULL )
		{
			if ( options->verbose )
				fprintf(stdout,"  Symbol already in list. Ignored: %s\n",symbol);
			return 1;
		}
		/* just set the random name */
		if ( found->randname && strlen(found->randname) > 0)
		{
			if ( options->verbose )
				fprintf(stderr,
					"Error: Symbol with random name already in list. %s, %s\n",
					symbol, randName);
			return 0;
		}
		found->randname = (char *)malloc(strlen(randName) + 1);
		strcpy(found->randname , randName);
		return 1;
	}	

	/* it wasn't already in the list so add it to the user list */
	if ( user_names_size <= user_names_entries )
	{
		/* need more memory */
		user_names_size += USER_LIST_INCREMENT;
		if ( user_names )
		{
			user_names = (NAMELIST *)realloc(user_names, user_names_size);
			/* clear the new entries */
			memset(&user_names[user_names_entries], 0, (user_names_size - user_names_entries) * sizeof(NAMELIST));
		}
		else
			user_names = (NAMELIST *)calloc(user_names_size , sizeof(NAMELIST));
	}
	user_names[user_names_entries].name = (char *)malloc(strlen(symbol) + 1);
	strcpy(user_names[user_names_entries].name , symbol);
	if ( randName )
	{
		user_names[user_names_entries].randname = (char *)malloc(strlen(randName) + 1);
		strcpy(user_names[user_names_entries].randname , randName);
	}
	user_names_entries++;
	return 1;
}	
/***********************************************************************/
/** @brief remove a symbol from the list to be stripped
 *
 * @param options	the options to use
 * @param symbol	the symbol to be removed from the list
 *
 * @return
 * 		- 1 if there were no errors
 * 		- 0 if error
 ************************************************************************/
int removeSymbol(L_STRIP_OPTIONS *options,char *symbol)
{
	NAMELIST *	np;
	int		offset;
	
	if ( options->verbose)
		fprintf(stdout,"Removing symbol from list to be stripped: %s\n",symbol);
	np = findNP(options,symbol, &offset);
	if ( np != NULL )
	{
		/* set it to the empty string */
		np->name[0] = '\0';
		np->randname = NULL;	/* may cause a memory leak but who cares */
	}
	else if ( options->verbose )
		fprintf(stdout,"  Not found. Ignored: %s\n",symbol);
	return 1;
}	

/***********************************************************************/
/** @brief read the map file
 *
 * @param options	the options to use
 * @param mapfile	File name for the map file
 *
 * @return
 * 		- 1 if there were no errors
 * 		- 0 if error
 ************************************************************************/
int read_mapfile(L_STRIP_OPTIONS * options, char * mapfile)
{
	int i;
	char s1[256];
	char s2[256];
	char s[513];
	NAMELIST *np;
	FILE * mapfp;
	int		line = 0;

	if (!(mapfp = fopen(mapfile, "r")))
		return err_exit("Failed to open mapfile for reading	",0);
	if ( ! options->quiet )
		fprintf(stdout,"Using mapfile: %s\n", mapfile);	
	fflush(stdout);
	
	/*************************************************************
	 * User Names
	 **************************************************************/
	np = user_names;
	*s = 0;
	fgets(s, 255, mapfp);	/* overrun */
	line++;
	if (strcmp(s, "USER NAMES\n") != 0)
		return err_exit(" Expected USER NAMES", line);
	i = 0;
	while (fgets(s, 512, mapfp))	/* overrun */
	{
		line++;
		if (!strcmp(s, "EXTERNAL NAMES\n"))
			break;
		if (sscanf(s, "%s %s", s1, s2) != 2)	/* overrun */
			return err_exit("Overrun error", line);
		if (! addSymbol(options,s1, s2))
			return err_exit("Failed to add symbol to list", line);
	}

	/*************************************************************
	 * External 
	 **************************************************************/
	if (strcmp(s, "EXTERNAL NAMES\n"))
		return err_exit("Expected 'EXTERNAL NAMES'", line);
	np = external_names;
	i = 0;
	while (fgets(s, 512, mapfp))	/* overrun */
	{
		line++;
		if (!strcmp(s, "INTERNAL NAMES\n"))
			break;
		if (sscanf(s, "%s %s", s1, s2) != 2)	/* overrun */
			return err_exit("Overrun error", line);
		for (np = external_names; np->name; np++)
			if (!strcmp(s1, np->name))
				break;
		if (!np->name) 
			return err_exit("Name not found in external list", line);
		np->randname = (char *)malloc(strlen(s2) + 1);
		strcpy(np->randname, s2);

	}
	/*************************************************************
	 * Internal names
	 **************************************************************/
	if (strcmp(s, "INTERNAL NAMES\n"))
		return err_exit("Expected INTERNAL NAMES", line);
	np = external_names;
	i = 0;
	while (fgets(s, 512, mapfp))	/* overrun */
	{
		line++;
		if (sscanf(s, "%s %s", s1, s2) != 2)	/* overrun */
			return err_exit("Overrun error", line);
		for (np = internal_names; np->name; np++)
			if (!strcmp(s1, np->name))
				break;
		if (!np->name) 
			return err_exit("Name not found in internal list", line);
		np->randname = (char *)malloc(strlen(s2) + 1);
		strcpy(np->randname, s2);
	}
	fclose(mapfp);
	return 1;
}

/***********************************************************************/
/** @brief write the map file
 *
 * @param options	the options to use
 * @param mapfile	File name for the map file
 *
 * @return
 * 		- 1 if there were no errors
 * 		- 0 if error
 ************************************************************************/
int write_mapfile(L_STRIP_OPTIONS * options, char * mapfile)
{
	FILE *		mapfp;
	int			ok = 1;

	if ( ! options->quiet )
	{
		if (options->readMapFile)
			fprintf(stdout,"Updating mapfile: %s\n", mapfile);	
		else
			fprintf(stdout,"Creating mapfile: %s\n", mapfile);	
	}
	if (!(mapfp = fopen(mapfile, "w")))
	{
		fprintf(stderr,"Failed to open map file for writing: %s\n",mapfile);
		return 0;
	}
		  
	if ((ok = writeNameList(mapfp,user_names,"USER NAMES")) != 0)
		if ((ok = writeNameList(mapfp,external_names,"EXTERNAL NAMES")) != 0)
			if ((ok = writeNameList(mapfp,internal_names,"INTERNAL NAMES")) != 0)
	fclose(mapfp);
	if ( !ok )
		fprintf(stderr,"Write failure to map file: %s\n", mapfile);
	return ok;
}
/***********************************************************************/
/** @brief Write a single name list to the map file
 *
 * @param fp	the file pointer to write to
 * @param np	The name list to write 
 * @param title	The title to write to the file before the name list
 *
 * @return
 * 		- 1 if there were no errors
 * 		- 0 if error. Write error
 *
 ************************************************************************/
static int writeNameList(FILE * fp, NAMELIST * np, char * title)
{
	fprintf(fp, "%s\n",title);
	if ( np != NULL )
	{
		for ( ; np->name; np++)
			if (np->randname && *(np->randname)&& np->name && *(np->name))
				if (fprintf(fp, "%-40s%s\n", np->name, np->randname) < 0)
					return 0;
	}
	return 1;
}	
/***********************************************************************/
/** @brief exit with an error message. Not really exit . this is a left 
 * over from the original version of lmstrip. It now prints the message and
 * returns zero.
 * 
 * @param str	the string to print
 * @param line	line number where the error occured in the mapfile
 * 
 * @return 0 always
 ************************************************************************/
static int err_exit(char *str ,int line)
{
	if ( line )
	{
		fprintf(stderr, "Map file corrupt: %s: line:%d\n", str,line);
	}
	else
	{
		fprintf(stderr, "Map file corrupt: %s:\n", str);
	}
	return 0;
}

/***********************************************************************/
/** @brief print all the names from the internal tables
 ************************************************************************/
void listNames( L_STRIP_OPTIONS *options)
{
	NAMELIST *np;

	fprintf(stdout,"External symbol names:\n");
	for ( np = external_names; np->name; np++)
		fprintf(stdout,"%s\n",np->name);
	fprintf(stdout,"Internal symbol names:\n");
	for ( np = internal_names; np->name; np++)
		fprintf(stdout,"%s\n",np->name);
}


#include <time.h>	  
/***********************************************************************/
/** @brief return the number of microseconds since some point in the past
 * 
 * @return number of microseconds
 *
 ************************************************************************/
static unsigned int getUsec(void)
{
#if defined(PC) 
	
	return clock() + time(NULL);
#else
	struct timeval *tp;
	struct timezone *tzp;
	
	gettimeofday(tp,tzp);	
	return tp->tv_usec;
#endif

}
