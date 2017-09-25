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
 *	Module: $Id: lm_username.c,v 1.4 2003/04/18 23:48:08 sluu Exp $
 *
 *	Function: lc_username(job, flag)
 *
 *	Description: 	Returns the current user's name
 *
 *	Parameters:	(LM_HANDLE *) job - current job
 *			(int) flag - 0 for host system's idea of name
 *				   - <> 0 for FLEXlm's idea of name
 *
 *	Return:		(char *) username
 *
 *	M. Christiano
 *	3/16/90
 *
 *	Last changed:  10/11/98
 *
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#ifndef PC
#include "lgetattr.h"
#include <stdio.h>
#ifndef VXWORKS
#include <pwd.h>
#endif 
#ifdef ANSI
#include <unistd.h>
#include <sys/types.h>  
#endif /* ANSI */
#ifdef apollo
#include <apollo/base.h>
#include <apollo/error.h>
#include <apollo/pm.h>
/* std_$call void pm_$get_sid_txt();  Returns string: name.prj.org.node_id */
#endif	/* apollo */
#else	/* !PC */
#include "pcsock.h"
#endif


#ifdef PC16
int API_ENTRY gethostname (char FAR *name, int namelen);
#endif

#if defined(PC) && !defined(WINNT) && !defined(OS2)
BOOL API_ENTRY GetUserName(LPBYTE lpBuffer, LPDWORD nSize);       
#endif                                                  

char * API_ENTRY
lc_username(job, flag)
LM_HANDLE *job;		/* Current license job */
int flag;
{
  char *username;
  static char static_username[MAX_USER_NAME] = {'\0'};/* LONGNAMES */
#ifdef PC
  DWORD  uname_len = MAX_USER_NAME;/* LONGNAMES */
  char buffer[MAX_LONGNAME_SIZE] = {'\0'};
#endif /* PC */

	if (flag && job->options->user_override[0])
		username = job->options->user_override;
	else
	{

#ifdef NO_USERNAME_IN_OS
#if defined(PC) && !defined(OS2)
		uname_len = l_getUserName(job, buffer, MAX_LONGNAME_SIZE);
		if(uname_len)
		{
			memcpy(static_username, buffer, MAX_USER_NAME - 1);
			username = static_username;
		}
        else
        {
			if (!gethostname( static_username, MAX_USER_NAME-1) )/* LONGNAMES */
				username = static_username;
			else
				username = "Unknown";
        }
#else  /* defined(PC) && !defined(OS2) */
        username = "Unknown";
#endif /* PC */
#else

#if !defined(ANSI) || defined(OPENVMS)
	  struct passwd *getpwuid(); 
#endif
	  struct passwd *pw;

		pw = getpwuid(getuid());
		if (pw == (struct passwd *) NULL)
		{
#ifdef apollo
		
		  char sid[150];/* pm_$get_sid_text() returns 140 max */
		  short len, MAXLEN = 140;      

			pm_$get_sid_txt(MAXLEN, sid, &len);
			sid[len] = '\0';
			username = strchr(sid, '.');
			if (username == (char *) NULL)
			{
				username = "???";
			}
			else
			{
				*username = '\0';
				username = sid;
			}
#else
			if (!(username = getenv("USER")))
			{
				sprintf(static_username, "%d", getuid());
				username = static_username;
			}
#endif			
		}
		else
		{
			username = pw->pw_name;
			job->group_id = pw->pw_gid;
		}

#endif	/* !NO_USERNAME_IN_OS */

	}
	if (!job->options->user_override[0])
	{
		strncpy(job->options->user_override, username, 
							MAX_USER_NAME);/* LONGNAMES */
		job->options->user_override[MAX_USER_NAME] = '\0';/* LONGNAMES */
	}
	return(username);
}
