/******************************************************************************

	    COPYRIGHT (c) 1997, 2003by Macrovision Corporation.
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
 *	Module: $Id: lm_swr.c,v 1.5 2003/01/13 21:58:49 kmaclean Exp $
 *
 *	Function:	switchr functions
 *
 *	D. Birns
 *	7/17/97
 *
 *	Last changed:  8/28/97
 *
 */
#include "lmutil.h"
lmutil_switchr(newargc, newargv, cmd)
int newargc;
char **newargv;
int cmd;
{
/*-
 *		Notes on LMNEWLOG:
 *
 *	lmnewlog is the same as switchr, except:
 * 
 *       Assume a) "REPORTLOG report.log"
 *              b) lmnewlog daemon savreport.log
 *        1) report.log is closed out, and there's a switchto
 *           at the end to report.log
 *        2) It renames the report.log to savreport.log
 *        3) A new log is started at report.log
 * 
 *	The programmatic interface (which is only available to lmutil and
 *	FLEXadmin) is also the same as lmswitchr, except:
 * 
 *       msg[MSG_CMD] = LM_NEWREPLOG;  [ instead of LM_SWITCH_REPORT ]
 */

  char *file;
  CONFIG *c; 
  char *feature;
  int d;
  char msg[LM_MSG_LEN+1];

	exit_code = 0; /* assume success */
	if (newargc < 3)
	{
		usage();
		exit_code = -1;
		return -1;
	}
	feature = newargv[1];
	file = newargv[2];
	l_init_file (lm_job);
/*
*			See if it's a daemon name
*/
	for (c = lm_job->line; c; c = c->next)
	{
		if (L_STREQ(c->daemon, feature))
			break;
	}
	if (!c) c = l_lookup(lm_job, feature);
	if (!c)
	{
		lm_job->lm_errno = NOFEATURE;
		fprintf(ofp, 
		lmtext("%s: cannot lookup specified feature: %s\n"),
		myname, lmtext(lc_errstring(lm_job)));
		exit_code = lm_job->lm_errno;
	}
	else if ((d = l_connect(lm_job, c->server, c->daemon, 
			lm_job->daemon->commtype)) >= 0)
	{

/*
*				Connect to the daemon and send the
*				SWITCH/SWITCH_REPORT message
*/
		if (cmd == CMD_LMSWITCH)
			msg[MSG_CMD] = LM_SWITCH; 
		else if (cmd == CMD_LMSWITCHR)
			msg[MSG_CMD] = LM_SWITCH_REPORT; 
		else if (cmd == CMD_LMNEWLOG)
			msg[MSG_CMD] = LM_NEWREPLOG; 
		(void) l_zcp(&msg[MSG_DATA], file, 
					LM_MSG_LEN - MSG_DATA);
		l_msg_cksum(msg, COMM_NUMREV, LM_TCP);
		network_write(d, msg, LM_MSG_LEN);
		/* P5671 */
		if (lm_job->daemon->ver >=  8)
		{
		  int sav_err = lm_job->lm_errno;
		  int err = 0;
		  char *msgparam;
		  char type = 0;
			if (l_rcvmsg(lm_job, &type, &msgparam) == 0)
			{
				fprintf(ofp, "Error:  %s\n", 
					lc_errstring(lm_job));
				exit_code = lm_job->lm_errno;
			}
			else if (type != LM_OK || (err != lm_job->lm_errno))
			{
				fprintf(ofp, "Server cannot switch log files, see debug log for further information\n");
				exit_code = -1;
				if (msgparam && *msgparam) 
					fprintf(ofp, "Server error: \"%s\"",
						msgparam);
			}
		}
		lc_disconn(lm_job, 1);
#ifndef PC
		shutdown(d, 2);
#endif
	}
	else
	{
		fprintf(ofp, lmtext("Can't connect to daemon for %s: %s\n"),
			feature, 
		lmtext(lc_errstring(lm_job)));
		exit_code = lm_job->lm_errno;
	}
	return exit_code;
}
