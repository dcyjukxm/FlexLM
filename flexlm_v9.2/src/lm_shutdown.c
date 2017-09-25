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
 *	Module: $Id: lm_shutdown.c,v 1.23 2003/04/18 23:48:08 sluu Exp $
 *
 *	Function: lc_shutdown(job, prompt, flags)
 *
 *	Description: Shuts down the license manager servers.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(int) prompt - Prompt before shutting down.
 *			(int) flags - Print what it shut down.
 *
 *	M. Christiano
 *	8/27/90 - Adapted from lmdown.c
 *
 *	Last changed:  9/9/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_attr.h"
#include "l_prot.h"
#include "lgetattr.h"
#include "lm_comm.h"
#include <stdio.h>

#ifdef USE_WINSOCK
#include <pcsock.h>
#else
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

#ifndef NO_UIO_H
#include <sys/uio.h>
#endif
#include <errno.h>

static int l_shutdown_one_file 	lm_args(( LM_HANDLE *, int, int, LM_SERVER *, char *));
static int shutdown_license_file lm_args(( LM_HANDLE *, int, int, char *, char *));
static int shutdown_one lm_args(( LM_HANDLE *, int , LM_SERVER *, char *));

API_ENTRY
lc_shutdown(job, prompt, flags)
LM_HANDLE *job;
int prompt;
int flags;
{
	return l_shutdown_vendor(job, prompt, flags, 0);
}

API_ENTRY
l_shutdown(job, prompt, flags, license_file, vendor_name, in_lmgrd,
		ret_lmgrds)
LM_HANDLE *job;		/* Current license job */
int prompt;
int flags;
char *license_file; /* file, port@host */
char *vendor_name;
LMGRD_STAT_PTR in_lmgrd;
LMGRD_STAT_PTR_PTR ret_lmgrds; /* ptr to LMGRD_INFO * should free'd by caller */
{
  int ret = 0;
  LMGRD_STAT *lmgrds = 0, *first_good_lmgrd = 0, *l;
  int cnt = 1;


	if (LM_API_ERR_CATCH) return job->lm_errno;

	if (in_lmgrd )
		ret = l_shutdown_one_file(job, prompt, flags, in_lmgrd->server, vendor_name);
	else if (license_file && *license_file)
		ret = shutdown_license_file(job, prompt, flags, license_file, vendor_name);
	else
	{

		lmgrds = l_lmgrds(job, vendor_name);
		for (cnt = 0, l = lmgrds; l; l= l->next)
		{
			if (l->up)
			{
				cnt ++;
				if (!first_good_lmgrd)
					first_good_lmgrd = l;
			}
		}
		if (!first_good_lmgrd)
		{
			if (lmgrds)
				l_err_info_cp(job, &job->err_info, &lmgrds->e);
			lc_free_lmgrd_stat(job, lmgrds);
			LM_API_RETURN(int, job->lm_errno)
		}

		if ((cnt >= 1) && (ret_lmgrds))
		{
			*ret_lmgrds = lmgrds;
			ret = 0;
		}
		else
		{
			ret = l_shutdown_one_file(job, prompt, flags,
				first_good_lmgrd ? first_good_lmgrd->server : 0,
					vendor_name);
		}
	}
	if ((!ret_lmgrds || !*ret_lmgrds) && lmgrds)
		lc_free_lmgrd_stat(job, lmgrds);
	LM_API_RETURN(int, ret)
}

static
int
shutdown_license_file(job, prompt, flags, license_file, vendor)
LM_HANDLE *job;
int prompt;
int flags;
char *license_file;
char *vendor;
{
  LM_SERVER *server;

	lc_set_attr(job, LM_A_DISABLE_ENV, (LM_A_VAL_TYPE)1);
	if (lc_set_attr(job, LM_A_LICENSE_FILE,
			(LM_A_VAL_TYPE)license_file))
		return job->lm_errno;
	if (!(server = lc_master_list(job)))
		return job->lm_errno;
	return l_shutdown_one_file(job, prompt, flags, server, vendor);
}
int API_ENTRY
la_shutdown_vendor(job, vendor_name)
LM_HANDLE *job;
char *vendor_name;
{
        if (l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL)
        {
                LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 112, 0);
                return(job->lm_errno);
        }
	return l_shutdown_vendor(job, 0, 0, vendor_name);
}

int API_ENTRY
l_shutdown_vendor(job, prompt, flags, vendor_name)
LM_HANDLE *job;
int prompt;
int flags;
char *vendor_name;
{

  LMGRD_STAT *lmgrds;
  int ret;

	lmgrds = l_lmgrds(job, vendor_name);
	if (!lmgrds) return 0;
	else if (!lmgrds->up)
	{
		l_err_info_cp(job, &job->err_info, &lmgrds->e);
		lc_free_lmgrd_stat(job, lmgrds);
		return job->lm_errno;
	}
	ret =  l_shutdown_one_file(job, prompt, flags, lmgrds->server,
		vendor_name);
	lc_free_lmgrd_stat(job, lmgrds);
	return ret;
}

static
int
l_shutdown_one_file(job, prompt, flags, server, vendor)
LM_HANDLE *job;		/* Current license job */
int prompt;
int flags;
LM_SERVER *server;
char *vendor;
{
#ifndef VMS
  int doit = 1;
  int shutdown_cnt = 0;
  LM_SERVER *s;

  prompt = 0;

	if (doit)
	{

		for (s = server; s; s = s->next)
		{

			shutdown_cnt += shutdown_one(job, flags, s, vendor);
		}
	}
	return(shutdown_cnt);


#else /* VMS */
  LM_SERVER *server, *s, *master_list;
  CONFIG *x;
  char **flist;
  int doit = 1;
  int nshut = 0;
#define MAX_DAEMONS 100
  char daemons[MAX_DAEMONS][MAX_DAEMON_NAME+1];
  char msg[LM_MSG_LEN+1];
  char str[100];
  int d;
  int lastd = 0;

        master_list = lc_master_list(job);
        if (!master_list && (flags & LM_DOWN_PRINT))
        {
                lc_perror(job, "Can't get server list");
                return(job->lm_errno);
        }

        flist = lc_feat_list (job, 1, (void *) NULL);
        if (!flist)
        {
                lc_perror (job, "Can't get feature list");
                exit (0);
        }
        for (; flist && *flist; flist++)
        {
                int match = 0;
                int i;
/*
 *              Check to see if we already shut this one down.  IF
 *              so, skip it and go on to the next one.
 */
                doit = 0;
                x = lc_get_config(job, *flist);
		if (!x)
			break;
                for (i = 0; i < lastd; i++)
                {
                        if (!strcmp(x->daemon, daemons[i]))
                        {
                                match++;
                                break;
                        }
                }
                if (match) continue;
                 strcpy(daemons[lastd], x->daemon);
                lastd++;
/*
 *              Connect to the server and shut it down
 */
                if (prompt)
                {
                         fprintf(stdout,
                        "\nShutting down FLEXlm server \"%s\": are you sure? "
                                                        , x->daemon);
                        gets (str);	/* overrun threat */
                        if (*str == 'Y' || *str == 'y' || *str == '1') doit = 1;
                }
                else doit = 1;
                if (doit)
                {
			lc_disconn(job, 1);	/* In case we're already in */
                        if ((d = l_connect(job, master_list, x->daemon,
                                                                LM_TCP)) >= 0)
                        {
			  char hname[500];
                                msg[MSG_CMD] = LM_SHUTDOWN;

				l_zcp(&msg[MSG_DOWN_NAME],
					lc_username(job, 0), MAX_USER_NAME);/* LONGNAMES */

				gethostname(hname, 500);
				l_encode_32bit_packed(&msg[MSG_DOWN_IPADDR],
					(unsigned long)l_get_ipaddr(hname, 0, 0, 0));
				msg[MSG_DOWN_FLAGS] = flags;
                                l_msg_cksum(msg,
                                        job->daemon->our_comm_revision, LM_TCP);
                                network_write(d, msg, LM_MSG_LEN);
                               network_close(d);
                                (void *) shutdown(d, 2);
                                ++nshut;
                        }
                        else
                        {
                           char string[100];

                                sprintf(string, "Can't connect to daemon for %s"
                                                                , *flist);
                                lc_perror(job, string);
                        }
                }
        }
        return (nshut);
#endif /* !VMS */
}

API_ENTRY
l_shutdown_one(job, flags, s)
LM_HANDLE *job;
int flags;
LM_SERVER *s;
{
	return shutdown_one(job, flags, s, 0);
}

static
int
shutdown_one(job, flags, s, vendor)
LM_HANDLE *job;
int flags;
LM_SERVER *s;
char *vendor;
{

  char msg[LM_MSG_LEN+1];
  int msgsize = l_msg_size(job->daemon->our_comm_revision);
  int cur_comm_rev;
  int didit;
  COMM_ENDPOINT endpoint;
  LM_SOCKET d;
  int saverrno = job->lm_errno;
  int rc;

	cur_comm_rev = COMM_NUMREV;
	didit = 0;
	l_conn_msg(job, "", msg, LM_TCP, 1);
	l_get_endpoint(job, s, "", job->options->commtype, &endpoint);

	job->flags |= LM_FLAG_CONNECT_NO_THREAD;
	while (1)
	{

		d = l_basic_conn(job, msg, &endpoint, s->name, msg);
		if (d != LM_BAD_SOCKET)
		{
		  char hname[500];
			bzero(msg, LM_MSG_LEN+1);
			gethostname(hname, 500);
			msg[MSG_CMD] = LM_SHUTDOWN;
			strncpy(&msg[MSG_DOWN_NAME],
				lc_username(job, 0), MAX_USER_NAME);/* LONGNAMES */
			strncpy(&msg[MSG_DOWN_HOST],
				lc_hostname(job, 0), MAX_SERVER_NAME);/* LONGNAMES */
			l_encode_32bit_packed(&msg[MSG_DOWN_IPADDR],
					l_get_ipaddr(hname, 0, 0, 0));
			if (vendor && *vendor)
				strncpy(&msg[MSG_DOWN_DAEMON],
				vendor, MAX_VENDOR_NAME);
			msg[MSG_DOWN_FLAGS] = flags;
			l_msg_cksum(msg,
			 job->daemon->our_comm_revision, LM_TCP);
			if ((rc = network_write(d, msg, msgsize)) == msgsize)
			{
			    if (job->daemon->our_comm_revision >= 3)
			    {
			      char type, *msgptr;

				job->daemon->socket = d;
				if (l_rcvmsg(job, &type, &msgptr)
#ifdef PC
					|| (net_errno == NET_ECONNRESET)
#endif /* PC */
					)
				{
#ifdef PC
					if ((net_errno == NET_ECONNRESET))
					{
						l_clear_error(job);
						type = LM_OK;
					}
#endif /* PC */

					if ((type == LM_OK))
					{
					    if (flags & LM_DOWN_PRINT)
						 fprintf(stdout, "Shut down FLEXlm server on node %s\n"
							, s->name);
					}
					else if (type == LM_NOT_ADMIN)
					{
					  int err = LM_NOTLICADMIN;
						if (*msgptr)
							l_decode_int(msgptr,
									&err);
						LM_SET_ERRNO(job, err, 214,
							0);
						didit = err;
						if (flags & LM_DOWN_PRINT)
						{
							flags = 0;
							 lc_perror(job,
							 "shutdown");
						}


					}
					else if (type ==
						LM_NO_SUCH_FEATURE)
					{
					 LM_SET_ERRNO(job, LM_NOSERVSUPP, 215, 0);
					    if (flags & LM_DOWN_PRINT)
						 lc_perror(job,
							 "lmdown");
					}
				}
				else
				{
					LM_SET_ERRNO(job, LM_CANTREAD, 216, 0);
					if (flags & LM_DOWN_PRINT)
						 lc_perror(job,
							    "lmdown");
				}
			    }
			    else if (flags & LM_DOWN_PRINT)
				      fprintf(stdout, "Shut down node %s\n"
							, s->name);
			}
			if (job->lm_errno == saverrno || (job->lm_errno == 0))
			{
				didit = 1;
			}
#ifndef PC
			shutdown(d, 2);
#endif
			break;
		}
		else if (job->options->flags & LM_OPTFLAG_TRY_OLD_COMM)
		{
			cur_comm_rev--;
			/*- If rev4 didn't work, rev3 won't either.
			    we always skip 2 and try 1 immediately */
			if (cur_comm_rev >= 3) cur_comm_rev = 1;
			if (cur_comm_rev >= 0)
			{
				job->daemon->our_comm_revision
						= cur_comm_rev;
				l_conn_msg(job, "", msg, LM_TCP, 1);
			}
			else
				break;
		}
		else
			break;
	}
	if (!didit && (flags & LM_DOWN_PRINT)) lc_perror(job, "shutdown");

	return didit;
}

