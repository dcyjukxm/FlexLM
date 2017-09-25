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
 *	Module: $Id: ls_aprot.h,v 1.23 2003/01/13 22:22:33 kmaclean Exp $
 *
 *	Description: Vendor daemon prototypes
 *
 *	D. Birns
 *	4/93
 *
 *	Last changed:  12/29/98
 *
 */
int	f_add			lm_args((CLIENT_DATA *client, char *feature, 
					int nlic, char *ver,
					int dup_sel, int linger, char *code,
					int sn, int freeit, FEATURE_LIST * pFeature));
int	f_borrowed		(void);
void	f_checklinger		lm_args((int freeit));
int	f_count			lm_args((USERLIST *u, int *tot_res, int *max));
int	f_dequeue		lm_args((FEATURE_LIST *f, int freeit));
void	f_drop			lm_args((USERLIST *him));
void	f_dump			lm_args((FILE *fd));
void 	f_exp_list_check 	lm_args((FEATURE_LIST *));
void	f_featcount		lm_args((FEATURE_LIST *f, int *n, int *nlic));
struct feature_list *  f_get_feat	lm_args((char *feature, char *code,
						 int noforce));
FEATURE_LIST *f_get_feat_next	lm_args((FEATURE_LIST **, char *, char *));
FEATURE_LIST *f_get_feat_next_flag_removed(FEATURE_LIST **, char *, char *, int *);
USERLIST *f_lookup 		lm_args((CLIENT_DATA *, char *, 
					USERLIST **, FEATURE_LIST **, 
					int, char *, int, FEATURE_LIST **,int));
void	f_nousers		lm_args((lm_noargs));
int 	f_pooled		lm_args(( char *, char *));
int	f_queue			lm_args((CLIENT_DATA *client, char *feature, 
					int nlic, char *ver, 
					int linger, char *code, FEATURE_LIST * pFeature));
int f_dynamic_res_avail		lm_args((
	FEATURE_LIST *	fl,		/*- The feature desired */
	CLIENT_DATA *	client,	/*- User making request */
	int				want,	/*- How many we want (if we are grabbing them) */
	OPTIONS **		list));	/*- The listhead (if specfied, grab them and link them here) */

int	f_remove		lm_args((CLIENT_DATA *client, char *feature,
					int inactive, char *code,
					int freeit, int iIgnoreExp));
int	f_remove_all		lm_args((CLIENT_DATA *client, int freeit, int crash, int iForce, char * szFeature));
void	f_remove_all_old	lm_args((int freeit));
void 	f_remove_children	lm_args((void));
int	f_user_remove		lm_args((char *feature, CLIENT_DATA *client, int freeit, int iForce));
int	f_user_remove_handle	lm_args((char *feature, char *handle, 
					 int freeit, int iForce));
void free_opt		lm_args(( OPTIONS *));
void 	ls_app_init 		lm_args((int argc, char *argv[], LM_SERVER **,
					char **master_name, 
					LM_SOCKET *tcp_s, LM_SOCKET *spx_s));
void	ls_args			lm_args((int *argcp, char **argvp[]));
void	ls_attr_setup		lm_args((char *f, char *n, char *, 
					CLIENT_DATA *user, char *ver,
					char *dup_select, char *linger,
					char *code));
#if 0
int	ls_b_exclude		lm_args((char *host, FEATURE_LIST *feature));
int	ls_b_include		lm_args((char *host, FEATURE_LIST *feature));
#endif
void 	ls_borrow_encrypt	(char *str, int, unsigned short);
void	ls_borrow_check		lm_args((lm_noargs));
void	ls_build_reserve_opt		lm_args((int type, char *num, char *feature, 
					 char *kind, char *name,
					 FEATURE_LIST *featlist,
					 int borrow));
LM_SERVER *ls_checkhost		lm_args((LM_SERVER *master_list));
int 	ls_checkin		lm_args(( CLIENT_DATA *, char *));
int	ls_checkout		lm_args((char *feature, char *ndesired,
					 char *, CLIENT_DATA *who,
					 char *version, char *dup_sel, 
					 char *linger, char *code, int sn,
					 int freeit));
int	ls_checklock		lm_args((lm_noargs));
int	ls_checkroot		lm_args((lm_noargs));
int 	ls_ck_feats 		lm_args((lm_noargs));
int 	ls_ck_udp_clients 	lm_args((CLIENT_DATA *cd, void *currtime));
void 	ls_closeout_this_tcp_client lm_args(( CLIENT_DATA *, int, int));
void 	ls_closeout_this_udp_client lm_args((CLIENT_DATA *));
void	ls_daemon		lm_args((int argc, char *argv[]));
void 	ls_disown		(void);
void	ls_do_borrow		lm_args((char *msg, CLIENT_DATA *user, 
					 CONFIG *bconf));
/*void	ls_docmd		lm_args((char *cmd, CLIENT_DATA *thisuser, 
					 SELECT_MASK select_mask));
*/
int	ls_exclude		lm_args((CLIENT_DATA *user, 
					 FEATURE_LIST *feature));
int ls_feat_checkout            lm_args((FEATURE_LIST *flist,
					char *feature, char *ndesired,
                                        char *, CLIENT_DATA *who,
                                        char *version, 
                                        char *dup_sel, char *linger,
					char *code, int sn, int freeit, 
					int log_denied));
void	ls_feat_dump		lm_args((lm_noargs));
void	ls_feat_info		lm_args((lm_noargs));
void	ls_feat_prune		lm_args((lm_noargs));
void	ls_feat_start_validate 	lm_args((lm_noargs));
int	ls_fix_borrow		lm_args((FEATURE_LIST *featlist));
FEATURE_LIST * ls_get_info	lm_args(( char *, char *, int *, int *,
					int *, int *, int *));
void 	ls_get_opts 		lm_args((char *, char *, char **, char **));
int	ls_host			lm_args((lm_noargs));
char *	ls_i_master		lm_args((LM_SERVER *ls, char *hostname));
int	ls_include		lm_args((CLIENT_DATA *user, 
					 FEATURE_LIST *feature));
int 	ls_ingroup		(int type, CLIENT_DATA *c, char *group);
void 	ls_init_feat 		lm_args((LM_SERVER *));
void 	ls_init_fhostid		lm_args((FEATURE_LIST *));

int	ls_is_duplicate		lm_args(( USERLIST *, CLIENT_DATA *, int));
void	ls_list			lm_args((char *cmd, CLIENT_DATA *user));
void	ls_main			lm_args((LM_SOCKET tcp_s, LM_SOCKET spx_s,
						 char *master_name));
int 	ls_max_exceeded		lm_args(( FEATURE_LIST *, CLIENT_DATA *, int));
int 	ls_pool			lm_args((FEATURE_LIST *, CONFIG *));
void 	ls_print_feats		lm_args((lm_noargs));
int	ls_put_borrow		lm_args((char *s));
void	ls_put_crypt		lm_args((char *s, char *cry));
void	ls_reread		lm_args((lm_noargs));
#ifdef LM_GENERIC_VD
void	ls_upgrade_vkeys	lm_args((int, char **, char *));
#endif
int     ls_sendup               lm_args((int type, CLIENT_ADDR *ca, char *user,
					char *node, char *display,
					char *feature, char *n, char *,
					char *version, char *dup_sel,
					char *linger, char *code, char *inet,
					char *vendor_def, int use_vendor_def,
					HOSTID *hostids));
void	ls_send_user		lm_args((CLIENT_DATA *user, char *buf, 
					 char *name, char *node, 
					 char *display, int num,
					 long timeon, char *version, 
					 int linger, int handle, int flags));
void	ls_send_vendor_def  	lm_args((CLIENT_DATA *user, char *buf,
					char *data));
int	ls_setlock		lm_args((lm_noargs));
int	ls_shakehand		lm_args((char *msg, VENDORCODE *key,
					 CLIENT_DATA *user));
#ifdef VOID_SIGNAL
void
#else
int
#endif
	ls_sigchld		lm_args((int sig));
int	ls_since		lm_args((USERLIST *u, long d));
void	ls_store_master_list 	lm_args((LM_SERVER *));
void 	ls_user_based		lm_args((char *, char *));
int 	ls_user_or_hostbased	lm_args(( char *, int));
void	ls_vd_info		lm_args((char *cmd, CLIENT_DATA *user));
#ifdef VMS
int	ls_vms_call		lm_args((int *, char ***));
#endif
#define LM_FL_FLAG_KEY2_OK LM_FL_FLAG_INTERNAL1
