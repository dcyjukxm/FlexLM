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
 *	Module: $Id: l_rerd.c,v 1.9 2003/04/18 23:48:05 sluu Exp $
 *
 *	Function:	la_reread() -- admin only
 *
 *	Description: 	API for reread
 *
 *	Parameters:	(LM_HANDLE *)job, (char *)license file
 *
 *	Return:		0 - success, <>0 FLEXlm errno
 *
 *	D. Birns
 *	6/17/96
 *
 *	Last changed:  10/7/97
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "errno.h"
#include "lm_attr.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "lsmaster.h"

#ifdef PC
#include "pcsock.h"
#endif /* PC */

int API_ENTRY l_rerd();

int API_ENTRY
la_reread(job, daemon, this_lmgrd, ret_lmgrds)
LM_HANDLE_PTR job;
LM_CHAR_PTR daemon;
LMGRD_STAT_PTR this_lmgrd;
LMGRD_STAT_PTR_PTR ret_lmgrds;
{
        if (l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL)
        {
                LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 112, 0);
                return(job->lm_errno);
        }
	return l_rerd(job, daemon, this_lmgrd, ret_lmgrds);
}

/*-
 *	l_rerd should be hidden from customers.  It's name is
 *	slighly obscured for that reason
 */

int API_ENTRY
l_rerd(job, daemon, this_lmgrd, ret_lmgrds)
LM_HANDLE_PTR job;
LM_CHAR_PTR daemon;
LMGRD_STAT_PTR this_lmgrd;
LMGRD_STAT_PTR_PTR ret_lmgrds;
{

  char msg[LM_MSG_LEN+1];
  int ret = 1;
  LM_SERVER *server, *s;
  int d = LM_BAD_SOCKET;
  char *which = "";
#ifdef VMS
  DAEMON ds, *dlist;
  char *dname;
#else
  LMGRD_STAT *lmgrds, *lp;
#endif


#ifdef VMS
	if (daemon)
	{
		strcpy(ds.name, daemon);
		ds.next = (DAEMON *)NULL;
		dlist = &ds;
	}
	else
		dlist = l_get_dlist(job);
	which = dlist->name;
	server = lc_master_list(job);
#else
	if (this_lmgrd)
		lmgrds = this_lmgrd;
	else
		lmgrds = l_lmgrds(job, daemon);
	if (!lmgrds)
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 381, 0);
		ret = job->lm_errno;
		goto exit_reread;
	}
	for (lp = lmgrds; lp; lp = lp->next)
		if (lp->up) break;
	if (!lp)
	{
		l_err_info_cp(job, &job->err_info, &lmgrds->e);
		ret = job->lm_errno;
		goto exit_reread;
	}
	if (!this_lmgrd && lmgrds->next)
	{
		if (ret_lmgrds) *ret_lmgrds = lmgrds;
		ret = 0;
		goto exit_reread;
	}
	server = lmgrds->server;

	if (daemon) which = daemon;
#endif
	if (!server || server->commtype == LM_FILE_COMM)
	{
		ret = job->lm_errno;
		goto exit_reread;
	}

#ifdef VMS
	s = server;
	for (; dlist; dlist = dlist->next)
#else
	for (s = server; s; s = s->next)
#endif
	{
		COMM_ENDPOINT endpoint;
		int c_rev, comm_rev;
		char resp[LM_MSG_LEN+1];
		int t = job->options->commtype == LM_UDP ? LM_TCP :
			job->options->commtype;

		c_rev = job->daemon->our_comm_revision;
		job->daemon->our_comm_revision = comm_rev = 3;
						/* Connect to lmgrd */
#ifdef VMS
		dname = dlist->name;
                l_conn_msg(job, dname, msg, LM_TCP, 1);
		l_get_endpoint(job, s, dname, t, &endpoint);
#else
                l_conn_msg(job, "", msg, LM_TCP, 1);
		l_get_endpoint(job, s, "", t, &endpoint);
#endif
		job->flags |= LM_FLAG_CONNECT_NO_THREAD;
		d = l_basic_conn(job, msg, &endpoint, s->name,
							resp);
		if (d < 0)
		{
			ret  = job->lm_errno;
			goto exit_reread;
		}
		else
		{
		  int scomm_rev = job->daemon->our_comm_revision;
		  int i;
		  char hname[500];

			if (resp[MSG_CMD] == LM_OK)
			{
			    if (resp[MSG_DATA] == '\0')
				scomm_rev = 1;
			    else
			    {
				l_decode_int(&resp[MSG_OK_COMM_REV],
						&scomm_rev);
			    }
			}
			(void) memset(msg, '\0', LM_MSG_LEN+1);
			msg[MSG_CMD] = LM_REREAD;
			gethostname(hname, 500);
			strncpy(&msg[MSG_DOWN_NAME],
					lc_username(job, 0), MAX_USER_NAME);/* LONGNAMES */
			strncpy(&msg[MSG_DOWN_HOST],
				       lc_hostname(job, 0), MAX_SERVER_NAME);/* LONGNAMES */

			/* use display because DAEMON is used for other stuff */
			strncpy(&msg[MSG_DOWN_DISPLAY],
				       which, MAX_DAEMON_NAME);
			l_encode_int(&msg[MSG_DOWN_IPADDR],
					l_get_ipaddr(hname, 0, 0, 0));
			l_msg_cksum(msg, scomm_rev, LM_TCP);
			i = network_write(d, msg,
					l_msg_size(scomm_rev));
			if (i < l_msg_size(scomm_rev))
			{
				LM_SET_ERRNO(job, LM_CANTWRITE, 293, errno);
				ret = job->lm_errno;
				goto exit_reread;
			}
			else
			{
			  char type, *msgdata;

			    if (scomm_rev >= 3)
			    {
				job->daemon->socket = d;
				if (l_rcvmsg(job, &type, &msgdata))
				{
					if (type == LM_OK)
						;
					else if (type == LM_NOT_ADMIN)
					{
					   int err = LM_NOTLICADMIN;
						if (msgdata[MSG_DATA])
							l_decode_int(&msgdata[MSG_DATA-MSG_DATA], &err);

						if (err == LM_MUST_BE_LOCAL)
						{
						  char buf[1000];
							sprintf(buf, "%s <> %s",
				       lc_hostname(job, 0), s->name);

							LM_SET_ERROR(job,
							err, 565, 0, buf,
							LM_ERRMASK_ALL);
						}
						else
						LM_SET_ERRNO(job, err, 294, 0);
						ret =  job->lm_errno;
						goto exit_reread;
					}
					else if (type == LM_TRY_ANOTHER)
					{
						LM_SET_ERRNO(job, LM_BADPARAM, 295, 0);
						ret = job->lm_errno;
						goto exit_reread;
					}
					else
					{
						LM_SET_ERRNO(job, LM_BADCOMM, 296, 0);
						ret = job->lm_errno;
						goto exit_reread;
					}
				}
				else
				{
					ret = job->lm_errno;
					goto exit_reread;
				}
			    }
			    else
			    {
				; /* success */

			    }
			}
		}
		job->daemon->our_comm_revision = c_rev;
	} /* for () */
	l_select_one(0, -1,
#ifdef PC
                10000 /* 10 seconds */
#else
                8000 /* 8 seconds */
#endif
                        );
exit_reread:
	if (!this_lmgrd && (!ret_lmgrds || !*ret_lmgrds) && lmgrds)
		lc_free_lmgrd_stat(job, lmgrds);
	if (d >= 0) network_close (d);
	return ret;
}
