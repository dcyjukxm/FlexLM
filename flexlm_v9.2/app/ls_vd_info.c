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
 *	Module: $Id: ls_vd_info.c,v 1.26.2.1 2003/06/25 20:53:36 sluu Exp $
 *
 *	Function: ls_vd_info(cmd, user)
 *
 *	Description: Processes the "VENDOR_DAEMON_INFO" command
 *
 *	Parameters:	(char *) cmd - The command msg from the client
 *			(CLIENT_DATA *) user - The address for the client.
 *
 *	Return:		None - response is returned directly to the client.
 *
 *	D. Birns
 *	7/94
 *
 *	Last changed:  12/29/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lgetattr.h"
#include "l_prot.h"
#include "ls_sprot.h"
#include "lsserver.h"
#include "lsfeatur.h"	/* Dynamic feature data */
#include "ls_aprot.h"
#include "lsmaster.h"
#include "ls_log.h"
#include "flexevent.h"

extern FEATURE_LIST *ls_get_info lm_args((char *, char *, int *, int *, int *,
				int *, int *));

/*
 *	externs from ls_vendor.c
 */
extern void (*ls_user_init1)();
extern void (*ls_user_init2)();
extern int (*ls_outfilter)() ;
extern int (*ls_infilter)() ;
extern int (*ls_incallback)() ;
extern char *(*ls_vendor_msg)() ;
extern char *(*ls_vendor_challenge)() ;
extern VENDORCODE vendorkeys[] ;
extern int keysize ;
extern int ls_crypt_case_sensitive ;
extern char *ls_user_lockfile ;
extern int ls_read_wait ;
extern int ls_dump_send_data ;
extern int ls_normal_hostid ;
extern int ls_conn_timeout ;
extern int ls_enforce_startdate ;
extern int ls_tell_startdate ;
extern int ls_minimum_user_timeout ;
extern int ls_min_lmremove ;
extern int ls_use_featset ;
extern char *ls_dup_sel ;
extern int ls_use_all_feature_lines ;
extern int ls_do_checkroot ;
extern int ls_show_vendor_def ;
extern int ls_allow_borrow ;
extern int (*ls_hostid_redirect_verify)() ;
extern void (*ls_daemon_periodic)() ;
extern int ls_compare_vendor_on_increment ;
extern int ls_compare_vendor_on_upgrade ;

int ls_in_vd_info;

/*
 *	static functions here
 */
static void geninfo 		lm_args((CLIENT_DATA *));
static void featinfo 		lm_args(( char *,char *, CLIENT_DATA *));
static void encode_flag 	lm_args((char *, int));
static void getconfig		lm_args((char *, char *, CLIENT_DATA *));
static void getflist		lm_args((/*int, */CLIENT_DATA *));
static void verify_key lm_args((char *, char *, CLIENT_DATA *));



void
ls_vd_info(cmd, user)
char *cmd;
CLIENT_DATA *user;
{
  char feat[MAX_FEATURE_LEN + 1];
  char code[MAX_CRYPT_LEN + 1];
  char msg[LM_MSG_LEN+1];
  int i;
  unsigned short salt = 0;

	ls_in_vd_info = 1;
	switch(cmd[MSG_VD_INFO_PARAM])
	{
	case  LM_VD_GENINFO_HANDLE_TYPE :
		geninfo(user);
		break;
	case LM_VD_FEATINFO_HANDLE_TYPE:
		l_zcp(feat, &cmd[MSG_VD_INFO_FEATURE], MAX_FEATURE_LEN);
		l_zcp(code, &cmd[MSG_VD_INFO_CODE], MAX_CRYPT_LEN);
		featinfo(feat, code, user);
		break;
	case LM_VD_GET_CONFIG_TYPE:
                {
                  int oflags  = lm_job->flags;
		  int flag;

			flag = LM_FLAG_PRT_CONF_OLDEST_FMT;
			/* P5276 */
			if (user->flexlm_ver <7 || (user->flexlm_ver == 7 &&
						user->flexlm_rev == 0))
				flag = LM_FLAG_PRT_CONF_OLDEST_FMT_NO_SIGN;
                        lm_job->flags |=  flag;
                        l_zcp(feat, &cmd[MSG_VD_INFO_FEATURE], MAX_FEATURE_LEN);
                        l_zcp(code, &cmd[MSG_VD_INFO_CODE], MAX_CRYPT_LEN);
                        getconfig(feat, code, user);
                        lm_job->flags = oflags; /* reset */
                }
		break;
	case LM_VD_GET_FLIST_TYPE:
		getflist(/*(int)cmd[MSG_VD_INFO_FEATURE], */ user);
		break;
/*
 *		LM_VD_VERIFY_KEY:
 *		Clients sends a feature line and the license-key
 *		and the server verifies the license key
 */
	case  LM_VD_VERIFY_KEY :
		l_zcp(feat, &cmd[MSG_VD_INFO_FEATURE], MAX_FEATURE_LEN);
		l_zcp(code, &cmd[MSG_VD_INFO_CODE], MAX_CRYPT_LEN);
		verify_key(feat, code, user);
		break;
/*
 *		LM_VD_BORROW:
 *		Client sends a string, a length, a "salt" -- a seed
 *		ls_borrow_encrypt takes these args and encrypts them
 *		Only the server knows how to encrypt.
 *		The client can only decrypt (in theory).  In reality
 *		if one reverse engineers the decrypt routine, you
 *		could encrypt.  Doing that would be one step towards
 *		counterfeit borrow files
 */
	case  LM_VD_BORROW:
		l_decode_int(&cmd[MSG_VD_INFO_BMSG_LEN], &i);
		l_decode_16bit_packed(&cmd[MSG_VD_INFO_BMSG_SALT], &salt);
		ls_borrow_encrypt(&cmd[MSG_VD_INFO_BMSG], i, salt);
		ls_client_send(user, LM_OK, cmd);
		break;

	default:
		memset(msg, 0, sizeof (msg));
		l_encode_int(&msg[MSG_DATA], -1);
		ls_client_send(user, LM_WHAT, msg);
	}
	ls_in_vd_info = 0;
}

static
void
featinfo(feature, code, addr)
char *feature;
char *code;
CLIENT_DATA *addr;
{
  char msg[LM_MSG_LEN+1];
  int num_users;
  int float_lic_used;
  int tot_lic_used;
  int num_lic;
  int q_cnt;
  FEATURE_LIST *f, *fp;
  USERLIST *u;
  unsigned long borrowed = 0;

	memset(msg, 0, sizeof(msg));
	f = ls_get_info(feature, code, &num_users, &float_lic_used,
					&tot_lic_used, &num_lic, &q_cnt);
	if (!f)
	{
		l_encode_int(&msg[MSG_DATA], lm_job->lm_errno);
		ls_client_send(addr, LM_WHAT, msg);
		return;
	}
	else if (!l_keyword_eq(lm_job, f->code, code))
	{
		l_encode_int(&msg[MSG_DATA], LM_NOSERVSUPP);
		ls_client_send(addr, LM_WHAT, msg);
		return;
	}
	for (u = f->u; u; u = u->next)
	{
		if (u->flags & LM_ULF_BORROWED)
			borrowed++;
	}


/*
 *	code it up and send it out!
 */
#if DEBUG
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_REV], 1);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_TIMEOUT], 2);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_LINGER], 3);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_DUP_SELECT], 4);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_RES], 5);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_TOT_LIC_IN_USE], 6);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_FLOAT_IN_USE], 7);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_USER_CNT], 8);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_LIC_AVAIL], 9);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_QUEUE_CNT], 10);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_OVERDRAFT], 11);
#else
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_REV], 1);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_TIMEOUT], f->timeout);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_LINGER], f->linger);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_DUP_SELECT], f->dup_select);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_RES], f->res);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_TOT_LIC_IN_USE], tot_lic_used);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_FLOAT_IN_USE], float_lic_used);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_USER_CNT], num_users);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_NUM_LIC], num_lic);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_QUEUE_CNT], q_cnt);
	l_encode_int(&msg[MSG_FEAT_BUNDLE1_OVERDRAFT], f->overdraft);
	l_encode_32bit_packed(&msg[MSG_FEAT_BUNDLE1_BORROWED], borrowed);
#endif

	ls_client_send(addr, LM_VD_FEAT_BUNDLE_1, msg);
}

/*
 *	geninfo
 */
static
void
geninfo(user)
CLIENT_DATA *user;
{
  char msg[LM_MSG_LEN+1];
  extern time_t ls_start_time;
  extern time_t ls_lmgrd_start;

	memset(msg, 0, sizeof(msg));
	encode_flag(&msg[MSG_G_USER_INIT1], (int)ls_user_init1);

	encode_flag(&msg[MSG_G_USER_INIT2], (int)ls_user_init2);
	encode_flag(&msg[MSG_G_OUTFILTER], (int)ls_outfilter);
	encode_flag(&msg[MSG_G_INFILTER], (int)ls_infilter);
	encode_flag(&msg[MSG_G_CALLBACK], (int)ls_incallback);
	encode_flag(&msg[MSG_G_VENDOR_MSG], (int)ls_vendor_msg);
	encode_flag(&msg[MSG_G_VENDOR_CHALLENGE],
						(int)ls_vendor_challenge);
	encode_flag(&msg[MSG_G_LOCKFILE], (int) ls_user_lockfile);
	l_encode_int(&msg[MSG_G_READ_WAIT], (int)ls_read_wait);
	encode_flag(&msg[MSG_G_DUMP_SEND_DATA], (int)ls_dump_send_data);
	encode_flag(&msg[MSG_G_NORMAL_HOSTID], (int)ls_normal_hostid);
	l_encode_int(&msg[MSG_G_CONN_TIMEOUT], (int)ls_conn_timeout);
	encode_flag(&msg[MSG_G_ENFORCE_STARTDATE],
					(int)ls_enforce_startdate);
	encode_flag(&msg[MSG_G_TELL_STARTDATE], (int)ls_tell_startdate);
	l_encode_int(&msg[MSG_G_MIN_USER_TIMEOUT],
						(int)ls_minimum_user_timeout);
	l_encode_int(&msg[MSG_G_MIN_LMREMOVE],
						(int)ls_min_lmremove);
	encode_flag(&msg[MSG_G_USE_FEATSET], (int)ls_use_featset);
	strncpy(&msg[MSG_G_DUP_SEL], ls_dup_sel, MAX_LONG_LEN);
	encode_flag(&msg[MSG_G_USE_ALL_FEATURE_LINES],
						(int)ls_use_all_feature_lines);
	encode_flag(&msg[MSG_G_DO_CHECKROOT], (int)ls_do_checkroot);
	encode_flag(&msg[MSG_G_SHOW_VENDOR_DEF], (int)ls_show_vendor_def);
	encode_flag(&msg[MSG_G_ALLOW_BORROW], (int)ls_allow_borrow);
	encode_flag(&msg[MSG_G_HOSTID_REDIRECT_VERIFY],
					(int)ls_hostid_redirect_verify);

	encode_flag(&msg[MSG_G_DAEMON_PERIODIC], (int)ls_daemon_periodic);
	encode_flag(&msg[MSG_G_COMPARE_ON_INCREMENT],
					(int)ls_compare_vendor_on_increment);
	encode_flag(&msg[MSG_G_COMPARE_ON_UPGRADE],
					(int)ls_compare_vendor_on_upgrade);

	l_encode_int(&msg[MSG_G_VERSION], (int)
		lm_job->code.flexlm_version);
	l_encode_int(&msg[MSG_G_REVISION], (int)
		lm_job->code.flexlm_revision);
	if (l_getattr(lm_job, FULL_FLEXLM) == FULL_FLEXLM_VAL)
		encode_flag(&msg[MSG_G_LITE], 0);
	else
		encode_flag(&msg[MSG_G_LITE], 1);

	l_encode_long_hex(&msg[MSG_G_LMGRD_START], ls_lmgrd_start);
	l_encode_long_hex(&msg[MSG_G_VD_START], ls_start_time);
	l_encode_long_hex(&msg[MSG_G_VD_TIME], ls_currtime);
	ls_client_send(user, LM_VD_GEN_INFO, msg);
}


static
void
encode_flag(out, in)
char *out;
int in;
{
	if (in)
		*out = '1';
	else
		*out = ' ';
}
/*
 *	getconfig -- given a feature, license-key, and CLIENT_ADDR, send
 *	printed config to user
 */
static
void
getconfig(feat, code, addr)
char *feat;
char *code;
CLIENT_DATA *addr;
{
  CONFIG *conf;
  CONFIG *pos = (CONFIG *)0;
  char buf[MAX_CONFIG_LINE+1 + ((MAX_HOSTID_LEN +3)* 3)], *cp;
  int next_conf = 0;
	if (*code == 'P')
	{
		sscanf(&code[2], "%lx", (unsigned long)&pos);	/* overrun threat, pos is a pointer */
		next_conf = 1;
/*
 *		Check it's validity
 */
		if (pos)
		{
			for (conf = lm_job->line; conf; conf = conf->next)
				if (pos == conf) break;
			if (!conf)
			{
				  char msg[LM_MSG_LEN+1];

				DLOG(("Port@host plus error -- next conf pos 0x%x\n",
					   pos));
				memset(msg, 0, sizeof(msg));

				ls_client_send_str(addr, "NOMORE");
				return;
			}
		}
	}

	while (conf = lc_next_conf(lm_job, feat, &pos))
	{
		if (*code == 'P')
			break;
		else if (l_keyword_eq(lm_job, conf->code, code))
			break;
	}
	if (!conf)
	{
	  char msg[LM_MSG_LEN+1];

		memset(msg, 0, sizeof(msg));
		ls_client_send_str(addr, "NOMORE");
		return;
	}
	cp = buf;
	if (next_conf)
	{
		sprintf(buf, "%lx", (unsigned long)pos);
		cp = buf + strlen(buf);
		*cp = ' ';
		cp++;
	}

	l_print_config(lm_job, conf, cp);
	ls_client_send_str(addr, buf);

}
static
void
getflist(addr)
CLIENT_DATA *addr;
{
  char **flist, **cpp;
  char *cp, *ret;
  int len;


	flist = lm_job->feat_ptrs;

	if (!flist)
	{
		ls_client_send_str(addr, "");
		return;
	}

	for (len = 0, cpp = flist; *cpp; cpp++)
	{
		len += strlen(*cpp);
		len++;
	}
	ret = (char *)LS_MALLOC(len);

	for (cp = ret, cpp = flist; *cpp; cpp++)
	{
		strcpy(cp, *cpp);
		cp += (strlen(*cpp));
		*cp = ' ';
		cp++;
	}
	cp[-1] = '\0';
	ls_client_send_str(addr, ret);
	free(ret);
}

static
void
verify_key(feature, code, user)
char *feature;
char *code;
CLIENT_DATA *user;
{
  int unused;
  char buf[(MAX_CONFIG_LINE * 5)]; /* room for server/ packages lines + */
  FEATURE_LIST *f;
  LM_SERVER *s;
  char *cp;
  int x;
	memset(buf, 0, sizeof(buf));
	if (!(f = ls_get_info(feature, code, &x, &x, &x, &x, &x)) || !f->conf)
	{
		l_encode_int(&buf[MSG_DATA], lm_job->lm_errno);
		ls_client_send(user, LM_WHAT, buf);
		return;
	}
	for (cp = buf, s = f->conf->server; s; s = s->next)
	{
		sprintf(cp, "%s ", l_asc_hostid_len(lm_job, s->idptr, 1));
		cp += strlen(cp);
	}
	*cp++ = '\n';
	if (f->conf->package_mask == LM_LICENSE_PKG_COMPONENT)
	{
		l_print_config(lm_job, f->conf->parent_pkg, cp);
		cp += strlen(cp);
		*cp++ = '\n';
		l_print_config(lm_job, f->conf->parent_feat, cp);
	}
	else
		l_print_config(lm_job, f->conf, cp);
	ls_client_send_str(user, buf);
}
