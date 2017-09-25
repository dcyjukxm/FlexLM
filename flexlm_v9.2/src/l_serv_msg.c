/******************************************************************************

	    COPYRIGHT (c) 1994, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_serv_msg.c,v 1.7 2003/01/13 22:41:55 kmaclean Exp $
 *
 *	Function:	l_serv_msg -- send and receive a message from
 *					vendor daemon
 *	Arguments:	job
 *			type -- 	message type
 *			conf -- 	CONFIG * -- for server, daemon
 *			data -- 	char * -- cast to correct struct
 *					depending on message type
 *					Any returned data is in this struct.
 *	Returns:	0 if success, lm_errno if not. 
 *			data arg is filled in if successful.
 *
 *	D. Birns 
 *	Date:   7/94
 *
 *	Last changed:  9/9/98
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "lgetattr.h"
#include "l_prot.h"
#include "lsmaster.h"
#include <errno.h>

/* 
 *	static declarations here
 */
static void decode_gen_msg 	lm_args((LM_HANDLE *, LM_VD_GENERIC_INFO *, char *));
static void decode_feat_msg 	lm_args((LM_VD_FEATURE_INFO *, char *));
static void decode_flag 	lm_args(( int , char *));
/* 
 *	end static declarations 
 */

int
l_serv_msg(job, conf, data)
LM_HANDLE *job;
CONFIG *conf;
LM_HANDLE *data; /* to be cast to correct struct type */
{
  char msg[LM_MSG_LEN + 1];
  char type;
  char *ret_msg;
  int ret = job->lm_errno;

/*
 *	File-based - no need to talk to server
 */
	if (conf->server && conf->server->commtype == LM_FILE_COMM)
	{
		l_file_sdata(job, conf, data->type, (void *) data);
		return((ret == job->lm_errno) ? 0 : job->lm_errno);
	}

	if (l_connect_by_conf(job, conf))
			return(job->lm_errno);
/*
 *	Check the vendor key bit, and if it's not ADMIN, make sure
 *	the license is for their own vendor daemon
 */
        if (l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL && 
		*conf->code && strcmp(conf->daemon, job->vendor))
	{
		LM_SET_ERRNO(job, LM_NOADMINAPI, 93, 0);
		return(job->lm_errno);
	}
/*
 *	set msg based on type
 */
	(void) memset(msg, 0, sizeof(msg));
	msg[MSG_VD_INFO_PARAM-MSG_DATA] =  data->type;
	strncpy(&msg[MSG_VD_INFO_FEATURE-MSG_DATA], conf->feature, 
							MAX_FEATURE_LEN);
	if (*conf->code)
		strncpy(&msg[MSG_VD_INFO_CODE-MSG_DATA], conf->code, 
							MAX_CRYPT_LEN);

/*
 *	send and receive
 */
	if (!l_sndmsg(job, LM_VENDOR_DAEMON_INFO, msg))
		return job->lm_errno;
	if (!l_rcvmsg(job, &type, &ret_msg))
		return job->lm_errno;
	if ((data->type == LM_VD_GENINFO_HANDLE_TYPE &&
				type != LM_VD_GEN_INFO) ||
		(data->type != LM_VD_GENINFO_HANDLE_TYPE &&
				type != LM_VD_FEAT_BUNDLE_1))
	{
	  int i;
		l_decode_int(ret_msg, &i);
		LM_SET_ERRNO(job, i, 94, 0);
		if (job->lm_errno == 0)
		{
			LM_SET_ERRNO(job, LM_NOSERVCAP, 292,  0);
		}
		job->u_errno = 0;
		return job->lm_errno;
	}

/*
 *	decode
 */
	if (data->type == LM_VD_GENINFO_HANDLE_TYPE)
		decode_gen_msg(job, (LM_VD_GENERIC_INFO *)data, ret_msg);
	else
		decode_feat_msg((LM_VD_FEATURE_INFO *)data, ret_msg);
	
/*
 *	If the lm_errno has changed, return that, otherwise 0
 */
	return ((ret == job->lm_errno) ? 0 : job->lm_errno);
}

/*
 *	setup_gen_msg
 */	
static
void
decode_gen_msg(LM_HANDLE *job, LM_VD_GENERIC_INFO *data, char *msg)
{
	memset(data, 0, sizeof(data));
	decode_flag(msg[MSG_G_USER_INIT1-MSG_DATA], &data->user_init1);
	decode_flag(msg[MSG_G_USER_INIT2-MSG_DATA], &data->user_init2);
	decode_flag(msg[MSG_G_OUTFILTER-MSG_DATA], &data->outfilter);
	decode_flag(msg[MSG_G_INFILTER-MSG_DATA], &data->infilter);
	decode_flag(msg[MSG_G_CALLBACK-MSG_DATA], &data->callback);
	decode_flag(msg[MSG_G_VENDOR_MSG-MSG_DATA], &data->vendor_msg);
	decode_flag(msg[MSG_G_VENDOR_CHALLENGE-MSG_DATA],
						&data->vendor_challenge);
	decode_flag(msg[MSG_G_LOCKFILE-MSG_DATA], &data->lockfile);
	l_decode_int(&msg[MSG_G_READ_WAIT-MSG_DATA], &data->read_wait);
	decode_flag(msg[MSG_G_DUMP_SEND_DATA-MSG_DATA], 
						&data->dump_send_data);
	decode_flag(msg[MSG_G_NORMAL_HOSTID-MSG_DATA], 
						&data->normal_hostid);
	l_decode_int(&msg[MSG_G_CONN_TIMEOUT-MSG_DATA], 
						&data->conn_timeout);
	decode_flag(msg[MSG_G_ENFORCE_STARTDATE-MSG_DATA],
						&data->enforce_startdate);
	decode_flag(msg[MSG_G_TELL_STARTDATE-MSG_DATA], 
						&data->tell_startdate);
	l_decode_int(&msg[MSG_G_CONN_TIMEOUT-MSG_DATA], 
						&data->conn_timeout);
	l_decode_int(&msg[MSG_G_MIN_USER_TIMEOUT-MSG_DATA], 
						&data->minimum_user_timeout);
	l_decode_int(&msg[MSG_G_MIN_LMREMOVE-MSG_DATA], 
						&data->min_lmremove);

	decode_flag(msg[MSG_G_USE_FEATSET-MSG_DATA], &data->use_featset);
	l_decode_int(&msg[MSG_G_DUP_SEL-MSG_DATA], &data->dup_sel);
	decode_flag(msg[MSG_G_USE_ALL_FEATURE_LINES-MSG_DATA], 
						&data->use_all_feature_lines);
	decode_flag(msg[MSG_G_DO_CHECKROOT-MSG_DATA], &data->do_checkroot);
	decode_flag(msg[MSG_G_SHOW_VENDOR_DEF-MSG_DATA], 
						&data->show_vendor_def);
	decode_flag(msg[MSG_G_ALLOW_BORROW-MSG_DATA], 
						&data->allow_borrow);
	decode_flag(msg[MSG_G_HOSTID_REDIRECT_VERIFY-MSG_DATA], 
						&data->redirect_verify);
	decode_flag(msg[MSG_G_DAEMON_PERIODIC-MSG_DATA], 
						&data->periodic_call);
	decode_flag(msg[MSG_G_COMPARE_ON_INCREMENT-MSG_DATA], 
						&data->compare_on_increment);
	decode_flag(msg[MSG_G_COMPARE_ON_UPGRADE-MSG_DATA], 
						&data->compare_on_upgrade);
	l_decode_int(&msg[MSG_G_VERSION-MSG_DATA], &data->version);
	l_decode_int(&msg[MSG_G_REVISION-MSG_DATA], &data->revision);
	l_decode_int(&msg[MSG_G_LITE-MSG_DATA], &data->lite);
	if ((job->daemon->ver > 7) ||
		((job->daemon->ver == 7) && (job->daemon->rev >= 1)))
	{
	  long l1, l2;
		l_decode_long_hex(&msg[MSG_G_LMGRD_START-MSG_DATA], &l1);
		l_decode_long_hex(&msg[MSG_G_VD_START-MSG_DATA], &l2);
		data->lmgrd_start = l1;
		data->vd_start = l2;
		if (job->daemon->ver > 7) 
		{
			l_decode_long_hex(&msg[MSG_G_VD_TIME-MSG_DATA], &l1);
			data->server_current_time = l1;
		}
	}
}

/*
 *	decode_feat_msg
 */	
static
void
decode_feat_msg(data, msg)
LM_VD_FEATURE_INFO *data;
char *msg;
{
	
  int i;
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_REV-MSG_DATA], &data->rev);
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_TIMEOUT-MSG_DATA], &data->timeout);
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_LINGER-MSG_DATA], &data->linger);
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_DUP_SELECT-MSG_DATA], &i);
	data->dup_select = i;
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_RES-MSG_DATA], &data->res);
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_TOT_LIC_IN_USE-MSG_DATA], 
						&data->tot_lic_in_use);
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_FLOAT_IN_USE-MSG_DATA], 
						&data->float_in_use);
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_QUEUE_CNT-MSG_DATA], 
						&data->queue_cnt);
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_NUM_LIC-MSG_DATA], 
						&data->num_lic);
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_USER_CNT-MSG_DATA], 
						&data->user_cnt);
	l_decode_int(&msg[MSG_FEAT_BUNDLE1_OVERDRAFT-MSG_DATA], 
						&data->overdraft);
	l_decode_32bit_packed(&msg[MSG_FEAT_BUNDLE1_BORROWED-MSG_DATA], 
						&data->borrowed);
}

/*
 *	decode_flag  decodes a character flag to char;
 */
static
void
decode_flag(from, flag)
int from; /* gets promoted to int */
char *flag;
{
	*flag = ((char)from == ' ') ? 0 : 1;
}

/*
 *	l_get_conf_from_server -- return ptr to conf arg, or 0 on failure
 */
CONFIG_PTR  API_ENTRY
l_get_conf_from_server(job, conf)
LM_HANDLE *job;
CONFIG *conf;
{
  char msg[LM_MSG_LEN + 1];
  char *str, *cp, *pos;
  LICENSE_FILE lf;
  char feat[MAX_CONFIG_LINE + 1];
  int port_host_plus = 0;
  CONFIG *confp;
  char * server = job->daemon->server ? job->daemon->server->name : 0;


	if (server != NULL)
	{
		if (conf->server->name != NULL)
		{
			if (server && !L_STREQ(server, conf->server->name))
				*conf->php_next_conf_pos = 0; /* changed servers, no 'next' */
		}
	}

	if (l_connect_by_conf(job, conf))
		return (CONFIG *)0;
	memset(msg, 0, sizeof(msg));
	memset(&lf, 0, sizeof(lf));
	msg[MSG_VD_INFO_PARAM-MSG_DATA] =  LM_VD_GET_CONFIG_TYPE;
	strncpy(&msg[MSG_VD_INFO_FEATURE-MSG_DATA], conf->feature, 
							MAX_FEATURE_LEN);
	if (!strcmp(conf->code, CONFIG_PORT_HOST_PLUS_CODE))
	{
		port_host_plus = 1;
		sprintf(&msg[MSG_VD_INFO_CODE-MSG_DATA], "P=%s", 
						conf->php_next_conf_pos);
	}
	else
	{
		strncpy(&msg[MSG_VD_INFO_CODE-MSG_DATA], conf->code, 
								MAX_CRYPT_LEN);
	}
	if (!l_sndmsg(job, LM_VENDOR_DAEMON_INFO, msg))
		return (CONFIG *)0;
	if (!(str = l_rcvmsg_str(job)))
		return (CONFIG *)0;
	cp = str;
	if (port_host_plus)
	{
		pos = str;
		for (; *cp && *cp != ' '; cp++)
			;
/*
 *		This indicates that it's not a valid license
 *		"NOMORE"
 */
		if (!*cp) 
		{
			free(str);
			return (CONFIG *)0;
		}
		*cp = '\0';
		cp ++;
	}
	lf.type = LF_STRING_PTR;
	lf.ptr.str.s = lf.ptr.str.cur = cp;

	(void)l_lfgets(job, feat, MAX_CONFIG_LINE, &lf, 0);

	if (!(confp = (CONFIG *)l_malloc(job, sizeof(CONFIG))))
		return 0;
			
	if (!l_parse_feature_line(job, feat, confp, (char **)0))
	{
		free(str);
		l_free_conf(job, confp);
		return (CONFIG *)0;
	}
	if (port_host_plus)
		strcpy(confp->php_next_conf_pos, pos);
	confp->conf_state = LM_CONF_OPTIONS;
	free(str);
	return confp;
}
LM_CHAR_PTR_PTR
l_get_featlist_from_server(job, floating, conf)
LM_HANDLE *job;
int floating;
CONFIG *conf;
{
  char msg[LM_MSG_LEN + 1];
  char *str, *cp;
  int featcnt, i;

	if (l_connect_by_conf(job, conf))
		return (char **)0;

	(void) memset(msg, 0, sizeof(msg));
	msg[MSG_VD_INFO_PARAM-MSG_DATA] =  LM_VD_GET_FLIST_TYPE;
	if (floating)
		strcpy(&msg[MSG_VD_INFO_FEATURE-MSG_DATA] , "1");
	else
		strcpy(&msg[MSG_VD_INFO_FEATURE-MSG_DATA] , "0");
	if (!l_sndmsg(job, LM_VENDOR_DAEMON_INFO, msg))
		return (char **)0;
	if (!(str = l_rcvmsg_str(job)))
		return (char **)0;
/* 
 *	Count the number of features
 */
	for (featcnt=1, cp = str; *cp ; cp++)
	{
		for (; *cp && *cp != ' '; cp++)
		{
			;
		}
		featcnt ++;
		if (!*cp) break;
	}
	job->feat_ptrs = (char **)l_malloc(job, (featcnt + 1) * sizeof(char *));
	job->features = (char *)l_malloc(job, strlen(str) + 1); 
	if (!job->features || !job->feat_ptrs) return 0;

	strcpy(job->features, str);
	for (i=0, cp = job->features; i<featcnt; cp++, i++)
	{
		job->feat_ptrs[i] = cp;
		for (; *cp && *cp != ' '; cp++)
		{
			;
		}
		if (!*cp) 
		{
			i++;
			break;
		}
		*cp = '\0';
	}
	job->feat_ptrs[i] = (char *)0;
	free(str);
	return job->feat_ptrs;
}
DAEMON * API_ENTRY
l_get_dlist_from_server(job)
LM_HANDLE *job;
{
  int cnt, i;
  DAEMON *ret;
  char *str, *cp, *name;
  COMM_ENDPOINT e;

	if (!(str = l_get_lfile(job, lc_lic_where(job), LM_FINDER_GET_DLIST, &e)))
		return (DAEMON *)0;
/* 
 *	Count the number of daemons
 */
	for (cnt=0, cp = str; *cp ; cp++)
	{
		for (; *cp && *cp != ' '; cp++)
		{
			;
		}
		cnt ++;
	}
	if (!(ret = (DAEMON *)l_malloc(job, cnt * sizeof(DAEMON))))
		return 0;
	for (i=0, cp = str; i<cnt; cp++, i++)
	{
		for (name = cp; *cp && *cp != ' '; cp++)
		{
			;
		}
		*cp = '\0';
		memset(&ret[i], 0, sizeof(DAEMON));
		strncpy(ret[i].name, name, MAX_DAEMON_NAME);
		ret[i].name[MAX_DAEMON_NAME] = '\0';
		ret[i].udp_port = -1;
		ret[i].m_udp_port = -1;
		ret[i].tcp_port = -1;
		ret[i].m_tcp_port = -1;
		ret[i].pid = -1;
		ret[i].recommended_transport = 0;
		ret[i].transport_reason = 0;
		if (i > 0) 
			ret[i-1].next = &ret[i];
	}
	free(str);
	return ret;
}
