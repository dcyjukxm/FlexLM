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
 *	Module: $Id: l_get_dlist.c,v 1.9 2003/05/05 16:10:54 sluu Exp $
 *
 *	Function:	l_get_dlist(), l_cur_dlist(), l_free_daemon_list()
 *
 *	Description:	Gets the path name for all the DAEMONs
 *			ls_free_daemon_list frees what l_get_dlist returns.
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *
 *	Return:		(char * []) - The pathname to the DAEMONs, or
 *				  NULL, if no DAEMON line are found.
 *
 *	M. Christiano
 *	3/6/88
 *
 *	Last changed:  11/12/98
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsmaster.h"
#include "l_openf.h"
#include "lgetattr.h"
#include <stdio.h>

static DAEMON * parse_daemon lm_args((LM_HANDLE *, char *));
static DAEMON * parse_readable_daemon lm_args((LM_HANDLE *, char *));
static void API_ENTRY l_free_one_daemon lm_args(( LM_HANDLE *, DAEMON *));
static int getword			lm_args(( char **, char *));

DAEMON * l_get_dlist_lfp lm_args(( LM_HANDLE *, LICENSE_FILE *));

DAEMON * API_ENTRY
l_get_dlist(job)
LM_HANDLE *job;		/* Current license job */
{
  DAEMON *d = 0, *l_get_dlist_fp(), *l_get_dlist_str();
  DAEMON *dnext, *dtmp, *d1, *d2, *prev;
  int current = job->lfptr;


	for (job->lfptr = LFPTR_FILE1; job->lic_files[job->lfptr]; 
		job->lfptr++)
	{
		if (dnext = l_cur_dlist(job))
		{
			for (dtmp = d; dtmp && dtmp->next; dtmp = dtmp->next)
				;
			if (dtmp) dtmp->next = dnext;
			else d = dnext;
		}
	}
	job->lfptr = current;
/*
 *	remove dups
 */
	for (d1 = d; d1; d1 = d1->next)
		for (d2 = d1->next; d2; d2 = d2->next)
			if (L_STREQ(d2->name, d1->name))
				*d2->name = 0; /* remove later */
	for (prev = 0, d1 = d; d1; prev = d1, d1 = d1->next)
	{
		if (!*d1->name && prev) /* there should always be a prev */
		{
			prev->next = d1->next;
			l_free_one_daemon(job, d1);
			d1 = prev;
		}
	}
	
	return(d);
}

DAEMON * API_ENTRY
l_cur_dlist(job)
LM_HANDLE *job;		/* Current license job */
{
  LICENSE_FILE *lf;
  DAEMON *d = (DAEMON *) NULL, *lm_get_dlist_fp(), *lm_get_dlist_str();

	if ((lf = l_open_file(job, LFPTR_CURRENT)) != (LICENSE_FILE *)NULL)
	{
		d = l_get_dlist_lfp(job, lf);
		(void) l_lfclose(lf);
	}
	return(d);
}

DAEMON *
l_get_dlist_lfp(job, lf)
LM_HANDLE *job;		/* Current license job */
LICENSE_FILE *lf;
{
  char line[MAX_CONFIG_LINE+1];
  DAEMON *dp, *sav = 0, *ret = 0, *dp2;

#ifndef NO_FLEXLM_CLIENT_API
	if (lf->type == LF_PORT_HOST_PLUS)
		return l_get_dlist_from_server(job);
#endif /* NO_FLEXLM_CLIENT_API */
	memset(line, 0, sizeof(line));
	l_lfseek(lf, (long) 0, 0);
	while (l_lfgets(job, line, MAX_CONFIG_LINE, lf, 0))
	{

		if (dp = parse_daemon(job, line))
		{
/*
 *			make sure it's not a dup
 */
			for (dp2 = ret; dp2; dp2 = dp2->next)
			{
				if (L_STREQ(dp2->name, dp->name))
					break;
			}
			if (dp2) /* it's a dup */
			{
				l_free_daemon_list(job, dp);
				continue;
			}
			if (sav) 
				sav->next = dp;
			if (!ret) 
				ret = dp; /* head of list */
			sav = dp;
		}
	}
	return ret;
}

DAEMON * API_ENTRY
la_parse_daemon(job, line)
LM_HANDLE *job;
char *line;
{
        if (l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL)
        {
                LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 304, 0);
                return(0);
        }
	return parse_daemon(job, line);
}
static
DAEMON *
parse_daemon(job, line)
LM_HANDLE *job;
char *line;
{
  char dname[MAX_DAEMON_NAME + 1];
  DAEMON *dp;
  extern void l_get_dec_daemon lm_args(( char *, char *));


	if (l_decimal_format((unsigned char *)line))
	{
		l_get_dec_daemon(line, dname);
		if (!*dname) return 0;
		dp = (DAEMON *)l_malloc(job, sizeof (DAEMON));
		strncpy(dp->name, dname, MAX_DAEMON_NAME);
		dp->udp_port = -1;
		dp->tcp_port = -1;
		dp->m_udp_port = -1;
		dp->file_tcp_port = dp->tcp_port = 0;
		dp->m_tcp_port = -1;
		dp->pid = -1;
		dp->recommended_transport = 0;
		dp->transport_reason = 0;
	}
	else dp = parse_readable_daemon(job, line);
	return dp;
		
}

/*
 *	Syntax: {DAEMON|VENDOR} name [path] [[options=]options] [[port=]port]
 *		If "VENDOR name str\n", str is presumed to be the path
 *		
 */
static
DAEMON *
parse_readable_daemon(job, line)
LM_HANDLE *job;
char *line;
{
  char word[MAX_CONFIG_LINE + 1];
#define DP_ST_START 0
#define DP_ST_GOT_KEY 1
#define DP_ST_GOT_NAME 2
#define DP_ST_GOT_OPTIONS_KEY 3
#define DP_ST_GOT_PORT_KEY 4
  int state = DP_ST_START;
  int flags = 0;
#define DP_FL_GOT_PATH 		0x1
#define DP_FL_GOT_OPTIONS_PATH 	0x2
#define DP_FL_GOT_PORT_NUM 	0x4
  DAEMON d;
  DAEMON *dp = 0;
	


	memset(&d, 0, sizeof(d));
	while (getword(&line, word))
	{
		switch(state)
		{
		case DP_ST_START:
			if (!l_keyword_eq(job, word, LM_RESERVED_PROG) &&
						!l_keyword_eq(job, word, 
						LM_RESERVED_PROG_ALIAS))
				return 0; /* not valid */
			state = DP_ST_GOT_KEY;
			break;
		case DP_ST_GOT_KEY:
			strncpy(d.name, word, MAX_DAEMON_NAME);
			state = DP_ST_GOT_NAME;
			break;
		case DP_ST_GOT_NAME:
/*
 *			We're waiting for path, or keyword
 */
			if (l_keyword_eq(job, word, LM_OPTIONS_PREFIX))
			{
				state  = DP_ST_GOT_OPTIONS_KEY;
			}
			else if (l_keyword_eq(job, word, LM_TCP_PREFIX) ||
				l_keyword_eq(job, word, LM_TCP_PREFIX_ALT))
			{
				state = DP_ST_GOT_PORT_KEY;
			}
			else
			{
				if (!(flags & DP_FL_GOT_PATH) &&
					!d.options_path && !d.tcp_port)
				{
					d.path = (char *)
						l_malloc(job, strlen(word) + 1);
					strcpy(d.path, word);
					flags |= DP_FL_GOT_PATH;
				} 
				else if (!(flags & DP_FL_GOT_OPTIONS_PATH))
				{
					d.options_path = (char *)
						l_malloc(job, strlen(word) + 1);	/* memory threat */
					strcpy(d.options_path, word);
					flags |= DP_FL_GOT_OPTIONS_PATH;
				}
				else if (!(flags & DP_FL_GOT_PORT_NUM))
				{
					sscanf(word, "%d", &d.tcp_port);	/* overrun checked */
					flags |= DP_FL_GOT_PORT_NUM;
				}
				else goto errexit_parse_daemon;
			}
			break;
		case DP_ST_GOT_OPTIONS_KEY:
			if (d.options_path) goto errexit_parse_daemon;
			d.options_path = (char *)
					l_malloc(job, strlen(word) + 1);
			strcpy(d.options_path, word);
			flags |= DP_FL_GOT_OPTIONS_PATH;
			state = DP_ST_GOT_NAME;
			break;
		case DP_ST_GOT_PORT_KEY:
			if (d.tcp_port > 0 || 
				(sscanf(word, "%d", &d.tcp_port) != 1))	/* overrun checked */
			{
				goto errexit_parse_daemon;
			}
			flags |= DP_FL_GOT_PORT_NUM;
			state = DP_ST_GOT_NAME;
			break;
		default:
			goto errexit_parse_daemon;
		}
	}
	if (state == DP_ST_GOT_NAME)
	{
		dp = (DAEMON *)l_malloc(job, sizeof (DAEMON));
		memcpy(dp, &d, sizeof(DAEMON));
	}
	else return 0;
		
	if (!dp->path)
	{
		dp->path = (char *)
				l_malloc(job, strlen(job->vendor) + 1);
		strcpy(dp->path, job->vendor);
	}
	dp->udp_port = -1;
	dp->m_udp_port = -1;
	dp->file_tcp_port = dp->tcp_port;
	dp->m_tcp_port = -1;
	dp->pid = -1;
	dp->recommended_transport = 0;
	dp->transport_reason = 0;
	return dp;

errexit_parse_daemon:
	LM_SET_ERROR(job, LM_BADFILE, 399, 0, (*word ? word : 0), LM_ERRMASK_ALL);
	if (d.path) free(d.path);
	if (d.options_path) free(d.options_path);
	return 0;
}

static
int
getword(str, word)
char **str;
char *word;
{
  char *cp = *str;
  char *wp = word;
  int gotquote = 0;


	if (!cp || !*cp) return 0;
	while (isspace(*cp)) cp++;
	if (*cp == '"') 
	{
		gotquote = 1;
		cp++;
	}
	while (isspace(*cp)) cp++;
	if (*cp == '=')
	{
		*wp++ = *cp++;
	}
	else
	{

		while (*cp && 
			((gotquote && *cp != '"') 
			|| (!gotquote && !isspace(*cp))))
		{
			*wp++ = *cp++;
			if (cp[-1] == '=') break;
		}
	}
	if (gotquote && *cp == '"') cp++;
	*wp = 0;
	*str = cp;
	return *word;
}


void
la_free_daemon(job, dp)
LM_HANDLE *job;
DAEMON *dp;
{
        if (l_getattr(job, LMADMIN_API) != LMADMIN_API_VAL)
        {
                LM_SET_ERRNO(job, LM_FUNCNOTAVAIL, 303, 0);
                return;
        }
	l_free_daemon_list(job, dp);
}

/* frees up a list returned by lm_get_dlist */

void API_ENTRY
l_free_daemon_list(job, sp)
LM_HANDLE *job;		/* Current license job */
DAEMON *sp;
{
  DAEMON *next;
	while (sp)
	{
		next = sp->next;
		l_free_one_daemon(job, sp);
		sp = next;
	}
}
static
void API_ENTRY
l_free_one_daemon(job, sp)
LM_HANDLE *job;		/* Current license job */
DAEMON *sp;
{
	if (sp->path)free(sp->path);
	if (sp->options_path) free(sp->options_path);
	free(sp);
}
