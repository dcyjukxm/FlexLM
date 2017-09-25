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
 *	Module: $Id: l_prot.h,v 1.77 2003/06/17 23:32:43 jwong Exp $
 *
 *	Description: Prototypes for internal application functions
 *
 *	D. Birns
 *	6/93
 *
 *	Last changed:  1/5/99
 *
 */

#ifndef _L_PROT_H
#define _L_PROT_H

#include "l_privat.h"
#ifndef FLEXLM_ULTRALITE
typedef struct timeval *LM_TIMEVAL_PTR;
void 		l_add_key_filter(LM_HANDLE*, char *, char *,  int,
					unsigned int, int [LM_PUBKEYS],
		unsigned char [LM_PUBKEYS][LM_MAXPUBKEYSIZ],
		LM_VENDORCODE_PUBKEYINFO *, int);
void API_ENTRY	l_addr_to_inet 	lm_args(( short [], int , char *));

int		l_allfeat 	lm_args((LM_HANDLE_PTR, LF_POINTER,
					LM_SERVER_PTR, int));
int		l_any_set	lm_args((LM_INT_PTR, int));
int		l_apollo_id	lm_args((lm_noargs));
LM_CHAR_PTR API_ENTRY
		l_asc_date 	lm_args((LM_CHAR_PTR));
lm_extern
LM_CHAR_PTR API_ENTRY
		l_asc_hostid	lm_args((LM_HANDLE_PTR, HOSTID_PTR ));
LM_CHAR_PTR API_ENTRY
		l_asc_id_one 	lm_args((LM_HANDLE_PTR, HOSTID_PTR ,
					int, LM_CHAR_PTR));
LM_CHAR_PTR API_ENTRY
		l_asc_hostid_len lm_args((LM_HANDLE_PTR, HOSTID_PTR, int));
lm_extern CONFIG_PTR API_ENTRY
                l_auth_data lm_args((LM_HANDLE_PTR, LM_CHAR_PTR));
lm_extern API_ENTRY
                l_baddate lm_args((LM_HANDLE *));
lm_extern int   API_ENTRY
                l_basic_conn	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
					 COMM_ENDPOINT_PTR, LM_CHAR_PTR,
					 LM_CHAR_PTR));
lm_extern
LM_CHAR_PTR API_ENTRY
                l_bin_date 	lm_args((LM_CHAR_PTR date));
int 		l_borrow_stat	(LM_HANDLE *,  void *, int);
long 		l_borrow_string_to_time(LM_HANDLE *job, char *str);
int API_ENTRY	l_check_conf	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR, LM_CHAR_PTR,
					 int, VENDORCODE_PTR, CONFIG_PTR,
					 int, int));
int API_ENTRY   l_check_fmt_ver		lm_args(( LM_HANDLE_PTR, CONFIG_PTR));
lm_extern int API_ENTRY
                l_checkout      lm_args((LM_HANDLE_PTR, const LM_CHAR_PTR,
			       const LM_CHAR_PTR, int, int ,
				const VENDORCODE_PTR,
				int ));
lm_extern
void            l_checkin
                                lm_args(( LM_HANDLE *, const LM_CHAR_PTR, int));
LM_CHAR_PTR	l_checkoff	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
							LM_CHAR_PTR ));

void l_ckout_ok lm_args((LM_HANDLE *, char *, char *, int, VENDORCODE *,
						CONFIG **, int, char *, int));
int API_ENTRY   l_cksum_ok lm_args (( LM_CHAR_PTR, int, int ));
void API_ENTRY	l_clear_error lm_args((LM_HANDLE_PTR ));
void		l_clr_rcv_queue lm_args((LM_HANDLE_PTR ));
void		l_close_localcomm lm_args((LM_HANDLE_PTR ));
int   API_ENTRY	l_compare_version lm_args((LM_HANDLE_PTR, LM_CHAR_PTR, LM_CHAR_PTR));
char * API_ENTRY l_composite_id lm_args((LM_HANDLE *job));
void 		l_conf_copy	lm_args((LM_HANDLE_PTR, CONFIG_PTR,
						CONFIG_PTR));
CONFIG * is_confg_in_list(LM_HANDLE *job, CONFIG *lookfor);
void API_ENTRY	l_config	lm_args((CONFIG_PTR, int, LM_CHAR_PTR, LM_CHAR_PTR,
					 LM_CHAR_PTR, LM_CHAR_PTR, int, LM_CHAR_PTR,
					 LM_SERVER_PTR));
lm_extern void  API_ENTRY l_conn_msg	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
						LM_CHAR_PTR, int, int));
int   API_ENTRY	l_connect	lm_args((LM_HANDLE_PTR, LM_SERVER_PTR,
					 LM_CHAR_PTR, int));
int API_ENTRY	l_connect_by_conf lm_args((LM_HANDLE_PTR, CONFIG_PTR));
int API_ENTRY	l_connect_by_conf_for_vsend lm_args((LM_HANDLE_PTR, CONFIG_PTR));

int API_ENTRY   l_connect_host_or_list lm_args((LM_HANDLE *, COMM_ENDPOINT_PTR,
					    char *, LM_SERVER *, char *, int));
int API_ENTRY	l_connect_endpoint	lm_args((LM_HANDLE_PTR,
					COMM_ENDPOINT_PTR, LM_CHAR_PTR));
void API_ENTRY	l_CopyPackageInfoToComponent lm_args((LM_HANDLE_PTR, CONFIG_PTR, CONFIG_PTR));

#ifndef LM_CRYPT
LM_CHAR_PTR     l_crypt_private lm_args((LM_HANDLE_PTR job,
				      CONFIG_PTR conf, LM_CHAR_PTR sdate,
					      VENDORCODE_PTR code));
#endif /* LM_CRYPT */
int   API_ENTRY l_date		lm_args((LM_HANDLE_PTR, LM_CHAR_PTR, int ));
lm_extern long API_ENTRY
                l_date_compare  lm_args((LM_HANDLE_PTR job,
					LM_CHAR_PTR date1, LM_CHAR_PTR date2));
lm_extern long API_ENTRY
                l_date_to_time  lm_args(( LM_HANDLE_PTR job,
						LM_CHAR_PTR datestr));

int API_ENTRY	l_decimal_format lm_args((unsigned char *));
void 		l_decode_16bit_packed( char *where, unsigned short *);
void API_ENTRY 	l_decode_32bit_packed( char *, unsigned long *);
void  API_ENTRY l_decode_int	lm_args((LM_CHAR_PTR, LM_INT_PTR ));
void  API_ENTRY	l_decode_long	lm_args((LM_CHAR_PTR, LM_LONG_PTR));
void  API_ENTRY	l_decode_long_hex lm_args((LM_CHAR_PTR, LM_LONG_PTR));
int		l_delete_job	lm_args((LM_HANDLE_PTR job));
#ifdef UNIX
int API_ENTRY l_delete_registry_entry lm_args((LM_HANDLE_PTR, char *, int));
#endif
void		l_disable_timers lm_args((lm_noargs));
void		l_do_redir	lm_args((LM_HANDLE_PTR ));
int   API_ENTRY	l_dup_code 	lm_args((CONFIG *, char *));
HOSTID *        l_eintelid       lm_args((LM_HANDLE *));
void  API_ENTRY l_encode_int	lm_args((LM_CHAR_PTR, int));
void  API_ENTRY l_encode_long	lm_args((LM_CHAR_PTR, long));
void  API_ENTRY l_encode_long_hex lm_args((LM_CHAR_PTR, long));
void 		l_encode_16bit_packed( char *where, unsigned short val);
void API_ENTRY  l_encode_32bit_packed( char *, unsigned long );
#if 0
void  API_ENTRY l_enumerate_dallas lm_args(( LM_HANDLE *, LM_METERS **,
                                                int * ));
#endif
int   API_ENTRY	l_err_info_cp	lm_args(( LM_HANDLE_PTR, LM_ERR_INFO_PTR,
						LM_ERR_INFO_PTR));
LM_U_CHAR_PTR	l_ether_id	lm_args((LM_HANDLE_PTR ));
void		l_ether_str_to_num lm_args((char *, unsigned char *));
LM_CHAR_PTR 	l_extract_licstring lm_args((char *, char *));
LM_CHAR_PTR API_ENTRY
		l_extract_date 	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR ));
void		l_featon	lm_args((LM_HANDLE_PTR,  LM_CHAR_PTR, LM_CHAR_PTR,
					 int, VENDORCODE_PTR, CONFIG_PTR));

int API_ENTRY l_feature_is_borrowed lm_args((LM_HANDLE * job,	char * pszFeature, char * pszCode));

int API_ENTRY	l_file_check	lm_args((LM_HANDLE_PTR));
char API_ENTRY	l_file_checkout lm_args((LM_HANDLE_PTR, CONFIG_PTR, int,
					 LM_CHAR_PTR, int, int, int,
					 LM_CHAR_PTR, LM_CHAR_PTR_PTR));
int API_ENTRY	l_file_checkin  lm_args((LM_HANDLE_PTR job, LM_CHAR_PTR feature,
					 LM_CHAR_PTR code,
					 LM_CHAR_PTR vendor_checkout_data));
int API_ENTRY	l_file_checkq	lm_args((LM_HANDLE_PTR, char *));
void API_ENTRY	l_file_log	lm_args((LM_HANDLE_PTR job, char *msg));
void API_ENTRY	l_file_remove	lm_args((LM_HANDLE_PTR job, CONFIG_PTR conf,
					 LM_CHAR_PTR user, LM_CHAR_PTR host,
					 LM_CHAR_PTR display));
void API_ENTRY	l_file_removeh	lm_args((LM_HANDLE_PTR job, CONFIG_PTR conf,
					LM_CHAR_PTR handle));
int API_ENTRY	l_file_sdata	lm_args((LM_HANDLE_PTR job, CONFIG_PTR conf,
					int type, void *ret));
void API_ENTRY	l_file_userlist lm_args((LM_HANDLE_PTR job, CONFIG_PTR conf,
					 LM_USERS **ret));
LM_CHAR_PTR API_ENTRY l_files_in_dir	lm_args(( LM_HANDLE_PTR, char *, char *,
						char *));

extern int API_ENTRY   l_filter_gen        (int argc, char **argv);
LM_CHAR_PTR API_ENTRY
		l_finder	lm_args((LM_HANDLE_PTR ));
unsigned short API_ENTRY
		l_finder_port	lm_args((lm_noargs));

lm_extern void API_ENTRY	l_free		lm_args((void *));

void API_ENTRY  l_free_conf	lm_args((LM_HANDLE *, CONFIG *c));
void		l_free_conf_data	lm_args((LM_HANDLE *, CONFIG *c));
void		l_free_conf_no_data	lm_args((LM_HANDLE *, CONFIG *c));
void API_ENTRY  l_free_err_info	lm_args((LM_ERR_INFO *));
void API_ENTRY 	l_free_ids	lm_args((HOSTID *));
void 		l_free_job_conf	lm_args((LM_HANDLE *,CONFIG *));
void 		l_free_job_meters lm_args(( LM_HANDLE *));
void 		l_free_job_featdata lm_args((LM_HANDLE *, LM_VOID_PTR));
void 		l_free_job_lf_pointers lm_args((LM_HANDLE*));
void 		l_free_job_license lm_args((LM_HANDLE *));
void API_ENTRY	l_free_job_servers	lm_args((LM_HANDLE_PTR, LM_SERVER_PTR));
void		l_free_keylist lm_args((LM_HANDLE_PTR, LM_KEYLIST *));
void 		l_free_list(void *l);
void		l_free_pubkey_context	(LM_HANDLE *job);
void API_ENTRY	l_free_server	lm_args((LM_HANDLE *, LM_SERVER *));
void 		l_gen_pkey_headers(unsigned int [4][3],int ,VENDORCODE *, char *);
int API_ENTRY       l_get_attr      lm_args((LM_HANDLE *, int , short * ));
API_ENTRY     unsigned char l_get_cksum	lm_args((LM_CHAR_PTR, int, int));
CONFIG_PTR	API_ENTRY
		l_get_conf_from_server
				lm_args(( LM_HANDLE *, CONFIG *));
#ifdef THREAD_SAFE_TIME
void  API_ENTRY	l_get_date lm_args((LM_INT_PTR, LM_INT_PTR, LM_INT_PTR, LM_LONG_PTR, struct tm *));
#else /* !THREAD_SAFE_TIME */
void  API_ENTRY	l_get_date lm_args((LM_INT_PTR, LM_INT_PTR, LM_INT_PTR, LM_LONG_PTR));
#endif

LM_CHAR_PTR_PTR  	l_get_featlist_from_server
				lm_args((LM_HANDLE *, int, CONFIG *));

lm_extern
int  API_ENTRY	l_get_id lm_args((LM_HANDLE_PTR, HOSTID_PTR_PTR, LM_CHAR_PTR));

unsigned int API_ENTRY
		l_get_ipaddr	(LM_CHAR_PTR , LM_CHAR_PTR , void *, int);
LM_CHAR_PTR	l_get_lfile	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR, LM_CHAR_PTR,  COMM_ENDPOINT_PTR));
lm_extern int   API_ENTRY	l_get_endpoint	lm_args((LM_HANDLE_PTR, LM_SERVER_PTR,
					 LM_CHAR_PTR,int,COMM_ENDPOINT_PTR));

int API_ENTRY  	l_get_registry	(LM_HANDLE_PTR, char *, char **, int *, int);
long  API_ENTRY	l_getattr	lm_args((LM_HANDLE_PTR, int));
int		l_getattr_init	lm_args((LM_HANDLE_PTR, VENDORCODE_PTR,
					 LM_CHAR_PTR ));
LM_CHAR_PTR l_getEnvUTF8 lm_args((LM_HANDLE_PTR, LM_CHAR_PTR, LM_CHAR_PTR, int));
LM_CHAR_PTR  l_getenv lm_args(( LM_HANDLE_PTR, LM_CHAR_PTR));
void  l_getenv_clear lm_args((lm_noargs));
void copy_conf_data(LM_HANDLE * job, CONFIG *dstConf, CONFIG *srcConf);

HOSTID_PTR API_ENTRY l_gethostid lm_args((LM_HANDLE_PTR job));
int API_ENTRY l_gethostname lm_args((char *, int ));
HOSTID * API_ENTRY	l_getid_type	lm_args(( LM_HANDLE *job, int));
LM_CHAR_PTR API_ENTRY l_getexp	lm_args((lm_noargs));
char * API_ENTRY l_getlastfile lm_args((char *));
char * API_ENTRY l_getnextfile	lm_args((char *));
#ifdef UNIX
char * API_ENTRY l_get_next_registry lm_args((char *, char *, int *));
#endif
int API_ENTRY	l_good_bin_date	lm_args((char *hexdatestr));
int 		l_good_lic_key	lm_args(( LM_HANDLE *, CONFIG *, VENDORCODE *));
int       	l_handshake	lm_args((LM_HANDLE_PTR ));
int		l_hbs		lm_args((int));
int 		l_heartbeat	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
						LM_CHAR_PTR));
int API_ENTRY	l_host		lm_args((LM_HANDLE_PTR, HOSTID_PTR ));
int API_ENTRY 	l_hostid( LM_HANDLE_PTR job, int type, char *buf);
int API_ENTRY 	l_hostid_cmp 	lm_args((LM_HANDLE *, HOSTID *, HOSTID *));
int API_ENTRY 	l_hostid_cmp_exact lm_args((LM_HANDLE *, HOSTID *, HOSTID *));
int API_ENTRY 	l_hostid_cmp_one lm_args((LM_HANDLE *, HOSTID *, HOSTID *));
int 		l_hostid_copy	lm_args((LM_HANDLE_PTR, HOSTID **, HOSTID *));
int API_ENTRY	l_id_types_match lm_args((int, int ));
int API_ENTRY	l_inet_cmp	lm_args((short addr1[], short addr2[]));
void API_ENTRY	l_inet_to_addr 	lm_args(( char *, char *, short []));
HOSTID *        l_intelid       lm_args((LM_HANDLE *));
int API_ENTRY 	l_init		lm_args(( LM_HANDLE_PTR, char *,
					VENDORCODE *, LM_HANDLE **, int));
int API_ENTRY	l_init_file	lm_args((LM_HANDLE_PTR ));
int API_ENTRY   l_install_license lm_args(( LM_HANDLE_PTR, char *, char **,
					int *, char *));
int API_ENTRY	l_int_month	lm_args((LM_CHAR_PTR ));
long API_ENTRY 	l_ipaddr	lm_args((LM_CHAR_PTR));
int API_ENTRY	l_IsComponentOfPackage lm_args((LM_HANDLE_PTR, char *, char *, char *));
int 		l_is_inet	(char *s);
char 		l_itox		(int i);
int API_ENTRY 	l_keyword_eq 	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
					LM_CHAR_PTR));
int API_ENTRY 	l_keyword_eq_n 	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
					LM_CHAR_PTR, int));
LM_CHAR_PTR API_ENTRY l_lfgets 	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR, int,
						LF_POINTER, LM_INT_PTR));
void		l_lfseek 	lm_args((LF_POINTER, long, int ));
void API_ENTRY	l_lfclose 	lm_args((LF_POINTER));
int 		l_local_verify_conf (LM_HANDLE *,CONFIG *,char *,
					char *,VENDORCODE *, int, int);

int API_ENTRY 	l_lmgrd_running	lm_args(( LM_HANDLE_PTR, int, LM_CHAR_PTR,
						int));
LMGRD_STAT * l_lmgrd_vsupported lm_args((LM_HANDLE_PTR, LMGRD_STAT *, char *));
LMGRD_STAT_PTR API_ENTRY l_lmgrds lm_args((LM_HANDLE *, LM_CHAR_PTR));
CONFIG_PTR API_ENTRY
		l_lookup	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR ));
CONFIG_PTR 	l_lookup_serv	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
					LM_CHAR_PTR, int));
void  API_ENTRY l_lowercase	lm_args((LM_CHAR_PTR));
void * API_ENTRY l_malloc	lm_args((LM_HANDLE_PTR, size_t));
LM_SERVER_PTR API_ENTRY
		l_master_list_from_job lm_args((LM_HANDLE_PTR));
LM_SERVER_PTR API_ENTRY
		l_master_list_lfp lm_args((LM_HANDLE_PTR, LF_POINTER,
						LM_SERVER_PTR));
LM_VOID_PTR 		l_mem_malloc 	lm_args((LM_HANDLE *, int, void **));
void 		l_mem_free 	lm_args((void *, void **));
int API_ENTRY	l_midnight	lm_args((long));
long  API_ENTRY	l_modify	lm_args((long));

const char * API_ENTRY l_month_int	(int m);
LM_VOID_PTR	l_more_featdata	lm_args((LM_HANDLE_PTR, LM_VOID_PTR));
void  API_ENTRY	l_msg_cksum 	lm_args((LM_CHAR_PTR, int, int));
int   API_ENTRY	l_msg_size	lm_args((int));
int		l_msgrdy	lm_args((LM_HANDLE_PTR, int));
void		l_mt_lock	(LM_HANDLE *, char *, int);
void		l_mt_unlock	(LM_HANDLE *, char *, int);
CONFIG_PTR API_ENTRY
	l_next_conf_or_marker   lm_args((LM_HANDLE_PTR, LM_CHAR_PTR ,
					CONFIG_PTR_PTR, int, LM_CHAR_PTR));

long l_now lm_args((lm_noargs));
int l_now_usec(void);
LF_POINTER API_ENTRY l_open_file	lm_args((LM_HANDLE_PTR, int));
LM_CHAR_PTR API_ENTRY l_parse_attr lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
					CONFIG_PTR));
void l_parse_commtype lm_args((LM_HANDLE *, char *, LM_SERVER *));
LM_CHAR_PTR API_ENTRY l_parse_component lm_args((LM_HANDLE *, char *, CONFIG **,
								int));
int API_ENTRY l_parse_daemon 	lm_args(( LM_HANDLE_PTR, LM_CHAR_PTR,
						LM_CHAR_PTR, LM_CHAR_PTR));
int API_ENTRY l_parse_decimal	lm_args(( LM_HANDLE *, unsigned char *,
					CONFIG *, char **));
int   API_ENTRY l_parse_package lm_args((LM_HANDLE_PTR, LM_CHAR_PTR, CONFIG_PTR,
						LM_CHAR_PTR, LM_CHAR_PTR_PTR));
int   API_ENTRY l_parse_feature_line lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
				      CONFIG_PTR, LM_CHAR_PTR_PTR));
char * API_ENTRY l_parse_info_borrow(LM_HANDLE *, char *, char *, char *,
					long *, long *, char *, int *);
int   API_ENTRY	l_parse_server_line lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
					     LM_SERVER_PTR));
lm_extern
LM_CHAR_PTR
API_ENTRY       l_platform_name lm_args((lm_noargs));
void 		l_post_pkg	lm_args((LM_HANDLE_PTR));
int		l_print_conf_dec  lm_args((LM_HANDLE *, CONFIG *, char *));
void 		l_print_server	lm_args((LM_HANDLE_PTR, LM_SERVER_PTR,
					 LM_CHAR_PTR ));
lm_extern
int   API_ENTRY l_print_config  lm_args((LM_HANDLE_PTR, CONFIG_PTR,
							LM_CHAR_PTR));
void 		l_pubkey_free(LM_HANDLE *job);
int 		l_pubkey_verify	(LM_HANDLE *, unsigned char *, int, char *,
					int, int[],
					unsigned char pubkey[][LM_MAXPUBKEYSIZ], int, int);
int		l_rcvheart	lm_args((LM_HANDLE_PTR ));
int   API_ENTRY l_rcvmsg	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR type,
					 LM_CHAR_PTR_PTR));
LM_CHAR_PTR API_ENTRY
                l_rcvmsg_str	lm_args((LM_HANDLE_PTR));
int   API_ENTRY l_rcvmsg_timeout	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR ,
						 LM_CHAR_PTR_PTR, int));
#define L_RCVMSG_DEFAULT_TIMEOUT -9999
#define L_RCVMSG_BLOCK -999
char 		l_rcvmsg_type	lm_args((LM_HANDLE_PTR, int, LM_CHAR_PTR_PTR));
LM_CHAR_PTR  	l_read_lfile	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR ));
int   API_ENTRY	l_read_sernum	lm_args((LM_CHAR_PTR, LM_INT_PTR ));
int   API_ENTRY	l_read_timeout  lm_args((int, LM_CHAR_PTR, long, long));
LM_CHAR_PTR API_ENTRY l_real_getenv	lm_args((LM_CHAR_PTR));
int   API_ENTRY	l_redundant	lm_args((LM_HANDLE_PTR));
int   API_ENTRY l_replog	lm_args((LM_HANDLE_PTR));
lm_extern int API_ENTRY l_rerd lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
					LMGRD_STAT_PTR, LMGRD_STAT_PTR_PTR));
void		l_resend_cmd	lm_args((LM_HANDLE_PTR ));

LM_SERVER_PTR 	l_reset_job_servers lm_args((LM_HANDLE_PTR));

int API_ENTRY l_return_early lm_args((
	LM_HANDLE *	job,
	char *		feature,
	char *		vendor,
	char *		user,
	char *		host,
	char *		display));

int		l_select	lm_args((int, int *, int *, int *,
						LM_TIMEVAL_PTR));
int   API_ENTRY	l_select_one	lm_args((int, int, int));
#define L_SELECT_BLOCK -999
#define L_SELECT_NONBLOCK 0
int 		l_serv_msg	lm_args((LM_HANDLE_PTR, CONFIG_PTR,
					 LM_HANDLE_PTR ));
int API_ENTRY l_set_attr	lm_args(( LM_HANDLE_PTR, int, LM_A_VAL_TYPE));
void API_ENTRY l_set_error	lm_args(( LM_HANDLE_PTR, int, int, int,
					LM_CHAR_PTR, int, char *));
void  API_ENTRY	l_set_error_path lm_args((LM_HANDLE_PTR ));
int   API_ENTRY	l_set_license_path lm_args((LM_HANDLE_PTR, LM_CHAR_PTR, int ));
int API_ENTRY 	l_set_registry	( LM_HANDLE_PTR, char *, char *, int, int);
int   API_ENTRY	l_shutdown	lm_args(( LM_HANDLE *, int , int , char *,
					char *, LMGRD_STAT_PTR,
					LMGRD_STAT_PTR_PTR ));
int   API_ENTRY l_shutdown_one	lm_args((LM_HANDLE *, int, LM_SERVER *));
int   API_ENTRY	l_shutdown_vendor lm_args(( LM_HANDLE *, int, int, char *));
void		l_sg		lm_args((LM_HANDLE *, LM_CHAR_PTR,
					VENDORCODE_PTR));
int   API_ENTRY l_sndmsg	lm_args((LM_HANDLE_PTR, int, LM_CHAR_PTR ));
int   API_ENTRY	l_start_ok	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR ));
LM_CHAR_PTR API_ENTRY
		l_str_crypt	lm_args((LM_CHAR_PTR, int, long, int));
LM_CHAR_PTR API_ENTRY
		l_str_dcrypt	lm_args((LM_CHAR_PTR, int, long, int));
int 		l_strncmp 	lm_args((char *, char *, int));
void		l_supersede	lm_args((LM_HANDLE_PTR, int));
int API_ENTRY 	l_timer_heart 	(LM_HANDLE_PTR );
void  API_ENTRY	l_text		lm_args((lm_noargs));
int API_ENTRY 	l_update_license_file
				lm_args(( LM_HANDLE_PTR , char * , char *));
int API_ENTRY 	l_update_license_path lm_args(( LM_HANDLE_PTR, char *, char *));
int 		l_update_vkeys	lm_args((LM_HANDLE *, VENDORCODE *));
long 		l_update_mask 	lm_args((char *vendor_name));
void  API_ENTRY	l_upgrade_feat	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR ));
int   API_ENTRY	l_validdate	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR ));
long		l_vol_id	lm_args((lm_noargs));
int   API_ENTRY	l_write_sernum	lm_args((LM_CHAR_PTR, LM_INT_PTR ));
void  API_ENTRY l_uppercase	lm_args((LM_CHAR_PTR));
int		l_valid_job	lm_args((LM_HANDLE_PTR ));
int   API_ENTRY	l_valid_version	lm_args((LM_CHAR_PTR ));
char * API_ENTRY l_vendor_license_file ( LM_HANDLE_PTR, int);
int 		l_xtoi		(unsigned char c);
void 		l_xorname	lm_args((char *, VENDORCODE *));
void  API_ENTRY	l_zcp		lm_args(( LM_CHAR_PTR d, LM_CHAR_PTR, int));
long		l_zinit		lm_args((LM_CHAR_PTR ));
int   API_ENTRY lc_xstrcmp	lm_args((LM_HANDLE_PTR, LM_CHAR_PTR,
							LM_CHAR_PTR ));
int l_la_init(LM_HANDLE *job);
int la_init(LM_HANDLE *job);
lm_extern int API_ENTRY lc_sleep lm_args((LM_HANDLE_PTR job, int seconds));

#ifdef PC
UINT SetTimer_NT( HWND , UINT , UINT , TIMERPROC );
#endif
#ifndef lm_sleep
#define lm_sleep(x) lc_sleep(lm_job, x);
#endif
#ifdef LM_GENERIC_VD
void  API_ENTRY lg_encrypt	lm_args((char *, long, long));
#endif /* GENERIC */

#ifndef RELEASE_VERSION
LM_CHAR_PTR 		l_debug_datastr lm_args((unsigned char *, int));
#endif
#endif  /* FLEXLM_ULTRALITE  */

void	l_key		lm_args(( LM_CHAR_PTR, LM_U_LONG_PTR, LM_U_LONG_PTR, int));
unsigned long  	l_svk		lm_args((LM_CHAR_PTR, VENDORCODE_PTR));

#endif /* _L_PROT_H_ */
